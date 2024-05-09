#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <I2CKeyPad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
I2CKeyPad keypad(0x20);

uint32_t period_lcd;
uint32_t period_time;
int second;

void setup() {
    Wire.begin();
    Wire.setClock(400000);

    lcd.init();
    lcd.backlight();

    if(!keypad.begin()) {
        lcd.println("     Error     ");
        lcd.println("No Keypad found");
        while(1);
    }
}

void loop() {
    if (keypad.isPressed())
    {
        lcd.clear();
        lcd.printstr("Hello, World!");
    }

    if (millis() - period_time > 1000)
    {
        second++;

        period_time = millis();
    }

    if ((millis() - period_lcd > 1000) && (!keypad.isPressed()))
    {
        lcd.clear();
        lcd.print("Time: " + String(second, DEC));

        period_lcd = millis();
    }
}