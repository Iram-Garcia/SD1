#include <esp_now.h>
#include <WiFi.h>

volatile bool receivingImage = false;
volatile uint32_t expectedSize = 0;
volatile uint32_t receivedSize = 0;
volatile int packetCount = 0;

void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  if (len == 4 && !receivingImage) {
    expectedSize = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    receivedSize = 0;
    packetCount = 0;
    receivingImage = true;
    Serial.println("START_IMAGE");
    Serial.printf("Expected size: %u bytes\n", expectedSize);
    return;
  }

  if (len < 3 || !receivingImage) return;

  packetCount++;
  uint16_t offset = (data[0] << 8) | data[1];
  size_t chunkSize = len - 2;
  receivedSize += chunkSize;

  // Serial.write(data, len); // Comment out to reduce load
  Serial.printf("Packet %d, Offset: %u, Chunk size: %u\n", packetCount, offset, chunkSize);

  if (receivedSize >= expectedSize) {
    receivingImage = false;
    Serial.println("END_IMAGE");
    Serial.printf("Total packets: %d, Total size: %u\n", packetCount, receivedSize);
  }
}

void setup() {
  Serial.begin(921600);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin("dummy_ssid", "dummy_password");
  delay(2000);
  Serial.println("Receiver MAC: " + WiFi.macAddress());
  WiFi.disconnect();
  WiFi.channel(1);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  static unsigned long lastPacketTime = 0;
  static unsigned long lastPrintTime = 0;

  if (receivingImage && (millis() - lastPacketTime > 5000)) {
    receivingImage = false;
    Serial.println("END_IMAGE (timeout)");
    Serial.printf("Received %d packets, %u bytes\n", packetCount, receivedSize);
    expectedSize = 0;
    receivedSize = 0;
    packetCount = 0;
  }

  if (receivingImage) {
    lastPacketTime = millis();
  }

  if (millis() - lastPrintTime >= 5000) {
    Serial.println("Listening...");
    lastPrintTime = millis();
  }

  delay(10);
}