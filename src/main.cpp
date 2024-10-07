#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define COUNTDOWN_DIGITS_LENGTH 5
#define CLOCK_DIGIT_LENGTH 4
#define SERIAL_BAUD_RATE 115200
#define SHIFTOUT_FREQ 50000
#define BIT_ORDER LSBFIRST

#define COUNTDOWN_LATCH_PIN 32
#define COUNTDOWN_DATA_PIN 25
#define COUNTDOWN_CLOCK_PIN 14
#define CLOCK_LATCH_PIN 26
#define CLOCK_DATA_PIN 27
#define CLOCK_PIN 33

#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 13

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

RF24 radio(CE_PIN, CSN_PIN);

uint32_t lastTime;
uint32_t lastClockTime;
int initCountDown[COUNTDOWN_DIGITS_LENGTH];
int timeCountDown[COUNTDOWN_DIGITS_LENGTH];
int timeClock[CLOCK_DIGIT_LENGTH];
bool flagCountDown;
bool flagRadioAvailable;
bool flagCountdownReset;
bool flagClockReset;

void shiftoutFreq(int dataPin, int clockPin, byte val)
{
    // Calculate the delay based on the desired frequency
    unsigned long clockDelay = 500000 / SHIFTOUT_FREQ; // 500,000 = 1 second in microseconds divided by 2 (for the half period)

    for (int i = 0; i < 8; i++)
    {
        // Check if bitOrder is LSBFIRST or MSBFIRST
        if (BIT_ORDER == LSBFIRST)
        {
            digitalWrite(dataPin, !!(val & (1 << i))); // Send least significant bit first
        }
        else
        {
            digitalWrite(dataPin, !!(val & (1 << (7 - i)))); // Send most significant bit first
        }

        // Toggle the clock pin
        digitalWrite(clockPin, HIGH);  // Set clock pin HIGH
        delayMicroseconds(clockDelay); // Wait for the high duration of the clock

        digitalWrite(dataPin, LOW);   // Set data pin LOW
        digitalWrite(clockPin, LOW);   // Set clock pin LOW
        delayMicroseconds(clockDelay); // Wait for the low duration of the clock
    }
}

void multiShiftout(int dataPin, int clockPin, int latchPin, int registerSize, int vals[])
{
    digitalWrite(latchPin, LOW);

    for (int i = registerSize - 1; i >= 0; i--){
        Serial.println(vals[i]);
        byte val = ledDigitBytes[vals[i]];
        shiftoutFreq(dataPin, clockPin, val);
    }

    digitalWrite(latchPin, HIGH);
}

void countdownPrint(int data[])
{
    multiShiftout(COUNTDOWN_DATA_PIN, CLOCK_PIN, COUNTDOWN_LATCH_PIN, COUNTDOWN_DIGITS_LENGTH, data);
}

void clockPrint(int data[])
{
    multiShiftout(CLOCK_DATA_PIN, CLOCK_PIN, CLOCK_LATCH_PIN, CLOCK_DIGIT_LENGTH, data);
}

void radioSetup()
{
    if (!radio.begin())
    {
        int radioErrors[COUNTDOWN_DIGITS_LENGTH];
        memset(radioErrors, 9, COUNTDOWN_DIGITS_LENGTH);
        countdownPrint(radioErrors);
        clockPrint(radioErrors);

        Serial.println(F("radio hardware is not responding!"));
        while (true)
            ;
    }

    radio.openReadingPipe(1, PIPE_ADDRESS);
    radio.startListening();

    Serial.println(F("radio setup: done"));
}

void isr_function()
{
    flagRadioAvailable = true;
}

void countdownTask()
{
    if (flagCountdownReset)
    {
        int zeros[COUNTDOWN_DIGITS_LENGTH];
        memset(initCountDown, 0, COUNTDOWN_DIGITS_LENGTH);
        memset(timeCountDown, 0, COUNTDOWN_DIGITS_LENGTH);
        countdownPrint(initCountDown);

        flagCountdownReset = false;
        return;
    }

    if (!flagCountDown)
        return;

    if (timeCountDown[4] < 0)
    {
        timeCountDown[4] = 9;
        timeCountDown[3]--;
    }

    if (timeCountDown[3] < 0)
    {
        timeCountDown[3] = 5;
        timeCountDown[2]--;
    }

    if (timeCountDown[2] < 0)
    {
        timeCountDown[1] = 9;
        timeCountDown[0]--;
    }

    if (timeCountDown[0] < 0)
    {
        timeCountDown[0] = 0;
    }

    if (timeCountDown[0] == 0 && timeCountDown[1] == 0 && timeCountDown[2] == 0 && timeCountDown[3] == 0 && timeCountDown[4] == 0)
    {
        flagCountDown = false;
        flagCountdownReset = true;
    }

    Serial.println("Countdown: " + String(timeCountDown[0]) + " " + String(timeCountDown[1]) + " " + String(timeCountDown[2]) + " " + String(timeCountDown[3]) + " " + String(timeCountDown[4]) + " ");

    countdownPrint(timeCountDown);

    timeCountDown[4]--;
}

