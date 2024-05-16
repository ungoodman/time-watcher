#include <Arduino.h>
#include <Wire.h>              // เป็นคำสั่งเรียกใช้ libary wire
#include <I2CKeyPad.h>         //  เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <LiquidCrystal_I2C.h> //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c

I2CKeyPad keypad(0x20);             //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2); //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

void setup()
{
    Wire.begin();          //  เป็นการเริ่มใช้งาน Libary wire
    Wire.setClock(400000); //  เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )
    lcd.init();            //  เป็นการเริ่มต้นของ lcd
    lcd.backlight();       //  เป็นคำสั่งให้ backlight ของ lcd ติด
    Serial.begin(115200);  //  การแสดงผลใน Serial mointor โดยมีความเร็วในการส่ง-รับ เป็น 115200 bit ต่อ วินาที

    if (keypad.begin() == false) //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        lcd.println("\nkeypadError");
        while (1)
            ; //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }
    keypad.loadKeyMap(keymap); //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา
}

char getkeypadPressed()
{
    if (keypad.isPressed())
    {
        char ch = keypad.getChar();
        return ch;
    }
    else
    {
        return ' ';
    }
}

bool flaggetkey = false;

void loop()
{
    if (millis() % 100 == 0)
    {
        char re = getkeypadPressed();

        if (flaggetkey == false && re != ' ')
        {

            flaggetkey = true;
            lcd.clear();
            lcd.print(re);
            Serial.println(re);
        }
        if (flaggetkey == true && !keypad.isPressed())
        {
            flaggetkey = false;
        }
    }
}

