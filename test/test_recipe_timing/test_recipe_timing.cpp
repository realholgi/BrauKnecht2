#include <stdint.h>

#include <unity.h>

#include "recipe_timing.h"

void setUp(void) {}
void tearDown(void) {}

void test_start_reset_and_whole_second_flooring(void) {
    BrewClockState state;

    startBrewClock(state, 100U);
    TEST_ASSERT_TRUE(state.running);
    TEST_ASSERT_EQUAL_UINT32(0U, brewElapsedSeconds(state, 1099U));
    TEST_ASSERT_EQUAL_UINT32(1U, brewElapsedSeconds(state, 1100U));

    resetBrewClock(state);
    TEST_ASSERT_FALSE(state.running);
    TEST_ASSERT_EQUAL_UINT32(0U, brewElapsedSeconds(state, 9000U));
}

void test_pause_excludes_stopped_time_and_resume_adds_active_time(void) {
    BrewClockState state;

    startBrewClock(state, 1000U);
    pauseBrewClock(state, 4000U);
    TEST_ASSERT_EQUAL_UINT32(3U, brewElapsedSeconds(state, 99999U));

    resumeBrewClock(state, 99999U);
    TEST_ASSERT_EQUAL_UINT32(5U, brewElapsedSeconds(state, 101999U));
}

void test_pause_and_resume_are_idempotent(void) {
    BrewClockState state;

    startBrewClock(state, 0U);
    pauseBrewClock(state, 3000U);
    pauseBrewClock(state, 9000U);
    TEST_ASSERT_EQUAL_UINT32(3U, brewElapsedSeconds(state, 9000U));

    resumeBrewClock(state, 9000U);
    resumeBrewClock(state, 20000U);
    TEST_ASSERT_EQUAL_UINT32(4U, brewElapsedSeconds(state, 10000U));
}

void test_active_interval_spanning_millis_wrap_is_correct(void) {
    BrewClockState state;

    startBrewClock(state, UINT32_MAX - 499U);
    TEST_ASSERT_EQUAL_UINT32(1U, brewElapsedSeconds(state, 500U));
}

void test_hops_are_not_due_before_deadline(void) {
    HopDeadlineState state;
    const int hops[] = {0, 5};

    TEST_ASSERT_EQUAL_UINT8(0U, collectDueHops(state, hops, 1, 299U));
    TEST_ASSERT_EQUAL_UINT8(1U, collectDueHops(state, hops, 1, 300U));
}

void test_crossed_hop_deadlines_emit_once_as_batch(void) {
    HopDeadlineState state;
    const int hops[] = {0, 5, 10};

    TEST_ASSERT_EQUAL_UINT8(0U, collectDueHops(state, hops, 2, 299U));
    TEST_ASSERT_EQUAL_UINT8(0x03U, collectDueHops(state, hops, 2, 600U));
    TEST_ASSERT_EQUAL_UINT8(0U, collectDueHops(state, hops, 2, 601U));
}

void test_duplicate_deadlines_emit_separate_bits(void) {
    HopDeadlineState state;
    const int hops[] = {0, 5, 5};

    TEST_ASSERT_EQUAL_UINT8(0x03U, collectDueHops(state, hops, 2, 300U));
}

void test_zero_hops_and_zero_time_deadlines(void) {
    HopDeadlineState empty;
    const int noHops[] = {0};
    TEST_ASSERT_EQUAL_UINT8(0U, collectDueHops(empty, noHops, 0, 0U));

    HopDeadlineState state;
    const int hops[] = {0, 0, -1};
    TEST_ASSERT_EQUAL_UINT8(0x03U, collectDueHops(state, hops, 2, 0U));
}

void test_next_pending_hop_index_tracks_emitted_bits(void) {
    HopDeadlineState state;
    const int hops[] = {0, 1, 2, 3};

    TEST_ASSERT_EQUAL_UINT8(1U, nextPendingHopIndex(state, 3));
    TEST_ASSERT_EQUAL_UINT8(1U, collectDueHops(state, hops, 3, 60U));
    TEST_ASSERT_EQUAL_UINT8(2U, nextPendingHopIndex(state, 3));
    TEST_ASSERT_EQUAL_UINT8(0x06U, collectDueHops(state, hops, 3, 180U));
    TEST_ASSERT_EQUAL_UINT8(0U, nextPendingHopIndex(state, 3));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_start_reset_and_whole_second_flooring);
    RUN_TEST(test_pause_excludes_stopped_time_and_resume_adds_active_time);
    RUN_TEST(test_pause_and_resume_are_idempotent);
    RUN_TEST(test_active_interval_spanning_millis_wrap_is_correct);
    RUN_TEST(test_hops_are_not_due_before_deadline);
    RUN_TEST(test_crossed_hop_deadlines_emit_once_as_batch);
    RUN_TEST(test_duplicate_deadlines_emit_separate_bits);
    RUN_TEST(test_zero_hops_and_zero_time_deadlines);
    RUN_TEST(test_next_pending_hop_index_tracks_emitted_bits);
    return UNITY_END();
}
