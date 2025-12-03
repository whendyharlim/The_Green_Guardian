// // sensor BME280 dan BH1750 (GY-30) dengan ESP32 s3 nano
// // +sensor NPK 7-in-1 ditambahkan
// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
// #include <BH1750.h>
// #include <HardwareSerial.h>

// // ==================== KONFIGURASI PIN ====================
// // I2C untuk GY-30 dan BME280
// #define I2C_SDA 11
// #define I2C_SCL 12

// // UART untuk RS485 (NPK Sensor)
// // COBA SWAP RX/TX JIKA TIDAK DAPAT DATA!
// #define RS485_RX 43  // SWAP: Coba 43 jika 44 tidak work
// #define RS485_TX 44  // SWAP: Coba 44 jika 43 tidak work

// // Pin alternatif jika masih error:
// // #define RS485_RX 17
// // #define RS485_TX 18

// // Optional: DE/RE Control Pin untuk RS485 (jika modul XY-485 punya pin ini)
// // Uncomment jika modul Anda punya pin DE/RE
// // #define RS485_DE_RE 10  // Pin untuk kontrol DE/RE

// // Digital Pin untuk Rain Sensor
// #define RAIN_SENSOR_PIN 10

// // ==================== KONFIGURASI SENSOR ====================
// // Modbus Config untuk NPK 7-in-1 Sensor (Model China standar)
// #define SENSOR_ADDRESS 0x01  // Default: 0x01 (jika gagal coba: 0x02, 0x03, 0xFF)
// #define BAUD_RATE 4800       // Sensor NPK 7-in-1 kebanyakan pakai 4800!
// // Jika 4800 gagal, coba: 9600
// #define READ_HOLDING_REGISTERS 0x03
// #define READ_INPUT_REGISTERS 0x04  // Beberapa sensor pakai function 0x04

// // Register addresses NPK Sensor
// #define REG_MOISTURE    0x0000
// #define REG_TEMPERATURE 0x0001
// #define REG_EC          0x0002
// #define REG_PH          0x0003
// #define REG_NITROGEN    0x0004
// #define REG_PHOSPHORUS  0x0005
// #define REG_POTASSIUM   0x0006

// // ==================== OBJEK SENSOR ====================
// Adafruit_BME280 bme;
// BH1750 lightMeter;
// HardwareSerial rs485(1); // UART1 untuk RS485

// // ==================== STRUKTUR DATA ====================
// struct NPKData {
//   float soilMoisture;
//   float soilTemp;
//   float ec;
//   float ph;
//   int nitrogen;
//   int phosphorus;
//   int potassium;
//   bool valid;
// };

// struct EnvironmentData {
//   float airTemp;
//   float airHumidity;
//   float pressure;
//   float lightLevel;
//   bool isRaining;
//   bool bmeValid;
//   bool lightValid;
// };

// NPKData npkData;
// EnvironmentData envData;

// // ==================== FUNGSI CRC MODBUS ====================
// uint16_t calculateCRC(uint8_t *data, uint8_t length) {
//   uint16_t crc = 0xFFFF;
//   for (uint8_t i = 0; i < length; i++) {
//     crc ^= data[i];
//     for (uint8_t j = 0; j < 8; j++) {
//       if (crc & 0x0001) {
//         crc >>= 1;
//         crc ^= 0xA001;
//       } else {
//         crc >>= 1;
//       }
//     }
//   }
//   return crc;
// }

// // ==================== FUNGSI BACA MODBUS ====================
// bool readModbusRegisters(uint8_t address, uint16_t startReg, uint16_t numRegs, uint16_t *data) {
//   uint8_t request[8];
//   uint8_t response[64];
  
//   // Buat Modbus request
//   request[0] = address;
//   request[1] = READ_HOLDING_REGISTERS;
//   request[2] = (startReg >> 8) & 0xFF;
//   request[3] = startReg & 0xFF;
//   request[4] = (numRegs >> 8) & 0xFF;
//   request[5] = numRegs & 0xFF;
  
