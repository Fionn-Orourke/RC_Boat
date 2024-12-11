#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "homepage.h"
WebServer server(80);
#define xpin 32
#define ypin 35
uint8_t broadcastAddress[] = {0x64, 0xb7, 0x08, 0x29, 0x1b, 0x68};

const char* ssid = "Shibby";
const char* password = "12345678";

// Structure to hold the data
typedef struct struct_message {
    double var1;
    double var2;
} struct_message;

struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleADC() {
 int a = analogRead(xpin);
 String adcValue = String(myData.var1);
 
 server.send(200, "text/plane", adcValue);
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

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
    //webSocket.begin();
    //webSocket.onEvent(webSocketEvent);
    server.on("/", handleRoot);      //This is display page
    server.on("/readADC", handleADC);
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

    if (x >= 2780) {
        myData.var1 = map(x, 3080, 4095, 90, 180);
    }
    else if (x <= 2780) {
        myData.var1 = map(x, 0, 2780, 0, 90);
    }
    if (y >= 2880) {
        myData.var2 = map(y, 2880, 4095, 90, 180);
    }
    else if (y <= 2880) {
        myData.var2 = map(y, 0, 2880, 0, 90);
    }

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    Serial.println(myData.var1);

    // Broadcast data over WebSocket
    //String jsonData = "{\"x\":" + String(x) + ",\"y\":" + String(y) + "}";
    //webSocket.broadcastTXT(jsonData);

    delay(50);
}

 //c0:49:ef:44:d0:68 sender

 //64:b7:08:29:1b:68 reciever