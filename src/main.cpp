#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#pragma GCC optimize("O3") // code optimisation controls - "O2" & "O3" code performance, "Os" code size

#define TOTAL_DIGITS_LENGTH 9
#define SERIAL_BAUD_RATE 115200

#define LATCH_PIN 4
#define DATA_PIN 5
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
uint32_t radioListenTime;
int initCountDown[5] = {0, 0, 0, 0, 0};
int timeCountDown[5] = {0, 0, 0, 0, 0};
int timeClock[4] = {0, 0, 0, 0};
bool flagCountDown = true;
bool flagDisplayUpdate;

void writeSegmentDigit(byte value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

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

    for (int i = 0; i < TOTAL_DIGITS_LENGTH; i++)
        writeSegmentDigit(ledDigitBytes[0]);
    
    radioSetup();

    Serial.println("program setup: done");
    Serial.println("program start");
}

void timeTask() {
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
}

void showTime() {
    if (!flagDisplayUpdate)
        return;

    int toShow[TOTAL_DIGITS_LENGTH] = {
        timeCountDown[0], 
        timeCountDown[1], 
        timeCountDown[2], 
        timeCountDown[3],
        timeCountDown[4],
        timeClock[0], 
        timeClock[1], 
        timeClock[2],
        timeClock[3]
    };
    
    for (int i = 0; i < TOTAL_DIGITS_LENGTH; i++)
    {
        int index = toShow[i];
        writeSegmentDigit(ledDigitBytes[index]);
    }

    flagDisplayUpdate = false;
    
    if (timeCountDown[0] == 0 && timeCountDown[1] == 0 && flagCountDown)
        flagCountDown = false;    
}

void listenRadio() {
    if (!radio.available())
        return;
    
    char message[10];
    while (radio.available())
        radio.read(&message, 10);

    String messageStr = String(message);
    Serial.println("command: " + messageStr);

    int menu = messageStr.substring(0, 1).toInt();

    if (menu == 1)
    {
        String timeStr = messageStr.substring(2, 7);

        for (int i = 0; i < sizeof(timeCountDown); i++)
        {
            timeCountDown[i] = timeStr[i] - '0';
            initCountDown[i] = timeStr[i] - '0';
        }

        Serial.println("time: " + String(timeCountDown[0]) + ":" + String(timeCountDown[1]));
    } else if (menu == 2)
    {
        String timeStr = messageStr.substring(2, 7);

        for (int i = 0; i < sizeof(timeClock); i++)
        {
            timeClock[i] = timeStr[i] - '0';
        }

        Serial.println("count down time: " + String(timeCountDown[0]) + ":" + String(timeCountDown[1]));
    } else if (menu == 3)
    {
        flagCountDown = messageStr.substring(7, 8).toInt();
        Serial.println("count down: " + String(flagCountDown));
    } else if (menu == 4)
    {
        for (int i = 0; i < sizeof(timeCountDown); i++)
        {
            timeCountDown[i] = initCountDown[i];
        }
        
        flagCountDown = false;
        Serial.println("count down reset");
    }

    flagDisplayUpdate = true;
}

void loop()
{
    if (millis() - radioListenTime >= 250)
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