//   uint16_t crc = calculateCRC(request, 6);
//   request[6] = crc & 0xFF;
//   request[7] = (crc >> 8) & 0xFF;
  
//   // Clear buffer sebelum kirim
//   while(rs485.available()) rs485.read();
  
//   // === DEBUGGING: Print request yang dikirim ===
//   Serial.print("TX: ");
//   for(int i = 0; i < 8; i++) {
//     Serial.printf("%02X ", request[i]);
//   }
//   Serial.println();
  
//   // Kirim request
//   rs485.write(request, 8);
//   rs485.flush(); // Tunggu sampai data selesai dikirim
//   delay(200);    // Tunggu lebih lama untuk response
  
//   // Baca response
//   uint8_t index = 0;
//   unsigned long startTime = millis();
//   while (millis() - startTime < 2000) { // Timeout diperpanjang ke 2 detik
//     if (rs485.available()) {
//       response[index++] = rs485.read();
//       if (index >= 5 + numRegs * 2 + 2) break;
//     }
//   }
  
//   // === DEBUGGING: Print response yang diterima ===
//   if (index > 0) {
//     Serial.print("RX: ");
//     for(int i = 0; i < index; i++) {
//       Serial.printf("%02X ", response[i]);
//     }
//     Serial.printf("(%d bytes)\n", index);
//   } else {
//     Serial.println("RX: NO DATA (Timeout!)");
//   }
  
//   if (index < 5 + numRegs * 2 + 2) {
//     Serial.println("ERROR: Response timeout atau data kurang");
//     return false;
//   }
  
//   if (response[0] != address || response[1] != READ_HOLDING_REGISTERS) {
//     Serial.println("ERROR: Response address/function code salah");
//     return false;
//   }
  
//   uint16_t receivedCRC = response[index-2] | (response[index-1] << 8);
//   uint16_t calculatedCRC = calculateCRC(response, index - 2);
//   if (receivedCRC != calculatedCRC) {
//     Serial.printf("ERROR: CRC mismatch (RX: %04X, Calc: %04X)\n", receivedCRC, calculatedCRC);
//     return false;
//   }
  
//   for (uint16_t i = 0; i < numRegs; i++) {
//     data[i] = (response[3 + i*2] << 8) | response[4 + i*2];
//   }
  
//   return true;
// }

// // ==================== BACA NPK SENSOR ====================
// void readNPKSensor() {
//   uint16_t registers[7];
  
//   // Coba baca dengan Function Code 0x03 (Holding Registers)
//   if (readModbusRegisters(SENSOR_ADDRESS, REG_MOISTURE, 7, registers)) {
//     npkData.soilMoisture = registers[0] / 10.0;
//     npkData.soilTemp = registers[1] / 10.0;
//     npkData.ec = registers[2];
//     npkData.ph = registers[3] / 10.0;
//     npkData.nitrogen = registers[4];
//     npkData.phosphorus = registers[5];
//     npkData.potassium = registers[6];
//     npkData.valid = true;
//   } else {
//     // Jika gagal dengan 0x03, coba Function Code 0x04 (Input Registers)
//     Serial.println("Trying Function Code 0x04...");
    
//     uint8_t request[8];
//     uint8_t response[64];
    
//     request[0] = SENSOR_ADDRESS;
//     request[1] = READ_INPUT_REGISTERS;  // 0x04
//     request[2] = 0x00;
//     request[3] = 0x00;
//     request[4] = 0x00;
//     request[5] = 0x07;
    
//     uint16_t crc = calculateCRC(request, 6);
//     request[6] = crc & 0xFF;
//     request[7] = (crc >> 8) & 0xFF;
    
//     while(rs485.available()) rs485.read();
//     rs485.write(request, 8);
//     rs485.flush();
//     delay(200);
    
