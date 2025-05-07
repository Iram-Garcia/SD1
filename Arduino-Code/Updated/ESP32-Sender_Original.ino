#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <string.h>

// Trigger definitions
#define IMG_START "IMG_START"
#define IMG_END   "IMG_END"

// Flash pin definition (onboard LED on AI-Thinker ESP32-CAM)
#define FLASH_PIN 4

// Replace with the actual MAC address of your ESP32-WROOM
uint8_t receiverMacAddress[] = {0xCC, 0xDB, 0xA7, 0x92, 0x9D, 0x10};  // Update as needed

// Pin definitions
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

// Callback for ESP-NOW send status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(100); // Allow serial to stabilize

  // Initialize flash pin
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW); // Ensure flash is off initially

  // Initialize camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  
  config.jpeg_quality = 12;           // 10-63 (lower is higher quality)
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (true);  // Halt on error
  }

  // Initialize Wi-Fi in STA mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Ensure no prior connections
  delay(100); // Wait for Wi-Fi to stabilize
  Serial.print("ESP32-CAM MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true);  // Halt on error
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Add peer (ESP32-WROOM)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;   // Use default channel
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true);  // Halt if peer addition fails
  }
  Serial.println("Peer added successfully");
}

void loop() {
  // Turn on the flash
  digitalWrite(FLASH_PIN, HIGH);
  delay(100); // Short delay to simulate flash effect (adjust as needed)

  // Capture photo from the camera
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    digitalWrite(FLASH_PIN, LOW); // Turn off flash on failure
    return;
  }

  // Turn off the flash
  digitalWrite(FLASH_PIN, LOW);

  // Calculate total chunks
  const int chunkSize = 196; // Reduced to account for 4-byte sequence number
  uint32_t totalChunks = (fb->len + chunkSize - 1) / chunkSize;

  // Send IMG_START with total chunks
  uint8_t startMsg[16];
  memcpy(startMsg, IMG_START, strlen(IMG_START));
  startMsg[strlen(IMG_START)] = (totalChunks >> 24) & 0xFF;
  startMsg[strlen(IMG_START) + 1] = (totalChunks >> 16) & 0xFF;
  startMsg[strlen(IMG_START) + 2] = (totalChunks >> 8) & 0xFF;
  startMsg[strlen(IMG_START) + 3] = totalChunks & 0xFF;
  esp_err_t result = esp_now_send(receiverMacAddress, startMsg, strlen(IMG_START) + 4);
  if (result != ESP_OK) {
    Serial.println("Error sending IMG_START trigger");
  }
  delay(50); // Increased delay to allow receiver time to prepare

  // Send image data in chunks with sequence numbers
  const int maxRetries = 10;
  for (size_t i = 0; i < fb->len; i += chunkSize) {
    size_t len = min(chunkSize, (int)(fb->len - i));
    uint8_t packet[200]; // Total packet size: 4 bytes sequence + 196 bytes data
    // Add sequence number (i / chunkSize gives chunk index)
    uint32_t seq = i / chunkSize;
    packet[0] = (seq >> 24) & 0xFF;
    packet[1] = (seq >> 16) & 0xFF;
    packet[2] = (seq >> 8) & 0xFF;
    packet[3] = seq & 0xFF;
    // Copy image data
    memcpy(packet + 4, fb->buf + i, len);

    int retries = 0;
    do {
      result = esp_now_send(receiverMacAddress, packet, len + 4);
      if (result != ESP_OK) {
        Serial.print("Send error at offset ");
        Serial.print(i);
        Serial.println(", retrying...");
        delay(50); // Increased delay before retry
        retries++;
      }
    } while (result != ESP_OK && retries < maxRetries);

    if (result != ESP_OK) {
      Serial.printf("Failed to send packet at offset %u after %d retries\n", i, retries);
    }
    delay(50); // Increased delay between packets
  }

  // Send IMG_END trigger
  result = esp_now_send(receiverMacAddress, (uint8_t*)IMG_END, strlen(IMG_END));
  if (result != ESP_OK) {
    Serial.println("Error sending IMG_END trigger");
  }
  delay(50);

  Serial.printf("Sent image of size %u bytes in %u chunks\n", fb->len, totalChunks);

  // Return the frame buffer for reuse
  esp_camera_fb_return(fb);

  // Wait before capturing the next image
  delay(30000);
}