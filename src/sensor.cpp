#include "sensor.h"
#include <Wire.h>
#include <ADS1115_WE.h>




// sensor/ADC
ADS1115_WE adc(0x48);
float tf = 0.1;                                 // yếu tố tin cậy để làm mịn bộ lọc
float sensor[4] = {100, 100, 100, 100};         // Đặt giá trị bắt đầu cao nhất để tránh các kích hoạt không mong muốn
float sensorDry[4] = {2760, 2680, 2780, 2760};  // Đọc từ khi nổi lên hoàn toàn trong nước
float sensorWet[4] = {1460, 1210, 1510, 1500};  // Đọc từ khi ở trong không khí 'khô'



// Khởi tạo các cảm biến
void setupSensors(int SCL, int SDA) {
    Wire.begin(SCL, SDA);
    if (!adc.init()) {
        Serial.println("ADS1115 not connected!");
    }
    adc.setVoltageRange_mV(ADS1115_RANGE_6144);
}

// Đọc giá trị từ kênh ADC
float readChannel(ADS1115_MUX channel) {
    float voltage = 0.0;
    adc.setCompareChannels(channel);
    adc.startSingleMeasurement();
    voltage = adc.getResult_mV();
    Serial.println(voltage);
    return voltage;
}


// Đọc giá trị từ các cảm biến
void readSensors() {
    float reading[4];

    // Đọc giá trị thô từ các kênh
    reading[0] = readChannel(ADS1115_COMP_0_GND);
    reading[1] = readChannel(ADS1115_COMP_1_GND);
    reading[2] = readChannel(ADS1115_COMP_2_GND);
    reading[3] = readChannel(ADS1115_COMP_3_GND);

    // Chuyển đổi thành phần trăm, lọc và ràng buộc giá trị
    for (int i = 0; i < 4; i++) {
        reading[i] = map(reading[i], sensorDry[i], sensorWet[i], 0, 100);
        sensor[i] = tf * reading[i] + (1 - tf) * sensor[i];
        sensor[i] = constrain(sensor[i], 0, 100);
    }

}

