#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Konfigurasi Wi-Fi Router Rumah / Kantor
const char* ssid = "NAMA_WIFI_ROUTER";
const char* password = "PASSWORD_WIFI";

// Struktur data yang dikirim dan diterima (HARUS SAMA di kedua alat)
typedef struct struct_message {
  int node_id;
  float temperature;
  float humidity;
  int battery_level;
} struct_message;

struct_message incomingData;

// Buffer agar pemrosesan tidak menabrak interupsi Wi-Fi (Anti-Crash)
volatile bool newDataAvailable = false;

// Callback saat data ESP-NOW diterima
// Kompatibel dengan ESP32 Board Manager versi baru maupun lama
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  newDataAvailable = true; // Set flag untuk diproses di void loop
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP-NOW to Wi-Fi Gateway ===");

  // 1. Konek ke Wi-Fi Router
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke Wi-Fi");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 25) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Terhubung ke Router!");
    Serial.print("IP Gateway: ");
    Serial.println(WiFi.localIP());
    Serial.print("Operating Radio Channel: ");
    Serial.println(WiFi.channel()); // Channel yang dipaksakan oleh Router
  } else {
    Serial.println("\n[WARN] Wi-Fi tidak terhubung, berjalan di mode offline.");
  }

  // 2. Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] Gagal inisialisasi ESP-NOW!");
    return;
  }

  // Daftarkan callback penerima
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[OK] ESP-NOW Receiver Siap Mendengarkan!");
}

void loop() {
  // Proses data hanya di loop (bukan di dalam interupsi callback)
  if (newDataAvailable) {
    newDataAvailable = false;

    Serial.println("\n--- Data Masuk dari Node Sensor ---");
    Serial.printf("Node ID     : %d\n", incomingData.node_id);
    Serial.printf("Suhu        : %.2f °C\n", incomingData.temperature);
    Serial.printf("Kelembapan  : %.2f %%\n", incomingData.humidity);
    Serial.printf("Baterai     : %d %%\n", incomingData.battery_level);

    // Di sini Anda bisa teruskan ke MQTT, Firebase, atau HTTP Request
    // Contoh: sendToMQTT(incomingData);
  }

  // Auto-reconnect Wi-Fi jika koneksi router sempat drop
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
  }
}
