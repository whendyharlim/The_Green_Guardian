# The Green Guardian - ESP32-S3 Smart Agriculture System

## 📋 Deskripsi
Sistem monitoring pertanian pintar menggunakan ESP32-S3 dengan integrasi Firebase Realtime Database dan WiFiManager untuk konfigurasi mudah.

## 🔧 Hardware
- **ESP32-S3 DevKit**
- **BME280** - Sensor suhu, kelembaban udara, dan tekanan
- **BH1750 (GY-30)** - Sensor intensitas cahaya
- **Rain Sensor** - Sensor deteksi hujan

## 📚 Library Dependencies
```ini
- Adafruit BME280 Library (^2.3.0)
- Adafruit Unified Sensor (^1.1.4)
- BH1750 (^1.3.0)
- Firebase Arduino Client Library for ESP8266 and ESP32 (^4.4.14)
- WiFiManager (^2.0.17)
```

## 🔌 Pin Configuration
| Komponen | Pin ESP32-S3 |
|----------|--------------|
| I2C SDA  | GPIO 11      |
| I2C SCL  | GPIO 12      |
| Rain Sensor | GPIO 10   |

## 🚀 Cara Setup

### 1. Upload Code ke ESP32-S3
```bash
pio run --target upload
```

### 2. Konfigurasi WiFi & Firebase
Saat pertama kali dinyalakan:
1. ESP32 akan membuat WiFi Access Point bernama: **ESP32-GreenGuardian**
2. Password AP: **12345678**
3. Connect ke AP tersebut dari smartphone/laptop
4. Browser akan otomatis membuka portal konfigurasi (jika tidak, buka http://192.168.4.1)
5. Pilih WiFi yang ingin diconnect dan masukkan password
6. Isi konfigurasi Firebase:
   - **Firebase Host**: `your-project.firebaseio.com` (tanpa https://)
   - **Firebase Auth Token**: Database Secret atau kosongkan jika menggunakan public rules

### 3. Firebase Realtime Database Setup
1. Buka Firebase Console: https://console.firebase.google.com
2. Pilih atau buat project baru
3. Aktifkan **Realtime Database**
4. Copy Database URL (contoh: `your-project.firebaseio.com`)
5. **Database Rules** (untuk testing, gunakan public rules):
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```
   ⚠️ **WARNING**: Rules di atas untuk testing saja! Gunakan rules yang lebih secure untuk production.

### 4. Mendapatkan Firebase Auth Token (Optional)
Jika ingin menggunakan Database Secret:
1. Project Settings > Service Accounts
2. Database Secrets > Show (jika tidak ada, klik Add secret)
3. Copy token dan masukkan saat konfigurasi WiFiManager

## 📊 Struktur Data di Firebase
```
/sensors
  ├── /environment
  │   ├── temperature: 28.5
  │   ├── humidity: 65.2
  │   ├── pressure: 1013.25
  │   ├── light: 1520.5
  │   ├── raining: false
  │   └── timestamp: 1234567890
  │
  └── /device
      ├── status: "online"
      ├── uptime: 3600
      └── timestamp: 1234567890
```

## 📱 Monitoring Data
Data dapat diakses melalui:
1. **Firebase Console** - Realtime Database tab
2. **REST API** - `https://your-project.firebaseio.com/sensors.json`
3. **Custom Web/Mobile App** - Gunakan Firebase SDK

## ⚙️ Konfigurasi Interval
Edit di `main.cpp`:
```cpp
#define SENSOR_READ_INTERVAL 5000     // Baca sensor setiap 5 detik
#define FIREBASE_SEND_INTERVAL 15000  // Kirim ke Firebase setiap 15 detik
#define WIFI_CHECK_INTERVAL 30000     // Cek WiFi setiap 30 detik
```

## 🔄 Reset Konfigurasi WiFi
Jika ingin mengubah WiFi atau Firebase config:
1. Tekan tombol **RESET** pada ESP32
2. Atau panggil fungsi `resetWiFiSettings()` dalam code

## 📖 API Functions

### WiFiConfig.h
```cpp
bool initWiFiManager();        // Inisialisasi WiFiManager
void resetWiFiSettings();      // Reset WiFi settings
String getFirebaseHost();      // Get Firebase host dari config
String getFirebaseAuth();      // Get Firebase auth dari config
bool isWiFiConnected();        // Cek status WiFi
int getWiFiRSSI();            // Get WiFi signal strength
bool reconnectWiFi();         // Reconnect WiFi jika terputus
```

### FirebaseConfig.h
```cpp
bool initFirebase(host, auth);                    // Init Firebase
bool sendEnvironmentData(...);                    // Kirim data environment
bool sendSoilData(...);                          // Kirim data tanah
bool updateDeviceStatus(status);                 // Update device status
bool sendAllSensorData(envData, npkData);        // Kirim semua data batch
bool isFirebaseConnected();                      // Cek koneksi Firebase
String getFirebaseError();                       // Get error message
```

## 🐛 Troubleshooting

### WiFi tidak connect
- Pastikan password WiFi benar
- Cek jarak antara ESP32 dengan router
- Reset konfigurasi dan coba lagi

### Firebase error "host not found"
- Pastikan format host benar: `your-project.firebaseio.com` (tanpa https://)
- Pastikan WiFi sudah terhubung internet

### Firebase error "permission denied"
- Cek Database Rules di Firebase Console
- Pastikan rules sudah diset dengan benar

### Sensor tidak terbaca
- Cek koneksi I2C (SDA & SCL)
- Cek power supply sensor (3.3V)
- Scan I2C address dengan I2C scanner sketch

## 📝 To-Do List
- [ ] Tambahkan sensor NPK 7-in-1 via RS485
- [ ] Implementasi deep sleep untuk hemat power
- [ ] Tambahkan OTA (Over-The-Air) update
- [ ] Buat web dashboard untuk monitoring
- [ ] Tambahkan notifikasi push

## 👨‍💻 Developer
**The Green Guardian Team**

## 📄 License
MIT License

---
**Note**: Konfigurasi Firebase (API Key & Auth Token) disimpan di ESP32 memory. Pastikan tidak share credentials ke public repository!
