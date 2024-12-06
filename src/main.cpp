#include <Arduino.h>
#include "wifi_mqtt.h"
#include "sensor.h"
#include "irrigation.h"



// Đã được định nghĩa trong thư viện, không cần định nghĩa lại
// const int SCL = 22; // esp32: D22, esp8288: D1
// const int SDA = 21; // esp32: D21, esp8288: D2

// Relay1. esp32: D34, esp8266: D4
// Relay2. esp32: D35, esp8266: D5
// Relay3. esp32: D32, esp8266: D6
// Relay4. esp32: D33, esp8266: D7

int RL[4] = {34, 35, 32, 33};


// test
void printSensorValues() {
    for (int i = 0; i < 4; i++) {
        Serial.print("Sensor[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(sensor[i]); // In giá trị của từng phần tử
    }
}


void setup() {
    Serial.begin(115200);
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                  // Cấu hình MQTT
    setupSensors(SCL, SDA);       // Cấu hình cảm biến (ADC: ADS1115)
    setupIrrigation(RL);    // Cấu hình hệ thống tưới cây
    // test
    status[1]=false;
    isAuto[1]=true;
    sensor[4]=1;
    // test
}

void loop() {
    handleMQTT();                 // Xử lý kết nối MQTT
    readSensors();                // Đọc cảm biến và gửi dữ liệu
    manageIrrigation(RL, status);           // Quản lý tưới cây
    printSensorValues();
    delay(1000);
}
