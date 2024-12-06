#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

extern bool isAuto[4];
extern bool status[4];

void setupWiFi();
void setupMQTT();
void reconnectMQTT();
void publishData(const char* topic, const char* payload);
void handleMQTT();


#endif
