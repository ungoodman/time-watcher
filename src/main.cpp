#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define SERIAL_BAUD_RATE 115200

#define LATCH_PIN 4
#define DATA_PIN 5
#define CLOCK_PIN 3

#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define CE_PIN 8
#define CSN_PIN 7

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

byte error = B10011110;

RF24 radio(CE_PIN, CSN_PIN);

uint32_t lastTime;
uint32_t radioListenTime;
int time[2] = {0, 0};
bool flagCountDown = true;
bool flagDisplayUpdate;

void radioSetup() {
    if (!radio.begin())
    {
        writeSegmentDigit(error);
        writeSegmentDigit(ledDigitBytes[0]);
        Serial.println(F("radio hardware is not responding!"));
        while (true);
    }

    radio.openReadingPipe(1, PIPE_ADDRESS);
    radio.startListening();
    
    Serial.println(F("radio setup: done"));
}

void setup()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(SERIAL_BAUD_RATE);

    radioSetup();

    Serial.println("program setup: done");
    Serial.println("program start");
}

void writeSegmentDigit(byte value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

void timeTask() {
    if (!flagCountDown)
        return;
    
    time[1]--;

    if (time[1] < 0)
    {
        time[1] = 59;
        time[0]--;
    }
    
    if (time[0] < 0)
    {
        time[0] = 0;
    }

    flagDisplayUpdate = true;
}

void showTime() {
    if (!flagDisplayUpdate)
        return;

    int toShow[4] = {time[0] / 10, time[0] % 10, time[1] / 10, time[1] % 10};
    
    for (int i = 0; i < 4; i++)
    {
        int index = toShow[i];
        writeSegmentDigit(ledDigitBytes[index]);
    }

    flagDisplayUpdate = false;
    
    if (time[0] == 0 && time[1] == 0 && flagCountDown)
        flagCountDown = false;    
}

void listenRadio() {
    if (!radio.available())
        return;
    
    char message[10];
    while (radio.available())
        radio.read(&message, 10);

    String messageStr = String(message);
    int menu = messageStr.substring(0, 1).toInt();

    if (menu == 1)
    {
        time[0] = messageStr.substring(3, 5).toInt();
        time[1] = messageStr.substring(5, 7).toInt();

        flagDisplayUpdate = true;
    } else if (menu == 3)
    {
        flagCountDown = messageStr.substring(6, 7).toInt();
    }
}

void loop()
{
    if (millis() - radioListenTime >= 500)
    {
        listenRadio();

        radioListenTime = millis();
    }
    

    if (millis() - lastTime >= 1000)
    {
        showTime();
        timeTask();
        
        lastTime = millis();
    }
}
