#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define COUNTDOWN_DIGITS_LENGTH 5
#define CLOCK_DIGIT_LENGTH 4
#define SERIAL_BAUD_RATE 115200

#define COUNTDOWN_LATCH_PIN 27
#define COUNTDOWN_DATA_PIN 14
#define CLOCK_LATCH_PIN 12
#define CLOCK_DATA_PIN 13
#define CLOCK_PIN 17

#define PIPE_ADDRESS 0xE8E8F0F0E1LL
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 2

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

// byte error = B10011110;

RF24 radio(CE_PIN, CSN_PIN);

uint32_t lastTime;
uint32_t lastTime2;
uint32_t radioListenTime;
int initCountDown[5] = {0, 0, 0, 0, 0};
int timeCountDown[5] = {0, 0, 0, 0, 0};
int timeClock[4] = {0, 0, 0, 0};
bool flagCountDown;
bool flagDisplayUpdate;
bool flagRadioAvailable;

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

void radioSetup()
{
    if (!radio.begin())
    {
        for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
        {
            writeCountdownSegment(ledDigitBytes[9]);
        }

        for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
        {
            writeClockSegment(ledDigitBytes[9]);
        }
        
        Serial.println(F("radio hardware is not responding!"));
        while (true)
            ;
    }

    radio.openReadingPipe(1, PIPE_ADDRESS);
    radio.startListening();

    Serial.println(F("radio setup: done"));
}

void isr_function() {
    flagRadioAvailable = true;
}

void setup()
{
    pinMode(CLOCK_DATA_PIN, OUTPUT);
    pinMode(CLOCK_LATCH_PIN, OUTPUT);
    pinMode(COUNTDOWN_DATA_PIN, OUTPUT);
    pinMode(COUNTDOWN_LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(IRQ_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(IRQ_PIN), isr_function, FALLING);

    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();

    for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
        writeCountdownSegment(ledDigitBytes[0]);

    for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
        writeClockSegment(ledDigitBytes[0]);
    delay(2000);

    radioSetup();

    Serial.println("program setup: done");
    Serial.println("program start");
}

void countdownTask()
{
    if (!flagCountDown)
        return;

    timeCountDown[4]--;

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

    flagDisplayUpdate = true;

    if (timeCountDown[0] == 0 && timeCountDown[1] == 0 && timeCountDown[2] == 0 && timeCountDown[3] == 0 && timeCountDown[4] == 0)
        flagCountDown = false;
}

void clockTask()
{
    timeClock[3]++;

    if (timeClock[3] > 9)
    {
        timeCountDown[3] = 0;
        timeCountDown[2]++;
    }

    if (timeCountDown[2] > 5)
    {
        timeCountDown[2] = 0;
        timeCountDown[1]++;
    }

    if (timeCountDown[0] == 2 && timeCountDown[1] == 4)
    {
        timeCountDown[3] = 0;
        timeCountDown[2] = 0;
        timeCountDown[1] = 0;
        timeCountDown[0] = 0;
    }

    flagDisplayUpdate = true;
}

void showTime()
{
    if (!flagDisplayUpdate)
        return;

    for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
    {
        int index = timeCountDown[i];
        writeCountdownSegment(ledDigitBytes[index]);
    }

    for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
    {
        int index = timeClock[i];
        writeClockSegment(ledDigitBytes[index]);
    }

    flagDisplayUpdate = false;
}

String readRadio(int length)
{
    Serial.println("Reading Radio");

    char message[length];
    while (radio.available())
        radio.read(&message, length);

    String messageStr = String(message);
    Serial.println("Radio Command: " + messageStr);
    return messageStr;
}

void extractData(String messageStr, int menuOutput, String dataOut)
{
    // 1#00400
    menuOutput = messageStr.substring(0, 1).toInt();
    dataOut = messageStr.substring(2);

    Serial.println("menu: " + String(menuOutput) + " data: " + dataOut);
}

void selectMenu(int menu, String dataStr)
{
    switch (menu)
    {
    case 1: // Countdown Menu
    {
        String timeStr;
        for (int i = 0; i < sizeof(timeCountDown); i++)
        {
            timeCountDown[i] = dataStr[i] - '0';
            initCountDown[i] = dataStr[i] - '0';

            timeStr += timeCountDown[i];
        }

        flagCountDown = true;

        Serial.println("Countdown Set: " + timeStr);
        break;
    }
    case 2: // Clock Menu
    {
        String timeStr;
        for (int i = 0; i < sizeof(timeClock); i++)
        {
            timeClock[i] = dataStr[i] - '0';

            timeStr += timeClock[i];
        }

        Serial.println("Clock Time Set: " + timeStr);
        break;
    }
    case 3: // Countdown Pause Menu
    {
        flagCountDown = dataStr.substring(dataStr.length() - 1).toInt();

        Serial.println("Countdown Status: " + flagCountDown ? "Run" : "Pause");
        break;
    }
    case 4: // Countdown Reset Menu
    {
        for (int i = 0; i < sizeof(timeCountDown); i++)
        {
            timeCountDown[i] = initCountDown[i];
        }

        flagCountDown = false;
        Serial.println("Countdown Reset!");
        break;
    }
    default:
        break;
    }

    flagDisplayUpdate = true;
}

void listenRadio()
{
    if (!radio.available())
        return;

    String messageStr = readRadio(7);

    int menu;
    String dataStr;

    extractData(messageStr, menu, dataStr);
    selectMenu(menu, dataStr);
}

void loop()
{
    // if (millis() - radioListenTime >= 250)
    // {
    //     // listenRadio();

    //     radioListenTime = millis();
    // }

    if (flagRadioAvailable) {
        listenRadio();

        flagRadioAvailable = false;
    }


    if (millis() - lastTime >= 1000)
    {
        // showTime();
        // countdownTask();
        // clockTask();

        for (int i = 0; i < CLOCK_DIGIT_LENGTH; i++)
        {
            writeClockSegment(ledDigitBytes[timeClock[7]]);
        }

        for (int i = 0; i < COUNTDOWN_DIGITS_LENGTH; i++)
        {
            writeCountdownSegment(ledDigitBytes[timeClock[3]]);
        }
        

        lastTime = millis();
    }
}
