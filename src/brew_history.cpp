#include "brew_history.h"

#include <cmath>
#include <limits.h>
namespace {
uint32_t crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1U) ^ ((crc & 1U) ? 0xEDB88320U : 0U);
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

void put16(uint8_t *out, size_t &at, uint16_t value) { out[at++] = static_cast<uint8_t>(value); out[at++] = static_cast<uint8_t>(value >> 8U); }
void put32(uint8_t *out, size_t &at, uint32_t value) { for (uint8_t i = 0; i < 4; ++i) out[at++] = static_cast<uint8_t>(value >> (8U * i)); }
uint16_t get16(const uint8_t *in, size_t &at) { const uint16_t v = uint16_t(in[at]) | (uint16_t(in[at + 1]) << 8U); at += 2; return v; }
uint32_t get32(const uint8_t *in, size_t &at) { uint32_t v = 0; for (uint8_t i = 0; i < 4; ++i) v |= uint32_t(in[at++]) << (8U * i); return v; }
int16_t boundedCenti(float value) {
    if (value > 327.67F) return INT16_MAX;
    if (value < -327.68F) return INT16_MIN;
    return static_cast<int16_t>(std::lround(value * 100.0F));
}
uint32_t elapsedSeconds(uint64_t elapsedMs) {
    const uint64_t seconds = elapsedMs / 1000U;
    return seconds > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(seconds);
}
bool validKind(uint8_t value) { return value == uint8_t(HistorySessionKind::Mash) || value == uint8_t(HistorySessionKind::Boil); }
bool validResult(uint8_t value) { return value >= uint8_t(HistoryResult::Success) && value <= uint8_t(HistoryResult::Interrupted); }

bool appendObservation(HistoryRecorderState &state, const HistoryObservation &o) {
    if (state.observations == 0) {
        state.actualMin = o.actualCentiC;
        state.actualMax = o.actualCentiC;
        state.mode = o.mode;
        state.step = o.step;
    } else {
        if (o.actualCentiC < state.actualMin) state.actualMin = o.actualCentiC;
        if (o.actualCentiC > state.actualMax) state.actualMax = o.actualCentiC;
    }
    state.actualSum += o.actualCentiC;
    state.lastTarget = o.targetCentiC;
    ++state.observations;
    if (o.heaterOn) ++state.heaterOnObservations;
    return true;
}
void buildAggregate(const HistoryRecorderState &state, HistoryAggregate &out) {
    out.elapsedSeconds = elapsedSeconds(state.elapsedMs);
    out.actualMinCentiC = state.actualMin;
    out.actualMeanCentiC = static_cast<int16_t>(state.actualSum / state.observations);
    out.actualMaxCentiC = state.actualMax;
    out.targetCentiC = state.lastTarget;
    out.heaterOnPermille = static_cast<uint16_t>((uint32_t(state.heaterOnObservations) * 1000U) / state.observations);
    out.mode = state.mode;
    out.step = state.step;
    out.observations = state.observations;
}
void resetBucket(HistoryRecorderState &state, uint32_t nowMs, const HistoryObservation &o) {
    state.bucketStartedMs = nowMs;
    state.lastObservationMs = nowMs;
    state.actualSum = 0;
    state.observations = 0;
    state.heaterOnObservations = 0;
    appendObservation(state, o);
}
size_t encodeFrame(uint8_t type, const uint8_t *payload, size_t payloadLength, uint8_t *out, size_t capacity) {
    if (payloadLength > UINT16_MAX || capacity < payloadLength + 7U) return 0;
    out[0] = type;
    out[1] = static_cast<uint8_t>(payloadLength);
    out[2] = static_cast<uint8_t>(payloadLength >> 8U);
    for (size_t i = 0; i < payloadLength; ++i) out[3 + i] = payload[i];
    size_t at = 3 + payloadLength;
    put32(out, at, crc32(out, at));
    return at;
}
}

bool historyObservationFromFloat(float actualC, int targetC, bool heaterOn, uint8_t mode, uint8_t step, HistoryObservation &out) {
    if (!std::isfinite(actualC)) return false;
    out.actualCentiC = boundedCenti(actualC);
    out.targetCentiC = targetC > 327 ? INT16_MAX : targetC < -328 ? INT16_MIN : static_cast<int16_t>(targetC * 100);
    out.heaterOn = heaterOn;
    out.mode = mode;
    out.step = step;
    return true;
}

void startHistoryRecorder(HistoryRecorderState &state, const HistoryObservation &initial, uint32_t nowMs) {
    state = HistoryRecorderState{};
    state.active = true;
    state.lastNowMs = nowMs;
    resetBucket(state, nowMs, initial);
}

