#include "list_menu.hpp"

// Helper function to convert UTF-8 characters for display
// This handles common Portuguese characters
String sanitizeText(const String& text) {
    String result = "";
    for (unsigned int i = 0; i < text.length(); i++) {
        unsigned char c = text[i];

        // Check for UTF-8 multibyte sequences
        if ((c & 0x80) != 0) { // Non-ASCII character
            // Handle 2-byte UTF-8 sequences (most common accented characters)
            if (i + 1 < text.length() && (c & 0xE0) == 0xC0) {
                unsigned char c2 = text[i + 1];
                uint16_t unicode = ((c & 0x1F) << 6) | (c2 & 0x3F);

                // Map common Portuguese characters to ASCII approximations
                switch(unicode) {
                    case 0xE1: result += 'a'; break; // á
                    case 0xE0: result += 'a'; break; // à
                    case 0xE2: result += 'a'; break; // â
                    case 0xE3: result += 'a'; break; // ã
                    case 0xE9: result += 'e'; break; // é
                    case 0xEA: result += 'e'; break; // ê
                    case 0xED: result += 'i'; break; // í
                    case 0xF3: result += 'o'; break; // ó
                    case 0xF4: result += 'o'; break; // ô
                    case 0xF5: result += 'o'; break; // õ
                    case 0xFA: result += 'u'; break; // ú
                    case 0xFC: result += 'u'; break; // ü
                    case 0xE7: result += 'c'; break; // ç
                    case 0xC1: result += 'A'; break; // Á
                    case 0xC0: result += 'A'; break; // À
                    case 0xC2: result += 'A'; break; // Â
                    case 0xC3: result += 'A'; break; // Ã
                    case 0xC9: result += 'E'; break; // É
                    case 0xCA: result += 'E'; break; // Ê
                    case 0xCD: result += 'I'; break; // Í
                    case 0xD3: result += 'O'; break; // Ó
                    case 0xD4: result += 'O'; break; // Ô
                    case 0xD5: result += 'O'; break; // Õ
                    case 0xDA: result += 'U'; break; // Ú
                    case 0xDC: result += 'U'; break; // Ü
                    case 0xC7: result += 'C'; break; // Ç
                    default: result += '?'; break; // Unknown character
                }
                i++; // Skip next byte
            } else {
                result += '?'; // Unknown multibyte sequence
            }
        } else {
            result += (char)c; // Regular ASCII character
        }
    }
    return result;
}

// Menu configuration constants
// Screen layout (240px wide): [5px margin][~200px label][~20px value][5px margin]
const int ITEM_HEIGHT = 20;        // Reduced for smaller font
const int MENU_TOP = 10;
const int MENU_LEFT = 5;
const int SCREEN_WIDTH = 240;      // Total screen width
const int VALUE_RIGHT_MARGIN = 5;  // Margin from right edge
const int VALUE_WIDTH = 20;        // Width reserved for value text (signal 0-4)
const int LABEL_MAX_WIDTH = SCREEN_WIDTH - MENU_LEFT - VALUE_WIDTH - VALUE_RIGHT_MARGIN - 10; // ~200px for label
const int TEXT_SIZE = 1;           // Use smallest font to fit more text
const int SCROLL_SPEED = 200;      // ms per pixel scroll (faster for smaller text)

static ListMenuState menuState;

void initListMenu(const std::vector<ListMenuItem>& items) {
    menuState.items = items;
    menuState.currentIndex = 0;
    menuState.previousIndex = 0;
    menuState.scrollOffset = 0;
    menuState.previousScrollOffset = 0;
    menuState.visibleItems = (240 - MENU_TOP * 2) / ITEM_HEIGHT; // Calculate based on screen height
    menuState.isSelected = false;
    menuState.scrollTimer = 0;
    menuState.labelScrollOffset = 0;
    menuState.needsFullRedraw = true;

    // Clear screen and render initial menu
    tft.fillScreen(TFT_BLACK);
    renderListMenu();
}

