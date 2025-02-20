#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>
#include "esp_wifi.h"

esp_err_t result;

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
    int chan1;
} struct_messagein;
int count = 0;
struct_messagein myData_in;

bool dataReceived = false;
int channel = 1;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

    Serial.print("Status Code: ");
    Serial.println(status);

    // Print MAC address of the receiver
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Receiver MAC Address: ");
    Serial.println(macStr);

    // Log Wi-Fi status
    Serial.print("WiFi Channel: ");
    Serial.println(WiFi.channel());
    Serial.print("WiFi Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());
}

void addPeer(int chan){
  if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        //return;
    }
    //for(channel = 0; channel <=20; channel++){}
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = chan;
    peerInfo.encrypt = false;
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE); 
    esp_wifi_set_promiscuous(false);
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer on cahnnel");
        //return;
    }
    else{
      Serial.println("peer added on channel");
      
    }
    Serial.print(chan);
}
void OnDataRecv(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
    memcpy(&myData_in, incomingData, sizeof(myData_in));
    dataReceived = true;
    Serial.print("Received var1: ");
    Serial.println(myData_in.var1);
    Serial.print("Received var2: ");
    Serial.println(myData_in.var2);
    Serial.print("Received chan1: ");
    Serial.println(myData_in.chan1);
    channel = myData_in.chan1;
    count = 0;

}

void setup() {
  //WiFi.channel();
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

    addPeer(channel);
    //}
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

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
        myData_out.sen2 = ay;
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
        count = 0;
        channel = myData_in.chan1;
        if(WiFi.channel()!= channel){
          addPeer(channel);
        }

    }
    else{
      count ++;
      if(count >= 50){
        
        if(channel >=15){
          channel = 0;
        }
        addPeer(channel);
        channel++;
        count = 0;
      }

    }
    
    sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    ay = map(ay, -17000, 17000, -90, 90);
    int servoPosition = myData_in.var1 - ay;
    sg90.write(servoPosition);

    Serial.print("WiFi Channel: ");
    Serial.println(WiFi.channel());

    delay(50); 
}
