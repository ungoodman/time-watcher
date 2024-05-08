#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <I2CKeyPad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
I2CKeyPad keypad(0x38);

// char keymap[19] = "123A456B789C*0#DNF";

uint32_t period;
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
    lcd.clear();

    if (keypad.isPressed())
    {
        lcd.printstr("Hello, World!");
    }

    if ((millis() - period > 1000) && (!keypad.isPressed()))
    {
        lcd.print("Time: " + String(second));

        second++;
        period = millis();
    }
}