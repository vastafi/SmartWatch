/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Penpheral Beat Amplitude (PBA) algorithm.

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

extern WiFiClientSecure wifiClient;
extern DynamicJsonDocument doc;
extern PubSubClient mqttClient;

extern Adafruit_SSD1306 display ;

extern MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

int no_finger_status = 0;

void ed_max30105_heart_rate_setup()
{
  Serial.println(F("MAX30105 Heart Rate app Started."));

  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void ed_max30105_heart_rate_loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  if (irValue < 50000){
    no_finger_status = 1;
    // Serial.print(" No finger?");
  }else{
    no_finger_status = 0;

  }

  // Serial.println();
}

void ed_max30105_heart_rate_report()
{
  Serial.print(" BPM = ");
  Serial.print(beatsPerMinute);
  Serial.print(" Avg BPM= ");
  Serial.print(beatAvg);
  if(no_finger_status == 1){
        Serial.print(" No finger detected");
  } else {
        Serial.print(" Finger detected");
  }

  Serial.println();
}

void ed_max30105_heart_rate_display()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Heart Beat Rate APP");
  display.println();
  display.print("-> BPM: ");
  display.println(beatsPerMinute);
  display.print("-> Avg BPM: ");
  display.print(beatAvg);
  display.println();
  if(no_finger_status == 1){
        display.print(" No finger detected");
  } else {
        display.print(" Finger detected");
  }
  display.display();
}

void    reconnect();

void ed_max30105_heart_rate_mqtt_publish()
{
  // put your main code here, to run repeatedly:

  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();

  // JSON mapping
  doc.clear();
  doc["app_id"] = "heart_rate";
  doc["bpm"] = beatsPerMinute;
  doc["avg_bpm"] = beatAvg;
  doc["no_finger_status"] = no_finger_status;

  char buffer[512];
  size_t n = serializeJson(doc, buffer);
  Serial.println(buffer);
  mqttClient.publish("microlab/hearth/device/smart_watch/heart_rate", buffer, n);
}