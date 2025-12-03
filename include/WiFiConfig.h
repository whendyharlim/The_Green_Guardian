#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <WiFiManager.h>

// ==================== WIFIMANAGER CONFIGURATION ====================
#define WM_PORTAL_TIMEOUT 180  // Timeout portal WiFi Manager (detik)
#define WM_AP_NAME "ESP32-GreenGuardian"  // Nama AP untuk konfigurasi
#define WM_AP_PASSWORD "12345678"  // Password AP (minimal 8 karakter)

// ==================== CUSTOM PARAMETERS ====================
// Parameter kustom untuk Firebase yang akan muncul di WiFi Manager
extern WiFiManagerParameter custom_firebase_host;
extern WiFiManagerParameter custom_firebase_auth;

// ==================== WIFI FUNCTIONS ====================
/**
 * @brief Inisialisasi WiFiManager dengan custom parameters untuk Firebase
 * @return true jika berhasil connect, false jika timeout atau gagal
 */
bool initWiFiManager();

/**
 * @brief Reset WiFi settings (untuk reconfigure)
 */
void resetWiFiSettings();

/**
 * @brief Get Firebase Host dari WiFiManager parameter
 * @return String Firebase host
 */
String getFirebaseHost();

/**
 * @brief Get Firebase Auth dari WiFiManager parameter
 * @return String Firebase auth token
 */
String getFirebaseAuth();

/**
 * @brief Cek status WiFi connection
 * @return true jika terhubung, false jika tidak
 */
bool isWiFiConnected();

/**
 * @brief Get WiFi RSSI (signal strength)
 * @return RSSI value in dBm
 */
int getWiFiRSSI();

/**
 * @brief Reconnect WiFi jika terputus
 * @return true jika berhasil reconnect
 */
bool reconnectWiFi();

#endif // WIFI_CONFIG_H
