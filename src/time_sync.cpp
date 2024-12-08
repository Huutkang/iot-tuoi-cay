#include "time_sync.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>



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
    if (pumpIndex < 0 || pumpIndex > 3 || timerIndex < 0 || timerIndex > 3) {
        Serial.println("Chỉ số máy bơm hoặc hẹn giờ không hợp lệ!");
        return;
    }
    pumpTimers[pumpIndex][timerIndex].startTime = startTimeInSeconds;
    pumpTimers[pumpIndex][timerIndex].isActive = true;
}

void ProcessTimerString(String& input) {
    // Kiểm tra độ dài chuỗi đầu vào
    if (input.length() < 3) {
        input = ""; // Xóa chuỗi sau khi xử lý
        return;
    }

    // Lấy số đầu tiên (2 số pumpIndex và timerIndex)
    int pumpIndex = input[0] - '0'; // Số đầu tiên
    int timerIndex = input[1] - '0'; // Số thứ hai

    // Kiểm tra tính hợp lệ của pumpIndex và timerIndex
    if (pumpIndex < 0 || pumpIndex > 3 || timerIndex < 0 || timerIndex > 3) {
        Serial.println("Chỉ số máy bơm hoặc hẹn giờ không hợp lệ!");
        input = ""; // Xóa chuỗi sau khi xử lý
        return;
    }

    // Lấy phần còn lại sau khoảng trắng
    String remaining = input.substring(2);
    remaining.trim(); // Loại bỏ khoảng trắng thừa

    if (remaining == "off") {
        // Nếu chuỗi là "off", tắt bộ hẹn giờ
        pumpTimers[pumpIndex][timerIndex].isActive = false;
        Serial.println("Đã tắt hẹn giờ cho máy bơm " + String(pumpIndex) + ", hẹn giờ " + String(timerIndex));
    } else {
        // Nếu chuỗi là số, chuyển đổi và gọi SetWateringTimer
        unsigned long startTimeInSeconds = remaining.toInt();
        if (startTimeInSeconds > 0) {
            SetWateringTimer(pumpIndex, timerIndex, startTimeInSeconds);
            Serial.println("Đã đặt hẹn giờ cho máy bơm " + String(pumpIndex) + ", hẹn giờ " + String(timerIndex) + " vào " + String(startTimeInSeconds) + " giây.");
        } else {
            Serial.println("Thời gian không hợp lệ!");
        }
    }

    // Xóa chuỗi sau khi xử lý
    input = "";
}


void checkAndActivateTimers() {
    unsigned long currentSeconds = getSecondsSinceMidnight();

    for (int pump = 0; pump < 4; pump++) {
        for (int timer = 0; timer < 4; timer++) {
            if (pumpTimers[pump][timer].isActive && // thời gian hiện tại phải lớn hơn thời gian hẹn giờ thì mới bật máy bơm
                pumpTimers[pump][timer].startTime +60 > currentSeconds && // nhưng không được lớn hơn quá 1 phút, nếu không là hẹn giờ cho ngày hôm sau
                currentSeconds >= pumpTimers[pump][timer].startTime) { // làm vậy để cho không bị lỗi là, bây giờ là 6 giờ, hẹn giờ là 1 giờ thì máy bơm bật luôn chứ không phải là 1 h hôm sau mới bật
                
                // Kích hoạt hẹn giờ
                watering_timer[pump] = true;
                pumpTimers[pump][timer].isActive = false; // Vô hiệu hóa hẹn giờ sau khi kích hoạt
            }
        }
    }
}
