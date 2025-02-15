#define xpin 32
#define ypin 35

#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include "homepage.h"






// WiFi and server setup
;const char* ssid = "Shibby";
const char* password = "12345678";
WebServer server(80);
double xtilt = 8;

uint8_t broadcastAddress[] = {0x64, 0xb7, 0x08, 0x29, 0x1b, 0x68};
// Structure for outgoing data
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
     


// ESP-NOW callbacks
void OnDataRecv(const esp_now_recv_info* info, const uint8_t* incomingData, int len) {
    memcpy(&myData_in, incomingData, sizeof(myData_in));
    dataReceived = true;
    Serial.print("Received val1: ");
    Serial.println(myData_in.val1);
    Serial.print("Received val2: ");
    Serial.println(myData_in.val2);
    xtilt = myData_in.val2;
    Serial.print(myData_in.val2);
    
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

// Web server handlers
void handleRoot() {
    String s = MAIN_page; // Read HTML contents from homepage.h
    server.send(200, "text/html", s);
}

String handleADC() {
    // Respond with ADC reading for tilt data (myData.var1)
    String tilt = String(myData.var1);
    server.send(200, "text/plain", tilt);
    return tilt;
}

String handleServo() {
    // Respond with servo value (myData_in.val1)
    String servoValue = String(myData_in.val2+90);
    server.send(200, "text/plain", servoValue);
    return servoValue;
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    //Wire.begin();

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //esp_wifi_set_promiscuous(true);
    //esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);  
    //esp_wifi_set_promiscuous(false);


    // ESP-NOW initialization
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = WiFi.channel();
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
    else{
      Serial.println("peer added");
    }

    // Register the callback for data sent/received
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

    // HTTP server route setup
    server.on("/", handleRoot); 
    server.on("/readADC", handleADC);  // Endpoint for ADC values
    server.on("/readServo", handleServo);  // Endpoint for servo values
    server.begin();  // Start server
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    // Read analog values from pins for tilt data (xpin, ypin)
    int x = analogRead(xpin);
    int y = analogRead(ypin);

    // Map ADC values to angles (for tilt)
    if (x >= 2680) {
        myData.var1 = map(x, 2680, 4095, 90, 180);
    } else {
        myData.var1 = map(x, 0, 2680, 0, 90);
    }

    if (y >= 2680) {
        myData.var2 = map(y, 2680, 4095, 90, 180);
    } else {
        myData.var2 = map(y, 0, 2680, 0, 90);
    }

    // Send data over ESP-NOW to the peer device
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
        Serial.println("ESP-NOW send initiated successfully");
    } else {
        Serial.print("ESP-NOW send failed with error: ");
        Serial.println(result);
    }

    delay(50); 
}

 //c0:49:ef:44:d0:68 sender

 //64:b7:08:29:1b:68 reciever
