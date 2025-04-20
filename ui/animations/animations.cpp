#include animations.h

void loop_wifi_icon()
{
    static const uint8_t *frames[] = {wifi_0_bits, wifi_1_bits, wifi_2_bits, wifi_3_bits};
    animateIcon(frames, 4);
}

void loop_bright_icon()
{
    static const uint8_t *frames[] = {bright_0_bits, bright_1_bits, bright_2_bits, bright_3_bits};
    animateIcon(frames, 4);
}

void animateIcon(const uint8_t *frames[], int frameCount)
{
    if (millis() - lastAnimTime >= icon_pos.anim_delay)
    {
        lastAnimTime = millis();
        tft.drawXBitmap(icon_pos.x, icon_pos.y, frames[currentAnimFrame], icon_pos.w, icon_pos.h, TFT_BLACK, TFT_WHITE);
        currentAnimFrame = (currentAnimFrame + 1) % frameCount;
    }
}