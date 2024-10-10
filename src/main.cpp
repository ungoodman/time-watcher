#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RtcDS1302.h>

#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

// Task Configuration
#define COUNTDOWN_INTERVAL 1000
#define CLOCK_INTERVAL 60000

// Serial Configuration
#define SERIAL_BAUD_RATE 115200

// Shift Register Configuration
#define COUNTDOWN_DIGITS_LENGTH 5
#define CLOCK_DIGIT_LENGTH 4
#define SHIFTOUT_FREQ 50000
#define BIT_ORDER LSBFIRST
#define COUNTDOWN_LATCH_PIN 32
#define COUNTDOWN_DATA_PIN 25
#define COUNTDOWN_CLOCK_PIN 14
#define CLOCK_LATCH_PIN 26
#define CLOCK_DATA_PIN 27
#define CLOCK_PIN 33

// RF24 Configuration
#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 13

// RTC Configuration
#define RTC_IO_PIN 15
#define RTC_SCLK_PIN 2
#define RTC_CE_PIN 12

// LED Configuration
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

ThreeWire myWire(RTC_IO_PIN, RTC_SCLK_PIN, RTC_CE_PIN); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

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
        byte val = ledDigitBytes[vals[i]];
        shiftoutFreq(dataPin, clockPin, val);
    }

    digitalWrite(latchPin, HIGH);
}

void countdownPrint(int data[])
{
    multiShiftout(COUNTDOWN_DATA_PIN, COUNTDOWN_CLOCK_PIN, COUNTDOWN_LATCH_PIN, COUNTDOWN_DIGITS_LENGTH, data);
}

void clockPrint(int data[])
{
    multiShiftout(CLOCK_DATA_PIN, CLOCK_PIN, CLOCK_LATCH_PIN, CLOCK_DIGIT_LENGTH, data);
}

void rtcSetup() {
    Rtc.Begin();
    RtcDateTime defaultTime = RtcDateTime("Jan 01 2024", "00:00:00");

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(defaultTime);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    Serial.println("RTC current time: " + String(now.Hour()) + ":" + String(now.Minute()) + ":" + String(now.Second()));
    if (now < defaultTime) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(defaultTime);
    }
    else if (now > defaultTime) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == defaultTime) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    timeClock[0] = now.Hour() / 10;
    timeClock[1] = now.Hour() % 10;
    timeClock[2] = now.Minute() / 10;
    timeClock[3] = now.Minute() % 10;

    clockPrint(timeClock);
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

void printArray(String title, int arr[], int size) {
    Serial.print(title);
    for (int i = 0; i < size; i++)
    {
        Serial.print(String(arr[i]) + " ");
    }
    Serial.println();
}

void updateCountdown()
{
    // Decrement the last digit (seconds)
    timeCountDown[4]--;

    // Handle overflow and borrow logic for each digit
    for (int i = 4; i >= 0; i--)
    {
        if (timeCountDown[i] < 0)
        {
            timeCountDown[i] = (i % 2 == 0) ? 9 : 5; // Use 9 for units, 5 for tens (e.g., minutes and seconds)
            if (i > 0) timeCountDown[i - 1]--; // Decrement the next higher digit
        }
    }
}

void countdownTask()
{
    if (flagCountdownReset)
    {
        for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
            timeCountDown[i] = initCountDown[i];
        
        countdownPrint(initCountDown);

        printArray("Countdown Reset to ", timeCountDown, COUNTDOWN_DIGITS_LENGTH);

        flagCountdownReset = false;
        return;
    }

    if (!flagCountDown)
        return;

    updateCountdown();

    if (timeCountDown[0] == 0 && timeCountDown[1] == 0 && timeCountDown[2] == 0 && timeCountDown[3] == 0 && timeCountDown[4] == 0)
    {
        flagCountDown = false;
        flagCountdownReset = true;
    }

    printArray("Countdown: ", timeCountDown, COUNTDOWN_DIGITS_LENGTH);

    countdownPrint(timeCountDown);
}

void updateClock() {
    timeClock[3]++;  // Increment seconds (unit digit)

    // Handle overflow logic for seconds and minutes
    if (timeClock[3] > 9)  // If seconds exceed 9
    {
        timeClock[3] = 0;
        timeClock[2]++;  // Increment the tens of seconds
    }

    if (timeClock[2] > 5)  // If tens of seconds exceed 5
    {
        timeClock[2] = 0;
        timeClock[1]++;  // Increment the minutes (unit digit)
    }

    if (timeClock[1] > 9)  // If minutes exceed 9
    {
        timeClock[1] = 0;
        timeClock[0]++;  // Increment the tens of minutes
    }

    // Handle overflow for hours (24-hour format)
    if (timeClock[0] >= 2 && timeClock[1] >= 4)
    {
        flagClockReset = true;  // Reset the clock if it exceeds 24:00

        RtcDateTime time = RtcDateTime(2024, 1, 1, 0, 0, 0);
        Rtc.SetDateTime(time);

        return;
    }

    int hour = timeClock[0] * 10 + timeClock[1];
    int min = timeClock[2] * 10 + timeClock[3];

    RtcDateTime time = RtcDateTime(2024, 1, 1, hour, min, 0);
    Rtc.SetDateTime(time);
}

void clockTask()
{
    updateClock();

    if (flagClockReset)
    {
        for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
            timeClock[i] = 0;

        clockPrint(timeClock);

        printArray("Clock Reset to ", timeClock, CLOCK_DIGIT_LENGTH);

        flagClockReset = false;
        return;
    }

    // if (timeClock[0] == 2 && timeClock[1] == 4 && timeClock[2] == 0 && timeClock[3] == 0)
    // {
    //     flagClockReset = true;
    // }

    clockPrint(timeClock);
    printArray("Clock: ", timeClock, CLOCK_DIGIT_LENGTH);
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

        int hour = timeClock[0] * 10 + timeClock[1];
        int min = timeClock[2] * 10 + timeClock[3];

        RtcDateTime time = RtcDateTime(2024, 1, 1, hour, min, 0);
        Rtc.SetDateTime(time);

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
        flagCountdownReset = true;
        flagCountDown = false;

        Serial.println("Countdown Reset Menu!");
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

    // RTC Setup
    rtcSetup();

    // Reset LEDs start
    int zeroByte[COUNTDOWN_DIGITS_LENGTH] = {0, 0, 0, 0, 0};
    countdownPrint(zeroByte);
    // Reset LEDs end

    Serial.println("program setup: done");
    Serial.println("program start");

    delay(1000);
}

void loop()
{
    listenRadio();

    if (millis() - lastTime >= COUNTDOWN_INTERVAL)
    {
        countdownTask();
        lastTime = millis();
    }

    if (millis() - lastClockTime >= CLOCK_INTERVAL)
    {
        clockTask();
        lastClockTime = millis();
    }
}