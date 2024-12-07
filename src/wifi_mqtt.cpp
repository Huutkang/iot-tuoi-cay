#include "wifi_mqtt.h"
// #include <WiFi.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <BearSSLHelpers.h>



// Cấu hình HiveMQ Broker
const char* mqtt_server = "da515a6f948a482bb656f7310841d60d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "huuthang";
const char* mqtt_pass = "123456";

// Chủ đề MQTT
const char* control_topic = "control";
const char* config_topic = "config";

// Chứng chỉ Root CA
const char* ca_cert = R"~~~(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)~~~";

BearSSL::X509List cert(ca_cert);


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

    // Xóa thông tin Wi-Fi đã lưu (nếu cần)
    // wifiManager.resetSettings();

    // Tự động kết nối hoặc tạo Access Point
    if (!wifiManager.autoConnect("ESP32_AP")) {
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
        // Điều khiển relay
        if (message.startsWith("ON")) {
            int relayIndex = message.substring(2).toInt() - 1; // Lấy số relay
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;       // Chuyển sang false
                status[relayIndex] = true;      // Bật relay
            }
        } else if (message.startsWith("OFF")) {
            int relayIndex = message.substring(3).toInt() - 1; // Lấy số relay
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = false;       // Chuyển sang false
                status[relayIndex] = false;     // Tắt relay
            }
        } else if (message.startsWith("AUTO")) {
            int relayIndex = message.substring(4).toInt() - 1; // Lấy số relay
            if (relayIndex >= 0 && relayIndex < 4) {
                isAuto[relayIndex] = true;        // Chuyển sang true
                status[relayIndex] = false;    // Đảm bảo relay tắt
            }
        }
    } else if (String(topic) == config_topic) {
        // Cập nhật min_moisture và max_moisture
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
    // Thử kết nối MQTT
    if (!mqttClient.connected()) {
        Serial.print("Đang kết nối MQTT...");
        if (mqttClient.connect("ESP32Client", mqtt_user, mqtt_pass)) {  // Đổi "ESP32Client" nếu cần đặt tên khác
            mqtt_connected=true;
            Serial.println("Đã kết nối MQTT!");
            mqttClient.subscribe(control_topic);
        } else {
            Serial.print("Lỗi MQTT: ");
            Serial.println(mqttClient.state());
            if (WiFi.status() != WL_CONNECTED||count_connect_wifi>5) { // lâu lâu rõ ràng tắt wifi nhưng WiFi.status() vẫn là đang kết nối nên phải thêm cái biến đếm này
                count_connect_wifi=0;
                Serial.println("Lỗi wifi");
                WiFi.reconnect();
                Serial.println("đã chạy xong câu lệnh reconnect wifi");
            }else{
                count_connect_wifi++;
            }
            for (int i=0; i<4; i++) {
                isAuto[i]=true;
            }
        }
    }
}


void setupMQTT() {    
    BearSSL::WiFiClientSecure espClient;
    espClient.setTrustAnchors(&cert); // Thiết lập chứng chỉ CA

    // Thiết lập kết nối MQTT
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    connect_MQTT();
}

bool publishData(const char* topic, const char* payload) {
    mqttClient.loop(); //
    // Gửi dữ liệu qua MQTT
    if (mqttClient.connected()) {
        mqttClient.publish(topic, payload);
        return true;
    }else{
        mqtt_connected = false;
        return false;
    }
}

void handleMQTT() {
    if (mqttClient.connected()) {
        mqttClient.loop();
    }else{
        mqtt_connected = false;
    }
}
