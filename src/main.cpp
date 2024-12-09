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

int RL[4] = {D4, D5, D6, D7};

unsigned long current_time;
unsigned long time1=0;
unsigned long time2=0;
unsigned long time3=0;
unsigned long time4=0;
unsigned long time5=0;

int t[4] = {max_time[0], max_time[1], max_time[2], max_time[3]};
int ActivationTime = 60;
bool count_status[4] = {false, false, false, false};


// với kiểu dữ liệu unsigned long: 10 - 4294967295 = 11 nên không lo tràn số ở hàm millis nhé
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

void updateStatus() {
    for (int i = 0; i < 4; i++) {
        if (!isAuto[i]) {
            // Nếu không ở chế độ tự động, bỏ qua máy bơm này
            continue;
        }
        if (t[i]<0){
            status[i] = false;
        }

        if (sensor[i] <= min_moisture[i]) {
            if (!count_status[i]){
                count_status[i] = true;
                status[i] = true;
            }
        } else if (sensor[i] >= max_moisture[i]) {
            status[i] = false; // Không tưới
        } else {
            // Độ ẩm nằm trong khoảng 60-90
            if (watering_timer[i]) {
                if (!count_status[i]){
                    count_status[i] = true;
                    status[i] = true;
                }
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
    setupSensors();       // Cấu hình cảm biến (ADC: ADS1115)
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
//     nếu độ ẩm bé hơn min thì tưới cây, nếu độ ẩm lớn hơn max thì không tưới, 
//     nếu độ ẩm nằm giữa min và max thì tưới theo lịch
//     
//     khi máy bơm được bật thì nó được tưới tối đa trong khoảng thời gian được lưu trong mảng max_time[4] (auto)
//     khi máy bơm tắt thì nó phải chờ một khoảng thời gian là ActivationTime mới bật lên lại được (auto)
//     logic trên được triển khải bằng ActivationTime, t[4], count_status[4]


void loop() {
    handleMQTT();                 // Xử lý kết nối MQTT
    if (Timer(&time1, 3000)){ // kết nối lại mqtt, wifi
        if(!mqtt_connected){
            connect_MQTT();
        }
    }
    if (Timer(&time2,5000)){ // đọc, gửi, in giá trị cảm biến
        readSensors();                
        for (int i = 0; i < 4; i++) {
            String message = String(i + 1) + " " + String(sensor[i]);
            publishData("RH", message.c_str());
            // Serial.println(message);
        }
        String pump_status = String(status[0]) + String(status[1]) + String(status[2]) + String(status[3]);
        publishData("PS", pump_status.c_str());
    }
    if (Timer(&time3,500)){ // thực thi bật tắt máy bơm
        manageIrrigation(RL, status);
    }
    if (Timer(&time4,1000)){ // thay đổi trạng thái
        updateStatus(); // thay đổi trạng thái máy bơm
        for (int i=0; i<4; i++){ // kiểm soát thời gian tưới tối đa và thời gian tối thiểu từ khi tắt đến khi bật (chế độ auto)
            if (count_status[i]){
                t[i]--; // bắt buộc để timer ở đây là 1 giây để hoạt động bình thường
            }
            if (t[i]<-ActivationTime){
                count_status[i]=false;
                t[i]=max_time[i];
            }
        }
    }
    if (Timer(&time5,990)){ // hẹn giờ
        ProcessTimerString(mqttMessage); // hẹn giờ bơm
        checkAndActivateTimers();  // kích hoạt các máy bơm đã hẹn giờ
    }
    // Đồng bộ thời gian mỗi 15 phút
    updateTimeSync();
}