//     uint8_t index = 0;
//     unsigned long startTime = millis();
//     while (millis() - startTime < 2000) {
//       if (rs485.available()) {
//         response[index++] = rs485.read();
//         if (index >= 19) break;
//       }
//     }
    
//     if (index >= 19 && response[0] == SENSOR_ADDRESS && response[1] == READ_INPUT_REGISTERS) {
//       npkData.soilMoisture = ((response[3] << 8) | response[4]) / 10.0;
//       npkData.soilTemp = ((response[5] << 8) | response[6]) / 10.0;
//       npkData.ec = (response[7] << 8) | response[8];
//       npkData.ph = ((response[9] << 8) | response[10]) / 10.0;
//       npkData.nitrogen = (response[11] << 8) | response[12];
//       npkData.phosphorus = (response[13] << 8) | response[14];
//       npkData.potassium = (response[15] << 8) | response[16];
//       npkData.valid = true;
//       Serial.println("✓ Success with Function Code 0x04!");
//     } else {
//       npkData.valid = false;
//     }
//   }
// }

// // ==================== BACA BME280 ====================
// void readBME280() {
//   if (envData.bmeValid) {
//     envData.airTemp = bme.readTemperature();
//     envData.airHumidity = bme.readHumidity();
//     envData.pressure = bme.readPressure() / 100.0F; // Convert to hPa
//   }
// }

// // ==================== BACA GY-30 (BH1750) ====================
// void readLightSensor() {
//   if (envData.lightValid) {
//     envData.lightLevel = lightMeter.readLightLevel();
//   }
// }

// // ==================== BACA RAIN SENSOR ====================
// void readRainSensor() {
//   // Rain sensor: LOW = hujan terdeteksi, HIGH = tidak hujan
//   envData.isRaining = (digitalRead(RAIN_SENSOR_PIN) == LOW);
// }

// // ==================== PRINT DATA KE SERIAL ====================
// void printAllSensorData() {
//   Serial.println("\n╔════════════════════════════════════════════════╗");
//   Serial.println("║     ESP32-S3 NANO MULTI SENSOR MONITOR         ║");
//   Serial.println("╚════════════════════════════════════════════════╝");
  
//   // Environment Data (BME280 & GY-30)
//   Serial.println("\n┌─── KONDISI LINGKUNGAN ───────────────────────┐");
//   if (envData.bmeValid) {
//     Serial.printf("│ Suhu Udara      : %.2f °C\n", envData.airTemp);
//     Serial.printf("│ Kelembaban Udara: %.2f %%RH\n", envData.airHumidity);
//     Serial.printf("│ Tekanan Udara   : %.2f hPa\n", envData.pressure);
//   } else {
//     Serial.println("│ BME280: ERROR - Sensor tidak terbaca!");
//   }
  
//   if (envData.lightValid) {
//     Serial.printf("│ Intensitas Cahaya: %.2f lux\n", envData.lightLevel);
//   } else {
//     Serial.println("│ GY-30: ERROR - Sensor tidak terbaca!");
//   }
  
//   Serial.printf("│ Status Hujan    : %s\n", envData.isRaining ? "🌧️  HUJAN" : "☀️  CERAH");
//   Serial.println("└──────────────────────────────────────────────┘");
  
//   // Soil Data (NPK Sensor)
//   Serial.println("\n┌─── DATA TANAH (NPK SENSOR) ──────────────────┐");
//   if (npkData.valid) {
//     Serial.printf("│ Kelembaban Tanah: %.1f %%RH\n", npkData.soilMoisture);
//     Serial.printf("│ Suhu Tanah      : %.1f °C\n", npkData.soilTemp);
//     Serial.printf("│ EC (Konduktivitas): %.0f uS/cm\n", npkData.ec);
//     Serial.printf("│ pH Tanah        : %.1f\n", npkData.ph);
//     Serial.printf("│ Nitrogen (N)    : %d mg/kg\n", npkData.nitrogen);
//     Serial.printf("│ Phosphorus (P)  : %d mg/kg\n", npkData.phosphorus);
//     Serial.printf("│ Potassium (K)   : %d mg/kg\n", npkData.potassium);
//   } else {
//     Serial.println("│ NPK SENSOR: ERROR - Tidak dapat membaca data!");
//     Serial.println("│ Periksa koneksi RS485 dan konfigurasi sensor.");
//   }
//   Serial.println("└──────────────────────────────────────────────┘\n");
// }

