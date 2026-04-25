#include "Logic.h"
#include "Setup.h"

// --- Memory Allocation for Global Variables ---
int feedHours[MAX_FEEDS];
int feedMinutes[MAX_FEEDS];
bool alreadyTriggered[MAX_FEEDS];
int cycles = 0;
bool synced = false;
float currentpHLevel = 0.0;
float currentTemperature = 0.0;
WaterChangeState currentWaterChangeState = IDLE;

unsigned long waterOutCooldownMs = 5 * 1000;
int currentShakeIntensityConfig = 0;

ShakeIntensity currentShakeIntensity = WEAK;

int cooldownHours = 0;
int cooldownMinutes = 0;
int cooldownSeconds = 5;

float maxPHLevelThreshold = 8.0;
float minPHLevelThreshold = 3.0;  
float waterOutDurationSec = 120.0; 


WaterChangeState previousWaterChangeState = IDLE;


void updateCurrentShakeIntensity() {
  switch (currentShakeIntensityConfig) {
    case 0:
      currentShakeIntensity = WEAK;
      break;
    case 1:
      currentShakeIntensity = MEDIUM;
      break;
    case 2:
      currentShakeIntensity = STRONG;
      break;
  }
}


void calculateDynamicCooldown() {
  waterOutCooldownMs = (cooldownHours * 3600000UL) + (cooldownMinutes * 60000UL) + (cooldownSeconds * 1000UL);
}

void monitorPHLevel() {
  int pHLebelRawAnalogValue = analogRead(pHLevelPinIn);
  currentpHLevel = pHLebelRawAnalogValue * (14.0 / 4095.0); 
}

void monitorTemperature() {
  int temperatureRawAnalogValue = analogRead(temperaturePinIn);
  currentTemperature = temperatureRawAnalogValue * (14.0 / 4095.0);
}



void timedWaterOut() {
  unsigned long currentMillis = millis();
  unsigned long waterOutDurationMs = waterOutDurationSec * 1000;
  
  if (!isWaterOutOpen) {
    openWaterOutServo();
  } else {
    if (currentMillis - waterOutOpenedTime >= waterOutDurationMs) {
      closeWaterOutServo();
      currentWaterChangeState = STABILIZING;
    }
  }
}

void processFeedingSchedule() {
  struct tm timeinfo;
  
  // 10ms timeout prevents loop blocking if NTP drops
  if (synced && getLocalTime(&timeinfo, 10)) {
    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;

    for (int i = 0; i < MAX_FEEDS; i++) {
      if (feedHours[i] == -1) continue; 

      if (currentHour == feedHours[i] && currentMinute == feedMinutes[i]) {
        if (!alreadyTriggered[i]) {
          Serial.print("Action: Scheduled Feeding Triggered for slot ");
          Serial.println(i + 1);
          
          shakeFeederServo(currentShakeIntensity); 
          
          alreadyTriggered[i] = true; 
        }
      } else {
        alreadyTriggered[i] = false;
      }
    }
  }
}

void processWaterChangeState() {
  if (previousWaterChangeState != currentWaterChangeState) {
    String stateStr = "";
    
    // Map the enum to a string for the console and the cloud
    switch(currentWaterChangeState) {
      case IDLE:  stateStr = "IDLE";  break;
      case DRAINING:    stateStr = "DRAINING";    break;
      case STABILIZING: stateStr = "STABILIZING"; break;
    }

    Serial.print("Water Change State: ");
    Serial.println(stateStr);
    
    // Only transmit to the cloud if the system is currently connected
    if (synced) {
      sendCurrentWaterChangeState(stateStr);
    }
    
    // Lock the state tracker to prevent further transmissions until the next change
    previousWaterChangeState = currentWaterChangeState;
  }
  monitorPHLevel();
  monitorTemperature();
  unsigned long currentMillis = millis();

  
  switch (currentWaterChangeState) {
    case IDLE:
      if (currentpHLevel > maxPHLevelThreshold || currentpHLevel < minPHLevelThreshold) {
    currentWaterChangeState = DRAINING;
}
      break;
    case DRAINING:
      timedWaterOut();
      break;
    case STABILIZING:
      if (currentMillis - lastWaterOutTriggerTime >= waterOutCooldownMs) {
        currentWaterChangeState = IDLE;
      }
  }
}

