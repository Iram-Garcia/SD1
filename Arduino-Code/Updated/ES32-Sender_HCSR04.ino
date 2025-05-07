#include "esp_camera.h"
#include <WiFi.h>
#include <esp_now.h>
#include <string.h>

// Trigger definitions
#define IMG_START "IMG_START"
#define IMG_END   "IMG_END"

// Flash pin definition (onboard LED on AI-Thinker ESP32-CAM)
#define FLASH_PIN 4

// HC‑SR04 pins and motion parameters
#define TRIG_PIN        13   // GPIO13
#define ECHO_PIN        12   // GPIO12 (through voltage divider)
#define MAX_DISTANCE_CM 8    // Motion if object within ~8 cm

// Indicator LED pin (optional visual feedback)
#define INDICATOR_LED_PIN 15 // GPIO15

// Debounce interval between captures (ms)
const unsigned long CAPTURE_COOLDOWN = 5000;
unsigned long lastCapture = 0;

// Replace with the actual MAC address of your ESP32-WROOM receiver
uint8_t receiverMacAddress[] = {0xCC, 0xDB, 0xA7, 0x92, 0x9D, 0x10};

// Camera pin definitions (required by esp_camera)
#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

// Callback for ESP-NOW send status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Flash LED
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);

  // HC‑SR04 setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Indicator LED
  pinMode(INDICATOR_LED_PIN, OUTPUT);
  digitalWrite(INDICATOR_LED_PIN, LOW);

  // Camera init
  camera_config_t config;
  config.ledc_timer    = LEDC_TIMER_0;
  config.ledc_channel  = LEDC_CHANNEL_0;
  config.pin_d0        = Y2_GPIO_NUM;
  config.pin_d1        = Y3_GPIO_NUM;
  config.pin_d2        = Y4_GPIO_NUM;
  config.pin_d3        = Y5_GPIO_NUM;
  config.pin_d4        = Y6_GPIO_NUM;
  config.pin_d5        = Y7_GPIO_NUM;
  config.pin_d6        = Y8_GPIO_NUM;
  config.pin_d7        = Y9_GPIO_NUM;
  config.pin_xclk      = XCLK_GPIO_NUM;
  config.pin_pclk      = PCLK_GPIO_NUM;
  config.pin_vsync     = VSYNC_GPIO_NUM;
  config.pin_href      = HREF_GPIO_NUM;
  config.pin_sscb_sda  = SIOD_GPIO_NUM;
  config.pin_sscb_scl  = SIOC_GPIO_NUM;
  config.pin_pwdn      = PWDN_GPIO_NUM;
  config.pin_reset     = RESET_GPIO_NUM;
  config.xclk_freq_hz  = 20000000;
  config.pixel_format  = PIXFORMAT_JPEG;
  config.frame_size    = FRAMESIZE_VGA;
  config.jpeg_quality  = 12;
  config.fb_count      = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    while (true);
  }

  // Wi‑Fi + ESP‑NOW init
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP‑NOW init failed!");
    while (true);
  }
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receiverMacAddress, 6);
  peer.channel = 0;
  peer.encrypt = false;
  peer.ifidx   = WIFI_IF_STA;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer!");
    while (true);
  }

  Serial.println("Setup complete");
    digitalWrite(FLASH_PIN, HIGH);
    digitalWrite(FLASH_PIN, LOW);
}

void loop() {
  // 1) Trigger HC‑SR04
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 2) Measure echo
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance_cm = (duration > 0)
    ? (duration * 0.0343f / 2.0f)
    : -1.0f;

  // 3) Drive indicator LED
  bool motion = (distance_cm > 0 && distance_cm <= MAX_DISTANCE_CM);
  digitalWrite(INDICATOR_LED_PIN, motion ? HIGH : LOW);

  // 4) On motion + cooldown, capture & send
  if (motion && (millis() - lastCapture >= CAPTURE_COOLDOWN)) {
    lastCapture = millis();
    Serial.printf("Detected %.1f cm — capturing image\n", distance_cm);

    // flash
    digitalWrite(FLASH_PIN, HIGH);
    delay(100);

    camera_fb_t *fb = esp_camera_fb_get();
    digitalWrite(FLASH_PIN, LOW);
    if (!fb) {
      Serial.println("Capture failed");
      return;
    }

    // IMG_START
    const int chunkSize = 196;
    uint32_t totalChunks = (fb->len + chunkSize - 1) / chunkSize;
    uint8_t startMsg[16];
    memcpy(startMsg, IMG_START, strlen(IMG_START));
    startMsg[strlen(IMG_START)  ] = (totalChunks >> 24) & 0xFF;
    startMsg[strlen(IMG_START)+1] = (totalChunks >> 16) & 0xFF;
    startMsg[strlen(IMG_START)+2] = (totalChunks >>  8) & 0xFF;
    startMsg[strlen(IMG_START)+3] =  totalChunks        & 0xFF;
    esp_now_send(receiverMacAddress, startMsg, strlen(IMG_START)+4);
    delay(50);

    // send image chunks
    for (size_t i = 0; i < fb->len; i += chunkSize) {
      size_t len = min(chunkSize, (int)(fb->len - i));
      uint8_t packet[200];
      uint32_t seq = i / chunkSize;
      packet[0] = (seq >> 24) & 0xFF;
      packet[1] = (seq >> 16) & 0xFF;
      packet[2] = (seq >>  8) & 0xFF;
      packet[3] =  seq         & 0xFF;
      memcpy(packet + 4, fb->buf + i, len);

      int retries = 0;
      esp_err_t result;
      do {
        result = esp_now_send(receiverMacAddress, packet, len + 4);
        if (result != ESP_OK) {
          delay(50);
          retries++;
        }
      } while (result != ESP_OK && retries < 10);

      if (result != ESP_OK) {
        Serial.printf("Chunk %u failed after %d retries\n", seq, retries);
      }
      delay(50);
    }

    // IMG_END
    esp_now_send(receiverMacAddress, (uint8_t*)IMG_END, strlen(IMG_END));
    delay(50);
    Serial.printf("Image sent (%u bytes)\n", fb->len);

    esp_camera_fb_return(fb);
  }

  delay(100); // small idle
}