// // ==================== SETUP ====================
// void setup() {
//   Serial.begin(115200);
//   delay(1000);
  
//   Serial.println("\n\n╔════════════════════════════════════════════════╗");
//   Serial.println("║   ESP32-S3 NANO MULTI SENSOR INITIALIZATION    ║");
//   Serial.println("╚════════════════════════════════════════════════╝\n");
  
//   // Inisialisasi I2C
//   Wire.begin(I2C_SDA, I2C_SCL);
//   Serial.printf("✓ I2C initialized (SDA: GPIO%d, SCL: GPIO%d)\n", I2C_SDA, I2C_SCL);
  
//   // Inisialisasi BME280
//   if (bme.begin(0x76, &Wire)) { // Address bisa 0x76 atau 0x77
//     envData.bmeValid = true;
//     Serial.println("✓ BME280 detected!");
//   } else if (bme.begin(0x77, &Wire)) {
//     envData.bmeValid = true;
//     Serial.println("✓ BME280 detected (address 0x77)!");
//   } else {
//     envData.bmeValid = false;
//     Serial.println("✗ BME280 not found! Check wiring.");
//   }
  
//   // Inisialisasi GY-30 (BH1750)
//   if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
//     envData.lightValid = true;
//     Serial.println("✓ BH1750 (GY-30) detected!");
//   } else if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &Wire)) {
//     envData.lightValid = true;
//     Serial.println("✓ BH1750 (GY-30) detected (address 0x5C)!");
//   } else {
//     envData.lightValid = false;
//     Serial.println("✗ BH1750 not found! Check wiring.");
//   }
  
//   // Inisialisasi Rain Sensor
//   pinMode(RAIN_SENSOR_PIN, INPUT);
//   Serial.printf("✓ Rain sensor initialized (GPIO%d)\n", RAIN_SENSOR_PIN);
  
//   // Inisialisasi RS485 untuk NPK Sensor
//   rs485.begin(BAUD_RATE, SERIAL_8N1, RS485_RX, RS485_TX);
//   Serial.printf("✓ RS485 initialized (RX: GPIO%d, TX: GPIO%d, Baud: %d)\n", 
//                 RS485_RX, RS485_TX, BAUD_RATE);
  
//   delay(1000);
//   Serial.println("\n✓ All sensors initialized!");
//   Serial.println("✓ Starting data collection...\n");
//   delay(2000);
// }

// // ==================== LOOP ====================
// void loop() {
//   // Baca semua sensor
//   readBME280();
//   readLightSensor();
//   readRainSensor();
//   readNPKSensor();
  
//   // Tampilkan data
//   printAllSensorData();
  
//   // Delay sebelum pembacaan berikutnya
//   delay(3000); // Baca setiap 3 detik
// }


// ESP32-S3 Nano Multi Sensor with Firebase & WiFiManager
// The Green Guardian - Smart Agriculture Monitoring System
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include "WiFiConfig.h"
#include "FirebaseConfig.h"

// ==================== PIN CONFIGURATION ====================
#define I2C_SDA 11
#define I2C_SCL 12

// Digital Pin untuk Rain Sensor
#define RAIN_SENSOR_PIN 10

// ==================== TIMING CONFIGURATION ====================
#define SENSOR_READ_INTERVAL 5000    // Baca sensor setiap 5 detik
#define FIREBASE_SEND_INTERVAL 15000 // Kirim ke Firebase setiap 15 detik
#define WIFI_CHECK_INTERVAL 30000    // Cek WiFi setiap 30 detik

