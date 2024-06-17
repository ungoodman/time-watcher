#include <Arduino.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>

#define DATA_PIN 8
#define CLOCK_PIN 9
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

// int menu;

// bool flagLedUpdate;
// bool flagClockUpdate;
// bool flagTimerUpdate;
// bool flagTimerPause;
// bool flagReceiveCmd;

// int timerTime[3];
int clockTime[6];

RF24 radio(4, 5);

// void executeCmd()
// {
//     if (menu == 1 || menu == 2)
//     {
//         /* code */
//     }
//     else if (menu == 3)
//     {
//         /* code */
//     }

//     flagTimerUpdate = true;
// }

void clockControl()
{
    int secondStDigit = clockTime[5] / 10;
    int secondNdDigit = clockTime[5] % 10;

    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ledDigitBytes[secondStDigit]);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ledDigitBytes[secondNdDigit]);
}

void timeTask()
{
    if (clockTime[5] >= 59)
    {
        clockTime[5] = 0;
        clockTime[4]++;
    }

    if (clockTime[4] >= 59)
    {
        clockTime[4] = 0;
        // clockTime[3]++;
    }

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
}

// void radioSetup()
// {
//     if (!radio.begin())
//     {
//         Serial.println("Radio Begin Failed");
//         while (true)
//             ;
//     }

//     radio.openReadingPipe(1, PIPE_ADDRESS);
//     radio.startListening();

//     Serial.println("Radio Setup: done");
// }

void setup()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(115200);
    // EEPROM.begin();

    // radioSetup();

    Serial.println("Setup: done");
    delay(2000);

    Serial.println("Clock Start");
}

// void listenRadio()
// {
//     if (!radio.available())
//         return;

//     char getFromRead[10];
//     while (radio.available())
//         radio.read(getFromRead, 10);

//     String cmd = String(getFromRead);
//     menu = cmd.substring(0, 1).toInt();
//     inputTime = cmd.substring(2, 11);
//     Serial.println("Radio Receive: " + String(getFromRead));
// }

void loop()
{
    // if (millis() % 300 == 0)
    //     listenRadio();

    if (millis() % 100 == 0)
        clockControl();

    if (millis() % 1000 == 0)
        timeTask();
}
