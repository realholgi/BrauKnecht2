#include <unity.h>
#include <string.h>
#include "brew_history.h"
#include "history_export.h"

void setUp(void) {}
void tearDown(void) {}

static HistoryObservation observation(int actual, int target, bool heater, uint8_t mode = 3, uint8_t step = 1) {
    return {static_cast<int16_t>(actual), static_cast<int16_t>(target), heater, mode, step};
}

void test_one_minute_bucket_and_next_boundary(void) {
    HistoryRecorderState state;
    HistoryAggregate aggregate{};
    startHistoryRecorder(state, observation(1000, 6500, true), 0);
    for (uint32_t second = 5; second <= 55; second += 5)
        TEST_ASSERT_FALSE(observeHistory(state, observation(static_cast<int>(1000 + second), 6500, (second % 10) == 0), second * 1000U, aggregate));
    TEST_ASSERT_TRUE(observeHistory(state, observation(1100, 6600, false), 60000U, aggregate));
    TEST_ASSERT_EQUAL_UINT32(60, aggregate.elapsedSeconds);
    TEST_ASSERT_EQUAL_UINT8(12, aggregate.observations);
    TEST_ASSERT_EQUAL_INT16(1000, aggregate.actualMinCentiC);
    TEST_ASSERT_EQUAL_INT16(1055, aggregate.actualMaxCentiC);
    TEST_ASSERT_EQUAL_INT16(1027, aggregate.actualMeanCentiC);
    TEST_ASSERT_EQUAL_INT16(6500, aggregate.targetCentiC);
    TEST_ASSERT_EQUAL_UINT16(500, aggregate.heaterOnPermille);
    TEST_ASSERT_FALSE(observeHistory(state, observation(1110, 6600, false), 64000U, aggregate));
}

void test_phase_changes_flush_and_target_changes_coalesce(void) {
    HistoryRecorderState state;
    HistoryAggregate aggregate{};
    startHistoryRecorder(state, observation(1000, 6000, true), 100);
    TEST_ASSERT_FALSE(observeHistory(state, observation(1010, 6100, false), 1000, aggregate));
    TEST_ASSERT_TRUE(observeHistory(state, observation(1020, 6200, false, 4, 2), 2000, aggregate));
    TEST_ASSERT_EQUAL_UINT8(1, aggregate.observations);
    TEST_ASSERT_EQUAL_INT16(6000, aggregate.targetCentiC);
    TEST_ASSERT_TRUE(finishHistoryRecorder(state, 3000, aggregate));
    TEST_ASSERT_EQUAL_UINT8(1, aggregate.observations);
    TEST_ASSERT_EQUAL_INT16(6200, aggregate.targetCentiC);
}

void test_codec_round_trip_and_corrupt_tail(void) {
    Recipe recipe{};
    recipe.name[0] = 'X'; recipe.maischtemp = 65; recipe.rasten = 1; recipe.rastTemp[1] = 66; recipe.rastZeit[1] = 30; recipe.endtemp = 78; recipe.kochzeit = 60; recipe.hopfenanzahl = 1; recipe.hopfenZeit[1] = 15;
    uint8_t bytes[HISTORY_HEADER_FRAME_BYTES + HISTORY_AGGREGATE_FRAME_BYTES + HISTORY_COMPLETION_FRAME_BYTES];
    size_t used = encodeHistoryHeader(bytes, sizeof(bytes), 7, HistorySessionKind::Mash, recipe);
    HistoryAggregate a{60, -100, 0, 100, 6500, 500, 4, 1, 12};
    used += encodeHistoryAggregate(bytes + used, sizeof(bytes) - used, a);
    used += encodeHistoryCompletion(bytes + used, sizeof(bytes) - used, HistoryResult::Aborted, 60, 1, false);
    HistoryParseResult parsed = parseHistoryFrames(bytes, used);
    TEST_ASSERT_TRUE(parsed.valid);
    TEST_ASSERT_EQUAL_UINT32(7, parsed.summary.id);
    TEST_ASSERT_EQUAL_INT16(-100, a.actualMinCentiC);
    TEST_ASSERT_EQUAL_UINT16(1, parsed.summary.recordCount);
    bytes[20] ^= 1;
    parsed = parseHistoryFrames(bytes, used);
    TEST_ASSERT_FALSE(parsed.valid);
    TEST_ASSERT_EQUAL_UINT(0, parsed.validBytes);
}

void test_retention_boundaries(void) {
    TEST_ASSERT_TRUE(historyRetentionNeedsEviction(HISTORY_STORAGE_BUDGET_BYTES, 0, 1, 0));
    TEST_ASSERT_TRUE(historyRetentionNeedsEviction(0, 32, 1, 1));
    TEST_ASSERT_FALSE(historyRetentionNeedsEviction(0, 31, 1, 1));
    TEST_ASSERT_TRUE(historyActiveCanAppend(HISTORY_MAX_SESSION_BYTES - 24, 24, 0));
    TEST_ASSERT_TRUE(historyActiveCanAppend(HISTORY_MAX_SESSION_BYTES - 39, 24, 15));
}

struct TextBuffer { char bytes[512]{}; size_t used = 0; };
static bool captureText(const char *text, size_t length, void *context) {
    TextBuffer *buffer = static_cast<TextBuffer *>(context);
    if (length > sizeof(buffer->bytes) - buffer->used - 1) return false;
    for (size_t i = 0; i < length; ++i) buffer->bytes[buffer->used++] = text[i];
    return true;
}
void test_export_writer_escapes_json_and_csv_formula_names(void) {
    HistorySessionSummary session{}; session.id = 2; session.recipe.name[0] = '='; session.recipe.name[1] = 'x'; session.recipe.name[2] = '"'; session.recipe.name[3] = '\0';
    TextBuffer json; HistoryExportWriter jsonWriter(HistoryExportFormat::SessionListJson, {captureText, &json});
    TEST_ASSERT_TRUE(jsonWriter.begin(HISTORY_STORAGE_BUDGET_BYTES, 0, nullptr));
    TEST_ASSERT_TRUE(jsonWriter.writeSession(session)); TEST_ASSERT_TRUE(jsonWriter.finish());
    TEST_ASSERT_NOT_NULL(strstr(json.bytes, "=x"));
    TextBuffer csv; HistoryExportWriter csvWriter(HistoryExportFormat::SessionCsv, {captureText, &csv});
    TEST_ASSERT_TRUE(csvWriter.begin(HISTORY_STORAGE_BUDGET_BYTES, 0, &session)); TEST_ASSERT_TRUE(csvWriter.finish());
    TEST_ASSERT_NOT_NULL(strstr(csv.bytes, "'=x"));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_one_minute_bucket_and_next_boundary);
    RUN_TEST(test_phase_changes_flush_and_target_changes_coalesce);
    RUN_TEST(test_codec_round_trip_and_corrupt_tail);
    RUN_TEST(test_retention_boundaries);
    RUN_TEST(test_export_writer_escapes_json_and_csv_formula_names);
    return UNITY_END();
}
