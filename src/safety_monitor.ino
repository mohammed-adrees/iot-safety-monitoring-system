#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// --- WiFi ---
const char* ssid     = "Majdal";
const char* password = "smnmmo22";

// --- Pins ---
#define DHTPIN      21
#define DHTTYPE     DHT11
#define FLAME_PIN   22
#define BUZZER_PIN  23
#define GAS_PIN     34

// --- Thresholds ---
#define GAS_THRESHOLD  2000
#define TEMP_THRESHOLD 50

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

// --- Sensor values ---
float temperature   = 0;
float humidity      = 0;
int   gasValue      = 0;
bool  flameDetected = false;
bool  danger        = false;

// ============================================================
//  HTML PAGE
// ============================================================
String buildPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Safety Monitor</title>
  <style>
    * { margin:0; padding:0; box-sizing:border-box; }
    body {
      font-family: 'Segoe UI', sans-serif;
      background: #0a0a0a;
      color: #fff;
      min-height: 100vh;
      padding: 30px 20px;
    }
    h1 {
      text-align: center;
      font-size: 2rem;
      letter-spacing: 3px;
      text-transform: uppercase;
      margin-bottom: 8px;
      color: #00e5ff;
    }
    .subtitle {
      text-align: center;
      color: #555;
      font-size: 0.85rem;
      margin-bottom: 40px;
      letter-spacing: 2px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 20px;
      max-width: 900px;
      margin: 0 auto 30px auto;
    }
    .card {
      background: #141414;
      border: 1px solid #222;
      border-radius: 16px;
      padding: 28px 20px;
      text-align: center;
    }
    .card .icon { font-size: 2.5rem; margin-bottom: 12px; }
    .card .label {
      font-size: 0.75rem;
      letter-spacing: 2px;
      text-transform: uppercase;
      color: #555;
      margin-bottom: 10px;
    }
    .card .value {
      font-size: 2.4rem;
      font-weight: 700;
      color: #00e5ff;
    }
    .card .unit { font-size: 1rem; color: #444; margin-top: 4px; }
    .danger-card { border-color: #ff1744 !important; background: #1a0000 !important; }
    .danger-card .value { color: #ff1744 !important; }
    .ok-card { border-color: #00e676 !important; background: #001a0a !important; }
    .ok-card .value { color: #00e676 !important; }
    .alert-box {
      max-width: 900px;
      margin: 0 auto 30px auto;
      border-radius: 16px;
      padding: 24px 30px;
      text-align: center;
      font-size: 1.4rem;
      font-weight: 700;
      letter-spacing: 2px;
      text-transform: uppercase;
    }
    .alert-danger {
      background: #ff1744;
      color: white;
      animation: pulse 0.8s infinite alternate;
    }
    .alert-safe { background: #00e676; color: #000; }
    @keyframes pulse {
      from { opacity:1; transform:scale(1); }
      to   { opacity:0.7; transform:scale(1.02); }
    }
    .footer {
      text-align: center;
      color: #333;
      font-size: 0.75rem;
      letter-spacing: 2px;
      margin-top: 20px;
    }
  </style>
  <script>
    // Auto refresh every 2 seconds without full page reload
    setTimeout(function(){ location.reload(); }, 2000);
  </script>
</head>
<body>

  <h1>🛡️ Safety Monitor</h1>
  <p class="subtitle">Live readings — updates every 2 seconds</p>
)rawliteral";

  // Alert box
  if (danger) {
    html += "<div class='alert-box alert-danger'>";
    if (flameDetected) html += "🔥 FIRE DETECTED! DANGER!";
    else html += "⚠️ HIGH GAS LEVEL! DANGER!";
    html += "</div>";
  } else {
    html += "<div class='alert-box alert-safe'>✅ ALL SYSTEMS NORMAL</div>";
  }

  html += "<div class='grid'>";

  // Temperature card
  html += "<div class='card ";
  html += (temperature >= TEMP_THRESHOLD) ? "danger-card" : "";
  html += "'><div class='icon'>🌡️</div><div class='label'>Temperature</div>";
  html += "<div class='value'>" + String(temperature, 1) + "</div>";
  html += "<div class='unit'>°C</div></div>";

  // Humidity card
  html += "<div class='card'><div class='icon'>💧</div><div class='label'>Humidity</div>";
  html += "<div class='value'>" + String(humidity, 1) + "</div>";
  html += "<div class='unit'>%</div></div>";

  // Gas card
  html += "<div class='card ";
  html += (gasValue >= GAS_THRESHOLD) ? "danger-card" : "";
  html += "'><div class='icon'>💨</div><div class='label'>Gas Level</div>";
  html += "<div class='value'>" + String(gasValue) + "</div>";
  html += "<div class='unit'>/ 4095</div></div>";

  // Flame card
  html += "<div class='card ";
  html += flameDetected ? "danger-card" : "ok-card";
  html += "'><div class='icon'>";
  html += flameDetected ? "🔥" : "✅";
  html += "</div><div class='label'>Flame Sensor</div>";
  html += "<div class='value'>";
  html += flameDetected ? "FIRE!" : "SAFE";
  html += "</div><div class='unit'> </div></div>";

  html += "</div>";
  html += "<p class='footer'>ESP32 Safety Monitor • Connected</p>";
  html += "</body></html>";

  return html;
}

// ============================================================
//  BUZZER
// ============================================================
void soundAlarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }
  delay(300);
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(FLAME_PIN,  INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  // Connect WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📡 Go to: http://");
  Serial.println(WiFi.localIP());

  // Web server route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", buildPage());
  });

  server.begin();
  Serial.println("🌐 Server started!");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  // Read sensors
  temperature   = dht.readTemperature();
  humidity      = dht.readHumidity();
  gasValue      = analogRead(GAS_PIN);
  flameDetected = (digitalRead(FLAME_PIN) == HIGH);

  // Danger check
  danger = flameDetected || (gasValue >= GAS_THRESHOLD);

  // Buzzer
  if (danger) {
    soundAlarm();
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Serial
  Serial.println("==============================");
  Serial.print("🌡️  Temp: ");     Serial.print(temperature);  Serial.println(" °C");
  Serial.print("💧  Humidity: "); Serial.print(humidity);     Serial.println(" %");
  Serial.print("💨  Gas: ");      Serial.println(gasValue);
  Serial.print("🔥  Flame: ");    Serial.println(flameDetected ? "FIRE!" : "Safe");
  Serial.println("==============================");

  delay(1000);
}
