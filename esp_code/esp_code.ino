#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// MQ Sensor pins
const int mq7Pin = 35;    
const int mq135Pin = 34;  

// Wi-Fi credentials
const char* ssid = "AirFiber-Ayushman";
const char* password = "12341234@";

unsigned long lastSwitchTime = 0;
unsigned long lastDataPushTime = 0;
const unsigned long dataPushInterval = 600000; // 10 minutes in milliseconds
int page = 1;

void setup() {
  Serial.begin(9600);

  // LCD initialization
  Wire.begin(21, 22);  
  lcd.init();
  lcd.backlight();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { 
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nFailed to connect to WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    delay(2000);
  }
}

void loop() {
  // Read ADC values
  int mq7Value = analogRead(mq7Pin) / 4;    // Convert 12-bit to 10-bit
  int mq135Value = analogRead(mq135Pin) / 4;

  // Gas estimations
  int co = mq7Value / 2.3;
  int smoke = mq7Value / 2.1;
  int hydrogen = mq7Value / 2.9;

  int nh3 = mq135Value / 2.5;
  int co2 = mq135Value / 2.3;
  int benzene = mq135Value / 2.8;
  int toluene = mq135Value / 2.9;
  int xylene = mq135Value / 3.0;
  int no = mq135Value / 3.2;
  int no2 = mq135Value / 3.4;
  int nox = mq135Value / 3.5;
  int lpg = mq135Value / 3.1;

  int aqi_percent = ((mq135Value + mq7Value) / 2.0) + 5;
  String remark = "MODERATE";
  if (aqi_percent > 80) remark = "POOR";
  if (aqi_percent < 50) remark = "GOOD";

  // Serial Output
  Serial.println("----- Sensor Readings -----");
  Serial.print("NO: "); Serial.println(no);
  Serial.print("NO2: "); Serial.println(no2);
  Serial.print("NOX: "); Serial.println(nox);
  Serial.print("NH3: "); Serial.println(nh3);
  Serial.print("CO: "); Serial.println(co);
  Serial.print("BENZENE: "); Serial.println(benzene);
  Serial.print("TOLUENE: "); Serial.println(toluene);
  Serial.print("XYLENE: "); Serial.println(xylene);
  Serial.print("AQI: "); Serial.print(aqi_percent); Serial.print(" %"); 
  Serial.print(" | Status: "); Serial.println(remark);
  Serial.println("----------------------------\n");

  // -----------------------------
  // Send data to remote server (every 10 minutes)
  // -----------------------------
  if(WiFi.status() == WL_CONNECTED && (millis() - lastDataPushTime >= dataPushInterval)){
    HTTPClient http;
    http.begin("https://server.uemcseaiml.org/aqi-predictor/update");
    // your endpoint
    http.addHeader("Content-Type", "application/json");

    // Build JSON payload with only the keys your backend expects
    String payload = "{";
    payload += "\"NO\":" + String(no) + ",";
    payload += "\"NO2\":" + String(no2) + ",";
    payload += "\"NOX\":" + String(nox) + ",";
    payload += "\"NH3\":" + String(nh3) + ",";
    payload += "\"CO\":" + String(co) + ",";
    payload += "\"BENZENE\":" + String(benzene) + ",";
    payload += "\"TOLUENE\":" + String(toluene) + ",";
    payload += "\"XYLENE\":" + String(xylene) + ",";
    payload += "\"AQI\":" + String(aqi_percent);
    payload += "}";

    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Data pushed to server at: " + String(millis()/1000) + " seconds");

    http.end();
    
    // Update the last push time
    lastDataPushTime = millis();
  }

  // -----------------------------
  // LCD display logic
  // -----------------------------
  if (millis() - lastSwitchTime > 4000) {
    page++;
    if (page > 7) page = 1;
    lastSwitchTime = millis();
  }

  lcd.clear();
  lcd.setCursor(0, 0);

  switch (page) {
    case 1:
      lcd.print("CO:");
      lcd.print(co);
      lcd.print("ppm");
      lcd.setCursor(0, 1);
      lcd.print("Smoke:");
      lcd.print(smoke);
      lcd.print("ppm");
      break;

    case 2:
      lcd.print("NH3:");
      lcd.print(nh3);
      lcd.print("ppm");
      lcd.setCursor(0, 1);
      lcd.print("CO2:");
      lcd.print(co2);
      lcd.print("ppm");
      break;

    case 3:
      lcd.print("Benzene:");
      lcd.print(benzene);
      lcd.print("ppm");
      lcd.setCursor(0, 1);
      lcd.print("Toluene:");
      lcd.print(toluene);
      lcd.print("ppm");
      break;

    case 4:
      lcd.print("Xylene:");
      lcd.print(xylene);
      lcd.print("ppm");
      lcd.setCursor(0, 1);
      lcd.print("Hydrogen:");
      lcd.print(hydrogen);
      lcd.print("ppm");
      break;

    case 5:
      lcd.print("NO:");
      lcd.print(no);
      lcd.print("ppm");
      lcd.setCursor(0, 1);
      lcd.print("NO2:");
      lcd.print(no2);
      lcd.print("ppm");
      break;

    case 6:
      lcd.print("Sensors Active:");
      break;

    case 7:
      lcd.print("AQI: ");
      lcd.print(aqi_percent);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("Status:");
      lcd.print(remark);
      break;
  }

  delay(1000); // Short delay for responsive LCD updates
}
