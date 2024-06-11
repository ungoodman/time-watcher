#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //   เป็นคำสั่งเรียกใช้ Libary ของ lcd i2c
#include <I2CKeyPad.h>         //   เป็นคำสั่งเรียกใช้ libary keypad i2c
#include <Keypad.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(8, 7);
I2CKeyPad keypad(0x20);             //  เป็นคำสั่งเก็บค่า address ของ keypad address = 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2); //  เป็นการตั้งค่า ของจอ Lcd (0*27 คือขนาดของจอ,16 ตัวอักษร ,2 บรรทัด)

char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array
const uint64_t pipe = 0xE8E8F0F0E1LL;

int lcdRow = 0;
String inputTime = ""; //  ตัวแปร  ค่าล่าสุด
String latestValue = "";
bool lockKeypad; //  ตัวแปร  ล็อคปุ่มกด
int menu;
bool pass;
bool flagMenuChange;
bool flagCommit;
bool flagSendCmd;

// Functions
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
        flagSendCmd = true;
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
    if (buttonValue < '0' || buttonValue > '9' || menu < 1 || menu > 2 || flagCommit)
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

        if ((flagCommit && inputTime.length() >= 6) || menu == 4)
        {
            flagSendCmd = true;
            flagCommit = false;
            return;
        }

        if (inputTime.length() >= 6)
            flagCommit = true;
    }

    if (buttonValue == '*')
    {
        inputTime = "";
        flagCommit = false;
    }
}

void showHomeMenu()
{
    lcd.print("  CLOCK REMOTE  ");
    lcd.setCursor(0, 1);
    lcd.print("PRESS MENU A - D");
}

void showTimerMenu()
{
    if (flagCommit)
    {
        lcd.print(" PRESS # TO RUN ");
        lcd.setCursor(0, 1);
        lcd.print("     " + inputTime + "      ");
        latestValue = inputTime;
        return;
    }

    lcd.print("TIMER    " + inputTime);
    latestValue = inputTime;
}

void showClockMenu()
{
    if (flagCommit)
    {
        lcd.print(" PRESS # TO SET ");
        lcd.setCursor(0, 1);
        lcd.print("     " + inputTime + "      ");
        Serial.println("LCD Display: " + inputTime);
        latestValue = inputTime;
        return;
    }

    lcd.print("CLOCK    " + inputTime);
    Serial.println("Menu 2");
    latestValue = inputTime;
}

void showPauseMenu()
{
    if (pass)
    {
        lcd.print("   STOP TIMER   ");
        return;
    }

    lcd.print("   RUN  TIMER   ");
}

void showResetMenu()
{
    if (flagCommit)
    {
        lcd.print("   RESET DONE   ");
        menu = 0;
        delay(3000);
        latestValue = inputTime;
        return;
    }

    lcd.print("    PRESS  #    ");
    lcd.setCursor(0, 1);
    lcd.print(" TO RESET TIMER ");
    latestValue = inputTime;
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
        showHomeMenu();
        return;
    case 1:
        showTimerMenu();
        return;
    case 2:
        showClockMenu();
        return;
    case 3:
        showPauseMenu();
        return;
    case 4:
        showResetMenu();
        return;
    default:
        break;
    }
}

void readKeypad()
{
    bool pressed = keypad.isPressed();
    Serial.println("Keypad Pressed: " + String(pressed));

    if (pressed == true && lockKeypad == false)
    {
        char keypadValue = keypad.getChar();

        lockKeypad = true;
        Serial.println("Keypad Lock: " + String(lockKeypad));

        checkNumberValue(keypadValue);
        checkConfirm(keypadValue);
        selectMenu(keypadValue);
    }

    if (pressed == false && lockKeypad == true)
    {
        lockKeypad = false;
        Serial.println("Keypad Lock: " + String(lockKeypad));
        Serial.println("Keypad RELEASE!");
    }
}

void sendRadio()
{
    if (!flagSendCmd)
        return;

    if (menu > 4)
    {
        Serial.println("Send Radio: Error");
        Serial.println("Invalid Menu Type: " + String(menu));
        flagSendCmd = false;
        return;
    }

    int stringLength = 9;

    String dataToSend = String(menu) + "#";
    if (menu == 1 || menu == 2)
        dataToSend += inputTime;
    else if (menu == 3)
        dataToSend += "00000" + String(pass);
    else
        dataToSend += "000000";

    char byteToSend[stringLength];
    dataToSend.toCharArray(byteToSend, stringLength);

    radio.write(byteToSend, stringLength);
    radio.flush_tx();

    Serial.println("Send Radio: " + dataToSend);

    flagSendCmd = false;
}

// Run
void setup()
{
    lcd.init();
    lcd.backlight();

    Wire.setWireTimeout(1000, false);

    Serial.begin(115200);

    if (!radio.begin())
    {
        Serial.println("radio hardware is not responding!!!");
        while (1)
            ;
    }

    radio.openWritingPipe(pipe);

    if (!keypad.begin()) //  ถ้า (keypad.begin เป็นการตรวจสอบว่าสื่อสารกันได้) keypad เป็น เท็จ
    {
        lcd.println("keypadError");
        while (1)
            ; //  เป็นคำสั่งทำซํ้าตลอดไปไม่หยุด
    }
    keypad.loadKeyMap(keymap); //  เป็นการตั้งค่า layout ของ keypad เป็นการดึงค่าจาก keymap มา

    flagMenuChange = true;
}

void loop()
{
    if (millis() % 200 == 0)
        readKeypad();

    if (millis() % 250 == 0)
        sendRadio();

    if (millis() % 350 == 0)
        showMenu();
}
