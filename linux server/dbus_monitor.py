#!/usr/bin/env python3
"""
DBus Notification Monitor
Captures notifications using DBus monitoring and sends to notifier server
"""

import asyncio
import json
import subprocess
import re


async def monitor_notifications(notification_queue):
    """Monitor DBus for notifications using dbus-monitor"""

    # dbus-monitor command to watch for Notify method calls
    cmd = [
        "dbus-monitor",
        "--session",
        "interface='org.freedesktop.Notifications',member='Notify'"
    ]

    print("Starting DBus monitor...")
    print(f"Command: {' '.join(cmd)}")

    process = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )

    current_notification = {}
    in_array = False

    while True:
        line = await process.stdout.readline()
        if not line:
            break

        line = line.decode('utf-8').strip()

        # Look for string values in the notification
        if 'string "' in line:
            match = re.search(r'string "(.*)"', line)
            if match:
                value = match.group(1)

                # Build notification from consecutive strings
                # Format: app_name, replaces_id, icon, summary, body
                if 'app_name' not in current_notification:
                    current_notification['app_name'] = value
                elif 'summary' not in current_notification:
                    if value and not value.isdigit():  # Skip replaces_id
                        current_notification['summary'] = value
                elif 'body' not in current_notification:
                    if value and not value.startswith('file://'):  # Skip icon
                        current_notification['body'] = value

                        # We have enough info, send the notification
                        title = current_notification.get('summary', current_notification.get('app_name', 'Notification'))
                        message = current_notification.get('body', current_notification.get('summary', ''))

                        if title or message:
                            print(f"Captured: {title} - {message}")
                            await notification_queue.put((title, message))

                        # Reset for next notification
                        current_notification = {}

        # Reset on method call start
        if 'method call' in line.lower():
            current_notification = {}


if __name__ == "__main__":
    queue = asyncio.Queue()

    async def print_notifications():
        while True:
            title, message = await queue.get()
            print(f"NOTIFICATION: {title} | {message}")

    async def main():
        await asyncio.gather(
            monitor_notifications(queue),
            print_notifications()
        )

    asyncio.run(main())
