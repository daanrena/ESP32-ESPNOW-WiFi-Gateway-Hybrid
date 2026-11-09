#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// MAC Address Gateway (Ganti dengan MAC Address ESP32 Gateway Anda)
// Untuk cek MAC: Serial.println(WiFi.macAddress()) di Gateway
uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // FF = Broadcast ke semua

// PENTING: Samakan channel ini dengan channel Wi-Fi router yang dideteksi Gateway
// (Contoh: jika router Gateway di Channel 6, set ini ke 6)
#define WIFI_CHANNEL 6

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

  // Set sebagai Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Kunci radio ke channel yang sama dengan Gateway!
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Gagal inisialisasi ESP-NOW");
    return;
  }

  // Daftarkan Gateway sebagai Peer
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Gagal menambahkan Peer");
    return;
  }
}

void loop() {
  // Simulasi pembacaan data sensor
  myData.node_id = 1;
  myData.temperature = 28.5 + (random(-10, 10) / 10.0);
  myData.humidity = 65.0 + (random(-5, 5));
  myData.battery_level = 92;

  // Kirim data ke Gateway
  esp_err_t result = esp_now_send(gatewayAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Data terkirim ke Gateway!");
  } else {
    Serial.println("Gagal kirim data (Beda Channel atau Gateway Mati)");
  }

  // Kirim tiap 5 detik (atau gunakan Deep Sleep untuk hemat baterai)
  delay(5000);
}
