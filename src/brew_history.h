#pragma once

#include <stddef.h>
#include <stdint.h>

#include "recipe.h"

constexpr uint32_t HISTORY_STORAGE_BUDGET_BYTES = 256U * 1024U;
constexpr uint16_t HISTORY_MAX_SESSIONS = 32;
constexpr uint32_t HISTORY_MAX_SESSION_BYTES = 32U * 1024U;
constexpr uint32_t HISTORY_OBSERVATION_INTERVAL_MS = 5000U;
constexpr uint32_t HISTORY_AGGREGATE_INTERVAL_MS = 60000U;

enum class HistorySessionKind : uint8_t { Mash = 1, Boil = 2 };
enum class HistoryResult : uint8_t { Success = 1, Aborted = 2, SensorFault = 3, Interrupted = 4 };

struct HistoryObservation {
    int16_t actualCentiC;
    int16_t targetCentiC;
    bool heaterOn;
    uint8_t mode;
    uint8_t step;
};

struct HistoryAggregate {
    uint32_t elapsedSeconds;
    int16_t actualMinCentiC;
    int16_t actualMeanCentiC;
    int16_t actualMaxCentiC;
    int16_t targetCentiC;
    uint16_t heaterOnPermille;
    uint8_t mode;
    uint8_t step;
    uint8_t observations;
};

struct HistoryRecorderState {
    uint64_t elapsedMs = 0;
    uint32_t lastNowMs = 0;
    uint32_t lastObservationMs = 0;
    uint32_t bucketStartedMs = 0;
    int32_t actualSum = 0;
    int16_t actualMin = 0;
    int16_t actualMax = 0;
    int16_t lastTarget = 0;
    uint8_t mode = 0;
    uint8_t step = 0;
    uint8_t observations = 0;
    uint8_t heaterOnObservations = 0;
    bool active = false;
};
static_assert(sizeof(HistoryRecorderState) <= 64, "history recorder must remain bounded");

bool historyObservationFromFloat(float actualC, int targetC, bool heaterOn, uint8_t mode,
                                 uint8_t step, HistoryObservation &out);
void startHistoryRecorder(HistoryRecorderState &state, const HistoryObservation &initial, uint32_t nowMs);
bool observeHistory(HistoryRecorderState &state, const HistoryObservation &observation,
                    uint32_t nowMs, HistoryAggregate &ready);
bool finishHistoryRecorder(HistoryRecorderState &state, uint32_t nowMs, HistoryAggregate &ready);

struct HistorySessionSummary {
    uint32_t id = 0;
    HistorySessionKind kind = HistorySessionKind::Mash;
    HistoryResult result = HistoryResult::Interrupted;
    Recipe recipe{};
    uint32_t durationSeconds = 0;
    uint16_t recordCount = 0;
    bool truncated = false;
};
using HistoryRecordVisitor = bool (*)(const HistoryAggregate &, void *);

constexpr uint8_t HISTORY_FRAME_HEADER = 0x01;
constexpr uint8_t HISTORY_FRAME_AGGREGATE = 0x02;
constexpr uint8_t HISTORY_FRAME_COMPLETION = 0x03;
constexpr size_t HISTORY_HEADER_PAYLOAD_BYTES = 94;
constexpr size_t HISTORY_AGGREGATE_PAYLOAD_BYTES = 17;
constexpr size_t HISTORY_COMPLETION_PAYLOAD_BYTES = 8;
constexpr size_t HISTORY_HEADER_FRAME_BYTES = 101;
constexpr size_t HISTORY_AGGREGATE_FRAME_BYTES = 24;
constexpr size_t HISTORY_COMPLETION_FRAME_BYTES = 15;

struct HistoryParseResult {
    bool valid = false;
    size_t validBytes = 0;
    HistorySessionSummary summary{};
};

size_t encodeHistoryHeader(uint8_t *out, size_t capacity, uint32_t id,
                           HistorySessionKind kind, const Recipe &recipe);
size_t encodeHistoryAggregate(uint8_t *out, size_t capacity, const HistoryAggregate &aggregate);
size_t encodeHistoryCompletion(uint8_t *out, size_t capacity, HistoryResult result,
                               uint32_t durationSeconds, uint16_t recordCount, bool truncated);
HistoryParseResult parseHistoryFrames(const uint8_t *data, size_t length,
                                      HistoryRecordVisitor visitor = nullptr, void *context = nullptr);

bool historyRetentionNeedsEviction(uint32_t retainedBytes, uint16_t completedCount,
                                   uint32_t bytesToAdd, uint8_t slotsToReserve);
bool historyActiveCanAppend(uint32_t activeBytes, uint32_t frameBytes,
                            uint32_t completionReserveBytes);
