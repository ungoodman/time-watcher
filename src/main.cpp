#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define COUNTDOWN_DIGITS_LENGTH 5
#define CLOCK_DIGIT_LENGTH 2
#define SERIAL_BAUD_RATE 115200

#define COUNTDOWN_LATCH_PIN 4
#define COUNTDOWN_DATA_PIN 5
#define CLOCK_LATCH_PIN 9
#define CLOCK_DATA_PIN 6
#define CLOCK_PIN 3

#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define CE_PIN 8
#define CSN_PIN 7

#define DIGIT_ZERO B11111100
#define DIGIT_ONE B01100000
#define DIGIT_TWO B11011010
#define DIGIT_THREE B11110010
#define DIGIT_FOUR B01100110
#define DIGIT_FIVE B10110110
#define DIGIT_SIX B10111110
#define DIGIT_SEVEN B11100000
#define DIGIT_EIGHT B11111110
#define DIGIT_NINE B11110110

// digit 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
byte ledDigitBytes[] = {
    DIGIT_ZERO,
    DIGIT_ONE,
    DIGIT_TWO,
    DIGIT_THREE,
    DIGIT_FOUR,
    DIGIT_FIVE,
    DIGIT_SIX,
    DIGIT_SEVEN,
    DIGIT_EIGHT,
    DIGIT_NINE};

byte error = B10011110;

RF24 radio(CE_PIN, CSN_PIN);

uint32_t lastTime;
uint32_t lastTime2;
uint32_t radioListenTime;
int initCountDown[5] = {0, 0, 0, 0, 0};
int timeCountDown[5] = {0, 0, 0, 0, 0};
int timeClock[4] = {0, 0, 0, 0};
bool flagCountDown;
bool flagDisplayUpdate;

void writeCountdownSegment(byte value)
{
    digitalWrite(COUNTDOWN_LATCH_PIN, LOW);
    shiftOut(COUNTDOWN_DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(COUNTDOWN_LATCH_PIN, HIGH);
}

void writeClockSegment(byte value)
{
    digitalWrite(CLOCK_LATCH_PIN, LOW);
    shiftOut(CLOCK_DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(CLOCK_LATCH_PIN, HIGH);
}

void setup()
{
    pinMode(CLOCK_DATA_PIN, OUTPUT);
    pinMode(CLOCK_LATCH_PIN, OUTPUT);
    pinMode(COUNTDOWN_DATA_PIN, OUTPUT);
    pinMode(COUNTDOWN_LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(SERIAL_BAUD_RATE);

    for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
        writeCountdownSegment(ledDigitBytes[i]);

    for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
            writeClockSegment(ledDigitBytes[5]);
    delay(2000);

    Serial.println("program setup: done");
    Serial.println("program start");
}

void loop()
{
    if (millis() - lastTime >= 1000)
    {
        for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
            writeClockSegment(ledDigitBytes[5]);
        
        lastTime = millis();
    }
}
