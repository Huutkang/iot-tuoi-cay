#include "sensor.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>

// sensor/ADC
Adafruit_ADS1115 adc; // Sử dụng thư viện Adafruit ADS1115
float tf = 0.1;                                 // Yếu tố tin cậy để làm mịn bộ lọc
float sensor[4] = {100, 100, 100, 100};         // Đặt giá trị bắt đầu cao nhất để tránh các kích hoạt không mong muốn
float sensorDry[4] = {2760, 2680, 2780, 2760};  // Đọc từ khi nổi lên hoàn toàn trong nước
float sensorWet[4] = {1460, 1210, 1510, 1500};  // Đọc từ khi ở trong không khí 'khô'

// Khởi tạo các cảm biến
void setupSensors() {
    Wire.begin(); // Không cần chỉ định SCL và SDA
    if (!adc.begin()) {
        Serial.println("Không tìm thấy ADS1115. Kiểm tra kết nối!");
        while (1); // Dừng chương trình nếu không tìm thấy ADS1115
    }
    Serial.println("ADS1115 đã sẵn sàng.");
}

// Đọc giá trị từ kênh ADC
float readChannel(int channel) {
    if (channel < 0 || channel > 3) {
        Serial.println("Kênh không hợp lệ!");
        return 0.0;
    }
    int16_t adcValue = adc.readADC_SingleEnded(channel);
    float voltage = adcValue * 0.1875 / 1000.0; // Chuyển đổi sang volt
    Serial.println(adcValue);
    return adcValue;
}

// Đọc giá trị từ các cảm biến
void readSensors() {
    float reading[4];
    // Đọc giá trị thô từ các kênh
    for (int i = 0; i < 4; i++) {
        reading[i] = readChannel(i);
        reading[i] = map(reading[i], sensorDry[i], sensorWet[i], 0, 100);
        sensor[i] = tf * reading[i] + (1 - tf) * sensor[i];
        sensor[i] = constrain(sensor[i], 0, 100);
    }
}
