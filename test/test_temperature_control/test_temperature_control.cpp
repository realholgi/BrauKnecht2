#include <stdint.h>

#include <unity.h>

#include "temp_filter.h"
#include "temperature_control.h"

void setUp(void) {}
void tearDown(void) {}

void test_initializes_conservatively_at_turn_on_band(void) {
    TemperatureControlState state;

    TEST_ASSERT_TRUE(updateTemperatureControl(state, 64.6F, 65, 0));
    resetTemperatureControl(state);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 64.61F, 65, 0));
    resetTemperatureControl(state);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 65.6F, 65, 0));
}

void test_learns_only_valid_positive_heating_gradients(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    updateTemperatureControl(state, 60.5F, 65, 5000);
    TEST_ASSERT_TRUE(state.hasHeatingRate);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.1F, state.heatingRateCPerSecond);

    updateTemperatureControl(state, 60.5F, 65, 10000);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.1F, state.heatingRateCPerSecond);
    updateTemperatureControl(state, 63.0F, 65, 15000);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.1F, state.heatingRateCPerSecond);
}

void test_predictive_and_hard_cutoffs_obey_minimum_on_time(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.5F, 65, 5000));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 62.0F, 65, 4999));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 62.0F, 65, 5000));

    resetTemperatureControl(state);
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 65.5F, 65, 1));
}

void test_off_dwell_and_deep_undershoot_override(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 65.5F, 65, 1));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 64.5F, 65, 60000));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 64.5F, 65, 60001));

    resetTemperatureControl(state);
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 65.5F, 65, 1));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 62.0F, 65, 2));
}

void test_setpoint_changes_override_old_timers_without_forgetting_rate(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    updateTemperatureControl(state, 60.5F, 65, 5000);
    TEST_ASSERT_TRUE(state.hasHeatingRate);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 65.5F, 65, 5001));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 64.0F, 70, 5002));
    TEST_ASSERT_TRUE(state.hasHeatingRate);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 64.0F, 63, 5003));
}

void test_coast_observation_learns_and_clamps_horizon(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    updateTemperatureControl(state, 60.5F, 65, 5000);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 62.0F, 65, 5001));
    TEST_ASSERT_TRUE(state.observingCoast);
    updateTemperatureControl(state, 80.0F, 65, 6000);
    updateTemperatureControl(state, 79.9F, 65, 7000);
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 75.0F, state.coastSeconds);

    resetTemperatureControl(state);
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    updateTemperatureControl(state, 60.5F, 65, 5000);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 62.0F, 65, 5001));
    updateTemperatureControl(state, 62.1F, 65, 6000);
    updateTemperatureControl(state, 62.0F, 65, 7000);
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 17.5F, state.coastSeconds);
}

void test_reset_and_wraparound_keep_elapsed_time_correct(void) {
    TemperatureControlState state;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, UINT32_MAX - 1000U));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.5F, 65, 3999U));
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 62.0F, 65, 4000U));
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 64.5F, 65, 64000U));

    resetTemperatureControl(state);
    TEST_ASSERT_FALSE(state.initialized);
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, 0));
    TEST_ASSERT_FALSE(state.hasHeatingRate);
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 30.0F, state.coastSeconds);
}

void test_coast_timeout_is_wraparound_safe(void) {
    TemperatureControlState state;
    const uint32_t start = UINT32_MAX - 10000U;
    TEST_ASSERT_TRUE(updateTemperatureControl(state, 60.0F, 65, start));
    updateTemperatureControl(state, 60.5F, 65, start + 5000U);
    TEST_ASSERT_FALSE(updateTemperatureControl(state, 62.0F, 65, start + 5001U));
    TEST_ASSERT_TRUE(state.observingCoast);
    updateTemperatureControl(state, 62.0F, 65, 175001U);
    TEST_ASSERT_FALSE(state.observingCoast);
}

struct PlantResult {
    bool enteredBand;
    bool settled;
    float peak;
};

PlantResult runPlant(float heatRateCPerSecond, float sensorLagSeconds) {
    TemperatureControlState state;
    float vesselC = 20.0F;
    float sensorC = 20.0F;
    float acceptedC = 20.0F;
    int filterCounter = 0;
    bool enteredBand = false;
    bool settled = true;
    int holdSeconds = 0;
    float peak = acceptedC;

    for (uint32_t second = 0; second <= 9000; second++) {
        const bool heaterOn = updateTemperatureControl(
            state, acceptedC, 65, second * 1000U);
        vesselC += (heaterOn ? heatRateCPerSecond : 0.0F) -
                   (vesselC - 20.0F) * 0.00002F;
        sensorC += (vesselC - sensorC) / sensorLagSeconds;
        acceptedC = filterTemp(acceptedC, sensorC, filterCounter);
        peak = acceptedC > peak ? acceptedC : peak;

        if (!enteredBand && acceptedC >= 64.5F && acceptedC <= 65.5F) {
            enteredBand = true;
        }
        if (enteredBand && holdSeconds++ < 1200 &&
            (acceptedC < 64.49F || acceptedC > 65.51F)) {
            settled = false;
        }
    }
    return {enteredBand, settled, peak};
}

void test_adapts_to_low_and_high_thermal_mass(void) {
    const PlantResult lowMass = runPlant(0.08F, 30.0F);
    const PlantResult highMass = runPlant(0.015F, 25.0F);

    TEST_ASSERT_TRUE(lowMass.enteredBand);
    TEST_ASSERT_TRUE(lowMass.settled);
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(66.0F, lowMass.peak);
    TEST_ASSERT_TRUE(highMass.enteredBand);
    TEST_ASSERT_TRUE(highMass.settled);
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(66.0F, highMass.peak);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_initializes_conservatively_at_turn_on_band);
    RUN_TEST(test_learns_only_valid_positive_heating_gradients);
    RUN_TEST(test_predictive_and_hard_cutoffs_obey_minimum_on_time);
    RUN_TEST(test_off_dwell_and_deep_undershoot_override);
    RUN_TEST(test_setpoint_changes_override_old_timers_without_forgetting_rate);
    RUN_TEST(test_coast_observation_learns_and_clamps_horizon);
    RUN_TEST(test_reset_and_wraparound_keep_elapsed_time_correct);
    RUN_TEST(test_adapts_to_low_and_high_thermal_mass);
    RUN_TEST(test_coast_timeout_is_wraparound_safe);
    return UNITY_END();
}
