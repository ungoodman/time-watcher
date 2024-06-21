#include <Arduino.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define DATA_A_PIN 4
#define DATA_B_PIN 5
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
    pinMode(DATA_A_PIN, OUTPUT);
    pinMode(DATA_B_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    Serial.begin(115200);

    Serial.println("Setup: done");

    Serial.println("Clock Start");
}

uint32_t lastTime = 0;

int i;
void loop()
{
    if (millis() - lastTime >= 200)
    {
        shiftOut(DATA_A_PIN, CLOCK_PIN, LSBFIRST, B01100000);
        shiftOut(DATA_A_PIN, CLOCK_PIN, LSBFIRST, B01100000);

        shiftOut(DATA_B_PIN, CLOCK_PIN, LSBFIRST, B11011010);
        shiftOut(DATA_B_PIN, CLOCK_PIN, LSBFIRST, B11011010);

        lastTime = millis();
    }
}