void renderListMenu() {
    // Reset text settings to ensure clean rendering
    tft.setTextSize(TEXT_SIZE);
    tft.setTextDatum(TL_DATUM); // Top-left alignment
    tft.setTextPadding(0); // No padding

    // Check if we need full redraw or can do partial
    bool scrollChanged = (menuState.scrollOffset != menuState.previousScrollOffset);
    bool needsFullRedraw = menuState.needsFullRedraw || scrollChanged;

    // Only clear screen if we need full redraw
    if (needsFullRedraw) {
        tft.fillScreen(TFT_BLACK);
        menuState.needsFullRedraw = false;
    }

    int displayCount = min(menuState.visibleItems, (int)menuState.items.size());

    for (int i = 0; i < displayCount; i++) {
        int itemIndex = menuState.scrollOffset + i;
        if (itemIndex >= menuState.items.size()) break;

        int y = MENU_TOP + (i * ITEM_HEIGHT);
        bool isFocused = (itemIndex == menuState.currentIndex);
        bool wasPreviouslyFocused = (itemIndex == menuState.previousIndex && !scrollChanged);

        // Skip rendering if nothing changed for this item (partial redraw optimization)
        if (!needsFullRedraw && !isFocused && !wasPreviouslyFocused) {
            continue; // This item doesn't need redrawing
        }

        // Determine colors based on state
        uint16_t bgColor, fgColor, borderColor;
        if (isFocused && menuState.isSelected) {
            // Selected state: 1px border
            bgColor = TFT_BLACK;
            fgColor = TFT_WHITE;
            borderColor = TFT_WHITE;
        } else if (isFocused) {
            // Focused state: reversed colors
            bgColor = TFT_WHITE;
            fgColor = TFT_BLACK;
            borderColor = TFT_WHITE;
        } else {
            // Normal state
            bgColor = TFT_BLACK;
            fgColor = TFT_WHITE;
            borderColor = TFT_BLACK;
        }

        // Draw background (full width)
        int itemWidth = SCREEN_WIDTH - MENU_LEFT - VALUE_RIGHT_MARGIN;
        tft.fillRect(MENU_LEFT, y, itemWidth, ITEM_HEIGHT, bgColor);

        // Draw border if selected
        if (isFocused && menuState.isSelected) {
            tft.drawRect(MENU_LEFT, y, itemWidth, ITEM_HEIGHT, borderColor);
        }

        // Get label and value - sanitize for UTF-8 characters
        String label = sanitizeText(menuState.items[itemIndex].label);
        String value = menuState.items[itemIndex].value;

        // Handle label scrolling for focused item if label is too long
        int labelWidth = label.length() * 6 * TEXT_SIZE; // Approximate width
        int scrollOffset = 0;

        if (isFocused && labelWidth > LABEL_MAX_WIDTH) {
            // Animate scrolling
            unsigned long currentTime = millis();
            if (currentTime - menuState.scrollTimer > SCROLL_SPEED) {
                menuState.labelScrollOffset++;
                int maxScroll = labelWidth - LABEL_MAX_WIDTH + 20; // Add padding
                if (menuState.labelScrollOffset > maxScroll) {
                    menuState.labelScrollOffset = -20; // Reset with pause
                }
                menuState.scrollTimer = currentTime;
            }
            scrollOffset = menuState.labelScrollOffset;
        } else {
            menuState.labelScrollOffset = 0;
        }

        // Draw label with proper clipping to prevent wrapping
        tft.setTextColor(fgColor, bgColor);
        tft.setTextWrap(false); // Disable text wrapping

        int labelX = MENU_LEFT + 3 - scrollOffset;
        int labelY = y + (ITEM_HEIGHT - 8) / 2; // Center vertically (8 = text height for size 1)

        // Create a clipped version of the label if needed
        String displayLabel = label;
        int charWidth = 6 * TEXT_SIZE; // Each character is 6 pixels wide for font size 1
        int maxChars = LABEL_MAX_WIDTH / charWidth;

        if (labelWidth > LABEL_MAX_WIDTH && !isFocused) {
            // Truncate long labels when not focused
            if (label.length() > maxChars - 3) {
                displayLabel = label.substring(0, maxChars - 3) + "...";
            }
        } else if (isFocused && scrollOffset == 0) {
            // Even when focused, if not scrolling yet, truncate if too long
            if (label.length() > maxChars) {
                displayLabel = label.substring(0, maxChars);
            }
        }

        tft.setCursor(labelX, labelY);
        tft.print(displayLabel);

        // Draw value (right-aligned to screen edge)
        if (value.length() > 0) {
            int valueX = SCREEN_WIDTH - VALUE_RIGHT_MARGIN - (value.length() * charWidth);
            int valueY = y + (ITEM_HEIGHT - 8) / 2;
            tft.setCursor(valueX, valueY);
            tft.print(value);
        }
    }
    tft.setTextDatum(MC_DATUM); 
}

