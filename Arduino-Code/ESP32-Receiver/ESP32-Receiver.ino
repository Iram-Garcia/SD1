#include <WiFi.h>
#include <esp_now.h>
#include <string.h>

// Trigger definitions
#define IMG_START "IMG_START"
#define IMG_END   "IMG_END"

// Buffer to store incoming data
uint8_t* imageBuffer = nullptr;  // Dynamic buffer
size_t totalChunks = 0;
size_t receivedChunks = 0;
bool receivingImage = false;

// Updated callback function to match new ESP-NOW API
void OnDataRecv(const esp_now_recv_info *recvInfo, const uint8_t *incomingData, int len) {
  // Check for IMG_START
  if (len == strlen(IMG_START) + 4 && memcmp(incomingData, IMG_START, strlen(IMG_START)) == 0) {
    // Extract total chunks
    totalChunks = (incomingData[strlen(IMG_START)] << 24) |
                  (incomingData[strlen(IMG_START) + 1] << 16) |
                  (incomingData[strlen(IMG_START) + 2] << 8) |
                  incomingData[strlen(IMG_START) + 3];
    // Allocate buffer dynamically
    imageBuffer = (uint8_t*)malloc(totalChunks * 196);
    if (!imageBuffer) {
      Serial.println("Failed to allocate buffer");
      return;
    }
    receivingImage = true;
    receivedChunks = 0;
    Serial.println("Received IMG_START, expecting " + String(totalChunks) + " chunks");
    return;
  }

  // Check for IMG_END
  if (len == strlen(IMG_END) && memcmp(incomingData, IMG_END, strlen(IMG_END)) == 0) {
    if (receivingImage && receivedChunks == totalChunks) {
      // Send the complete image to Serial
      Serial.write(imageBuffer, totalChunks * 196);
      Serial.println("\nImage sent to Serial");
    } else {
      Serial.println("Image incomplete: received " + String(receivedChunks) + "/" + String(totalChunks) + " chunks");
    }
    free(imageBuffer);
    imageBuffer = nullptr;
    receivingImage = false;
    return;
  }

  // Process image chunk
  if (receivingImage && len > 4) {
    // Extract sequence number
    uint32_t seq = (incomingData[0] << 24) | (incomingData[1] << 16) | (incomingData[2] << 8) | incomingData[3];
    if (seq < totalChunks) {
      // Copy data to correct position in buffer
      memcpy(imageBuffer + (seq * 196), incomingData + 4, len - 4);
      receivedChunks++;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi in STA mode
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Set up complete!");
}

void loop() {
  delay(100); // Keep loop alive
}