#pragma once

void initCloud();
void configureTime();
void resetFeedSchedules();
void connectWifi();
void runCloud();

void resetFeedNowFlag();
void resetChangeWaterNowFlag();

void sendCurrentPHLevel();
void sendCurrentTemperature();
void sendCurrentWaterChangeState(String waterChangeState);