void clockTask()
{
    if (flagClockReset)
    {
        int zeros[CLOCK_DIGIT_LENGTH];
        clockPrint(zeros);

        flagClockReset = false;
        return;
    }

    if (timeClock[3] > 9)
    {
        timeClock[3] = 0;
        timeClock[2]++;
    }

    if (timeClock[2] > 5)
    {
        timeClock[2] = 0;
        timeClock[1]++;
    }

    if (timeClock[1] > 9)
    {
        timeClock[1] = 0;
        timeClock[0]++;
    }

    if (timeClock[0] == 2 && timeClock[1] == 4 && timeClock[2] == 0 && timeClock[3] == 0)
    {
        flagClockReset = true;
    }

    Serial.println("Clock: " + String(timeClock[0]) + " " + String(timeClock[1]) + " " + String(timeClock[2]) + " " + String(timeClock[3]));

    clockPrint(timeClock);

    timeClock[3]++;
}

int extractMenu(String messageStr)
{
    // 1#00400
    return messageStr.substring(0, 1).toInt();
}

String extractData(String messageStr)
{
    // 1#00400
    return messageStr.substring(2, 7);
}

void selectMenu(int menu, String dataStr)
{
    switch (menu)
    {
    case 1: // Countdown Menu
    {
        for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
        {
            timeCountDown[i] = dataStr[i] - '0';
            initCountDown[i] = dataStr[i] - '0';
        }

        countdownPrint(timeCountDown);

        Serial.println("Countdown Set: " + dataStr);
        break;
    }
    case 2: // Clock Menu
    {
        for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
        {
            timeClock[i] = dataStr[i] - '0';
        }

        Serial.println("Clock Time Set: " + dataStr);
        break;
    }
    case 3: // Countdown Pause Menu
    {
        flagCountDown = !flagCountDown;

        Serial.println("Countdown Status: " + flagCountDown ? "Run" : "Pause");
        break;
    }
    case 4: // Countdown Reset Menu
    {
        memcpy(initCountDown, timeCountDown, sizeof(timeCountDown));

        flagCountDown = false;
        Serial.println("Countdown Reset!");
        break;
    }
    default:
        Serial.println("Invalid Menu!");
        break;
    }
}

void listenRadio()
{
    if (flagRadioAvailable)
    {
        Serial.println("Reading Radio");

        char message[10];
        while (radio.available())
            radio.read(message, 10);

        String cmd = String(message);
        Serial.println("Radio Receive: " + String(message));

        int menu = extractMenu(cmd);
        String dataStr = extractData(cmd);

        Serial.println("Menu: " + String(menu) + " Data: " + dataStr);

        selectMenu(menu, dataStr);

        flagRadioAvailable = false;
    }
}

void setup()
{
    pinMode(CLOCK_DATA_PIN, OUTPUT);
    pinMode(CLOCK_LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(COUNTDOWN_DATA_PIN, OUTPUT);
    pinMode(COUNTDOWN_LATCH_PIN, OUTPUT);
    pinMode(COUNTDOWN_CLOCK_PIN, OUTPUT);
    pinMode(IRQ_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(IRQ_PIN), isr_function, FALLING);

    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();

    radioSetup();

    // Reset LEDs start
    int zeroByte[COUNTDOWN_DIGITS_LENGTH];
    clockPrint(zeroByte);
    countdownPrint(zeroByte);
    // Reset LEDs end

    Serial.println("program setup: done");
    Serial.println("program start");

    delay(1000);
}

int count;

void loop()
{
    listenRadio();

    if (millis() - lastTime >= 1000)
    {
        countdownTask();
        lastTime = millis();
    }

    if (millis() - lastClockTime >= 1000)
    {
        clockTask();
        lastClockTime = millis();
    }
}