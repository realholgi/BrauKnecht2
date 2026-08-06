#pragma once

void handle_http();
void setupWebserver();
void handleNotFound();
void handleDataJson();
void handleRoot();
bool setupWIFI();
void serviceWiFiAp();
bool isAccessPointEnabled();
bool setAccessPointEnabled(bool enabled);
