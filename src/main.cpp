#include <Wire.h>              //   เป็นคำสั่งเรียกใช้ libary wire
#include <LiquidCrystal_I2C.h> //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c
#include <I2CKeyPad.h>         //   เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>

// สร้างออบเจ็ค Keypad_I2C
String inputTime = ""; //  ตัวแปร  ค่าล่าสุด
String latestValue = "";
int menu;

void keypadTask(void *pvParameters);
void lcdTask(void *pvParameters);

void setup()
{
    // เริ่มต้นการทำงานของ I2C
    // Wire.begin();
    // Wire.setClock(400000); // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )

    // Serial.begin(115200);

    xTaskCreate(keypadTask, "keypad Task", 128, NULL, 1, NULL);
    // xTaskCreate(lcdTask, " lcdTask", 128, NULL, 1, NULL);
}

void keypadTask(void *pvParameters)
{
    I2CKeyPad keypad(0x20);             //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20

    char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

    Serial.begin(115200);

    // เริ่มต้นการทำงานของ I2C
    Wire.begin();
    Wire.setClock(400000); // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )

    if (!keypad.begin()) //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        Serial.println("Error keypad");
        while (1); //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }

    keypad.loadKeyMap(keymap); //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา

    bool lockKeypad; //  ตัวแปร  ล็อคปุ่มกด

    while (1)
    {
        if (keypad.isPressed() && !lockKeypad)
        {
            char keypadValue = keypad.getChar();

            lockKeypad = true;

            if (keypadValue == '#')
            {
            }
            else if (keypadValue == '*')
            {
                inputTime = "";
            }
            else if (keypadValue == 'A')
            {
                menu = 1;
            }
            else if (keypadValue == 'B')
            {
                menu = 2;
            }
            else if (keypadValue >= '0' && keypadValue <= '9')
            {
                if (inputTime.length() >= 6)
                {
                    inputTime = "";
                }

                inputTime += keypadValue;
            }

            Serial.println("Input Time: " + inputTime);
        }

        vTaskDelay(20);
    }
}

void lcdTask(void *pvParameters)
{
    LiquidCrystal_I2C lcd(0x27, 16, 2); //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

    Serial.begin(115200);

    // เริ่มต้นการทำงานของ I2C
    Wire.begin();
    Wire.setClock(400000); // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )

    lcd.init(); // เริ่มต้นการทำงานของ LCD
    lcd.backlight();

    while (1)
    {
        if (latestValue != inputTime)
        {
            lcd.clear();

            lcd.home();
            lcd.print("   Set  Time   ");

            lcd.setCursor(0, 1);
            lcd.print(inputTime);
            if (menu == 1)
            {
                lcd.print("Timer ");
            }
            if (menu == 2)
            {
                lcd.print("real Time");
            }
            Serial.println("LCD Display: " + inputTime);

            latestValue = inputTime;
        }

        vTaskDelay(20);
    }
}

void loop()
{
}