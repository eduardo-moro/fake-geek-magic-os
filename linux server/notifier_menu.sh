#!/bin/bash
#
# MiniDisplay Notification Menu
# dmenu/rofi script for managing notification history
#

HISTORY_FILE="$HOME/.local/share/minidisplay/notification_history.json"

# Check if history file exists
if [ ! -f "$HISTORY_FILE" ]; then
    echo "No notification history found" | dmenu -p "Notifications"
    exit 0
fi

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "Error: jq is required but not installed" | dmenu -p "Error"
    exit 1
fi

# Read notifications from history
notifications=$(jq -r '.[] | "\(.id)|\(.title)|\(.message)|\(.timestamp)"' "$HISTORY_FILE" 2>/dev/null)

if [ -z "$notifications" ]; then
    echo "No notifications" | dmenu -p "Notifications"
    exit 0
fi

# Create menu entries
menu_entries="[Clear All]\n"

while IFS='|' read -r id title message timestamp; do
    # Format timestamp nicely
    formatted_time=$(date -d "$timestamp" "+%H:%M %d/%m" 2>/dev/null || echo "")

    # Truncate message if too long
    display_message="$message"
    if [ ${#display_message} -gt 60 ]; then
        display_message="${display_message:0:57}..."
    fi

    menu_entries+="[$formatted_time] $title - $display_message|$id\n"
done <<< "$notifications"

# Show dmenu (or rofi if preferred)
if command -v rofi &> /dev/null; then
    # Use rofi if available
    selected=$(echo -e "$menu_entries" | sed 's/|[0-9]*$//' | rofi -dmenu -i -p "Notifications" -format s)
else
    # Use dmenu
    selected=$(echo -e "$menu_entries" | sed 's/|[0-9]*$//' | dmenu -i -l 10 -p "Notifications")
fi

# Exit if nothing selected
if [ -z "$selected" ]; then
    exit 0
fi

# Handle Clear All option
if [ "$selected" = "[Clear All]" ]; then
    # Confirm
    confirm=$(echo -e "Yes\nNo" | dmenu -p "Clear all notifications?")
    if [ "$confirm" = "Yes" ]; then
        echo "[]" > "$HISTORY_FILE"
        notify-send "Notifications" "All notifications cleared"
    fi
    exit 0
fi

# Extract the ID from the selected entry
selected_id=$(echo -e "$menu_entries" | grep -F "$selected" | sed 's/.*|\([0-9]*\)$/\1/')

if [ -n "$selected_id" ]; then
    # Remove the notification from history
    jq "del(.[] | select(.id == $selected_id))" "$HISTORY_FILE" > "$HISTORY_FILE.tmp"
    mv "$HISTORY_FILE.tmp" "$HISTORY_FILE"

    # Renumber IDs
    jq '[.[] | . + {id: .[]}] | to_entries | map(.value + {id: .key})' "$HISTORY_FILE" > "$HISTORY_FILE.tmp"
    mv "$HISTORY_FILE.tmp" "$HISTORY_FILE"

    notify-send "Notifications" "Notification removed"
fi
