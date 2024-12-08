#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ESP32Servo.h>   

Servo sg90;          
int servo_pin = 13;
MPU6050 sensor;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// Structure to hold the data
typedef struct struct_message {
    double var1;
    double var2;
} struct_message;

struct_message myData;
int xc, yc;


bool dataReceived = false;

void OnDataRecv(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
    
    memcpy(&myData, incomingData, sizeof(myData));
    
    
    dataReceived = true;

    // Print received joystick values
    //Serial.print("Received var1: ");
    //Serial.println(myData.var1);
    //Serial.print("Received var2: ");
    //Serial.println(myData.var2);
}

void setup() {
    
    Serial.begin(115200);  

    
    WiFi.mode(WIFI_STA);
    
    sg90.attach(servo_pin); 
    
    Wire.begin();  
    
    Serial.println("Initializing the sensor"); 
    sensor.initialize();
    
    if (sensor.testConnection()) {
        Serial.println("Successfully Connected");
    } else {
        Serial.println("Connection failed");
    }

    delay(1000); 
    Serial.println("Taking Values from the sensor");
    delay(1000);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

    if (dataReceived) {

        dataReceived = false;

        sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        ax = map(ax, -17000, 17000, -90, 90);

        Serial.print("Mapped ax (sensor value): ");
        Serial.println(ax);
        Serial.println(myData.var1);

        int servoPosition = myData.var1 - ax;

        // Ensure the servo position is within the valid range (0 to 180)
        servoPosition = constrain(servoPosition, 0, 180);

        sg90.write(servoPosition);

        Serial.print("Servo position: ");
        Serial.println(servoPosition);
    }

    sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    ax = map(ax, -17000, 17000, 0, 180);
    delay(50);
}
