#include <DallasTemperature.h>

///////////////////////// Pins
const int OUT_RED_PIN = 13;
const int OUT_YELLOW_PIN = 12;
const int OUT_GREEN_PIN = 11;
const int IN_MOTION_PIN = 10;
const int IO_TEMPERATURE_PIN = 9;

///////////////////////// Delays
const unsigned long MOTION_DELAY = 4000;
const unsigned long TEMPERATURE_DELAY = 15000;

///////////////////////// Motion
const byte STATE_MOTION_UNDEFINED = 0;
const byte STATE_MOTION_NOT_DETECTED = 1;
const byte STATE_MOTION_WAITING = 2;
const byte STATE_MOTION_DETECTED = 3;

unsigned long yellowOff = 0;
byte motionState = STATE_MOTION_UNDEFINED;

void initializeMotion() {
    pinMode(IN_MOTION_PIN, INPUT);
    pinMode(OUT_RED_PIN, OUTPUT);
    pinMode(OUT_YELLOW_PIN, OUTPUT);
    pinMode(OUT_GREEN_PIN, OUTPUT);

    digitalWrite(OUT_RED_PIN, LOW);
    digitalWrite(OUT_YELLOW_PIN, LOW);
    digitalWrite(OUT_GREEN_PIN, LOW);
}

void detectMotion(unsigned long now) {
    boolean motionDetected = (digitalRead(IN_MOTION_PIN) == HIGH);

    if (motionDetected) {
        // motion
        yellowOff = now + MOTION_DELAY;

        if (motionState != STATE_MOTION_DETECTED) {
            motionState = STATE_MOTION_DETECTED;
            Serial.println("Motion Detected.");
            // turn off green light and turn on red
            digitalWrite(OUT_RED_PIN, HIGH);
            digitalWrite(OUT_YELLOW_PIN, LOW);
            digitalWrite(OUT_GREEN_PIN, LOW);
        }
    }
    else {
        // no motion
        digitalWrite(OUT_RED_PIN, LOW);
        if (now < yellowOff) {
            if (motionState != STATE_MOTION_WAITING) {
                motionState = STATE_MOTION_WAITING;
                Serial.println("On Alert.");
                // turn on yellow light
                digitalWrite(OUT_YELLOW_PIN, HIGH);
                digitalWrite(OUT_GREEN_PIN, LOW);
            }
        }
        else {
            if (motionState != STATE_MOTION_NOT_DETECTED) {
                motionState = STATE_MOTION_NOT_DETECTED;
                Serial.println("All Clear.");
                // turn off yellow light, and turn on green
                digitalWrite(OUT_YELLOW_PIN, LOW);
                digitalWrite(OUT_GREEN_PIN, HIGH);
            }
        }
    }
}

///////////////////////// Temperature and Humidity
OneWire oneWire(IO_TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

void initializeThermastor() {
  sensors.begin();
}

double lastMeasuredTemperature = 0.0;
unsigned long nextCheck = TEMPERATURE_DELAY;

void detectTemperature(unsigned long now) {
    if (now <= nextCheck)
    {
        return; // Not time to check yet
    }
    nextCheck = now + TEMPERATURE_DELAY;
    
    sensors.requestTemperatures();
    float degreesFarenheit = sensors.getTempFByIndex(0);

    if (lastMeasuredTemperature != degreesFarenheit) {
        Serial.print("Temperature: ");
        Serial.print(degreesFarenheit);
        Serial.println("F.");
        lastMeasuredTemperature = degreesFarenheit;
    }
}

///////////////////////// Arduino
void setup() {
    Serial.begin(9600);

    initializeMotion();
    initializeThermastor();

    Serial.println("Ready");
}

void loop() {
    unsigned long now = millis();

    detectMotion(now);
    detectTemperature(now);

    delay(50);
}
