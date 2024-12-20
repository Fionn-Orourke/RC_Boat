#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "homepage.h"
#include "esp_wifi.h"

WebServer server(80);
#define xpin 32
#define ypin 35
uint8_t broadcastAddress[] = {0x64, 0xb7, 0x08, 0x29, 0x1b, 0x68};
int channel = WiFi.channel();
const char* ssid = "Shibby";
const char* password = "12345678";

// Structure to hold the data
typedef struct struct_message {
    double var1;
    double var2;
} struct_message;

struct_message myData;

typedef struct struct_messagein {
    double val1;
    double val2;
} struct_messagein;

struct_messagein myData_in;

bool dataReceived = false;

void OnDataRecv(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
    memcpy(&myData_in, incomingData, sizeof(myData_in));
    dataReceived = true;
    Serial.print("Received val1: ");
    Serial.println(myData_in.val1);
    Serial.print("Received val2: ");
    Serial.println(myData_in.val2);
}

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

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
void handleADC() {
    // Send only the joystick data (myData.var1)
    String adcValue = String(myData.var1) + " Joystick position"; 
    server.send(200, "text/plain", adcValue); 
}
void handleServo() {
    // Send only the servo data (myData_in.val1)
    String servoValue = String(myData_in.val1) + " Servo position"; 
    server.send(200, "text/plain", servoValue); 
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);  
    esp_wifi_set_promiscuous(false);


    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);
    

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
    esp_now_register_send_cb(OnDataSent);
    //webSocket.begin();
    //webSocket.onEvent(webSocketEvent);
    server.on("/", handleRoot);      //This is display page
    server.on("/readADC", handleADC);
    server.on("/readServo", handleServo);
    server.begin();                  //Start server
    Serial.println("HTTP server started");
}

/*void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
            break;
        }
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            break;
    }
}*/

void loop() {
    //webSocket.loop();
    server.handleClient();
    int x = analogRead(xpin);
    int y = analogRead(ypin);
    Serial.println(x);

    if (x >= 2680) {
        myData.var1 = map(x, 2680, 4095, 90, 180);
    }
    else if (x <= 2680) {
        myData.var1 = map(x, 0, 2680, 0, 90);
    }
    if (y >= 2680) {
        myData.var2 = map(y, 2680, 4095, 90, 180);
    }
    else if (y <= 2680) {
        myData.var2 = map(y, 0, 2680, 0, 90);
    }

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    Serial.println(myData.var1);
    if (result == ESP_OK) {
        Serial.println("esp_now_send initiated successfully");
    } else {
        Serial.print("esp_now_send failed with error: ");
        Serial.println(result);
    }

    // Broadcast data over WebSocket
    //String jsonData = "{\"x\":" + String(x) + ",\"y\":" + String(y) + "}";
    //webSocket.broadcastTXT(jsonData);

    delay(100);
}

 //c0:49:ef:44:d0:68 sender

 //64:b7:08:29:1b:68 reciever