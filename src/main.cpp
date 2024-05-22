#include <Wire.h>                              //   เป็นคำสั่งเรียกใช้ libary wire
#include <LiquidCrystal_I2C.h>                 //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c 
#include <I2CKeyPad.h>                         //   เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <Keypad.h>

I2CKeyPad keypad(0x20);                         //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2);             //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

char keymap[19] = "123A456B789C*0#DNF";         //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

                                                // สร้างออบเจ็ค Keypad_I2C
String inputTime = "";                           //  ตัวแปร  ค่าล่าสุด
String latestValue = "";
bool lockKeypad;                                 //  ตัวแปร  ล็อคปุ่มกด

void setup()
{                                              // เริ่มต้นการทำงานของ I2C

    Wire.begin();
    Wire.setClock(400000);                     // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )

    lcd.init();                                // เริ่มต้นการทำงานของ LCD
    lcd.backlight();

    Serial.begin(115200);

    if (!keypad.begin())                 //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        lcd.println("keypadError");
        while (1);                                    //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }

    keypad.loadKeyMap(keymap);                   //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา
}

void loop()
{
    if (millis() % 100 == 0)
    {
        if (keypad.isPressed() && !lockKeypad)              
        {
            char keypadValue = keypad.getChar();

            String inputFromKeypad = "Input from Keypad: ";
            Serial.println(inputFromKeypad.concat(keypadValue));

            lockKeypad = true;
            Serial.println("Keypad Lock: " + String(lockKeypad));
            
            if (keypadValue == '#')
            {
            }
            else if (keypadValue == '*')
            {
                inputTime = "";
            }
            else if (keypadValue >= '0' && keypadValue <= '9')
            {
                if (inputTime.length() >= 6)
                {
                    inputTime = "";
                }

                inputTime += keypadValue;
            }
            else
            {
                Serial.println("Menu: " + keypadValue);
            }
        }
    
        if (!keypad.isPressed() && lockKeypad)
        {
            lockKeypad = false;
            Serial.println("Keypad Lock: " + String(lockKeypad));
        }
    }
    


    if (millis() % 250 == 0 && latestValue != inputTime)
    {
        lcd.clear();

        lcd.home();
        lcd.print("   Set  Time   ");

        lcd.setCursor(0, 1);
        lcd.print(inputTime);

        Serial.println("LCD Display: " + inputTime);

        latestValue = inputTime;
    }
}
