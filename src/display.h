#pragma once


constexpr int DISPLAY_SIZE_X = 20;
constexpr int DISPLAY_SIZE_Y = 4;

constexpr int LEFT = 0;
constexpr int RIGHT = 9999;
constexpr int CENTER = 9998;

void lcd_init();
void lcd_clear();

extern bool lcdNeedsRedraw;  // true after a clear / screen change; renderers redraw once then idle
extern bool overlaysDirty;   // true after a clear; loop() overlays (temp/soll/H) redraw once then on change

void print_lcd(const char *st, int x, int y);
void print_lcdP(const char *st, int x, int y);
void printNumI_lcd(int num, int x, int y);
void printNumF_lcd(float num, int x, int y);
void print_lcd_minutes(int value, int x, int y);
void print_lcd_deg(int x, int y);
