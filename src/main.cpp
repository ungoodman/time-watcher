#include <Arduino.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define LATCH_PIN 4
#define DATA_PIN 5
#define CLOCK_PIN 3

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

void setup()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(115200);

    Serial.println("Setup: done");

    Serial.println("Clock Start");
}

uint32_t lastTime = 0;

void writeSegmentDigit(byte value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

int x;

void loop()
{
    if (millis() - lastTime >= 1000)
    {
        for (int i = 0; i < 5; i++)
        {
            writeSegmentDigit(ledDigitBytes[x]);
        }
        
        x++;

        if (x > 9)
        {
            x = 0;
        }
        
        lastTime = millis();
    }
}
