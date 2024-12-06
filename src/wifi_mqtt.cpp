#include "wifi_mqtt.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// Cấu hình HiveMQ Broker
const char* mqtt_server = "da515a6f948a482bb656f7310841d60d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "huuthang";
const char* mqtt_pass = "123456";

// Chủ đề MQTT
const char* control_topic = "control";
const char* config_topic = "config";

bool isAuto[4] = {true, true, true, true};  // true: AUTO, false: not AUTO
bool status[4] = {false, false, false, false};
bool mqtt_connected = false;

int count_connect_wifi = 0;
int min_moisture = 60;
int max_moisture = 90;

// Biến lưu trữ chuỗi MQTT nhận được
String mqttMessage = "";

// Định nghĩa MQTT và Wi-Fi
WiFiClientSecure espClient;          // Đối tượng WiFiClient
PubSubClient mqttClient(espClient);  // Đối tượng MQTT client

void setupWiFi() {
    WiFiManager wifiManager;

    // Tự động kết nối hoặc tạo Access Point
    if (!wifiManager.autoConnect("ESP8266_AP")) {
        Serial.println("Không kết nối được Wi-Fi");
        delay(3000);
        ESP.restart();  // Khởi động lại thiết bị
    }
    Serial.println("Đã kết nối Wi-Fi!");
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    mqttMessage = message; // Lưu lại toàn bộ chuỗi nhận được

    if (String(topic) == control_topic) {
        if (message.startsWith("ON")) {
            int relayIndex = message.substring(2).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;
                status[relayIndex] = true;
            }
        } else if (message.startsWith("OFF")) {
            int relayIndex = message.substring(3).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;
                status[relayIndex] = false;
            }
        } else if (message.startsWith("AUTO")) {
            int relayIndex = message.substring(4).toInt() - 1;
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = true;
                status[relayIndex] = false;
            }
        }
    } else if (String(topic) == config_topic) {
        if (message.startsWith("MIN:")) {
            int newMin = message.substring(4).toInt();
            if (newMin > 0 && newMin < max_moisture) {
                min_moisture = newMin;
                Serial.println("Cập nhật min_moisture: " + String(min_moisture));
            }
        } else if (message.startsWith("MAX:")) {
            int newMax = message.substring(4).toInt();
            if (newMax > min_moisture && newMax <= 100) {
                max_moisture = newMax;
                Serial.println("Cập nhật max_moisture: " + String(max_moisture));
            }
        }
    }
}

void connect_MQTT() {
    if (!mqttClient.connected()) {
        Serial.print("Đang kết nối MQTT...");
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
            mqtt_connected = true;
            Serial.println("Đã kết nối MQTT!");
            mqttClient.subscribe(control_topic);
        } else {
            Serial.print("Lỗi MQTT: ");
            Serial.println(mqttClient.state());
            if (WiFi.status() != WL_CONNECTED || count_connect_wifi > 5) {
                count_connect_wifi = 0;
                Serial.println("Lỗi wifi");
                WiFi.reconnect();
            } else {
                count_connect_wifi++;
            }
            for (int i = 0; i < 4; i++) {
                isAuto[i] = true;
            }
        }
    }
}

void setupMQTT() {
    espClient.setInsecure(); // Vô hiệu hóa bảo mật SSL/TLS
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    connect_MQTT();
}

bool publishData(const char* topic, const char* payload) {
    mqttClient.loop();
    if (mqttClient.connected()) {
        mqttClient.publish(topic, payload);
        return true;
    } else {
        mqtt_connected = false;
        return false;
    }
}

void handleMQTT() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        mqtt_connected = false;
    }
}
