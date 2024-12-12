#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>
#include "esp_wifi.h"

uint8_t broadcastAddress[] = {0xc0, 0x49, 0xef, 0x44, 0xd0, 0x68};  

const char* ssid = "Shibby";
const char* password = "12345678";

Servo sg90;          
int servo_pin = 13;
MPU6050 sensor;
int16_t ax, ay, az;
int16_t gx, gy, gz;

typedef struct struct_messageout {
    double sen1; 
    double sen2;
} struct_messageout;

struct_messageout myData_out;

typedef struct struct_messagein {
    double var1;
    double var2;
} struct_messagein;

struct_messagein myData_in;

bool dataReceived = false;

/*void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}*/

void OnDataRecv(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
    memcpy(&myData_in, incomingData, sizeof(myData_in));
    dataReceived = true;
    Serial.print("Received var1: ");
    Serial.println(myData_in.var1);
    Serial.print("Received var2: ");
    Serial.println(myData_in.var2);
}

void setup() {
    Serial.begin(115200);  
    WiFi.mode(WIFI_STA); 
    sg90.attach(servo_pin); 
    Wire.begin();  

    sensor.initialize();  
    if (sensor.testConnection()) {
        Serial.println("Successfully Connected to MPU6050");
    } else {
        Serial.println("MPU6050 Connection Failed");
    }

    delay(1000); 

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
    //esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE); 
    esp_wifi_set_promiscuous(false);
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    Serial.println("ESP-NOW initialized successfully");
}

void loop() {
    if (dataReceived) {
        dataReceived = false;

        sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        Serial.print("Accelerometer ay: ");
        Serial.println(ay);
        
        ay = map(ay, -17000, 17000, -90, 90); // Map accelerometer data to servo range
        Serial.print("Mapped ax: ");
        Serial.println(ax);

        int servoPosition = myData_in.var1 - ay;  // Adjust servo position based on joystick input
        Serial.print("Joystick var1: ");
        Serial.println(myData_in.var1);
        Serial.print("Calculated Servo Position: ");
        Serial.println(servoPosition);

        servoPosition = constrain(servoPosition, 0, 180);
        myData_out.sen1 = servoPosition;  // Set the servo position
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData_out, sizeof(myData_out));

        if (result == ESP_OK) {
            Serial.println("Data sent successfully");
        } else {
            Serial.print("Error sending data: ");
            Serial.println(result);
        }

        sg90.write(servoPosition);
        Serial.print("Servo position: ");
        Serial.println(servoPosition);
    }
    //sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    //ax = map(ax, -17000, 17000, -90, 90);
    //int servoPosition = myData_in.var1 - ax;
    //sg90.write(servoPosition);

    Serial.print("WiFi Channel: ");
    Serial.println(WiFi.channel());

    delay(50); 
}
