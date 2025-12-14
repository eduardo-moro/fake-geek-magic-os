#!/usr/bin/env python3
"""
Linux Notification Server for MiniDisplay
Listens to DBus notifications and forwards them to ESP32 via MQTT (XOR Encrypted)
"""

import asyncio
import json
import re
import subprocess
from datetime import datetime
from pathlib import Path

import paho.mqtt.client as mqtt

# Configuration
MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
MQTT_TOPIC_NOTIFICATIONS = "ehpmcp/esp/notification"
XOR_KEY = b'supersecretkey'  # Example key, must be shared with ESP8266
HISTORY_FILE = Path.home() / ".local/share/minidisplay/notification_history.json"
MAX_HISTORY_ITEMS = 100

# Global state
notification_queue = asyncio.Queue()
notification_history = []
mqtt_client = mqtt.Client()

def ensure_history_file():
    """Create history file directory if it doesn't exist"""
    HISTORY_FILE.parent.mkdir(parents=True, exist_ok=True)
    if not HISTORY_FILE.exists():
        with open(HISTORY_FILE, 'w', encoding='utf-8') as f:
            json.dump([], f)

def load_history():
    """Load notification history from file"""
    global notification_history
    ensure_history_file()
    try:
        if HISTORY_FILE.exists():
            with open(HISTORY_FILE, 'r', encoding='utf-8') as f:
                notification_history = json.load(f)
    except Exception as e:
        print(f"Error loading history: {e}")
        notification_history = []

def save_history():
    """Save notification history to file"""
    try:
        HISTORY_FILE.parent.mkdir(parents=True, exist_ok=True)
        with open(HISTORY_FILE, 'w', encoding='utf-8') as f:
            json.dump(notification_history, f, indent=2)
    except Exception as e:
        print(f"Error saving history: {e}")

def add_to_history(title: str, message: str):
    """Add a notification to history"""
    global notification_history

    notification = {
        "id": len(notification_history),
        "title": title,
        "message": message,
        "timestamp": datetime.now().isoformat(),
    }

    notification_history.append(notification)

    if len(notification_history) > MAX_HISTORY_ITEMS:
        notification_history = notification_history[-MAX_HISTORY_ITEMS:]
        for idx, notif in enumerate(notification_history):
            notif["id"] = idx

    save_history()

def encrypt_message(message: str) -> bytes:
    """Encrypt a message using XOR with a repeating key"""
    message_bytes = message.encode('utf-8')
    key_len = len(XOR_KEY)
    encrypted_bytes = bytearray(len(message_bytes))
    for i in range(len(message_bytes)):
        encrypted_bytes[i] = message_bytes[i] ^ XOR_KEY[i % key_len]
    return bytes(encrypted_bytes)

async def publish_notification(title: str, message: str):
    """Encrypt and publish notification to MQTT"""
    notification_json = json.dumps({
        "title": title,
        "message": message,
    })

    encrypted_message = encrypt_message(notification_json)
    
    print(f"Publishing to {MQTT_TOPIC_NOTIFICATIONS}: {title} - {message}")
    mqtt_client.publish(MQTT_TOPIC_NOTIFICATIONS, encrypted_message)

async def notification_sender():
    """Process notification queue and send to MQTT"""
    while True:
        title, message = await notification_queue.get()
        add_to_history(title, message)
        await publish_notification(title, message)

async def dbus_monitor():
    """Monitor DBus for notifications using dbus-monitor"""
    cmd = [
        "dbus-monitor",
        "--session",
        "interface='org.freedesktop.Notifications',member='Notify'"
    ]
    print("Starting DBus monitor...")
    process = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
    )

    current_strings = []
    while True:
        line = await process.stdout.readline()
        if not line:
            break
        line = line.decode('utf-8', errors='ignore').strip()

        if 'method call' in line.lower() and 'Notify' in line:
            current_strings = []
        
        if 'string "' in line:
            match = re.search(r'string "(.*)"', line)
            if match:
                value = match.group(1)
                current_strings.append(value)
                if len(current_strings) >= 5:
                    app_name = current_strings[0]
                    summary = current_strings[3]
                    body = current_strings[4]

                    title = summary if summary else app_name
                    message = body if body else summary

                    if title or message:
                        print(f"DBus notification: {title} - {message}")
                        await notification_queue.put((title, message))
                    current_strings = []

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}\n")

async def main():
    """Main server entry point"""
    print("=" * 60)
    print("MiniDisplay Notification Server")
    print("=" * 60)
    print(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    
    mqtt_client.on_connect = on_connect
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()

    load_history()
    print(f"Loaded {len(notification_history)} notifications from history")
    print("=" * 60)

    tasks = [
        asyncio.create_task(notification_sender()),
        asyncio.create_task(dbus_monitor()),
    ]
    await asyncio.gather(*tasks)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\nShutting down server...")
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
