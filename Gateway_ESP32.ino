#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>  // 1. Library Watchdog anti-hang

#define WDT_TIMEOUT 15     // Reset otomatis jika sistem freeze > 15 detik

// Konfigurasi Wi-Fi Router
const char* ssid = "NAMA_WIFI_ROUTER";
const char* password = "PASSWORD_WIFI";

// Struktur data (Wajib sama persis dengan sensor)
typedef struct struct_message {
  int node_id;
  float temperature;
  float humidity;
  int battery_level;
} struct_message;

struct_message incomingData;
volatile bool newDataAvailable = false;

// Timer non-blocking untuk cek Wi-Fi (anti delay)
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 10000; // Cek tiap 10 detik

// Callback ESP-NOW
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  newDataAvailable = true; // Setel bendera untuk diproses di loop
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n=== ESP-NOW to Wi-Fi Gateway 24/7 ===");

  // 1. Setup Wi-Fi
  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true); // Fitur otomatis ESP32 untuk nyambung lagi kalau putus
  WiFi.setSleep(false);        // Radio selalu aktif penuh, sensitivitas penerimaan maksimal
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
    Serial.println(WiFi.channel()); // Channel ini yang harus disamakan di Sensor Node
  } else {
    Serial.println("\n[WARN] Wi-Fi gagal konek saat boot, tetap lanjut mode offline.");
  }

  // 2. Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] Gagal inisialisasi ESP-NOW, me-restart...");
    ESP.restart(); // Auto-recovery
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[OK] ESP-NOW Siap Menerima Data!");

  // 3. Aktifkan Hardware Watchdog
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  Serial.println("[OK] Watchdog 24/7 Siap.");
}

void loop() {
  // Reset watchdog timer tiap siklus (tanda sistem sehat)
  esp_task_wdt_reset();

  // 1. Tangani data yang masuk dari Node Sensor
  if (newDataAvailable) {
    newDataAvailable = false;

    Serial.println("\n--- Data Masuk dari Node Sensor ---");
    Serial.printf("Node ID     : %d\n", incomingData.node_id);
    Serial.printf("Suhu        : %.2f °C\n", incomingData.temperature);
    Serial.printf("Kelembapan  : %.2f %%\n", incomingData.humidity);
    Serial.printf("Baterai     : %d %%\n", incomingData.battery_level);

    // Di sini tempat aman buat kirim ke MQTT, Firebase, atau Web API
    // Tidak akan bikin crash karena dieksekusi di loop(), bukan di interupsi
  }

  // 2. Pengecekan Wi-Fi tanpa 'delay()' (Non-blocking)
  if (millis() - lastWiFiCheck >= wifiCheckInterval) {
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[INFO] Wi-Fi terputus, mencoba rekoneksi...");
      WiFi.reconnect();
    }
  }

  // Berikan waktu jeda mikro untuk background task FreeRTOS
  delay(10);
}
