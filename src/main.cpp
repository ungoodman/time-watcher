#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <I2CKeyPad.h>

// Keypad defines
#define KEYPAD_ADDRESS 0x20

// Keypad variables
char keymap[19] = "123A456B789C*0#DNF";
bool lockPress;

// LCD defines
#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// LCD variables
String lcdText;

// Hardwares
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
I2CKeyPad keypad(KEYPAD_ADDRESS);

// Functions
bool isSchdule(int period)
{
    return millis() % period == 0;
}

bool isDigitChar(char c) {
    return int(c) >= 48 && int(c) <= 57;
}

// Main
void setup()
{
    Serial.begin(115200);

    lcd.init();
    lcd.backlight();

    if (!keypad.begin())
    {
        lcd.print("     Error     ");
        lcd.print("No Keypad found");

        Serial.println("No Keypad found");

        while (1);
    }

    keypad.loadKeyMap(keymap);
}

void loop()
{
    if (keypad.isPressed() && !lockPress)
    {
        const char charKey = keypad.getChar();

        if (isDigitChar(charKey))
        {
            lockPress = true;
            
            if (lcdText.length() >= LCD_COLS)
            {
                lcdText = "";
            }

            lcdText += charKey;
            
            lcd.clear();
            lcd.print(lcdText);
        }
        
    }

    if (!keypad.isPressed() && lockPress)
    {
        lockPress = false;
    }    
}