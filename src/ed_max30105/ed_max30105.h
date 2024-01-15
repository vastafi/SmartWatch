#ifndef ED_MAX30105_H
#define ED_MAX30105_H

// Include any necessary libraries here

// Define any constants or macros here

// Declare any global variables here

// Declare any function prototypes here


void ed_max30105_temp_setup();
void ed_max30105_temp_loop();
void ed_max30105_temp_report();
void ed_max30105_temp_display();
void ed_max30105_temp_mqtt_publish();

void ed_max30105_heart_rate_setup();
void ed_max30105_heart_rate_loop();
void ed_max30105_heart_rate_report();
void ed_max30105_heart_rate_display();
void ed_max30105_heart_rate_mqtt_publish();

void ed_max30105_spo2_setup();
void ed_max30105_spo2_loop();
void ed_max30105_spo2_report();
void ed_max30105_spo2_display();
void ed_max30105_spo2_mqtt_publish();


#endif // ED_MAX30105_H
