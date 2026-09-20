#include "banner.h"
#include "console.h"
#include "theme.h"
#include <cstdio>

namespace {

const char* TITLE = "yourpov.dev";
const char* SUBTITLE = "DLL Injector";

int utf8Width(const char* text) {
    int width = 0;
    for (const unsigned char* p = (const unsigned char*)text; *p; p++) {
        if ((*p & 0xC0) != 0x80) width++;
    }
    return width;
}

void addSpaces(int count) {
    for (int i = 0; i < count; i++) std::putchar(' ');
}

}

namespace banner {

void show() {
    int consoleWidth = getWidth();
    if (consoleWidth <= 0) consoleWidth = 80;

    int titleWidth = utf8Width(TITLE);
    int subtitleWidth = utf8Width(SUBTITLE);
    int contentWidth = (titleWidth > subtitleWidth ? titleWidth : subtitleWidth) + 4;
    int boxWidth = contentWidth + 2;

    int margin = (consoleWidth - boxWidth) / 2;
    if (margin < 0) margin = 0;

    int titlePad = (contentWidth - titleWidth) / 2;
    int subtitlePad = (contentWidth - subtitleWidth) / 2;

    std::printf("\n");
    addSpaces(margin);
    std::printf("%s┌", WHITE);
    for (int i = 0; i < boxWidth - 2; i++) std::fputs("─", stdout);
    std::printf("┐%s\n", RESET);

    addSpaces(margin);
    std::printf("%s│", WHITE);
    addSpaces(titlePad);
    std::printf("%s%s%s", WHITE, TITLE, WHITE);
    addSpaces(contentWidth - titleWidth - titlePad);
    std::printf("│%s\n", RESET);

    addSpaces(margin);
    std::printf("%s│ ", WHITE);
    for (int i = 0; i < contentWidth - 2; i++) std::fputs("─", stdout);
    std::printf(" │%s\n", RESET);

    addSpaces(margin);
    std::printf("%s│", WHITE);
    addSpaces(subtitlePad);
    std::printf("%s%s%s", PURPLE, SUBTITLE, WHITE);
    addSpaces(contentWidth - subtitleWidth - subtitlePad);
    std::printf("│%s\n", RESET);

    addSpaces(margin);
    std::printf("%s└", WHITE);
    for (int i = 0; i < boxWidth - 2; i++) std::fputs("─", stdout);
    std::printf("┘%s\n", RESET);
    std::printf("\n");
}

}