#include "wifi_menu.hpp"
#include "../../ui/menu/menu.hpp"
#include <algorithm>

// Convert signal strength to string representation (0-4)
String signalStrengthToString(int strength) {
    return String(strength);
}

// Handler for going back to main menu
void handleWifiMenuBack(const String& value) {
    Serial.println("Returning to main menu");
    destroyListMenu();
    route = "menu";
    initializeMenu();
}

// Handler for connecting to selected network
// Note: This receives the SSID as the parameter (from label, not value)
void handleWifiConnect(const String& ssid) {
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    // First, properly cleanup the list menu and set route
    destroyListMenu();
    route = "menu"; // Set route before connection attempt

    // Note: connectToNetwork() -> start_client() handles:
    // 1. Screen clearing
    // 2. WiFi animation while connecting
    // 3. Calling initializeMenu() when done
    // 4. Displaying the IP address
    // So we just need to call it directly
    connectToNetwork(ssid);
}

// Show the WiFi selection menu
void showWifiSelectionMenu() {
    Serial.println("Showing WiFi selection menu");

    // Get available networks
    std::vector<WifiItem> wifiItems = getAvailableNetworksMenuItems();

    // Sort by signal strength (descending - strongest first)
    std::sort(wifiItems.begin(), wifiItems.end(),
        [](const WifiItem& a, const WifiItem& b) {
            return a.signal_strength > b.signal_strength;
        });

    // Create list menu items
    std::vector<ListMenuItem> menuItems;

    // Add "Back" as first item
    menuItems.push_back(ListMenuItem("VOLTAR", "", handleWifiMenuBack, nullptr));

    // Add WiFi networks with signal strength as value
    for (const auto& wifi : wifiItems) {
        String strengthValue = signalStrengthToString(wifi.signal_strength);
        menuItems.push_back(ListMenuItem(
            wifi.ssid,
            strengthValue,
            nullptr,
            handleWifiConnect
        ));
    }

    // If no networks found, show message
    if (wifiItems.size() == 0) {
        menuItems.push_back(ListMenuItem("No networks found", "", nullptr, nullptr));
    }

    // Initialize the list menu
    route = "wifi_select";
    initListMenu(menuItems);
}
