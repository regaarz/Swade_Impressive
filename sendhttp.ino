#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "arzula";
const char* password = "12345678";

String serverName = "http://10.154.199.85:5000/send_distance";  // IP Flask server

// Sensor 1 - organik
#define TRIG1 5
#define ECHO1 18

// Sensor 2 - anorganik
#define TRIG2 17
#define ECHO2 16

// Sensor 3 - B3
#define TRIG3 33
#define ECHO3 32

void setup() {
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if (duration == 0) {
    return -1; // error baca
  }
  float distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    float organik   = getDistance(TRIG1, ECHO1);
    float anorganik = getDistance(TRIG2, ECHO2);
    float b3        = getDistance(TRIG3, ECHO3);

    String jsonData = "{\"organik\":" + String(organik, 2) +
                      ",\"anorganik\":" + String(anorganik, 2) +
                      ",\"b3\":" + String(b3, 2) + "}";

    Serial.println("Sending JSON: " + jsonData);

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println("Server response: " + response);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(2000); // kirim tiap 2 detik
}
