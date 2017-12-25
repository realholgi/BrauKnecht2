
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "display.h"

byte degC[8] = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111};

LiquidCrystal_I2C lcd(DISPLAY_ADDRESS, DISPLAY_SIZE_X, DISPLAY_SIZE_Y);

void lcd_init() {
    lcd.init();
    lcd.createChar(8, degC);
    lcd.backlight();
    lcd.clear();
    lcd.noCursor();
}

void print_lcdP(const char *st, int x, int y) {
    int stl = strlen(st);
    if (x == RIGHT) {
        x = DISPLAY_SIZE_X - stl;
    }
    if (x == CENTER) {
        x = (DISPLAY_SIZE_X - stl) / 2;
    }

    x = constrain(x, 0, DISPLAY_SIZE_X - 1);
    y = constrain(y, 0, DISPLAY_SIZE_Y - 1);

    lcd.setCursor(x, y);

    char buf[DISPLAY_SIZE_X + 1];
    strcpy_P(buf, st);
    lcd.print(buf);
}

void print_lcd(char *st, int x, int y) {
    int stl = strlen(st);
    if (x == RIGHT) {
        x = DISPLAY_SIZE_X - stl;
    }
    if (x == CENTER) {
        x = (DISPLAY_SIZE_X - stl) / 2;
    }

    x = constrain(x, 0, DISPLAY_SIZE_X - 1);
    y = constrain(y, 0, DISPLAY_SIZE_Y - 1);

    lcd.setCursor(x, y);
    lcd.print(st);
}

void printNumI_lcd(int num, int x, int y) {
    char st[DISPLAY_SIZE_X + 10];
    sprintf(st, "%i", num);
    print_lcd(st, x, y);
}

void printNumF_lcd(double num, int x, int y) {
    char st[DISPLAY_SIZE_X + 10];

    dtostrf(num, 0, 0, st);
    print_lcd(st, x, y);
}

#define LENGTH_OF_MIN_TEXT 4
#define LENGTH_OF_MIN_VALUE 3

void print_lcd_minutes(int value, int x, int y) {
    value = constrain(value, 0, 999); // max LENGTH_OF_MIN_VALUE

    if (x == RIGHT) {
        x = DISPLAY_SIZE_X - 1 - (LENGTH_OF_MIN_VALUE + LENGTH_OF_MIN_TEXT) + 1;
    }

    if (value < 10) {
        print_lcdP(PSTR("  "), x, y);
        printNumI_lcd(value, x + 2, y);
    }
    if ((value < 100) && (value >= 10)) {
        print_lcdP(PSTR(" "), x, y);
        printNumI_lcd(value, x + 1, y);
    }
    if (value >= 100) {
        printNumI_lcd(value, x, y);
    }
    print_lcdP(PSTR(" min"), x + LENGTH_OF_MIN_VALUE, y); // LENGTH_OF_MIN_TEXT
}

void print_lcd_deg(int x, int y) {
    x = constrain(x, 0, DISPLAY_SIZE_X - 1);
    y = constrain(y, 0, DISPLAY_SIZE_Y - 1);
    lcd.setCursor(x, y);

    lcd.write(8);
}

void lcd_clear() {
    lcd.clear();
}