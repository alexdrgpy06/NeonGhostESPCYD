#include <iostream>
#include <string.h>
#include <cstdint>

void drawSingleButton(int x, int y, int w, int h, const char* label, uint16_t color, bool pressed) {
    uint16_t bg = pressed ? color : 0x2104; // C_DARK
    uint16_t fg = pressed ? 0x2104 : color;

    // Calculate text centering (assuming size 1, 6px per char)
    int tw = strlen(label) * 6;
    int tx = x + (w - tw) / 2;
    int ty = y + (h - 8) / 2; // 8px font height

    std::cout << "tft.fillRoundRect(" << x << ", " << y << ", " << w << ", " << h << ", 6, " << bg << ")" << std::endl;
    std::cout << "tft.drawRoundRect(" << x << ", " << y << ", " << w << ", " << h << ", 6, " << color << ")" << std::endl;
    std::cout << "tft.setTextColor(" << fg << ", " << bg << ")" << std::endl;
    std::cout << "tft.setCursor(" << tx << ", " << ty << ")" << std::endl;
    std::cout << "tft.print(\"" << label << "\")" << std::endl;
}

int main() {
    drawSingleButton(15, 270, 70, 45, "FEED", 0x1234, false);
    drawSingleButton(15, 270, 70, 45, "FEED", 0x1234, true);
    return 0;
}