// ==================== OBJEK SENSOR ====================
Adafruit_BME280 bme;
BH1750 lightMeter;

// ==================== STRUKTUR DATA ====================
struct EnvironmentData {
  float airTemp;
  float airHumidity;
  float pressure;
  float lightLevel;
  bool isRaining;
  bool bmeValid;
  bool lightValid;
};

EnvironmentData envData;

// ==================== TIMING VARIABLES ====================
unsigned long lastSensorRead = 0;
unsigned long lastFirebaseSend = 0;
unsigned long lastWiFiCheck = 0;
bool firebaseReady = false;

// ==================== BACA BME280 ====================
void readBME280() {
  if (envData.bmeValid) {
    envData.airTemp = bme.readTemperature();
    envData.airHumidity = bme.readHumidity();
    envData.pressure = bme.readPressure() / 100.0F; // Convert to hPa
  }
}

// ==================== BACA GY-30 (BH1750) ====================
void readLightSensor() {
  if (envData.lightValid) {
    envData.lightLevel = lightMeter.readLightLevel();
  }
}

// ==================== BACA RAIN SENSOR ====================
void readRainSensor() {
  // Rain sensor: LOW = hujan terdeteksi, HIGH = tidak hujan
  envData.isRaining = (digitalRead(RAIN_SENSOR_PIN) == LOW);
}