bool observeHistory(HistoryRecorderState &state, const HistoryObservation &o, uint32_t nowMs, HistoryAggregate &ready) {
    if (!state.active) return false;
    state.elapsedMs += uint32_t(nowMs - state.lastNowMs);
    state.lastNowMs = nowMs;
    if (o.mode != state.mode || o.step != state.step) {
        buildAggregate(state, ready);
        resetBucket(state, nowMs, o);
        return true;
    }
    if (uint32_t(nowMs - state.lastObservationMs) < HISTORY_OBSERVATION_INTERVAL_MS) return false;
    if (uint32_t(nowMs - state.bucketStartedMs) >= HISTORY_AGGREGATE_INTERVAL_MS) {
        buildAggregate(state, ready);
        resetBucket(state, nowMs, o);
        return true;
    }
    state.lastObservationMs = nowMs;
    appendObservation(state, o);
    return false;
}

bool finishHistoryRecorder(HistoryRecorderState &state, uint32_t nowMs, HistoryAggregate &ready) {
    if (!state.active) return false;
    state.elapsedMs += uint32_t(nowMs - state.lastNowMs);
    state.lastNowMs = nowMs;
    state.active = false;
    if (state.observations == 0) return false;
    buildAggregate(state, ready);
    return true;
}

size_t encodeHistoryHeader(uint8_t *out, size_t capacity, uint32_t id, HistorySessionKind kind, const Recipe &recipe) {
    uint8_t payload[HISTORY_HEADER_PAYLOAD_BYTES] = {};
    size_t at = 0;
    payload[at++] = 1;
    put32(payload, at, id);
    payload[at++] = static_cast<uint8_t>(kind);
    for (size_t i = 0; i < 39 && recipe.name[i] != '\0'; ++i) payload[at + i] = static_cast<uint8_t>(recipe.name[i]);
    at += 40;
    put16(payload, at, static_cast<uint16_t>(recipe.maischtemp));
    payload[at++] = static_cast<uint8_t>(recipe.rasten);
    for (uint8_t i = 1; i <= 7; ++i) { put16(payload, at, static_cast<uint16_t>(i <= recipe.rasten ? recipe.rastTemp[i] : 0)); put16(payload, at, static_cast<uint16_t>(i <= recipe.rasten ? recipe.rastZeit[i] : 0)); }
    put16(payload, at, static_cast<uint16_t>(recipe.endtemp));
    put16(payload, at, static_cast<uint16_t>(recipe.kochzeit));
    payload[at++] = static_cast<uint8_t>(recipe.hopfenanzahl);
    for (uint8_t i = 1; i <= 6; ++i) put16(payload, at, static_cast<uint16_t>(i <= recipe.hopfenanzahl ? recipe.hopfenZeit[i] : 0));
    return encodeFrame(HISTORY_FRAME_HEADER, payload, sizeof(payload), out, capacity);
}
size_t encodeHistoryAggregate(uint8_t *out, size_t capacity, const HistoryAggregate &a) {
    uint8_t payload[HISTORY_AGGREGATE_PAYLOAD_BYTES]; size_t at = 0;
    put32(payload, at, a.elapsedSeconds); put16(payload, at, uint16_t(a.actualMinCentiC)); put16(payload, at, uint16_t(a.actualMeanCentiC)); put16(payload, at, uint16_t(a.actualMaxCentiC)); put16(payload, at, uint16_t(a.targetCentiC)); put16(payload, at, a.heaterOnPermille); payload[at++] = a.mode; payload[at++] = a.step; payload[at++] = a.observations;
    return encodeFrame(HISTORY_FRAME_AGGREGATE, payload, sizeof(payload), out, capacity);
}
size_t encodeHistoryCompletion(uint8_t *out, size_t capacity, HistoryResult result, uint32_t durationSeconds, uint16_t recordCount, bool truncated) {
    uint8_t payload[HISTORY_COMPLETION_PAYLOAD_BYTES] = {}; size_t at = 0;
    payload[at++] = uint8_t(result); put32(payload, at, durationSeconds); put16(payload, at, recordCount); payload[at++] = truncated ? 1 : 0;
    return encodeFrame(HISTORY_FRAME_COMPLETION, payload, sizeof(payload), out, capacity);
}

