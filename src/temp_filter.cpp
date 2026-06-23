#include "temp_filter.h"

float filterTemp(float accepted, float reading, int &counter) {
    if (reading != accepted && counter < 5) {
        counter++;
        return accepted;  // hold the old value until the change persists
    }
    counter = 0;
    return reading;
}
