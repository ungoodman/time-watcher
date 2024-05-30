#include <SPI.h>
// #include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 7);
const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup()
{
    Serial.begin(115200);

    if (!radio.begin())
    {
        Serial.println("Radio Begin Failed");
        while(true);
    }

    radio.openReadingPipe(1, pipe);

    radio.startListening();

    Serial.println("Radio Begin");
}

void loop()
{
    if (radio.available())
    {
        Serial.println("Radio Available");
        char getFromRead[10];
        while (radio.available())
        {
            // Fetch the payload, and see if this was the last one.
            radio.read(getFromRead, 10);
        }
        Serial.print(getFromRead);
    }
}
