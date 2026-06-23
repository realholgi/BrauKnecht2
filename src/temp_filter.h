#pragma once

// Glitch debounce for the temperature sensor: rejects up to 5 consecutive
// readings that differ from the accepted value before letting a change through.
// Returns the value to keep and updates `counter` in place. Pure — no Arduino
// deps, so it is unit-tested natively (test/test_temp_filter).
float filterTemp(float accepted, float reading, int &counter);
