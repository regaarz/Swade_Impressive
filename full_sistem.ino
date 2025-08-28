

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>


Servo myservo;
#define SERVO_PIN 19 
#define SERVO_PIN_2 21 

String jenisTerakhir = "";  
LiquidCrystal_I2C lcd(0x27, 20, 4); // alamat I2C 0x27, LCD 20x4



// ----------------- WiFi & Firebase Credentials -----------------
#define WIFI_SSID     "arzula"
#define WIFI_PASSWORD "12345678"

#define API_KEY       "AIzaSyCmXn0Mt6d0I07q7lf2agk0zmASptR5aIY"
#define DATABASE_URL  "https://tongsampah-fb84c-default-rtdb.firebaseio.com/"

// ----------------- Firebase Objects -----------------
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// ----------------- Ultrasonic Sensor Pins -----------------
// Sensor 1 - organik
#define TRIG1 5
#define ECHO1 18

// Sensor 2 - anorganik
#define TRIG2 17
#define ECHO2 16

// Sensor 3 - B3
#define TRIG3 33   // ganti pakai GPIO33
#define ECHO3 32  

// ----------------- Firebase Upload Function -----------------
void firebaseSetInt(const String& path, int value) {
  if (Firebase.RTDB.setInt(&fbdo, path, value)) {
    Serial.printf("OK    → %s : %d\n", path.c_str(), value);
  } else {
    Serial.printf("GAGAL → %s : %s\n", path.c_str(), fbdo.errorReason().c_str());
  }
}

// ----------------- Jarak Function -----------------
int bacaJarak(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH, 30000);  // timeout 30ms (jarak ~5m)
  int jarak = durasi * 0.034 / 2;
  return jarak;
}

// ----------------- Setup -----------------
void setup() {
  Serial.begin(115200);

  lcd.init();         
  lcd.backlight();    
  lcd.setCursor(0,0);
  lcd.print("Smart Trash Bin");
  delay(2000);
  lcd.clear();

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);

  myservo.attach(SERVO_PIN);
  myservo.write(90); // posisi netral


  // WiFi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("\nTerhubung - IP: %s\n", WiFi.localIP().toString().c_str());

  // Firebase Initialization
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign-in anonim berhasil");
    signupOK = true;
  } else {
    Serial.printf("Sign-in gagal: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void gerakServo(String jenis) {
  if (jenis == "organik") {
    myservo.write(0);    // posisi servo ke tempat organik
  } else if (jenis == "anorganik") {
    myservo.write(90);   // posisi ke tempat anorganik
  } else if (jenis == "b3") {
    myservo.write(180);  // posisi ke tempat B3
  } else {
    Serial.println("Jenis sampah tidak dikenal!");
  }
}


// ----------------- Loop -----------------
void loop() {
  if (signupOK) {
    int jarak1 = bacaJarak(TRIG1, ECHO1);
    int jarak2 = bacaJarak(TRIG2, ECHO2);
    int jarak3 = bacaJarak(TRIG3, ECHO3);

    Serial.printf("Jarak Organik   : %d cm\n", jarak1);
    Serial.printf("Jarak Anorganik : %d cm\n", jarak2);
    Serial.printf("Jarak B3        : %d cm\n", jarak3);

    firebaseSetInt("/Tongsampah1/organik", jarak1);
    firebaseSetInt("/Tongsampah1/anorganik", jarak2);
    firebaseSetInt("/Tongsampah1/B3", jarak3);

    delay(2000);  // tunggu 2 detik
  }

      // Ambil data deteksi dari Firebase
    if (Firebase.RTDB.getString(&fbdo, "/deteksi_sampah/label")) {
      String jenis = fbdo.stringData();

      if (jenis != jenisTerakhir) {
        Serial.printf("Deteksi baru: %s\n", jenis.c_str());
        gerakServo(jenis);

        // Tampilkan ke LCD
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Jenis Sampah:");
        lcd.setCursor(0,1);
        lcd.print(jenis);

        delay(2000);           // Tunggu servo bergerak
        myservo.write(90); 
        jenisTerakhir = jenis;
    }   
} else {
  Serial.printf("Gagal ambil gerak_ke: %s\n", fbdo.errorReason().c_str());
}
  delay(1000);
}
