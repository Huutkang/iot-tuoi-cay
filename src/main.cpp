#include <Arduino.h>
#include "wifi_mqtt.h"
#include "sensor.h"
#include "irrigation.h"



// Đã được định nghĩa trong thư viện, không cần định nghĩa lại
// const int SCL = 22; // esp32: D22, esp8288: D1
// const int SDA = 21; // esp32: D21, esp8288: D2

// Relay1. esp32: D32, esp8266: D4
// Relay2. esp32: D33, esp8266: D5
// Relay3. esp32: D26, esp8266: D6
// Relay4. esp32: D25, esp8266: D7

int RL[4] = {32, 33, 25, 26};

unsigned long current_time;
unsigned long time1=0;
unsigned long time2=0;
unsigned long time3=0;
unsigned long time4=0;


int Timer(unsigned long *time, int wait){
    current_time = millis();
    if (current_time-*time>wait){
        *time = current_time;
        return 1;
    }
    else{
        return 0;
    }
}

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
    setupIrrigation(RL);          // Cấu hình hệ thống tưới cây
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                  // Cấu hình MQTT
    setupSensors(SCL, SDA);       // Cấu hình cảm biến (ADC: ADS1115)
        

}

void loop() {
    handleMQTT();                 // Xử lý kết nối MQTT
    if (Timer(&time1, 3000)){
        if(!mqtt_connected){
            connect_MQTT();
        }
    }
    if (Timer(&time2,5000)){
        readSensors();                // Đọc cảm biến và gửi dữ liệu
    }
    if (Timer(&time3,500)){
        manageIrrigation(RL, status);
    }
    if (Timer(&time4,1000)){
        publishData("abc", "xyz");
    }
}
