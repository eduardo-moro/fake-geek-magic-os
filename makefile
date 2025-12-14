.PHONY: help install uninstall install-server install-esp update-settings uninstall-server clean test status logs

# Colors for terminal output
CYAN := \033[0;36m
GREEN := \033[0;32m
YELLOW := \033[0;33m
RED := \033[0;31m
NC := \033[0m # No Color

# Paths
PROJECT_DIR := $(shell pwd)
SERVER_DIR := $(PROJECT_DIR)/linux server
SYSTEMD_USER_DIR := $(HOME)/.config/systemd/user
LOCAL_BIN := $(HOME)/.local/bin
HISTORY_DIR := $(HOME)/.local/share/minidisplay

help:
	@printf "$(CYAN)MiniDisplay - Notification Display$(NC)\n"
	@printf "\n"
	@printf "$(GREEN)Available targets:$(NC)\n"
	@printf "  $(YELLOW)make install$(NC)          - Install complete project (server + ESP8266)\n"
	@printf "  $(YELLOW)make install-server$(NC)   - Install Linux server only\n"
	@printf "  $(YELLOW)make install-esp$(NC)    - Build and upload ESP8266 firmware\n"
	@printf "  $(YELLOW)make update-settings$(NC)  - Upload settings.json to ESP8266\n"
	@printf "  $(YELLOW)make uninstall$(NC)        - Uninstall everything\n"
	@printf "  $(YELLOW)make uninstall-server$(NC) - Uninstall Linux server only\n"
	@printf "  $(YELLOW)make clean$(NC)            - Clean build files\n"
	@printf "  $(YELLOW)make test$(NC)             - Test the installation\n"
	@printf "\n"
	@printf "$(GREEN)Quick start:$(NC)\n"
	@printf "  1. make install-server  # Install Linux server\n"
	@printf "  2. make install-esp   # Build and upload ESP8266 firmware\n"
	@printf "  3. make test            # Test with a notification\n"

install: install-server
	@printf "\n"
	@printf "$(GREEN)✓ Linux server installed!$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Next steps:$(NC)\n"
	@printf "  1. Build and upload firmware: $(CYAN)make install-esp$(NC)\n"
	@printf "  2. Start the server: $(CYAN)systemctl --user start notifier.service$(NC)\n"
	@printf "  3. Test it: $(CYAN)make test$(NC)\n"

install-server:
	@printf "$(CYAN)Installing MiniDisplay Linux Server...$(NC)\n"
	@printf "\n"

	# Check dependencies
	@printf "$(YELLOW)Checking dependencies...$(NC)\n"
	@command -v python3 >/dev/null 2>&1 || { printf "$(RED)Error: python3 is required$(NC)\n"; exit 1; }
	@command -v pip3 >/dev/null 2>&1 || { printf "$(RED)Error: pip3 is required$(NC)\n"; exit 1; }
	@command -v dbus-monitor >/dev/null 2>&1 || { printf "$(RED)Error: dbus-monitor is required (install 'dbus' package)$(NC)\n"; exit 1; }
	@command -v jq >/dev/null 2>&1 || { printf "$(YELLOW)Warning: jq not found, notification menu won't work$(NC)\n"; }
	@command -v dmenu >/dev/null 2>&1 || command -v rofi >/dev/null 2>&1 || { printf "$(YELLOW)Warning: neither dmenu nor rofi found, notification menu won't work$(NC)\n"; }

	# Install Python dependencies
	@printf "$(YELLOW)Installing Python dependencies...$(NC)\n"
	pip3 install --user -r "$(SERVER_DIR)/requirements.txt"

	# Create directories
	@printf "$(YELLOW)Creating directories...$(NC)\n"
	mkdir -p $(SYSTEMD_USER_DIR)
	mkdir -p $(LOCAL_BIN)
	mkdir -p $(HISTORY_DIR)

	# Make scripts executable
	@printf "$(YELLOW)Setting up scripts...$(NC)\n"
	chmod +x "$(SERVER_DIR)/notifier_server.py"
	chmod +x "$(SERVER_DIR)/notifier_menu.sh"

	# Install menu script to PATH
	ln -sf "$(SERVER_DIR)/notifier_menu.sh" "$(LOCAL_BIN)/notifier-menu"

	# Install systemd service
	@printf "$(YELLOW)Installing systemd service...$(NC)\n"
	sed 's|%h/source/minidisplay/linux server|$(SERVER_DIR)|g' "$(SERVER_DIR)/notifier.service" > "$(SYSTEMD_USER_DIR)/notifier.service"
	sed -i 's|%U|$(shell id -u)|g' "$(SYSTEMD_USER_DIR)/notifier.service"

	# Reload systemd
	systemctl --user daemon-reload

	@printf "\n"
	@printf "$(GREEN)✓ Server installation complete!$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)To start the server:$(NC)\n"
	@printf "  systemctl --user start notifier.service\n"
	@printf "\n"
	@printf "$(YELLOW)To enable auto-start on boot:$(NC)\n"
	@printf "  systemctl --user enable notifier.service\n"
	@printf "\n"
	@printf "$(YELLOW)To use the notification menu, add this to your window manager config:$(NC)\n"
	@printf "  $(CYAN)notifier-menu$(NC)\n"

install-esp:
	@printf "$(CYAN)Building and uploading ESP8266 firmware...$(NC)\n"
	@printf "\n"

	# Check if platformio is installed
	@command -v pio >/dev/null 2>&1 || { \
		printf "$(RED)Error: PlatformIO is not installed$(NC)\n"; \
		printf "$(YELLOW)Install with: pip3 install platformio$(NC)\n"; \
		exit 1; \
	}

	# Build and upload
	@printf "$(YELLOW)Building firmware...$(NC)\n"
	pio run

	@printf "\n"
	@printf "$(YELLOW)Uploading settings.json...$(NC)\n"
	@printf "$(YELLOW)Make sure your ESP8266 is connected!$(NC)\n"
	pio run --target uploadfs

	@printf "\n"
	@printf "$(YELLOW)Uploading firmware...$(NC)\n"
	pio run --target upload

	@printf "\n"
	@printf "$(GREEN)✓ ESP8266 firmware and settings uploaded!$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)To monitor serial output:$(NC)\n"
	@printf "  pio device monitor\n"

update-settings:
	@printf "$(CYAN)Updating ESP8266 settings...$(NC)\n"
	@printf "\n"

	# Check if platformio is installed
	@command -v pio >/dev/null 2>&1 || { \
		printf "$(RED)Error: PlatformIO is not installed$(NC)\n"; \
		printf "$(YELLOW)Install with: pip3 install platformio$(NC)\n"; \
		exit 1; \
	}

	@printf "$(YELLOW)Uploading settings.json...$(NC)\n"
	@printf "$(YELLOW)Make sure your ESP8266 is connected!$(NC)\n"
	pio run --target uploadfs

	@printf "\n"
	@printf "$(GREEN)✓ Settings uploaded!$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Note: ESP8266 will reboot and load new settings$(NC)\n"

uninstall: uninstall-server
	@printf "\n"
	@printf "$(GREEN)✓ MiniDisplay uninstalled$(NC)\n"

uninstall-server:
	@printf "$(CYAN)Uninstalling MiniDisplay Linux Server...$(NC)\n"
	@printf "\n"

	# Stop and disable service
	@printf "$(YELLOW)Stopping service...$(NC)\n"
	-systemctl --user stop notifier.service 2>/dev/null
	-systemctl --user disable notifier.service 2>/dev/null

	# Remove service file
	@printf "$(YELLOW)Removing service file...$(NC)\n"
	rm -f "$(SYSTEMD_USER_DIR)/notifier.service"
	systemctl --user daemon-reload

	# Remove menu script symlink
	@printf "$(YELLOW)Removing scripts...$(NC)\n"
	rm -f "$(LOCAL_BIN)/notifier-menu"

	# Ask about history
	@printf "\n"
	@printf "$(YELLOW)Do you want to remove notification history? [y/N]$(NC) "
	@read -n 1 -r REPLY; \
	printf "\n"; \
	if [ "$$REPLY" = "y" ] || [ "$$REPLY" = "Y" ]; then \
		printf "$(YELLOW)Removing history...$(NC)\n"; \
		rm -rf "$(HISTORY_DIR)"; \
		printf "$(GREEN)✓ History removed$(NC)\n"; \
	else \
		printf "$(YELLOW)Keeping history at: $(HISTORY_DIR)$(NC)\n"; \
	fi

	@printf "\n"
	@printf "$(GREEN)✓ Server uninstalled$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Python packages were not removed. To remove them manually:$(NC)\n"
	@printf "  pip3 uninstall websockets\n"

clean:
	@printf "$(CYAN)Cleaning build files...$(NC)\n"
	rm -rf .pio
	rm -rf .cache
	find . -type f -name "*.pyc" -delete
	find . -type d -name "__pycache__" -delete
	@printf "$(GREEN)✓ Clean complete$(NC)\n"

test:
	@printf "$(CYAN)Testing MiniDisplay...$(NC)\n"
	@printf "\n"

	# Check if service is running
	@printf "$(YELLOW)Checking if server is running...$(NC)\n"
	@if systemctl --user is-active --quiet notifier.service; then \
		printf "$(GREEN)✓ Server is running$(NC)\n"; \
	else \
		printf "$(RED)✗ Server is not running$(NC)\n"; \
		printf "$(YELLOW)Start it with: systemctl --user start notifier.service$(NC)\n"; \
		exit 1; \
	fi

	# Send test notification
	@printf "\n"
	@printf "$(YELLOW)Sending test notification...$(NC)\n"
	notify-send "MiniDisplay Test" "If you see this on your ESP8266, it's working!"

	@printf "\n"
	@printf "$(GREEN)✓ Test notification sent!$(NC)\n"
	@printf "$(YELLOW)Check your ESP8266 display$(NC)\n"

status:
	@printf "$(CYAN)MiniDisplay Status$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Server status:$(NC)\n"
	@systemctl --user status notifier.service --no-pager || printf "$(RED)Server not installed$(NC)\n"
	@printf "\n"
	@printf "$(YELLOW)Installation status:$(NC)\n"
	@[ -f "$(SYSTEMD_USER_DIR)/notifier.service" ] && printf "  $(GREEN)✓$(NC) Systemd service installed\n" || printf "  $(RED)✗$(NC) Systemd service not installed\n"
	@[ -f "$(LOCAL_BIN)/notifier-menu" ] && printf "  $(GREEN)✓$(NC) Menu script installed\n" || printf "  $(RED)✗$(NC) Menu script not installed\n"
	@[ -d "$(HISTORY_DIR)" ] && printf "  $(GREEN)✓$(NC) History directory exists\n" || printf "  $(YELLOW)○$(NC) History directory not created yet\n"
	@[ -f "$(HISTORY_DIR)/notification_history.json" ] && printf "  $(GREEN)✓$(NC) History file exists ($$(jq '. | length' '$(HISTORY_DIR)/notification_history.json' 2>/dev/null || echo '0') notifications)\n" || printf "  $(YELLOW)○$(NC) No history yet\n"

logs:
	@printf "$(CYAN)MiniDisplay Server Logs$(NC)\n"
	@printf "$(YELLOW)Press Ctrl+C to exit$(NC)\n"
	@printf "\n"
	journalctl --user -u notifier.service -f
