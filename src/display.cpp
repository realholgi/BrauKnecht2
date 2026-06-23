#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "display.h"
#include "display_format.h"

uint8_t degC[8] = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111};

// 0x27 here is just a placeholder; lcd_init() reassigns lcd with the detected
// address before any use.
LiquidCrystal_I2C lcd(0x27, DISPLAY_SIZE_X, DISPLAY_SIZE_Y);

static int clampY(int y) {
    return constrain(y, 0, DISPLAY_SIZE_Y - 1);
}

// These PCF8574 backpacks ship at one of two addresses; probe both and use
// whichever answers, falling back to the first if neither does.
static uint8_t detectDisplayAddress() {
    Wire.begin();
    const uint8_t candidates[] = {0x27, 0x3f};
    for (uint8_t a : candidates) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            return a;
        }
    }
    return candidates[0];
}

void lcd_init() {
    lcd = LiquidCrystal_I2C(detectDisplayAddress(), DISPLAY_SIZE_X, DISPLAY_SIZE_Y);
    lcd.init();
    lcd.createChar(8, degC);
    lcd.backlight();
    lcd.clear();
    lcd.noCursor();
}

void print_lcdP(const char *st, int x, int y) {
    lcd.setCursor(alignX(x, strlen(st)), clampY(y));

    char buf[DISPLAY_SIZE_X + 1];
    strcpy_P(buf, st);
    lcd.print(buf);
}

void print_lcd(const char *st, int x, int y) {
    lcd.setCursor(alignX(x, strlen(st)), clampY(y));
    lcd.print(st);
}

void printNumI_lcd(int num, int x, int y) {
    char st[DISPLAY_SIZE_X + 10];
    snprintf(st, sizeof(st), "%i", num);
    print_lcd(st, x, y);
}

void printNumF_lcd(float num, int x, int y) {
    char st[DISPLAY_SIZE_X + 10];
    dtostrf(num, 0, 1, st);
    print_lcd(st, x, y);
}

void print_lcd_minutes(int value, int x, int y) {
    char buf[8];
    formatMinutes(buf, sizeof(buf), value);
    print_lcd(buf, x, y);
}

void print_lcd_deg(int x, int y) {
    lcd.setCursor(constrain(x, 0, DISPLAY_SIZE_X - 1), clampY(y));
    lcd.write(8);
}

void lcd_clear() {
    lcd.clear();
}
