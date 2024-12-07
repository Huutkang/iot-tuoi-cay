#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "time_sync.h"



bool watering_timer[4] = {false, false, false, false};

struct Timer {
    unsigned long startTime; // Thời điểm kích hoạt (giây từ đầu ngày)
    bool isActive;           // Trạng thái của hẹn giờ (đang bật hay tắt)
};

Timer pumpTimers[4][4];

// Cấu hình NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 900000); // UTC+7, cập nhật 15 phút một lần

void setupTimeSync() {
    timeClient.begin(); // Khởi động NTP Client
    timeClient.update(); // Đồng bộ thời gian ngay khi khởi tạo
}

void updateTimeSync() {
    timeClient.update(); // Cập nhật thời gian từ máy chủ NTP
}

// Lấy thời gian hiện tại ở dạng HH:MM:SS
String getCurrentTime() {
    return timeClient.getFormattedTime();
}

// Lấy số giây kể từ đầu ngày
unsigned long getSecondsSinceMidnight() {
    unsigned long epochTime = timeClient.getEpochTime(); // Lấy thời gian epoch (giây từ 01/01/1970)
    return epochTime % 86400; // Lấy phần dư của số giây trong ngày (1 ngày = 86400 giây)
}

void initializeTimers() {
    // Khởi tạo tất cả các hẹn giờ là không hoạt động
    for (int pump = 0; pump < 4; pump++) {
        for (int timer = 0; timer < 4; timer++) {
            pumpTimers[pump][timer].startTime = 0;
            pumpTimers[pump][timer].isActive = false;
        }
    }
}

void SetWateringTimer(int pumpIndex, int timerIndex, unsigned long startTimeInSeconds) {
    if (pumpIndex < 0 || pumpIndex >= 4 || timerIndex < 0 || timerIndex >= 4) {
        Serial.println("Chỉ số máy bơm hoặc hẹn giờ không hợp lệ!");
        return;
    }
    pumpTimers[pumpIndex][timerIndex].startTime = startTimeInSeconds;
    pumpTimers[pumpIndex][timerIndex].isActive = true;
}

void checkAndActivateTimers() {
    unsigned long currentSeconds = getSecondsSinceMidnight();

    for (int pump = 0; pump < 4; pump++) {
        for (int timer = 0; timer < 4; timer++) {
            if (pumpTimers[pump][timer].isActive &&
                currentSeconds >= pumpTimers[pump][timer].startTime) {
                
                // Kích hoạt hẹn giờ
                watering_timer[pump] = true;
                pumpTimers[pump][timer].isActive = false; // Vô hiệu hóa hẹn giờ sau khi kích hoạt
                
                // Debug thông tin
                Serial.printf("Máy bơm %d đã được kích hoạt vào lúc %02lu:%02lu:%02lu\n",
                              pump,
                              currentSeconds / 3600,
                              (currentSeconds % 3600) / 60,
                              currentSeconds % 60);
            }
        }
    }
}
