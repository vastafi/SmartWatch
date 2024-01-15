/*
  MAX3010 Breakout: Read the onboard temperature sensor
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 20th, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This demo outputs the onboard temperature sensor. The temp sensor is accurate to +/-1 C but
  has an astonishing precision of 0.0625 C.

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
#include "MAX30105.h"  //Get it here: http://librarymanager/All#SparkFun_MAX30105

#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

extern Adafruit_SSD1306 display ;

extern MAX30105 particleSensor;

extern WiFiClientSecure wifiClient;
extern DynamicJsonDocument doc;
extern PubSubClient mqttClient;


void ed_max30105_temp_setup()
{
  Serial.println(F("MAX30105 Temperature app Started."));

  Serial.println("Initializing...");

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  //The LEDs are very low power and won't affect the temp reading much but
  //you may want to turn off the LEDs to avoid any local heating
  particleSensor.setup(0); //Configure sensor. Turn off LEDs
  //particleSensor.setup(); //Configure sensor. Use 25mA for LED drive

  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.
}

float temperature ;
float temperatureF;

void ed_max30105_temp_loop()
{
  temperature = particleSensor.readTemperature();

  // Serial.print(" temperatureC=");
  // Serial.print(temperature, 4);

  temperatureF = particleSensor.readTemperatureF(); //Because I am a bad global citizen

  // Serial.print(" temperatureF=");
  // Serial.print(temperatureF, 4);

  }

void ed_max30105_temp_report()
{
  Serial.print(" Temp C=");
  Serial.print(temperature, 4);

  Serial.print(" Temp F=");
  Serial.print(temperatureF, 4);

  Serial.println();
}

void ed_max30105_temp_display()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(" Temperature APP ");
  display.println();  
  display.print(" Temp C= ");
  display.println(temperature, 4);
  display.print(" Temp F= ");
  display.print(temperatureF, 4);
  display.display();
}

void reconnect();

void ed_max30105_temp_mqtt_publish()
{
  // put your main code here, to run repeatedly:

  if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();

  // JSON mapping
  doc.clear();
  doc["app_id"] = "temp";
  doc["temp_c"] = temperature;
  doc["temp_f"] = temperatureF;

  // Publishing data throgh MQTT
  char mqtt_message[128];
  serializeJson(doc, mqtt_message);
  mqttClient.publish("microlab/hearth/device/smart_watch/temp", mqtt_message, true);
}
