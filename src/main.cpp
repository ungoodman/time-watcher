#include <Arduino.h>
#include <Wire.h>      // เป็นคำสั่งเรียกใช้ libary wire
#include <I2CKeyPad.h> //  เป็นคำสั่งเรียกใช้ libary keypad i2c

I2CKeyPad keypad(0x20); //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20

char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

void setup()
{
    Wire.begin();          //  เป็นการเริ่มใช้งาน Libary wire
    Wire.setClock(400000); //  เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )
    Serial.begin(115200);  //  การแสดงผลใน Serial mointor โดยมีความเร็วในการส่ง-รับ เป็น 115200 bit ต่อ วินาที

    if (keypad.begin() == false) //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        Serial.println("\nkeypadError");
        while (1)
            ; //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }
    keypad.loadKeyMap(keymap); //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา
}

char  getkeypadPressed() {
    if (keypad.isPressed()) {
        char ch  = keypad.getChar();
        return ch;
    } else { 
        return ' ';
    }
}

String lastestValue;                                          //  ตัวแปร  ค่าล่าสุด
bool lockKeypad;                                              //   ตัวแปร  ล็อคปุ่มกด  

void loop()
{
    if (keypad.isPressed() && !lockKeypad)                    // ถ้า กดปุม่ keypad และ ไม่ล็อคปุ่มกด     
    {
        char keypadValue = keypad.getChar();                  // เก็บรับค่าตัวเลขจาก keypad  
        Serial.println("Input from Keypad: " + keypadValue);  // แสดงผลลัพ  "Input from Keypad:

        lockKeypad = true;                                    // ล็อคปุ่มกด เท่ากับ กด
        Serial.println("Keypad Lock: " + String(lockKeypad)); 
        
        if (keypadValue == '#')                               // ถ้ารับค่า ตัวเลขจาก keypad  = #
        {
             ('#' == true)


        }
        else if (keypadValue == '*')                          //  แล้วถ้ารับค่า ตัวเลขจาก keypad  = *
        {
            ('*' == true)
            lcd.clear();
            lcd.print("Time ");
        }
        else if (keypadValue >= '0' && keypadValue <= '9')    //  แล้วถ้าค่าที่รับจากตัวเลขจาก keypad มากกว่าหรือเท่ากับ 0 และ ค่าที่รับจากตัวเลขจาก keypad น้อยกว่า 9 
        {
            lastestValue = inputTime;                         //  ค่าล่าสุด เท่ากับ  รับข้อมูลค่าเวลา 
            inputTime.concat(keypadValue);                    //  รับข้อมูลค่าเวลาที่เชื่อมต่อใน ตัวเลขจาก keypad
        }
        else
        {
            Serial.println("Menu: " + keypadValue);           //  แสดงผลลัพธิ์ "Menu: " กับ ค่าตัวเลขจาก keypad
        }
    }

    if (!keypad.isPressed() && lockKeypad)                    //  ถ้า ไม่ได้กดปุ่ม และ ล็อคปุ่มกด
    {
        lockKeypad = false;                                   //  ล็อคปุ่มกด เท่ากับ  ไม่ได้กด
        Serial.println("Keypad Lock: " + String(lockKeypad)); //  แสดงผลลัพธิ์  "Keypad Lock: " และ ค่าล็อคปุ่มกด 
    }

    if (millis() % 100 == 0 && inputTime != lastestValue)     //  ถ้าวินาที และ  ไม่ได้รับค่าเวลาล่าสุด
    {
        lcd.clear();

        lcd.home();
        lcd.print("Set Time "+(inputTime));                   //  lcd แสดงผลลัพธิ์   Set  Time  และ  รับข้อมูลค่าเวลา

        lcd.setCursor(0, 1);
       

        Serial.println("LCD Display: " + inputTime); 
    }
}

    // char re = getkeypadPressed();

    // if (re)
    // {
    //     if (re == '#')
    //     {
    //         lcd.clear();
    //         lcd.setCursor(0, 0);
    //         lcd.print("Time:");
    //         lcd.setCursor(0, 1);
    //         lcd.print(inputTime);
    //         inputTime = "";
    //     }
    //     else if (re == '*')
    //     {
    //         inputTime = "";
    //         lcd.clear();
    //         lcd.setCursor(0, 0);
    //         lcd.print("Enter Time:");
    //         lcd.setCursor(0, 1);
    //     }
    //     else
    //     {
    //         inputTime += re;
    //         lcd.setCursor(0, 1);
    //         lcd.print(inputTime);
    //     }
    // }
}
