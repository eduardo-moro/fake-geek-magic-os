#include "test_menu.hpp"
#include "../../ui/menu/menu.hpp"

// Handler for going back to main menu
void handleTestMenuBack(const String& value) {
    Serial.println("Returning to main menu from test");
    destroyListMenu();
    route = "menu";
    initializeMenu();
}

// Handler for selecting a test item
void handleTestItemSelect(const String& label) {
    Serial.print("Selected test item: ");
    Serial.println(label);

    // Show selection message
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextPadding(0);

    tft.drawString("Selected:", 120, 100);
    tft.drawString(label, 120, 120);

    delay(1500);

    // Return to test menu
    showTestMenu();
}

// Show the test menu with 30 items
void showTestMenu() {
    Serial.println("Showing test menu with 30 items");

    // Create list menu items
    std::vector<ListMenuItem> menuItems;

    // Add "Back" as first item
    menuItems.push_back(ListMenuItem("VOLTAR", "", handleTestMenuBack, nullptr));

    // Add 30 test items with various lengths and values
    const char* testLabels[] = {
        "Item 1 - Short",
        "Item 2 - Medium length name",
        "Item 3 - This is a very long name to test scrolling",
        "Item 4 - Teste com caracteres especiais çãõ",
        "Item 5 - Another short one",
        "Item 6 - WiFi Network Name Example",
        "Item 7 - SuperLongNetworkNameThatNeverEnds",
        "Item 8 - Test 8",
        "Item 9 - Rede sem fio residencial principal",
        "Item 10 - Medium",
        "Item 11 - Short",
        "Item 12 - Testing vertical scroll behavior",
        "Item 13 - Lucky number",
        "Item 14 - Almost halfway there",
        "Item 15 - Halfway point reached!",
        "Item 16 - Continuing the journey",
        "Item 17 - More items to scroll through",
        "Item 18 - Keep scrolling",
        "Item 19 - Almost at twenty",
        "Item 20 - Twenty items milestone",
        "Item 21 - Past the halfway mark now",
        "Item 22 - Two more and we're at 24",
        "Item 23 - Getting close to the end",
        "Item 24 - Six more to go",
        "Item 25 - Quarter century of items",
        "Item 26 - Almost there now",
        "Item 27 - Three more items",
        "Item 28 - Two items remaining",
        "Item 29 - Just one more after this",
        "Item 30 - Final item in the list!"
    };

    // Add all test items
    for (int i = 0; i < 30; i++) {
        String value = String(i + 1); // Value is just the item number
        menuItems.push_back(ListMenuItem(
            testLabels[i],
            value,
            nullptr,
            handleTestItemSelect
        ));
    }

    // Initialize the list menu
    route = "test_menu";
    initListMenu(menuItems);
}
