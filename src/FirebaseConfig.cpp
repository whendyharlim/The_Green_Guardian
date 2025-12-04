#include "FirebaseConfig.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// ==================== FIREBASE GLOBAL OBJECTS ====================
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Internal state
static bool firebaseInitialized = false;
static String lastError = "";

// ==================== INITIALIZE FIREBASE ====================
bool initFirebase(const char* host, const char* auth) {
  if (strlen(host) == 0) {
    Serial.println("✗ Firebase: Host kosong!");
    lastError = "Firebase host is empty";
    return false;
  }

  // Sanitize host: ensure https:// prefix and remove trailing slash
  String dbUrl = String(host);
  dbUrl.trim();
  if (!dbUrl.startsWith("http://") && !dbUrl.startsWith("https://")) {
    dbUrl = String("https://") + dbUrl;
  }
  while (dbUrl.endsWith("/") || dbUrl.endsWith(" ")) {
    dbUrl.remove(dbUrl.length() - 1);
  }

  // Konfigurasi Firebase
  firebaseConfig.api_key = "";  // Kosong jika menggunakan database secret
  firebaseConfig.database_url = dbUrl.c_str();
  
  // Jika menggunakan auth token (database secret)
  if (strlen(auth) > 0) {
    firebaseConfig.signer.tokens.legacy_token = auth;
  }
  
  // Optional: Set timeout
  firebaseConfig.timeout.serverResponse = 10 * 1000; // 10 detik
  
  // Inisialisasi Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectNetwork(true);
  
  // Set SSL buffer size
  firebaseData.setBSSLBufferSize(4096, 1024);
  
  firebaseInitialized = true;
  Serial.println("✓ Firebase initialized!");
  Serial.print("  Database URL: ");
  Serial.println(firebaseConfig.database_url.c_str());
  Serial.print("  Auth token length: ");
  Serial.println(strlen(auth));
  
  return true;
}

// ==================== SEND ENVIRONMENT DATA ====================
bool sendEnvironmentData(float temp, float humidity, float pressure, float light, bool isRaining) {
  if (!firebaseInitialized) {
    lastError = "Firebase not initialized";
    return false;
  }

  FirebaseJson json;
  json.set("temperature", temp);
  json.set("humidity", humidity);
  json.set("pressure", pressure);
  json.set("light", light);
  json.set("raining", isRaining);
  json.set("timestamp", (unsigned long)(millis() / 1000)); // Unix-like timestamp
  
  if (Firebase.RTDB.setJSON(&firebaseData, FB_PATH_ENVIRONMENT, &json)) {
    Serial.println("✓ Environment data sent to Firebase");
    return true;
  } else {
    lastError = firebaseData.errorReason();
    Serial.println("✗ Failed to send environment data");
    Serial.print("  Error: ");
    Serial.println(lastError);
    return false;
  }
}

// ==================== SEND SOIL DATA ====================
bool sendSoilData(float soilMoisture, float soilTemp, float ec, float ph, 
                  int nitrogen, int phosphorus, int potassium) {
  if (!firebaseInitialized) {
    lastError = "Firebase not initialized";
    return false;
  }

  FirebaseJson json;
  json.set("moisture", soilMoisture);
  json.set("temperature", soilTemp);
  json.set("ec", ec);
  json.set("ph", ph);
  json.set("nitrogen", nitrogen);
  json.set("phosphorus", phosphorus);
  json.set("potassium", potassium);
  json.set("timestamp", (unsigned long)(millis() / 1000));
  
  if (Firebase.RTDB.setJSON(&firebaseData, FB_PATH_SOIL, &json)) {
    Serial.println("✓ Soil data sent to Firebase");
    return true;
  } else {
    lastError = firebaseData.errorReason();
    Serial.println("✗ Failed to send soil data");
    Serial.print("  Error: ");
    Serial.println(lastError);
    return false;
  }
}

// ==================== UPDATE DEVICE STATUS ====================
bool updateDeviceStatus(const char* status) {
  if (!firebaseInitialized) {
    lastError = "Firebase not initialized";
    return false;
  }

  FirebaseJson json;
  json.set("status", status);
  json.set("uptime", millis() / 1000);
  json.set("timestamp", (unsigned long)(millis() / 1000));
  
  if (Firebase.RTDB.setJSON(&firebaseData, FB_PATH_DEVICE_STATUS, &json)) {
    return true;
  } else {
    lastError = firebaseData.errorReason();
    return false;
  }
}

// ==================== SEND ALL SENSOR DATA (BATCH) ====================
bool sendAllSensorData(const void* envData, const void* npkData) {
  if (!firebaseInitialized) {
    lastError = "Firebase not initialized";
    return false;
  }

  FirebaseJson root;
  unsigned long timestamp = millis() / 1000;
  
  // Cast pointer ke struct yang sesuai (asumsi EnvironmentData dari main.cpp)
  struct EnvDataTemp {
    float airTemp;
    float airHumidity;
    float pressure;
    float lightLevel;
    bool isRaining;
    bool bmeValid;
    bool lightValid;
  };
  
  const EnvDataTemp* env = (const EnvDataTemp*)envData;
  
  if (env) {
    FirebaseJson envJson;
    envJson.set("temperature", env->airTemp);
    envJson.set("humidity", env->airHumidity);
    envJson.set("pressure", env->pressure);
    envJson.set("light", env->lightLevel);
    envJson.set("raining", env->isRaining);
    envJson.set("timestamp", timestamp);
    
    root.set("environment", envJson);
  }
  
  // NPK data (jika ada)
  if (npkData) {
    struct NPKDataTemp {
      float soilMoisture;
      float soilTemp;
      float ec;
      float ph;
      int nitrogen;
      int phosphorus;
      int potassium;
      bool valid;
    };
    
    const NPKDataTemp* npk = (const NPKDataTemp*)npkData;
    
    if (npk->valid) {
      FirebaseJson soilJson;
      soilJson.set("moisture", npk->soilMoisture);
      soilJson.set("temperature", npk->soilTemp);
      soilJson.set("ec", npk->ec);
      soilJson.set("ph", npk->ph);
      soilJson.set("nitrogen", npk->nitrogen);
      soilJson.set("phosphorus", npk->phosphorus);
      soilJson.set("potassium", npk->potassium);
      soilJson.set("timestamp", timestamp);
      
      root.set("soil", soilJson);
    }
  }
  
  // Device status
  FirebaseJson deviceJson;
  deviceJson.set("status", "online");
  deviceJson.set("uptime", millis() / 1000);
  deviceJson.set("timestamp", timestamp);
  root.set("device", deviceJson);
  
  // Kirim semua data
  if (Firebase.RTDB.setJSON(&firebaseData, FB_ROOT "/sensors", &root)) {
    Serial.println("✓ All sensor data sent to Firebase");
    return true;
  } else {
    lastError = firebaseData.errorReason();
    Serial.println("✗ Failed to send data to Firebase");
    Serial.print("  Error: ");
    Serial.println(lastError);
    return false;
  }
}

// ==================== CHECK FIREBASE CONNECTION ====================
bool isFirebaseConnected() {
  return firebaseInitialized && Firebase.ready();
}

// ==================== GET FIREBASE ERROR ====================
String getFirebaseError() {
  return lastError;
}
