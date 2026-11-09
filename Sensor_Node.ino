#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>  // 1. Tambah library Watchdog

#define WDT_TIMEOUT 10     // Reset otomatis jika macet > 10 detik
#define WIFI_CHANNEL 6

uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  int node_id;
  float temperature;
  float humidity;
  int battery_level;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);

  // 2. Set Wi-Fi STA dan MATIKAN mode hemat daya antena (Biar sinyal selalu ON 24/7)
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); 

  // Kunci channel radio
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Gagal inisialisasi ESP-NOW, me-restart...");
    ESP.restart(); // Auto-recovery jika gagal
  }

  // Daftarkan Peer
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Gagal tambah Peer, me-restart...");
    ESP.restart();
  }

  // 3. Aktifkan Hardware Watchdog Timer
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  Serial.println("Sensor Node Siap & Watchdog Aktif 24/7.");
}

void loop() {
  // Reset watchdog tiap siklus (tanda sistem sehat)
  esp_task_wdt_reset();

  // Baca / siapkan data sensor
  myData.node_id = 1;
  myData.temperature = 28.5 + (random(-10, 10) / 10.0);
  myData.humidity = 65.0 + (random(-5, 5));
  myData.battery_level = 95;

  // Kirim data
  esp_err_t result = esp_now_send(gatewayAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Data terkirim ke Gateway!");
  } else {
    Serial.println("Gagal kirim data (Beda Channel atau Gateway Mati)");
  }

  // Jeda 5 detik
  delay(5000);
}
