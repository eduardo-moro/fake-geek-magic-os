#ifndef LIST_MENU_H
#define LIST_MENU_H

#include <Arduino.h>
#include "TFT_eSPI.h"
#include <vector>

extern TFT_eSPI tft;

// Menu item structure
struct ListMenuItem {
    String label;           // Mandatory: display text (will be sanitized for UTF-8)
    String value;           // Optional: value to display on right
    void (*onClick)(const String& label);   // Optional: action on click (receives label)
    void (*onHold)(const String& label);    // Optional: action on hold/select (receives label)

    ListMenuItem(
        const String& lbl,
        const String& val = "",
        void (*clickAction)(const String&) = nullptr,
        void (*holdAction)(const String&) = nullptr
    ) : label(lbl), value(val), onClick(clickAction), onHold(holdAction) {}
};

// Menu state
struct ListMenuState {
    std::vector<ListMenuItem> items;
    int currentIndex;           // Currently focused item
    int previousIndex;          // Previous focused item (for partial redraw)
    int scrollOffset;           // Top visible item index
    int previousScrollOffset;   // Previous scroll offset (for partial redraw)
    int visibleItems;           // Number of items that fit on screen
    bool isSelected;            // Whether current item is selected (hold state)
    unsigned long scrollTimer;  // For label scrolling animation
    int labelScrollOffset;      // Horizontal scroll offset for long labels
    bool needsFullRedraw;       // Flag to force full redraw
};

// Initialize the list menu
void initListMenu(const std::vector<ListMenuItem>& items);

// Render the list menu
void renderListMenu();

// Navigation functions
void listMenuNext();        // Move to next item (single click)
void listMenuPrev();        // Move to previous item (double click)
void listMenuSelect();      // Select/activate current item (hold)

// Get current menu state
ListMenuState& getListMenuState();

// Cleanup
void destroyListMenu();

#endif // LIST_MENU_H
