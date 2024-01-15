
#include "Arduino.h"
#include "ed_max30105/ed_max30105.h"

#include <Adafruit_SSD1306.h>
#include "MAX30105.h"


#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>


// #define AOUT_PIN 36 // ESP32 pin GPIO36 (ADC0)

// put function declarations here:
void connectToWiFi();
void callback(char *topic, byte *payload, unsigned int length);
void publishMessage(int data);
void setupMQTT();
void reconnect();

const char *SSID = "FabLab-FREE";
const char *PWD = "";
//MQTT Broker
DynamicJsonDocument doc(1024);
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
char mqttServer[] = "9b7b323ee67e46d18f9317162c8e8841.s1.eu.hivemq.cloud";
char mqtt_username[] = "sergiu.doncila";
char mqtt_password[] = "QWEasd!@#123";
int mqttPort = 8883;
long last_time = 0;
char data[10];


const int buttonPin = 2; // the number of the pushbutton pin
#define APP_STATE_NR_OF_STATES 4

int buttonState = HIGH;    // variable for reading the pushbutton status
int buttonStateOld = HIGH; // variable for reading the pushbutton status
int app_state = 0;
int app_state_old = 0;




#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1



Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);


MAX30105 particleSensor;

void display_setup()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }

// text display tests
    display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  display.println();
  display.print("Smart Watch started");
  display.println();  
  display.print("Press button \n to select APP");
  display.println();


  display.setCursor(0,0);
  display.display(); // actually display all of the above
}



void setup()
{
  Serial.begin(115200);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  buttonState = digitalRead(buttonPin);
  buttonStateOld = buttonState;
  display_setup();
  // ed_max30105_setup();
  // ed_max30105_spo2_setup();

/// MQTT Setup

  connectToWiFi();
  wifiClient.setInsecure();
  setupMQTT();

}

int report_cnt = 0;
int report_cnt_max = 100;

void loop()
{

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  if (buttonState != buttonStateOld)
  {
    Serial.print("Buton:");
    Serial.println(buttonState);
    Serial.print("Buton old:");
    Serial.println(buttonStateOld);

    if (buttonState == HIGH && buttonStateOld == LOW)
    {
      app_state++;
      if (app_state >= APP_STATE_NR_OF_STATES)
      {
        app_state = 0;
      }
      Serial.print("App state: ");
      Serial.println(app_state);
    }
    buttonStateOld = buttonState;
  }

  // delay(100);

  switch (app_state)
  {

  case 1:
    /* code */
    if (app_state_old != app_state)
    {
      ed_max30105_spo2_setup();
      app_state_old = app_state;
    }
    else
    {
      ed_max30105_spo2_loop();
      ed_max30105_spo2_report();
      ed_max30105_spo2_display();
      ed_max30105_spo2_mqtt_publish();
    }
    break;

  case 2:
    /* code */
    if (app_state_old != app_state)
    {
      ed_max30105_heart_rate_setup();
      app_state_old = app_state;
    }
    else
    {
      ed_max30105_heart_rate_loop();
      if (report_cnt <= 0)
      {
        ed_max30105_heart_rate_report();
        ed_max30105_heart_rate_display();
        ed_max30105_heart_rate_mqtt_publish();
      }
    }
    break;

  case 3:
    /* code */
    if (app_state_old != app_state)
    {
      ed_max30105_temp_setup();
      app_state_old = app_state;
    }
    else
    {
      ed_max30105_temp_loop();
      if (report_cnt <= 0)
      {
        ed_max30105_temp_report();
        ed_max30105_temp_display();
        ed_max30105_temp_mqtt_publish();
      }
    }
    break;
  default:
    break;
  }

  if (report_cnt-- <= 0)
  {
    report_cnt = report_cnt_max;
  }
}

void connectToWiFi() {
  Serial.print("Connectiog to ");
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.\n");
}

void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);

  mqttClient.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.print("Actual state: ");
      Serial.println(mqttClient.state());
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);

      if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Connected to MQTT broker.");
        // subscribe to topic
        mqttClient.subscribe("microlab/hearth/device/smart_watch");
      }
  }
}

void publishMessage(int data) {

}