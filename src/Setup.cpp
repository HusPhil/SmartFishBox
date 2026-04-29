#include "Config.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Import shared state variables
#include "Logic.h"
#include "Setup.h"

// --- Blynk Datastream Handlers ---
BLYNK_WRITE(V0) { cycles = param.asInt(); Serial.println("[BLYNK RX] cycles = " + String(cycles)); }
BLYNK_WRITE(V1) { feedHours[0] = param.asInt(); Serial.println("[BLYNK RX] feedHour1 = " + String(feedHours[0])); }
BLYNK_WRITE(V2) { feedMinutes[0] = param.asInt(); Serial.println("[BLYNK RX] feedMinute1 = " + String(feedMinutes[0])); }
BLYNK_WRITE(V3) { feedHours[1] = param.asInt(); Serial.println("[BLYNK RX] feedHour2 = " + String(feedHours[1])); }
BLYNK_WRITE(V4) { feedMinutes[1] = param.asInt(); Serial.println("[BLYNK RX] feedMinute2 = " + String(feedMinutes[1])); }
BLYNK_WRITE(V5) { feedHours[2] = param.asInt(); Serial.println("[BLYNK RX] feedHour3 = " + String(feedHours[2])); }
BLYNK_WRITE(V6) { feedMinutes[2] = param.asInt(); Serial.println("[BLYNK RX] feedMinute3 = " + String(feedMinutes[2])); }
BLYNK_WRITE(V7) { feedHours[3] = param.asInt(); Serial.println("[BLYNK RX] feedHour4 = " + String(feedHours[3])); }
BLYNK_WRITE(V8) { feedMinutes[3] = param.asInt(); Serial.println("[BLYNK RX] feedMinute4 = " + String(feedMinutes[3])); }
BLYNK_WRITE(V9) { feedHours[4] = param.asInt(); Serial.println("[BLYNK RX] feedHour5 = " + String(feedHours[4])); }
BLYNK_WRITE(V10){ feedMinutes[4] = param.asInt(); Serial.println("[BLYNK RX] feedMinute5 = " + String(feedMinutes[4])); }

BLYNK_WRITE(V13) { 
  cooldownHours = param.asInt(); 
  calculateDynamicCooldown(); 
}

BLYNK_WRITE(V14) { 
  cooldownMinutes = param.asInt(); 
  calculateDynamicCooldown(); 
}

BLYNK_WRITE(V15) { 
  cooldownSeconds = param.asInt(); 
  calculateDynamicCooldown(); 
}

BLYNK_WRITE(V16) { 
  currentShakeIntensityConfig = param.asInt(); 
  updateCurrentShakeIntensity(); 
}

BLYNK_WRITE(V18) { 
  maxPHLevelThreshold = param.asFloat(); 
}

BLYNK_WRITE(V19) { 
  waterOutDurationSec = param.asInt(); 
}

BLYNK_WRITE(V20) { 
  minPHLevelThreshold = param.asFloat(); 
}

BLYNK_WRITE(V21) { 
  feedNow = param.asInt(); // Set the flag to trigger immediate feeding in the main loop
}

BLYNK_WRITE(V22) { 
  changeWaterNow = param.asInt(); // Set the flag to trigger immediate water change in the main loop
}


BLYNK_CONNECTED() {
  Serial.println("[BLYNK] Connected to Cloud. Syncing datastreams...");
  
  // Sync schedule pins
  Blynk.syncVirtual(V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, V10); 
  
  // Sync the missing settings pins
  Blynk.syncVirtual(V13, V14, V15, V16, V18, V19, V20, V21, V22); 
  
  synced = true;
}

void connectWifi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8));
  WiFi.begin(ssid, pass, 6); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
}

void configureTime() {
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("NTP configured (UTC+8).");
}

void resetFeedSchedules() {
  for(int i = 0; i < MAX_FEEDS; i++) {
    feedHours[i] = -1;
    feedMinutes[i] = -1;
    alreadyTriggered[i] = false;
  }
}

// --- Encapsulated Cloud Functions ---
void initCloud() {
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
}

void runCloud() {
  Blynk.run();
}


void resetFeedNowFlag() {
  feedNow = 0;
  Blynk.virtualWrite(V21, feedNow);
}

void resetChangeWaterNowFlag() {
  changeWaterNow = 0;
  Blynk.virtualWrite(V22, changeWaterNow);
}

void sendCurrentPHLevel(){
  Blynk.virtualWrite(V11, currentpHLevel);
};

void sendCurrentTemperature() {
  Blynk.virtualWrite(V12, currentTemperature);
};

void sendCurrentWaterChangeState(String waterChangeState) {
  Blynk.virtualWrite(V17, waterChangeState);
};