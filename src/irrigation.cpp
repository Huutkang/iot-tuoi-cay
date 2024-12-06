#include "irrigation.h"
#include <Arduino.h>



void setupIrrigation(int RL[4]) {
    for (int i = 0; i < 4; i++) {
        pinMode(RL[i], OUTPUT);
        digitalWrite(RL[i], HIGH); // Tắt bơm ban đầu
    }
}

void manageIrrigation(int RL[4], bool status[4]) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(RL[i], !status[i]);
    }
}
