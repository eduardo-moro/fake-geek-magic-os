#ifndef SETTINGS_ICON_H
#define SETTINGS_ICON_H

typedef struct MenuItem {
    const char* label;

    void (*onClick)();
    void (*onLongPress)();
    void (*onDoubleClick)();

    const uint8_t * iconFrames[4];
    int size = 3;

    MenuItem *subMenu;
} MenuItem;

extern MenuItem menu_items[];

#endif