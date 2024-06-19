#include <Arduino.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>

#define DATA_PIN 3
#define CLOCK_PIN 2
#define PIPE_ADDRESS 0xE8E8F0F0E1LL

// digit 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
byte ledDigitBytes[] = {
    B11111100,
    B01100000,
    B11011010,
    B11110010,
    B01100110,
    B10110110,
    B10111110,
    B11100000,
    B11111110,
    B11110110};

// String inputTime = ""; //  ตัวแปร  ค่าล่าสุด
// String latestValue = "";

int menu;

// bool flagLedUpdate;
// bool flagClockUpdate;
bool flagTimerUpdate;
bool flagTimerPause;
bool flagReceiveCmd;

int timerTime[3];
int clockTime[6];
String radioData;

RF24 radio(8, 7);

void setTime(String data)
{
    int hour = data.substring(0, 2).toInt();
    int minute = data.substring(3, 5).toInt();
    int second = data.substring(6, 8).toInt();

    timerTime[0] = hour;
    timerTime[1] = minute;
    timerTime[2] = second;
}

void executeCmd()
{
    if (menu == 1)
    {
        setTime(radioData);
        return;
    }
    else if (menu == 2)
    {
        return;
    }
    else if (menu == 3)
    {
        if (radioData == "000000")
            flagTimerPause = false;
        else
            flagTimerPause = true;
        return;
    }

    for (int i = 0; i < 3; i++)
        timerTime[i] = 0;

    flagTimerUpdate = true;
}

int lastFirstDigit;

void setLed(int timeValue)
{
    int firstDigit = timeValue / 10;
    int secondDigit = timeValue % 10;

    if (firstDigit != lastFirstDigit)
        shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ledDigitBytes[firstDigit]);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ledDigitBytes[secondDigit]);

    lastFirstDigit = secondDigit;
}

void ledControl()
{
    if (!flagTimerUpdate || flagTimerPause)
        return;

    int timerTimeSize = sizeof(timerTime) / sizeof(int);

    for (int i = 0; i < timerTimeSize; i++)
        setLed(timerTime[i]);
}

void timeTask()
{
    // if (clockTime[5] >= 60)
    // {
    //     clockTime[5] = 0;
    //     // clockTime[4]++;
    // }

    // if (clockTime[4] >= 60)
    // {
    //     clockTime[4] = 0;
    //     clockTime[3]++;
    // }

    // if (clockTime[3] >= 23)
    // {
    //     clockTime[3] = 0;
    //     // clockTime[2]++;
    // }

    // if (clockTime[2] >= 30)
    // {
    //     clockTime[2] = 0;
    //     clockTime[1]++;
    // }

    // if (clockTime[1] >= 12)
    // {
    //     clockTime[1] = 0;
    //     clockTime[0]++;
    // }
    // clockTime[5] += 1;

    // for (int i = 0; i < 6; i++)
    //     clockControl(timerTime[i]);

    // Serial.println(clockTime[5]);
}

bool isAllZero(int arr[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] != 0)
            return false;
    }
    return true;
}

void timerTask()
{
    if (flagTimerPause)
        return;

    int timerTimeSize = sizeof(timerTime);

    Serial.println("Size: " + String(timerTimeSize));

    if (isAllZero(timerTime, timerTimeSize))
        return;

    for (int i = timerTimeSize; i > 0; i--)
    {
        if (timerTime[i] == 0)
        {
            timerTime[i] = 9;
            timerTime[i]--;
        }
    }

    timerTime[timerTimeSize]--;
}

void radioSetup()
{
    if (!radio.begin())
    {
        Serial.println("Radio Begin Failed");
        while (true)
            ;
    }

    radio.openReadingPipe(1, PIPE_ADDRESS);
    radio.startListening();

    Serial.println("Radio Setup: done");
}

void setup()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(115200);
    EEPROM.begin();

    radioSetup();

    for (int i = 0; i < 3; i++)
        setLed(timerTime[i]);
    Serial.println("Setup: done");
    delay(2000);

    Serial.println("Clock Start");
}

void listenRadio()
{
    if (!radio.available())
        return;

    char getFromRead[10];
    while (radio.available())
        radio.read(getFromRead, 10);

    String cmd = String(getFromRead);
    menu = cmd.substring(0, 1).toInt();
    radioData = cmd.substring(2, 11);
    Serial.println("Radio Receive: " + String(getFromRead));
}

uint32_t lastTime = 0;
uint32_t radioTime;

void loop()
{
    if (millis() - radioTime >= 300)
    {
        listenRadio();

        radioTime = millis();
    }

    if (millis() - lastTime >= 1000)
    {
        // timeTask();
        timerTask();
        ledControl();

        lastTime = millis();
    }
}
