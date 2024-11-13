#include <Arduino.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <I2CKeyPad.h>
#include <SPI.h>
#include <RF24.h>

#define MAX_DIGITS_INPUT 5
#define COLUMS 20             // LCD columns
#define ROWS 4                // LCD rows
#define LCD_SPACE_SYMBOL 0x20 // space symbol from LCD ROM, see p.9 of GDM2004D datasheet
#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define RUN_BUTTON 32
#define PAUSE_BUTTON 33

char keymap[19] = "123A456B789C*0#DNF"; //  เป็นคำสั่งใช้ตัวแปร char โดยชื่อ keymap เป็นตัวเก็บจำนวนไว้ที่ตัวแปร ของ array

String savedTime = "";
String inputTime = ""; //  ตัวแปร  ค่าล่าสุด
String latestValue = "";
bool lockKeypad; //  ตัวแปร  ล็อคปุ่มกด
int menu;
bool pass;
bool flagMenuChange;
bool flagCommit;
bool flagSendCmd;
int32_t menuMillis = 0;
int32_t keypadMillis = 0;
int32_t radioMillis = 0;

RF24 radio(4, 5);
I2CKeyPad keyPad(0x20);
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

void selectMenu(char buttonValue)
{
    if (buttonValue < 'A' || buttonValue > 'D' || buttonValue == '*')
    {
        return;
    }

    flagMenuChange = true;
    inputTime = "";
    latestValue = "";

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
    // SET
    case 'C':
    {
        flagSendCmd = true;
        return;
    }
    // SET ZERO
    case 'D':
    {
        menu = 4;
        flagSendCmd = true;
        return;
    }
    // RUN
    case 'R':
    {
        menu = 3;
        pass = true;
        flagSendCmd = true;
        return;
    }
    // PAUSE
    case 'P':
    {
        menu = 3;
        pass = false;
        flagSendCmd = true;
        return;
    }
    // RESET
    case '#':
    {
        menu = 5;
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

    if (inputTime.length() >= MAX_DIGITS_INPUT)
    {
        inputTime = "";
    }

    inputTime += buttonValue;
}

void checkConfirm(char buttonValue)
{
    if (buttonValue != '#')
        return;

    flagMenuChange = true;

    if (inputTime.length() >= MAX_DIGITS_INPUT)
    {
        flagSendCmd = true;
    }
}

void showHomeMenu()
{
    lcd.print("  CLOCK REMOTE  ");
    lcd.setCursor(0, 1);
    lcd.print("PRESS MENU A | B");
}

void showTimerMenu()
{
    lcd.print("   SET TIMER   ");
    lcd.setCursor(0, 1);
    lcd.print("     " + inputTime + "      ");
    latestValue = inputTime;
}

void showClockMenu()
{
    lcd.print("   SET CLOCK   ");
    lcd.setCursor(0, 1);
    lcd.print("     " + inputTime + "      ");
    latestValue = inputTime;
}

void showRunMenu()
{
    lcd.print("   TIMER RUN   ");
}

void showPauseMenu()
{
    lcd.print("  TIMER PAUSE  ");
}

void showResetMenu()
{
    lcd.print("  TIMER RESET  ");
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
        showRunMenu();
        return;
    case 4:
        showPauseMenu();
        return;
    case 5:
        showResetMenu();
        return;
    default:
        break;
    }
}

void readKeypad()
{
    bool pressed = keyPad.isPressed();

    if (pressed == true && lockKeypad == false)
    {
        char keypadValue = keyPad.getChar();

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

    if (menu > 5)
    {
        Serial.println("Send Radio: Error");
        Serial.println("Invalid Menu Type: " + String(menu));
        flagSendCmd = false;
        return;
    }

    int stringLength = 9;

    String dataToSend = String(menu) + "#";
    if (menu == 1)
    {
        savedTime = inputTime;
        dataToSend += inputTime;
        Serial.println("InputTime: " + inputTime);
        Serial.println("SavedTime: " + savedTime);
    }
    else if (menu == 2)
        dataToSend += inputTime;
    else if (menu == 3)
        dataToSend += "0000" + String(pass);
    else if (menu == 4)
        dataToSend += "00000";
    else if (menu == 5)
    {
        dataToSend = "1#" +  savedTime;
        Serial.println("SavedTime: " + savedTime);
    }

    char byteToSend[stringLength];
    dataToSend.toCharArray(byteToSend, stringLength);

    radio.write(byteToSend, stringLength);
    radio.flush_tx();

    Serial.println("Send Radio: " + dataToSend);

    flagSendCmd = false;
}

void radioSetup()
{
    if (!radio.begin())
    {
        Serial.println("\nERROR: cannot communicate to radio.\nPlease reboot.\n");
        while (1)
            ;
    }
    radio.openWritingPipe(PIPE_ADDRESS);

    Serial.println("Radio: OK.");

    lcd.print(F("Radio is OK...")); //(F()) saves string to flash & keeps dynamic memory free
    delay(2000);
    lcd.clear();
}

void keypadSetup()
{
    if (!keyPad.begin())
    {
        Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
        while (1)
            ;
    }
    keyPad.loadKeyMap(keymap);

    Serial.println("Keypad: OK.");

    lcd.print(F("Keypad is OK...")); //(F()) saves string to flash & keeps dynamic memory free
    delay(2000);
    lcd.clear();
}

void lcdSetup()
{
    while (lcd.begin(COLUMS, ROWS, LCD_5x8DOTS) != 1) // colums, rows, characters size
    {
        Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
        delay(5000);
    }

    Serial.println("LCD: OK.");
    lcd.print(F("LCD is OK...")); //(F()) saves string to flash & keeps dynamic memory free
    delay(2000);
    lcd.clear();
}

void readRunButton()
{
    selectMenu('R');
}

void readPauseButton()
{
    selectMenu('P');
}

void setup()
{
    pinMode(RUN_BUTTON, INPUT);
    pinMode(PAUSE_BUTTON, INPUT);

    attachInterrupt(digitalPinToInterrupt(RUN_BUTTON), readRunButton, RISING);
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON), readPauseButton, RISING);

    Wire.begin();
    Wire.setClock(400000);
    Wire.setTimeOut(1000);

    Serial.begin(115200);

    lcdSetup();
    radioSetup();
    keypadSetup();

    Serial.println("Setup: done");
    lcd.print(F("Setup: done"));
    delay(2000);

    Serial.println("Remote Start");
    flagMenuChange = true;
}

void loop()
{
    if (millis() - menuMillis >= 200)
    {
        showMenu();
        menuMillis = millis();
    }

    if (millis() - keypadMillis >= 250)
    {
        readKeypad();
        keypadMillis = millis();
    }

    if (millis() - radioMillis >= 300)
    {
        sendRadio();
        radioMillis = millis();
    }
}
