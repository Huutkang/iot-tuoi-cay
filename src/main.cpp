#include <Arduino.h>
#include "wifi_mqtt.h"
#include "sensor.h"
#include "irrigation.h"
#include "time_sync.h"


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
unsigned long time5=0;


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

void updateStatus() {

    for (int i = 0; i < 4; i++) {
        if (!isAuto[i]) {
            // Nếu không ở chế độ tự động, bỏ qua máy bơm này
            continue;
        }

        if (sensor[i] < 60) {
            status[i] = true;  // Tưới cây
        } else if (sensor[i] > 90) {
            status[i] = false; // Không tưới
        } else {
            // Độ ẩm nằm trong khoảng 60-90
            if (watering_timer[i]) {
                status[i] = true;       // Tưới cây
                watering_timer[i] = false; // Reset lại bộ hẹn giờ
            }
        }
    }
}


void setup() {
    Serial.begin(115200);
    setupIrrigation(RL);          // Cấu hình hệ thống tưới cây
    initializeTimers();
    setupWiFi();                  // Cấu hình WiFi
    setupMQTT();                 // Cấu hình MQTT
    setupSensors(SCL, SDA);       // Cấu hình cảm biến (ADC: ADS1115)
    setupTimeSync();

    // Ví dụ: Đặt hẹn giờ cho máy bơm 0 vào lúc 08:00:00
    // SetWateringTimer(0, 0, 8 * 3600);
}



// logic tưới cấy:

//     có hai chế độ là tự động và điều khiển bằng tay
//     nếu mất kết nối thì tự động chuyển về chế độ tự động
//     khi ở chế độ điều khiển bằng tay, người dùng tự bật lên thì tự tắt đi
//     khi ở chế độ tự động, người dùng lập lịch tưới hàng ngày
//     có hai giá trị độ ẩm là mix và max. ví dụ min=60 và max=90
//     nếu độ ẩm bé hơn min thì tưới cây, nếu độ ẩm lớn hơn max thì không tưới, nếu độ ẩm nằm giữa min và mã thì tưới theo lịch



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
        updateStatus();
        ProcessTimerString(mqttMessage);
        publishData("abc", "xyz");
    }
    if (Timer(&time5,1000)){
        checkAndActivateTimers();
        // In thời gian hiện tại (test)
        Serial.print("Current time: ");
        Serial.println(getCurrentTime());

        // In số giây từ đầu ngày (test)
        Serial.print("Seconds since midnight: ");
        Serial.println(getSecondsSinceMidnight());
    }
    // Đồng bộ thời gian mỗi 15 phút
    updateTimeSync();
}
