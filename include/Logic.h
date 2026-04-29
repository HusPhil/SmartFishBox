#pragma once

#include <Arduino.h>
#include <time.h>
#include "Config.h"
#include "Actuators.h"

// --- Shared Global State Variables ---
// 'extern' tells the compiler these exist, but are defined in Logic.cpp
extern WaterChangeState currentWaterChangeState;

extern int feedHours[MAX_FEEDS];
extern int feedMinutes[MAX_FEEDS];
extern bool alreadyTriggered[MAX_FEEDS];
extern int cycles;
extern bool synced;
extern float currentpHLevel;
extern float currentTemperature;
extern unsigned long waterOutCooldownMs;
extern int currentShakeIntensityConfig;

extern int feedNow; // Flag to trigger immediate feeding from the cloud
extern int changeWaterNow; // Flag to trigger immediate water change from the cloud

extern int cooldownHours;
extern int cooldownMinutes;
extern int cooldownSeconds;


extern float maxPHLevelThreshold;
extern float minPHLevelThreshold;
extern float waterOutDurationSec; 



// --- High-Level Execution Prototypes ---
void monitorPHLevel();
void monitorTemperature();
void processFeedingSchedule();
void processWaterChangeState();
void calculateDynamicCooldown();
void updateCurrentShakeIntensity();