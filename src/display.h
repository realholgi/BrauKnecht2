#ifndef DISPLAY_H
#define DISPLAY_H


#define DISPLAY_SIZE_X 20
#define DISPLAY_SIZE_Y 4
#define DISPLAY_ADDRESS 0x27 //# 0x27=proto / 0x3f=box

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

void lcd_init();
void lcd_clear();

void print_lcd(char *st, int x, int y);
void print_lcdP(const char *st, int x, int y);
void printNumI_lcd(int num, int x, int y);
void printNumF_lcd(float num, int x, int y);
void print_lcd_minutes(int value, int x, int y);
void print_lcd_deg(int x, int y);

#endif //DISPLAY_H
