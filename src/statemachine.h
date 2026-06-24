#pragma once

#include "global.h"

void stateMachine();
bool warte_und_weiter(MODUS naechsterModus);
void goBackOneStep();   // long-press in the input/menu screens: back one step
void menu_zeiger(int pos);
