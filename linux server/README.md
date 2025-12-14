# MiniDisplay Linux Server

This directory contains the Linux notification server that forwards system notifications to the ESP32 MiniDisplay.

## Features

- WebSocket server for real-time communication with ESP32
- DBus listener to capture all system notifications (including notify-send)
- Notification history with persistent storage
- dmenu/rofi script for viewing and managing notification history

## Installation

### 1. Install Python Dependencies

```bash
pip install -r requirements.txt
```

Or using your system package manager:
```bash
# Arch Linux
sudo pacman -S python-websockets

# Debian/Ubuntu
sudo apt install python3-websockets
```

### 2. Make Scripts Executable

```bash
chmod +x notifier_server.py notifier_menu.sh
```

### 3. Setup Systemd Service (Optional)

To run the server automatically on startup:

```bash
# Copy service file to user systemd directory
mkdir -p ~/.config/systemd/user
cp notifier.service ~/.config/systemd/user/

# Enable and start the service
systemctl --user enable notifier.service
systemctl --user start notifier.service

# Check status
systemctl --user status notifier.service
```

## Usage

### Running Manually

```bash
./notifier_server.py
```

The server will:
- Start WebSocket server on port 8765
- Listen for DBus notifications
- Save notification history to `~/.local/share/minidisplay/notification_history.json`

### Notification Menu

Bind the notification menu script to a keyboard shortcut:

```bash
./notifier_menu.sh
```

This script:
- Shows all notification history in dmenu/rofi
- Allows you to select and delete individual notifications
- Provides a "Clear All" option

Example i3/sway config:
```
bindsym $mod+n exec ~/source/minidisplay/linux\ server/notifier_menu.sh
```

## Testing

Send a test notification:

```bash
notify-send "Test" "This is a test notification"
```

You should see it:
1. In the server console output
2. On the ESP32 display (if connected)
3. In the notification history file

## Troubleshooting

### No notifications showing up

Make sure dbus-monitor is installed:
```bash
# Arch Linux
sudo pacman -S dbus

# Debian/Ubuntu
sudo apt install dbus-x11
```

### WebSocket connection issues

Check firewall settings:
```bash
sudo ufw allow 8765
```

### Service not starting

Check logs:
```bash
journalctl --user -u notifier.service -f
```

## Configuration

Edit `notifier_server.py` to change:
- `WEBSOCKET_PORT`: WebSocket server port (default: 8765)
- `HISTORY_FILE`: Location of notification history file
- `MAX_HISTORY_ITEMS`: Maximum number of notifications to keep (default: 100)
