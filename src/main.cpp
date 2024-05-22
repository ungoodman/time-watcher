#include <Wire.h>                              //   เป็นคำสั่งเรียกใช้ libary wire
#include <LiquidCrystal_I2C.h>                 //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c 
#include <I2CKeyPad.h>                         //   เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <Keypad.h>

I2CKeyPad keypad(0x20);                         //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2);             //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

                                                // สร้างออบเจ็ค Keypad_I2C
int inputTime = 0;                           //  ตัวแปร  ค่าล่าสุด

void setup()
{                                              // เริ่มต้นการทำงานของ I2C
    Wire.begin();
    Wire.setClock(400000);                     // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )

    lcd.init();                                // เริ่มต้นการทำงานของ LCD
    lcd.backlight();

    keypad.begin();                            // เริ่มต้นการทำงานของ keypad
}

void loop()
{
    keypad.isPressed();
    
    if (millis() % 1000 == 0)
    {
        inputTime++;
    }

    if (millis() % 250 == 0)
    {
        lcd.clear();

        lcd.home();
        lcd.print("   Show Time   ");

        lcd.setCursor(0, 1);
        lcd.print(inputTime);
    }
}
