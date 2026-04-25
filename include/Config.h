#pragma once

// --- Network & Cloud Credentials ---
#define BLYNK_TEMPLATE_ID "TMPL6VptOyE3P"
#define BLYNK_TEMPLATE_NAME "automated feeder"
#define BLYNK_AUTH_TOKEN "MPC1bNbuaj-0-ksdiwBn4j6q5DHCYR5u"
#define BLYNK_PRINT Serial

const char ssid[] = "GFiber_6E19E";
const char pass[] = "8t393nKC";

// --- System Constraints ---
#define MAX_FEEDS 5 

// --- Hardware Pin Definitions ---
const int feederServoPinOut = 14;
const int waterOutServoPinOut = 16;
const int pHLevelPinIn = 33;
const int temperaturePinIn = 32;


// --- Kinematic States ---
enum ShakeIntensity {
  WEAK,
  MEDIUM,
  STRONG
};

enum WaterChangeState {
  IDLE, 
  DRAINING, 
  STABILIZING
};