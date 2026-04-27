#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#define BUTTON_Y 270
#define BUTTON_H 45
#define BUTTON_W 70
#define C_DARK 0x2104

class MockTFT {
public:
    void fillRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
        std::cout << "fillRoundRect(" << x << ", " << y << ", " << w << ", " << h << ", " << r << ", " << color << ")\n";
    }
    void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
        std::cout << "drawRoundRect(" << x << ", " << y << ", " << w << ", " << h << ", " << r << ", " << color << ")\n";
    }
    void setTextColor(uint16_t c, uint16_t bg) {
        std::cout << "setTextColor(" << c << ", " << bg << ")\n";
    }
    void setTextSize(int s) {
        std::cout << "setTextSize(" << s << ")\n";
    }
    void setCursor(int x, int y) {
        std::cout << "setCursor(" << x << ", " << y << ")\n";
    }
    void print(const char* s) {
        std::cout << "print(" << s << ")\n";
    }
};

MockTFT tft;

void drawSingleButton(int x, int y, int w, int h, const char* label, uint16_t color, bool pressed) {
    uint16_t bg = pressed ? color : C_DARK;
    uint16_t fg = pressed ? C_DARK : color;

    tft.fillRoundRect(x, y, w, h, 6, bg);
    tft.drawRoundRect(x, y, w, h, 6, color);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);

    // Center text dynamically
    int textWidth = strlen(label) * 6; // 6 pixels per char at size 1
    int textX = x + (w - textWidth) / 2;
    int textY = y + (h - 8) / 2;       // 8 pixels high

    tft.setCursor(textX, textY);
    tft.print(label);
}

int main() {
    drawSingleButton(15, BUTTON_Y, BUTTON_W, BUTTON_H, "FEED", 0xFFFF, false);
    drawSingleButton(15, BUTTON_Y, BUTTON_W, BUTTON_H, "FEED", 0xFFFF, true);
    return 0;
}
