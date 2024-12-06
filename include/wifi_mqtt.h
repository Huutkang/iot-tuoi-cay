#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

extern bool isAuto[4];
extern bool status[4];
extern bool mqtt_connected;

void setupWiFi();
void setupMQTT();
void connect_MQTT();
bool publishData(const char* topic, const char* payload);
void handleMQTT();


#endif
