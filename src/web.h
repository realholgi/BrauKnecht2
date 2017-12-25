#ifndef WEB_H
#define WEB_H

void handle_http();
void setupWebserver();
void handleNotFound();
void handleDataJson();
void handleRoot();
bool setupWIFI();

#endif //WEB_H
