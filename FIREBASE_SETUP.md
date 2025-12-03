# Firebase Configuration Guide

## 📌 Setup Firebase Realtime Database

### 1. Membuat Project Firebase
1. Buka [Firebase Console](https://console.firebase.google.com/)
2. Klik **"Add project"** atau **"Create a project"**
3. Masukkan nama project (contoh: "green-guardian")
4. Ikuti wizard setup (disable Google Analytics jika tidak diperlukan)

### 2. Aktifkan Realtime Database
1. Di sidebar, pilih **"Build"** > **"Realtime Database"**
2. Klik **"Create Database"**
3. Pilih lokasi server terdekat (contoh: asia-southeast1)
4. Pilih mode keamanan:
   - **Locked mode** (recommended untuk production)
   - **Test mode** (untuk development, expired 30 hari)

### 3. Konfigurasi Database Rules

#### Option A: Public Rules (untuk testing)
```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```
⚠️ **WARNING**: Semua orang bisa baca/tulis! Hanya untuk testing!

#### Option B: Private with Auth (recommended)
```json
{
  "rules": {
    ".read": "auth != null",
    ".write": "auth != null"
  }
}
```
✅ Hanya user yang authenticated yang bisa akses

#### Option C: Custom Rules (advanced)
```json
{
  "rules": {
    "sensors": {
      ".read": true,
      ".write": "auth != null"
    },
    "device": {
      ".read": "auth != null",
      ".write": "auth != null"
    }
  }
}
```
✅ Public bisa baca sensor data, tapi hanya authenticated yang bisa update

### 4. Mendapatkan Kredensial

#### A. Firebase Host URL
1. Di halaman Realtime Database, lihat di bagian atas
2. Copy URL database (contoh: `https://green-guardian-default-rtdb.asia-southeast1.firebasedatabase.app/`)
3. Untuk konfigurasi WiFiManager, **hilangkan `https://`**
   - ✅ `green-guardian-default-rtdb.asia-southeast1.firebasedatabase.app`
   - ❌ `https://green-guardian-default-rtdb.asia-southeast1.firebasedatabase.app/`

#### B. Database Secret (Legacy Auth)
1. Buka **Project Settings** (ikon gear di sidebar)
2. Pilih tab **"Service accounts"**
3. Klik **"Database secrets"** di bagian bawah
4. Copy secret token

**Alternatif jika Database Secret tidak ada:**
- Untuk testing dengan public rules, **kosongkan field Auth Token**
- Atau gunakan Firebase Authentication untuk production

### 5. Format Konfigurasi WiFiManager

Saat portal WiFiManager terbuka:

```
WiFi SSID     : [Nama WiFi Anda]
WiFi Password : [Password WiFi]

Firebase Host      : green-guardian-default-rtdb.asia-southeast1.firebasedatabase.app
Firebase Auth Token: [Database Secret atau kosongkan jika public]
```

## 📊 Struktur Data di Firebase

### Format JSON
```json
{
  "sensors": {
    "environment": {
      "temperature": 28.5,
      "humidity": 65.2,
      "pressure": 1013.25,
      "light": 1520.5,
      "raining": false,
      "timestamp": 1701648000
    },
    "soil": {
      "moisture": 45.5,
      "temperature": 25.3,
      "ec": 350,
      "ph": 6.8,
      "nitrogen": 45,
      "phosphorus": 30,
      "potassium": 55,
      "timestamp": 1701648000
    }
  },
  "device": {
    "status": {
      "status": "online",
      "uptime": 3600,
      "timestamp": 1701648000
    }
  }
}
```

## 🌐 Mengakses Data

### 1. Melalui Firebase Console
- Buka **Realtime Database** tab
- Data update secara real-time

### 2. REST API
```bash
# Get semua data
curl https://YOUR-PROJECT.firebaseio.com/sensors.json

# Get data environment saja
curl https://YOUR-PROJECT.firebaseio.com/sensors/environment.json

# Get temperature saja
curl https://YOUR-PROJECT.firebaseio.com/sensors/environment/temperature.json
```

### 3. Firebase SDK (Web)
```javascript
import { initializeApp } from "firebase/app";
import { getDatabase, ref, onValue } from "firebase/database";

const firebaseConfig = {
  databaseURL: "https://your-project.firebaseio.com"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

// Real-time listener
const sensorRef = ref(db, 'sensors/environment');
onValue(sensorRef, (snapshot) => {
  const data = snapshot.val();
  console.log(data);
});
```

### 4. Firebase SDK (Python)
```python
import firebase_admin
from firebase_admin import db

firebase_admin.initialize_app({
    'databaseURL': 'https://your-project.firebaseio.com'
})

ref = db.reference('sensors/environment')
data = ref.get()
print(data)
```

## 🔐 Security Best Practices

### 1. Jangan Share Credentials
- ❌ Jangan commit database secret ke GitHub
- ✅ Simpan di environment variables
- ✅ Gunakan `.gitignore` untuk file config

### 2. Gunakan Authentication
Untuk production, gunakan Firebase Authentication:
- Email/Password
- Google Sign-In
- Anonymous Auth (untuk device IoT)

### 3. Rate Limiting
Set rules untuk limit write operations:
```json
{
  "rules": {
    "sensors": {
      ".write": "auth != null && !data.exists() || (now - data.child('timestamp').val()) > 5000"
    }
  }
}
```
Ini membatasi update hanya setiap 5 detik

### 4. Data Validation
```json
{
  "rules": {
    "sensors": {
      "environment": {
        ".validate": "newData.hasChildren(['temperature', 'humidity', 'pressure'])",
        "temperature": {
          ".validate": "newData.isNumber() && newData.val() >= -50 && newData.val() <= 100"
        }
      }
    }
  }
}
```

## 🐛 Troubleshooting

### Error: "Permission Denied"
- Cek Database Rules
- Pastikan auth token benar
- Untuk testing, gunakan public rules dulu

### Error: "Host not found"
- Pastikan format URL benar (tanpa https://)
- Cek koneksi internet ESP32
- Pastikan WiFi sudah connected

### Data tidak update
- Cek Serial Monitor untuk error messages
- Pastikan `FIREBASE_SEND_INTERVAL` tidak terlalu pendek
- Verify Firebase credentials

### Connection timeout
- Cek signal WiFi
- Increase `firebaseConfig.timeout.serverResponse`
- Pastikan Firebase region tidak terlalu jauh

## 📱 Monitoring Options

### 1. Simple Web Dashboard
```html
<!DOCTYPE html>
<html>
<head>
  <title>Green Guardian Dashboard</title>
  <script src="https://www.gstatic.com/firebasejs/10.7.0/firebase-app-compat.js"></script>
  <script src="https://www.gstatic.com/firebasejs/10.7.0/firebase-database-compat.js"></script>
</head>
<body>
  <h1>Green Guardian Sensor Data</h1>
  <div id="data"></div>
  
  <script>
    const firebaseConfig = {
      databaseURL: "https://your-project.firebaseio.com"
    };
    
    firebase.initializeApp(firebaseConfig);
    const database = firebase.database();
    
    database.ref('sensors/environment').on('value', (snapshot) => {
      const data = snapshot.val();
      document.getElementById('data').innerHTML = `
        <p>Temperature: ${data.temperature}°C</p>
        <p>Humidity: ${data.humidity}%</p>
        <p>Pressure: ${data.pressure} hPa</p>
        <p>Light: ${data.light} lux</p>
        <p>Raining: ${data.raining ? 'Yes' : 'No'}</p>
      `;
    });
  </script>
</body>
</html>
```

### 2. Mobile App
- Firebase SDK for Android/iOS
- React Native with Firebase
- Flutter with FlutterFire

### 3. Data Logging & Analytics
- Export data ke Google Sheets
- Integrate dengan Google Cloud Functions
- Create alerts dengan Cloud Messaging

## 📚 Resources

- [Firebase Documentation](https://firebase.google.com/docs)
- [Realtime Database Guide](https://firebase.google.com/docs/database)
- [Security Rules Guide](https://firebase.google.com/docs/database/security)
- [REST API Reference](https://firebase.google.com/docs/reference/rest/database)

---
**Note**: Panduan ini untuk Firebase Realtime Database. Untuk Firestore (document database), setup dan API berbeda.