void listMenuNext() {
    if (menuState.items.size() == 0) return;

    // Save previous state for partial redraw
    menuState.previousIndex = menuState.currentIndex;
    menuState.previousScrollOffset = menuState.scrollOffset;

    menuState.currentIndex++;
    if (menuState.currentIndex >= menuState.items.size()) {
        menuState.currentIndex = 0; // Circular
        menuState.scrollOffset = 0;
    }

    // Handle scrolling: keep selection at second-to-last visible position when scrolling down
    int visiblePosition = menuState.currentIndex - menuState.scrollOffset;
    if (visiblePosition >= menuState.visibleItems - 1 && menuState.currentIndex < menuState.items.size() - 1) {
        menuState.scrollOffset++;
    }

    menuState.labelScrollOffset = 0;
    menuState.scrollTimer = millis();
    renderListMenu();
}

void listMenuPrev() {
    if (menuState.items.size() == 0) return;

    // Save previous state for partial redraw
    menuState.previousIndex = menuState.currentIndex;
    menuState.previousScrollOffset = menuState.scrollOffset;

    menuState.currentIndex--;
    if (menuState.currentIndex < 0) {
        menuState.currentIndex = menuState.items.size() - 1; // Circular
        // Scroll to show last item
        menuState.scrollOffset = max(0, (int)menuState.items.size() - menuState.visibleItems);
    }

    // Handle scrolling: keep selection at second position when scrolling up
    int visiblePosition = menuState.currentIndex - menuState.scrollOffset;
    if (visiblePosition <= 1 && menuState.scrollOffset > 0 && menuState.currentIndex > 0) {
        menuState.scrollOffset--;
    }

    // Reset scroll to top when at first item
    if (menuState.currentIndex == 0) {
        menuState.scrollOffset = 0;
    }

    menuState.labelScrollOffset = 0;
    menuState.scrollTimer = millis();
    renderListMenu();
}

void listMenuSelect() {
    if (menuState.items.size() == 0) return;

    ListMenuItem& item = menuState.items[menuState.currentIndex];

    // If item has no value and no hold action, just has label and click action
    // Automatically run the click action
    if (item.value.length() == 0 && item.onHold == nullptr && item.onClick != nullptr) {
        item.onClick(item.label);
        return;
    }

    // Toggle selected state
    menuState.isSelected = true;
    renderListMenu();

    // Execute hold action if available (pass label, not value)
    if (item.onHold != nullptr) {
        item.onHold(item.label);
    }

    // Reset selection state after brief delay
    delay(200);
    menuState.isSelected = false;
    renderListMenu();
}

ListMenuState& getListMenuState() {
    return menuState;
}

void destroyListMenu() {
    menuState.items.clear();
    menuState.currentIndex = 0;
    menuState.scrollOffset = 0;
    menuState.isSelected = false;

    // Reset text settings to match main menu defaults
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);  // Main menu uses middle-center alignment
    tft.setTextPadding(0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextWrap(true);  // Re-enable text wrapping for main menu
}
