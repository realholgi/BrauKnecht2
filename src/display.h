#pragma once


constexpr int DISPLAY_SIZE_X = 20;
constexpr int DISPLAY_SIZE_Y = 4;

constexpr int LEFT = 0;
constexpr int RIGHT = 9999;
constexpr int CENTER = 9998;

void lcd_init();
void lcd_clear();

void print_lcd(const char *st, int x, int y);
void print_lcdP(const char *st, int x, int y);
void printNumI_lcd(int num, int x, int y);
void printNumF_lcd(float num, int x, int y);
void print_lcd_minutes(int value, int x, int y);
void print_lcd_deg(int x, int y);
