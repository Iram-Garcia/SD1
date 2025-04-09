#include "esp_camera.h"
#include <esp_now.h>
#include <WiFi.h>

uint8_t receiverMac[] = {0xCC, 0xDB, 0xA7, 0x92, 0x9D, 0x10};

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define PACKET_SIZE 250

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  Serial.println("Initializing camera...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }
  Serial.println("Camera initialized");

  Serial.println("Starting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin("dummy_ssid", "dummy_password");
  delay(3000);
  Serial.println("Sender MAC: " + WiFi.macAddress());
  WiFi.disconnect();
  Serial.println("WiFi ready");

  Serial.println("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  Serial.println("ESP-NOW initialized");

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  Serial.println("Adding peer...");
  err = esp_now_add_peer(&peerInfo);
  if (err != ESP_OK) {
    Serial.printf("Peer add failed with error: 0x%x\n", err);
    return;
  }
  Serial.println("Peer added successfully");

  sendPhoto();
}

void sendPhoto() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed");
    return;
  }

  Serial.printf("Image size: %u bytes\n", fb->len);

  uint8_t sizePacket[4] = {(fb->len >> 24) & 0xFF, (fb->len >> 16) & 0xFF, 
                          (fb->len >> 8) & 0xFF, fb->len & 0xFF};
  Serial.println("Sending size packet...");
  esp_now_send(receiverMac, sizePacket, 4);
  delay(10);

  uint8_t packet[PACKET_SIZE];
  int packetCount = 0;
  for (size_t i = 0; i < fb->len; i += PACKET_SIZE - 2) {
    size_t chunkSize = min<size_t>(PACKET_SIZE - 2, fb->len - i);
    packet[0] = (i >> 8) & 0xFF;
    packet[1] = i & 0xFF;
    memcpy(&packet[2], fb->buf + i, chunkSize);
    esp_now_send(receiverMac, packet, chunkSize + 2);
    packetCount++;
    delay(10); // Slower for reliability
  }

  Serial.printf("Sent %d packets\n", packetCount);
  esp_camera_fb_return(fb);
  Serial.println("Photo sent");
}

void loop() {
  delay(10000);
  sendPhoto();
}