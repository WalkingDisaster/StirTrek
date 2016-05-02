#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

///////////////////////// Pins
const int IN_MOTION_PIN = 7;
const int OUT_RED_PIN = 13;
const int OUT_YELLOW_PIN = 12;
const int OUT_GREEN_PIN = 11;
const int IO_TEMP_HUMIDITY_PIN = 4;

///////////////////////// Delays
const unsigned long MOTION_DELAY = 4000;
const unsigned long TEMP_HUMIDITY_DELAY = 15000;

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
void initializeTemperatureAndHumidity() {
    pinMode(IO_TEMP_HUMIDITY_PIN, OUTPUT);
}

void measureTemperatureAndHumidity(float *temperatureInFarenheit, float *relativeHumidityInPercent) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  *temperatureInFarenheit = f;
  *relativeHumidityInPercent = h;
}

float lastMeasuredTemperature = 0.0;
float lastMeasuredHumidity = 0.0;
unsigned long nextCheck = TEMP_HUMIDITY_DELAY;

void detectTemperatureAndHumidity(unsigned long now) {
    if (now <= nextCheck)
    {
        return; // Not time to check yet
    }
    nextCheck = now + TEMP_HUMIDITY_DELAY;

    float degreesFarenheit = 0.0;
    float percentHumidity = 0.0;

    measureTemperatureAndHumidity(&degreesFarenheit, &percentHumidity);

    if (lastMeasuredTemperature != degreesFarenheit) {
        Serial.print("Temperature: ");
        Serial.print(degreesFarenheit);
        Serial.println("F.");
        lastMeasuredTemperature = degreesFarenheit;
    }
    if (lastMeasuredHumidity != percentHumidity) {
        Serial.print("Relative humidity: ");
        Serial.print(percentHumidity);
        Serial.println("%.");
        lastMeasuredHumidity = percentHumidity;
    }
}

///////////////////////// Arduino
void setup() {
    initializeMotion();
    initializeTemperatureAndHumidity();

    Serial.begin(9600);
    Serial.println("Ready");
}

void loop() {
    unsigned long now = millis();

    detectMotion(now);
    detectTemperatureAndHumidity(now);

    delay(1000);
}
