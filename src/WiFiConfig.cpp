#include "WiFiConfig.h"

// ==================== WIFIMANAGER OBJECTS ====================
WiFiManager wm;

// Custom parameters untuk Firebase
WiFiManagerParameter custom_firebase_host("firebase_host", "Firebase Host", "", 100);
WiFiManagerParameter custom_firebase_auth("firebase_auth", "Firebase Auth Token", "", 100);

// Internal variables
static String savedFirebaseHost = "";
static String savedFirebaseAuth = "";

// ==================== CALLBACK FUNCTION ====================
void saveConfigCallback() {
  Serial.println("✓ WiFi configuration saved!");
}

// ==================== INITIALIZE WIFIMANAGER ====================
bool initWiFiManager() {
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.println("║         WiFiManager Initialization             ║");
  Serial.println("╚════════════════════════════════════════════════╝\n");

  // Set debug output
  wm.setDebugOutput(true);
  
  // Set timeout
  wm.setConfigPortalTimeout(WM_PORTAL_TIMEOUT);
  
  // Set callback untuk save config
  wm.setSaveConfigCallback(saveConfigCallback);
  
  // Tambahkan custom parameters
  wm.addParameter(&custom_firebase_host);
  wm.addParameter(&custom_firebase_auth);
  
  // Info text
  WiFiManagerParameter custom_text("<p><b>Firebase Configuration</b></p>");
  wm.addParameter(&custom_text);
  
  WiFiManagerParameter custom_text2("<p>Masukkan Firebase Realtime Database URL<br>Contoh: <i>your-project.firebaseio.com</i></p>");
  wm.addParameter(&custom_text2);
  
  Serial.println("→ Trying to connect to saved WiFi...");
  
  // Coba auto connect
  if (wm.autoConnect(WM_AP_NAME, WM_AP_PASSWORD)) {
    Serial.println("\n✓ WiFi connected successfully!");
    Serial.print("  SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Simpan Firebase parameters
    savedFirebaseHost = custom_firebase_host.getValue();
    savedFirebaseAuth = custom_firebase_auth.getValue();
    
    if (savedFirebaseHost.length() > 0) {
      Serial.println("\n✓ Firebase configuration loaded:");
      Serial.print("  Host: ");
      Serial.println(savedFirebaseHost);
      Serial.print("  Auth: ");
      Serial.println(savedFirebaseAuth.length() > 0 ? "***configured***" : "empty");
    } else {
      Serial.println("\n⚠ Firebase configuration is empty!");
      Serial.println("  You can reconfigure by pressing RESET button");
    }
    
    return true;
  } else {
    Serial.println("\n✗ Failed to connect to WiFi");
    Serial.println("  Portal timeout reached");
    return false;
  }
}

// ==================== RESET WIFI SETTINGS ====================
void resetWiFiSettings() {
  Serial.println("\n⚠ Resetting WiFi settings...");
  wm.resetSettings();
  Serial.println("✓ WiFi settings cleared!");
  Serial.println("→ Please restart the device");
  delay(1000);
  ESP.restart();
}

// ==================== GET FIREBASE HOST ====================
String getFirebaseHost() {
  return savedFirebaseHost;
}

// ==================== GET FIREBASE AUTH ====================
String getFirebaseAuth() {
  return savedFirebaseAuth;
}

// ==================== CHECK WIFI CONNECTION ====================
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// ==================== GET WIFI RSSI ====================
int getWiFiRSSI() {
  return WiFi.RSSI();
}

// ==================== RECONNECT WIFI ====================
bool reconnectWiFi() {
  if (isWiFiConnected()) {
    return true;
  }
  
  Serial.println("\n→ WiFi disconnected! Attempting to reconnect...");
  
  int attempts = 0;
  while (!isWiFiConnected() && attempts < 10) {
    WiFi.reconnect();
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (isWiFiConnected()) {
    Serial.println("\n✓ WiFi reconnected!");
    return true;
  } else {
    Serial.println("\n✗ Failed to reconnect WiFi");
    return false;
  }
}