HistoryParseResult parseHistoryFrames(const uint8_t *data, size_t length, HistoryRecordVisitor visitor, void *context) {
    HistoryParseResult result{};
    if (data == nullptr) return result;
    bool header = false, completion = false;
    uint32_t lastElapsed = 0;
    size_t at = 0;
    while (at < length) {
        const size_t start = at;
        if (length - at < 7) { result.validBytes = start; return result; }
        const uint8_t type = data[at++]; const uint16_t payloadLength = get16(data, at);
        if (payloadLength > length - at - 4) { result.validBytes = start; return result; }
        const uint8_t *payload = data + at; at += payloadLength;
        const uint32_t storedCrc = get32(data, at);
        if (storedCrc != crc32(data + start, 3 + payloadLength)) { result.validBytes = start; return result; }
        if (completion) { result.validBytes = start; return result; }
        size_t p = 0;
        if (type == HISTORY_FRAME_HEADER && !header && payloadLength == HISTORY_HEADER_PAYLOAD_BYTES) {
            if (payload[p++] != 1) { result.validBytes = start; return result; }
            result.summary.id = get32(payload, p); const uint8_t kind = payload[p++];
            if (result.summary.id == 0 || !validKind(kind)) { result.validBytes = start; return result; }
            result.summary.kind = static_cast<HistorySessionKind>(kind);
            bool nul = false; for (size_t i = 0; i < 40; ++i) { result.summary.recipe.name[i] = char(payload[p++]); if (payload[p - 1] == 0) nul = true; }
            if (!nul) { result.validBytes = start; return result; }
            result.summary.recipe.maischtemp = int16_t(get16(payload, p)); result.summary.recipe.rasten = payload[p++];
            if (result.summary.recipe.rasten < 1 || result.summary.recipe.rasten > 7) { result.validBytes = start; return result; }
            for (uint8_t i = 1; i <= 7; ++i) { result.summary.recipe.rastTemp[i] = int16_t(get16(payload, p)); result.summary.recipe.rastZeit[i] = get16(payload, p); }
            result.summary.recipe.endtemp = int16_t(get16(payload, p)); result.summary.recipe.kochzeit = get16(payload, p); result.summary.recipe.hopfenanzahl = payload[p++];
            if (result.summary.recipe.hopfenanzahl < 1 || result.summary.recipe.hopfenanzahl > 6) { result.validBytes = start; return result; }
            for (uint8_t i = 1; i <= 6; ++i) result.summary.recipe.hopfenZeit[i] = get16(payload, p);
            header = true;
        } else if (type == HISTORY_FRAME_AGGREGATE && header && payloadLength == HISTORY_AGGREGATE_PAYLOAD_BYTES) {
            HistoryAggregate a{}; a.elapsedSeconds = get32(payload, p); a.actualMinCentiC = int16_t(get16(payload, p)); a.actualMeanCentiC = int16_t(get16(payload, p)); a.actualMaxCentiC = int16_t(get16(payload, p)); a.targetCentiC = int16_t(get16(payload, p)); a.heaterOnPermille = get16(payload, p); a.mode = payload[p++]; a.step = payload[p++]; a.observations = payload[p++];
            if (a.observations == 0 || a.observations > 12 || a.heaterOnPermille > 1000 || a.actualMinCentiC > a.actualMeanCentiC || a.actualMeanCentiC > a.actualMaxCentiC || a.elapsedSeconds < lastElapsed) { result.validBytes = start; return result; }
            lastElapsed = a.elapsedSeconds; ++result.summary.recordCount; if (visitor != nullptr && !visitor(a, context)) { result.validBytes = at; return result; }
        } else if (type == HISTORY_FRAME_COMPLETION && header && payloadLength == HISTORY_COMPLETION_PAYLOAD_BYTES) {
            const uint8_t rawResult = payload[p++]; result.summary.durationSeconds = get32(payload, p); const uint16_t count = get16(payload, p); const uint8_t flags = payload[p++];
            if (!validResult(rawResult) || (flags & ~1U) != 0 || count != result.summary.recordCount || result.summary.durationSeconds < lastElapsed) { result.validBytes = start; return result; }
            result.summary.result = static_cast<HistoryResult>(rawResult); result.summary.truncated = (flags & 1U) != 0; completion = true;
        } else { result.validBytes = start; return result; }
        result.validBytes = at;
    }
    result.valid = header && completion;
    return result;
}

bool historyRetentionNeedsEviction(uint32_t retainedBytes, uint16_t completedCount, uint32_t bytesToAdd, uint8_t slotsToReserve) {
    return bytesToAdd > HISTORY_STORAGE_BUDGET_BYTES - (retainedBytes > HISTORY_STORAGE_BUDGET_BYTES ? HISTORY_STORAGE_BUDGET_BYTES : retainedBytes) || uint32_t(completedCount) + slotsToReserve > HISTORY_MAX_SESSIONS;
}
bool historyActiveCanAppend(uint32_t activeBytes, uint32_t frameBytes, uint32_t completionReserveBytes) {
    return activeBytes <= HISTORY_MAX_SESSION_BYTES && frameBytes <= HISTORY_MAX_SESSION_BYTES - activeBytes && completionReserveBytes <= HISTORY_MAX_SESSION_BYTES - activeBytes - frameBytes;
}
