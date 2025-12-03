#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <Arduino.h>
#include <Firebase_ESP_Client.h>

// ==================== FIREBASE CONFIGURATION ====================
// Kosongkan kredensial ini, akan diisi melalui WiFiManager
#define FIREBASE_HOST ""  // Contoh: "your-project.firebaseio.com"
#define FIREBASE_AUTH ""  // Database secret atau empty string untuk rules "public"

// ==================== FIREBASE PATHS ====================
// Definisikan path untuk data di Realtime Database
#define FB_PATH_ENVIRONMENT "/sensors/environment"
#define FB_PATH_SOIL "/sensors/soil"
#define FB_PATH_DEVICE_STATUS "/device/status"
#define FB_PATH_TIMESTAMP "/device/lastUpdate"

// ==================== FIREBASE OBJECTS ====================
extern FirebaseData firebaseData;
extern FirebaseConfig firebaseConfig;
extern FirebaseAuth firebaseAuth;

// ==================== FIREBASE FUNCTIONS ====================
/**
 * @brief Inisialisasi Firebase dengan WiFiManager parameters
 * @param host Firebase Realtime Database host
 * @param auth Firebase authentication token/secret
 * @return true jika berhasil, false jika gagal
 */
bool initFirebase(const char* host, const char* auth);

/**
 * @brief Kirim data environment ke Firebase
 * @param temp Suhu udara (°C)
 * @param humidity Kelembaban udara (%RH)
 * @param pressure Tekanan udara (hPa)
 * @param light Intensitas cahaya (lux)
 * @param isRaining Status hujan (true/false)
 * @return true jika berhasil, false jika gagal
 */
bool sendEnvironmentData(float temp, float humidity, float pressure, float light, bool isRaining);

/**
 * @brief Kirim data tanah ke Firebase
 * @param soilMoisture Kelembaban tanah (%RH)
 * @param soilTemp Suhu tanah (°C)
 * @param ec Konduktivitas (uS/cm)
 * @param ph pH tanah
 * @param nitrogen Nitrogen (mg/kg)
 * @param phosphorus Phosphorus (mg/kg)
 * @param potassium Potassium (mg/kg)
 * @return true jika berhasil, false jika gagal
 */
bool sendSoilData(float soilMoisture, float soilTemp, float ec, float ph, 
                  int nitrogen, int phosphorus, int potassium);

/**
 * @brief Update device status dan timestamp
 * @param status Status device (online/offline/error)
 * @return true jika berhasil, false jika gagal
 */
bool updateDeviceStatus(const char* status);

/**
 * @brief Kirim semua data sensor ke Firebase dalam satu batch
 * @param envData Struct data environment
 * @param npkData Struct data NPK (optional, bisa NULL)
 * @return true jika berhasil, false jika gagal
 */
bool sendAllSensorData(const void* envData, const void* npkData = nullptr);

/**
 * @brief Cek koneksi Firebase
 * @return true jika terhubung, false jika tidak
 */
bool isFirebaseConnected();

/**
 * @brief Get Firebase error message
 * @return String berisi pesan error terakhir
 */
String getFirebaseError();

#endif // FIREBASE_CONFIG_H
