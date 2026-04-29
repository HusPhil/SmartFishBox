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
float neutralVoltage = 2.53;   // Voltage at pH 7
float sensitivity = 0.18;      // Voltage change per pH unit
WaterChangeState currentWaterChangeState = IDLE;

unsigned long waterOutCooldownMs = 5 * 1000;
int currentShakeIntensityConfig = 0;

ShakeIntensity currentShakeIntensity = WEAK;

int feedNow = 0; // Flag to trigger immediate feeding from the cloud
int changeWaterNow = 0; // Flag to trigger immediate water change from the cloud
bool manualTriggered = false;

int cooldownHours = 0;
int cooldownMinutes = 0;
int cooldownSeconds = 5;

float maxPHLevelThreshold = 8.0;
float minPHLevelThreshold = 3.0;  
float waterOutDurationSec = 120.0; 

//add averaging for pH level readings to stabilize the value
float readPHLevelVoltage() {
  float voltage = 0;
  for (int i = 0; i < 50; i++) {
      voltage += analogRead(pHLevelPinIn) * (3.3 / 4095.0);
      delay(10);
  }
  voltage = voltage / 50; // This is your stable average

  return voltage;
}

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
  int pHLevelRawAnalogValue  = analogRead(pHLevelPinIn);
  float voltage = readPHLevelVoltage();

  // Serial.print("Raw: ");
  // Serial.println(pHLevelRawAnalogValue);

  currentpHLevel = 7 + (neutralVoltage - voltage) / sensitivity;

  Serial.print("Raw: ");
  Serial.println(pHLevelRawAnalogValue);

  Serial.print("Voltage: ");
  Serial.println(voltage, 3);

  Serial.print("pH Level: ");
  Serial.println(currentpHLevel, 2);
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
  
  // --- 1. HANDLE MANUAL BLYNK TRIGGER ---
  if (feedNow == 1) {
    Serial.println("Action: Manual Feeding Triggered via Blynk");
    
    // Perform the action
    shakeFeederServo(currentShakeIntensity);
    
    // Reset the local variable immediately
    feedNow = 0; 
    
    resetFeedNowFlag(); // Ensure the flag is reset in the cloud as well
  }

  // --- 2. HANDLE SCHEDULED FEEDING ---
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
  // --- 1. HANDLE STATE TRANSITION LOGGING & CLOUD SYNC ---
  if (previousWaterChangeState != currentWaterChangeState) {
    String stateStr = "";
    switch(currentWaterChangeState) {
      case IDLE:        stateStr = "IDLE";        break;
      case DRAINING:    stateStr = "DRAINING";    break;
      case STABILIZING: stateStr = "STABILIZING"; break;
    }
    Serial.print("Water Change State: ");
    Serial.println(stateStr);

    if (synced) {
      sendCurrentWaterChangeState(stateStr);
    }
    previousWaterChangeState = currentWaterChangeState;
  }

  // --- 2. SENSOR MONITORING ---
  monitorPHLevel();
  monitorTemperature();
  delay(1500); 
  unsigned long currentMillis = millis();

  // --- 3. STATE MACHINE LOGIC ---
  switch (currentWaterChangeState) {
    case IDLE:
      // Check for Manual Blynk Trigger OR pH Threshold Breach
      if (changeWaterNow == 1 || currentpHLevel > maxPHLevelThreshold || currentpHLevel < minPHLevelThreshold) {
        
        if (changeWaterNow == 1) {
            Serial.println("Action: Manual Water Change Triggered via Blynk");
            changeWaterNow = 0;      // Reset local flag
            resetChangeWaterNowFlag(); // Reset remote Blynk flag
        } else {
            Serial.println("Action: Auto Water Change Triggered by pH Level");
        }

        currentWaterChangeState = DRAINING;
      }
      break;

    case DRAINING:
      timedWaterOut();
      break;

    case STABILIZING:
      // Ensure we clear the manual flag if it was pressed while already draining/stabilizing
      if (changeWaterNow == 1) {
          changeWaterNow = 0;
          resetChangeWaterNowFlag();
      }

      if (currentMillis - lastWaterOutTriggerTime >= waterOutCooldownMs) {
        currentWaterChangeState = IDLE;
      }
      break;
  }
}
