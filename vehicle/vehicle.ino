/*
 * OpenAlarm
 * Version 1.0.0
 * By Johnny Stene <johnny@stene.xyz>
 *
 * Requires the RF24 library.
 */

#define RANDOM_PIN A0
#define HORN_PIN 5
#define STOP_PIN 6
#define LIGHT_DETECT_PIN 7

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);
const byte radio_address[][6] = {"OAVCLE", "OARMTE"};
const byte private_key[16] = "TESTKEY DONTUSE!";

int armed = 0;
int alarm_active = 0;
int alarm_toggle = 0;

void sendPayload(char* payload) {
    radio.stopListening();
    // TODO: encrypt payload

    radio.write(payload, 16);
    radio.startListening();
}

void setup() {
    Serial.begin(9600);
    Serial.println("OpenAlarm");

    pinMode(RANDOM_PIN, INPUT);
    pinMode(LIGHT_DETECT_PIN, INPUT);
    pinMode(STOP_PIN, INPUT_PULLUP);
    pinMode(HORN_PIN, OUTPUT);
    Serial.println("GPIO init complete");

    randomSeed(analogRead(RANDOM_PIN));
    Serial.println("Seeded RNG");

    Serial.print("Init radio... ");
    radio.begin();
    radio.openWritingPipe(radio_address[0]);
    radio.openReadingPipe(1, radio_address[1]);
    // radio.setPALevel(RF24_PA_MIN);
    Serial.println("done.");
}

void loop() {
    if(armed) {
        if(digitalRead(LIGHT_DETECT_PIN) == HIGH) {
            alarm_active = 1;
            alarm_toggle = 1;
            digitalWrite(HORN_PIN, HIGH);
            delay(500);
        }

        if(alarm_active) {
            if(digitalRead(STOP_PIN) == LOW) {
                digitalWrite(HORN_PIN, LOW);
                alarm_active = 0;
                alarm_toggle = 0;
            } else {
                if(alarm_toggle) {
                    alarm_toggle = 0;
                    digitalWrite(HORN_PIN, LOW);
                } else {
                    alarm_toggle = 1;
                    digitalWrite(HORN_PIN, HIGH);    
                }

                delay(500);
            }
        }
    }

    if(radio.available()) {
        char received[16];
        radio.read(&received, 16);
        // TODO: decrypt payload
        if(received[0] == 'D') {
            armed = 0;
        } else if(received[0] == 'A') {
            armed = 1;
        } else {
            char payload[16];
            payload[0] = '?'; // resend
            payload[1] = 0xFF; // end of payload
            sendPayload(&payload[0]);
        }
    }
}