#!/usr/bin/env python3
"""
Linux Notification Server for MiniDisplay
Listens to DBus notifications and forwards them to ESP32 via WebSocket
"""

import asyncio
import json
import re
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Set

import websockets


# Configuration
WEBSOCKET_PORT = 8765
HISTORY_FILE = Path.home() / ".local/share/minidisplay/notification_history.json"
MAX_HISTORY_ITEMS = 100

# Global state
connected_clients: Set[websockets.WebSocketServerProtocol] = set()
notification_queue = asyncio.Queue()
notification_history = []


def ensure_history_file():
    """Create history file directory if it doesn't exist"""
    HISTORY_FILE.parent.mkdir(parents=True, exist_ok=True)
    if not HISTORY_FILE.exists():
        with open(HISTORY_FILE, 'w') as f:
            json.dump([], f)


def load_history():
    """Load notification history from file"""
    global notification_history
    ensure_history_file()
    try:
        if HISTORY_FILE.exists():
            with open(HISTORY_FILE, 'r') as f:
                notification_history = json.load(f)
    except Exception as e:
        print(f"Error loading history: {e}")
        notification_history = []


def save_history():
    """Save notification history to file"""
    try:
        HISTORY_FILE.parent.mkdir(parents=True, exist_ok=True)
        with open(HISTORY_FILE, 'w') as f:
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

    # Keep only last MAX_HISTORY_ITEMS
    if len(notification_history) > MAX_HISTORY_ITEMS:
        notification_history = notification_history[-MAX_HISTORY_ITEMS:]
        # Renumber IDs
        for idx, notif in enumerate(notification_history):
            notif["id"] = idx

    save_history()


def remove_from_history(notification_id: int):
    """Remove a notification from history by ID"""
    global notification_history
    notification_history = [n for n in notification_history if n["id"] != notification_id]
    # Renumber IDs
    for idx, notif in enumerate(notification_history):
        notif["id"] = idx
    save_history()


def clear_history():
    """Clear all notification history"""
    global notification_history
    notification_history = []
    save_history()


async def handle_client(websocket):
    """Handle WebSocket client connection"""
    connected_clients.add(websocket)
    client_addr = websocket.remote_address
    print(f"Client connected: {client_addr}")

    try:
        async for message in websocket:
            # Handle any messages from ESP32 (future feature)
            print(f"Received from {client_addr}: {message}")
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        connected_clients.remove(websocket)
        print(f"Client disconnected: {client_addr}")


async def broadcast_notification(title: str, message: str):
    """Send notification to all connected clients"""
    if not connected_clients:
        print(f"No clients connected. Notification queued: {title}")
        return

    notification_json = json.dumps({
        "title": title,
        "message": message,
    })

    print(f"Broadcasting: {title} - {message}")

    # Send to all connected clients
    disconnected_clients = set()
    for client in connected_clients:
        try:
            await client.send(notification_json)
        except websockets.exceptions.ConnectionClosed:
            disconnected_clients.add(client)

    # Clean up disconnected clients
    connected_clients.difference_update(disconnected_clients)


async def notification_sender():
    """Process notification queue and send to clients"""
    while True:
        title, message = await notification_queue.get()
        add_to_history(title, message)
        await broadcast_notification(title, message)


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
        env=None
    )

    current_strings = []

    while True:
        line = await process.stdout.readline()
        if not line:
            break

        line = line.decode('utf-8', errors='ignore').strip()

        # Start of new method call
        if 'method call' in line.lower() and 'Notify' in line:
            current_strings = []

        # Extract string values
        if 'string "' in line:
            match = re.search(r'string "(.*)"', line)
            if match:
                value = match.group(1)
                current_strings.append(value)

                # Notification format: [app_name, replaces_id, icon, summary, body, ...]
                # We need at least app_name and summary (indices 0 and 3)
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


async def main():
    """Main server entry point"""
    print("=" * 60)
    print("MiniDisplay Notification Server")
    print("=" * 60)
    print(f"WebSocket server on port {WEBSOCKET_PORT}")
    print(f"History file: {HISTORY_FILE}")

    # Load history
    load_history()
    print(f"Loaded {len(notification_history)} notifications from history")

    # Start WebSocket server
    async with websockets.serve(handle_client, "0.0.0.0", WEBSOCKET_PORT):
        print("WebSocket server started")
        print("Waiting for ESP32 connection...")
        print("=" * 60)

        # Start background tasks
        tasks = [
            asyncio.create_task(notification_sender()),
            asyncio.create_task(dbus_monitor()),
        ]

        # Wait for all tasks
        await asyncio.gather(*tasks)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\nShutting down server...")