// ==================== PRINT DATA KE SERIAL ====================
void printAllSensorData() {
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.println("║     ESP32-S3 NANO MULTI SENSOR MONITOR         ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  
  // WiFi & Firebase Status
  Serial.println("\n┌─── KONEKSI STATUS ───────────────────────────┐");
  Serial.printf("│ WiFi        : %s (RSSI: %d dBm)\n", 
                isWiFiConnected() ? "✓ Connected" : "✗ Disconnected",
                getWiFiRSSI());
  Serial.printf("│ Firebase    : %s\n", 
                firebaseReady ? "✓ Ready" : "✗ Not Ready");
  Serial.println("└──────────────────────────────────────────────┘");
  
  // Environment Data (BME280 & GY-30)
  Serial.println("\n┌─── KONDISI LINGKUNGAN ───────────────────────┐");
  if (envData.bmeValid) {
    Serial.printf("│ Suhu Udara      : %.2f °C\n", envData.airTemp);
    Serial.printf("│ Kelembaban Udara: %.2f %%RH\n", envData.airHumidity);
    Serial.printf("│ Tekanan Udara   : %.2f hPa\n", envData.pressure);
  } else {
    Serial.println("│ BME280: ERROR - Sensor tidak terbaca!");
  }
  
  if (envData.lightValid) {
    Serial.printf("│ Intensitas Cahaya: %.2f lux\n", envData.lightLevel);
  } else {
    Serial.println("│ GY-30: ERROR - Sensor tidak terbaca!");
  }
  
  Serial.printf("│ Status Hujan    : %s\n", envData.isRaining ? "🌧️  HUJAN" : "☀️  CERAH");
  Serial.println("└──────────────────────────────────────────────┘\n");
}

// ==================== SEND DATA TO FIREBASE ====================
void sendDataToFirebase() {
  if (!firebaseReady) {
    Serial.println("⚠ Firebase not ready, skipping send");
    return;
  }
  
  if (!isWiFiConnected()) {
    Serial.println("⚠ WiFi not connected, skipping Firebase send");
    return;
  }
  
  Serial.println("\n→ Sending data to Firebase...");
  
  // Kirim data environment
  bool success = sendEnvironmentData(
    envData.airTemp,
    envData.airHumidity,
    envData.pressure,
    envData.lightLevel,
    envData.isRaining
  );
  
  if (success) {
    updateDeviceStatus("online");
  } else {
    Serial.println("  Error: " + getFirebaseError());
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(2000); // Tunggu serial monitor ready
  
  Serial.println("\n\n╔════════════════════════════════════════════════╗");
  Serial.println("║      THE GREEN GUARDIAN - Smart Agriculture    ║");
  Serial.println("║      ESP32-S3 Multi Sensor + Firebase          ║");
  Serial.println("╚════════════════════════════════════════════════╝\n");
  
  // ========== INISIALISASI WIFI ==========
  if (!initWiFiManager()) {
    Serial.println("\n✗ WiFi initialization failed!");
    Serial.println("→ Device will work in offline mode");
    Serial.println("→ Press RESET to reconfigure WiFi\n");
  }
  
  // ========== INISIALISASI FIREBASE ==========
  String fbHost = getFirebaseHost();
  String fbAuth = getFirebaseAuth();
  
  if (fbHost.length() > 0) {
    if (initFirebase(fbHost.c_str(), fbAuth.c_str())) {
      firebaseReady = true;
      Serial.println("✓ Firebase ready to use!\n");
    } else {
      Serial.println("✗ Firebase initialization failed\n");
      firebaseReady = false;
    }
  } else {
    Serial.println("⚠ Firebase not configured");
    Serial.println("  Device will work in offline mode\n");
    firebaseReady = false;
  }
  
  // ========== INISIALISASI SENSOR ==========
  delay(500);
  Serial.println("→ Initializing sensors...\n");
  
  // Inisialisasi I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.printf("✓ I2C initialized (SDA: GPIO%d, SCL: GPIO%d)\n", I2C_SDA, I2C_SCL);
  
  // Inisialisasi BME280
  if (bme.begin(0x76, &Wire)) {
    envData.bmeValid = true;
    Serial.println("✓ BME280 detected (address 0x76)!");
  } else if (bme.begin(0x77, &Wire)) {
    envData.bmeValid = true;
    Serial.println("✓ BME280 detected (address 0x77)!");
  } else {
    envData.bmeValid = false;
    Serial.println("✗ BME280 not found! Check wiring.");
  }
  
  // Inisialisasi GY-30 (BH1750)
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    envData.lightValid = true;
    Serial.println("✓ BH1750 (GY-30) detected (address 0x23)!");
  } else if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &Wire)) {
    envData.lightValid = true;
    Serial.println("✓ BH1750 (GY-30) detected (address 0x5C)!");
  } else {
    envData.lightValid = false;
    Serial.println("✗ BH1750 not found! Check wiring.");
  }
  
  // Inisialisasi Rain Sensor
  pinMode(RAIN_SENSOR_PIN, INPUT);
  Serial.printf("✓ Rain sensor initialized (GPIO%d)\n", RAIN_SENSOR_PIN);
  
  delay(1000);
  Serial.println("\n✓ All systems initialized!");
  Serial.println("✓ Starting monitoring...\n");
  
  // Kirim status awal ke Firebase
  if (firebaseReady) {
    updateDeviceStatus("online");
  }
  
  delay(2000);
}

// ==================== LOOP ====================
void loop() {
  unsigned long currentMillis = millis();
  
  // ========== CEK WIFI CONNECTION ==========
  if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentMillis;
    
    if (!isWiFiConnected()) {
      Serial.println("\n⚠ WiFi disconnected! Attempting reconnect...");
      reconnectWiFi();
    }
  }
  
  // ========== BACA SENSOR ==========
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    
    readBME280();
    readLightSensor();
    readRainSensor();
    
    // Tampilkan data
    printAllSensorData();
  }
  
  // ========== KIRIM KE FIREBASE ==========
  if (currentMillis - lastFirebaseSend >= FIREBASE_SEND_INTERVAL) {
    lastFirebaseSend = currentMillis;
    sendDataToFirebase();
  }
  
  // Small delay untuk stabilitas
  delay(100);
}
