// 1. Configuration
#include "Config.h"

// 2. Standard libraries
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// 3. System Modules
#include "Actuators.h"
#include "Logic.h" 
#include "Setup.h" 

unsigned long lastTelemetryTime = 0;
const unsigned long telemetryIntervalMs = 5000;

// --- Setup ---
void setup() {
  Serial.begin(115200); 
  Serial.println("\n[BOOT] System starting...");
  
  pinMode(pHLevelPinIn, INPUT);
  pinMode(temperaturePinIn, INPUT);
  
  // WiFi Configuration
  connectWifi();
  // Abstracted Cloud Initialization
  initCloud();
  // NTP Configuration
  configureTime();
  // Reset Schedule Arrays
  resetFeedSchedules();
  // Hardware Initialization
  initActuators();
}

// --- Main Execution Loop ---
void loop() {
  runCloud();
  
  // monitorPHLevel();
  // monitorTemperature();

  processWaterChangeState();
  processFeedingSchedule();

  // Low-speed cloud telemetry transmission
  unsigned long currentMillis = millis();
  if (currentMillis - lastTelemetryTime >= telemetryIntervalMs) {
    lastTelemetryTime = currentMillis;
    
    if (synced) {
      sendCurrentPHLevel();
      sendCurrentTemperature();
      Serial.println(waterOutCooldownMs);
    }
  }

  delay(100); 
}