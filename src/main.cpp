#include <LiquidCrystal_I2C.h> //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c
#include <I2CKeyPad.h>         //   เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <Keypad.h>
#include <Arduino_FreeRTOS.h>

I2CKeyPad keypad(0x20);                 //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2);     //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)
char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array
                                        // สร้างออบเจ็ค Keypad_I2C
String inputTime = "";                  //  ตัวแปร  ค่าล่าสุด
String latestValue = "";
bool lockKeypad; //  ตัวแปร  ล็อคปุ่มกด
int menu;
bool pass;
bool flagMenuChange;
bool flagCommit;
bool flagSendCmd;

void setup()
{ // เริ่มต้นการทำงานของ I2C
    Wire.begin();
    Wire.setClock(400000); // เป็นคำสั่งตั้งค่าความเร็วในการสื่อสาร (400000 fast mode )
    lcd.init();            // เริ่มต้นการทำงานของ LCD
    lcd.backlight();
    Serial.begin(115200);
    if (!keypad.begin()) //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        lcd.println("keypadError");
        while (1)
            ; //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }
    keypad.loadKeyMap(keymap); //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา

    flagMenuChange = true;
}

void selectMenu(char buttonValue)
{
    if (buttonValue < 'A' || buttonValue > 'C')
    {
        return;
    }

    flagMenuChange = true;
    inputTime = "";
    latestValue = "";
    flagCommit = false;

    switch (buttonValue)
    {
    case 'A':
    {
        menu = 1;
        return;
    }
    case 'B':
    {
        menu = 2;
        return;
    }
    case 'C':
    {
        menu = 3;
        pass = !pass;
        return;
    }
    case 'D':
    {
        menu = 4;
        return;
    }
    default:
        break;
    }
}

void checkNumberValue(char buttonValue)
{
    if (buttonValue < '0' || buttonValue > '9' || menu < 1 || menu > 2)
        return;

    if (inputTime.length() >= 6)
    {
        inputTime = "";
    }

    inputTime += buttonValue;
}

void checkConfirm(char buttonValue)
{
    if (buttonValue != '#' && buttonValue != '*')
        return;

    if (buttonValue == '#')
    {
        flagMenuChange = true;

        if (flagCommit && inputTime.length() > 0)
        {
            flagCommit = false;
            // send cmd
            return;
        }

        flagCommit = true;
        return;
    }

    if (buttonValue == '*')
    {
        if (inputTime.length() > 0)
        {
            flagCommit = false;
            return;
        }
        
        inputTime = "";
    }
}

void showMenu()
{
    if (latestValue == inputTime && !flagMenuChange)
        return;

    if (flagMenuChange)
        flagMenuChange = false;
    
    lcd.clear();
    lcd.setCursor(0, 0);

    switch (menu)
    {
    case 0:
        lcd.print("  CLOCK REMOTE  ");
        lcd.setCursor(0, 1);
        lcd.print("PRESS MENU A - D");
        return;
    case 1:
        if (flagCommit)
        {
            lcd.print(" PRESS # TO RUN ");
            lcd.setCursor(0, 1);
            lcd.print("     " + inputTime + "      ");
            Serial.println("LCD Display: " + inputTime);
            latestValue = inputTime;
            return;
        }
        
        lcd.print("TIMER    " + inputTime);
        Serial.println("LCD Display: " + inputTime);
        latestValue = inputTime;
        return;
    case 2:
        lcd.print("CLOCK    " + inputTime);
        Serial.println("LCD Display: " + inputTime);
        latestValue = inputTime;
        return;
    case 3:
        if (pass)
        {
            lcd.print("   STOP TIMER   ");
            return;
        }

        lcd.print("   RUN  TIMER   ");
        return;
    default:
        break;
    }
}

void loop()
{
    if (millis() % 250 == 0)
    {
        bool pressed = keypad.isPressed();

        if (pressed && !lockKeypad)
        {
            char keypadValue = keypad.getChar();

            lockKeypad = true;
            Serial.println("Keypad Lock: " + String(lockKeypad));

            checkNumberValue(keypadValue);
            checkConfirm(keypadValue);
            selectMenu(keypadValue);
        }

        if (!pressed && lockKeypad)
        {
            lockKeypad = false;
            Serial.println("Keypad Lock: " + String(lockKeypad));
        }
    }

    if (millis() % 500 == 0)
    {
        showMenu();
    }
}
