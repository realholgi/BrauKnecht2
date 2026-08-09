#include "history_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <stdlib.h>

#include "input.h"

namespace {
constexpr char ACTIVE_PATH[] = "/history/active.bkh";
HistoryRecorderState recorder;
bool workflowLatched = false;
bool recordingEnabled = false;
bool unresolvedSessionFile = false;
bool sampleTruncated = false;
bool hasPendingResult = false;
HistoryResult pendingResult = HistoryResult::Interrupted;
uint32_t activeId = 0;

bool matchingName(const char *name) {
    if (name == nullptr) return false;
    if (strcmp(name, ACTIVE_PATH) == 0) return true;
    return strncmp(name, "/history/S", 10) == 0 || strncmp(name, "/history/F", 10) == 0;
}
void pathFor(uint32_t id, char *out, size_t capacity) { snprintf(out, capacity, "/history/S%08lX.bkh", static_cast<unsigned long>(id)); }
bool writeFrame(File &file, const uint8_t *bytes, size_t length) {
    if (file.write(bytes, length) != length) return false;
    file.flush();
    return true;
}
// Caller must hold EncoderTimerGuard while this helper accesses LittleFS.
uint32_t scanRetainedBytes() {
    uint32_t bytes = 0;
    Dir dir = LittleFS.openDir("/history");
    while (dir.next()) if (matchingName(dir.fileName().c_str())) bytes += dir.fileSize();
    return bytes;
}
// Caller must hold EncoderTimerGuard while this helper accesses LittleFS.
bool loadParsed(const char *path, HistoryParseResult &parsed, HistoryRecordVisitor visitor = nullptr, void *context = nullptr) {
    File file = LittleFS.open(path, "r");
    if (!file) return false;
    const size_t size = file.size();
    if (size == 0 || size > HISTORY_MAX_SESSION_BYTES) { file.close(); return false; }
    uint8_t *data = static_cast<uint8_t *>(malloc(size));
    if (data == nullptr) { file.close(); return false; }
    const size_t read = file.readBytes(reinterpret_cast<char *>(data), size);
    file.close();
    if (read == size) parsed = parseHistoryFrames(data, size, visitor, context);
    free(data);
    return read == size;
}
bool appendAggregate(const HistoryAggregate &aggregate) {
    EncoderTimerGuard guard;
    File file = LittleFS.open(ACTIVE_PATH, "a");
    if (!file) return false;
    const size_t priorSize = file.size();
    if (!historyActiveCanAppend(priorSize, HISTORY_AGGREGATE_FRAME_BYTES, HISTORY_COMPLETION_FRAME_BYTES)) { file.close(); return false; }
    uint8_t frame[HISTORY_AGGREGATE_FRAME_BYTES];
    const size_t length = encodeHistoryAggregate(frame, sizeof(frame), aggregate);
    const bool ok = writeFrame(file, frame, length);
    file.close();
    return ok;
}
void finalizePending() {
    if (!hasPendingResult || !recordingEnabled) return;
    char finalPath[20];
    snprintf(finalPath, sizeof(finalPath), "/history/F%u.bkh", unsigned(pendingResult));
    EncoderTimerGuard guard;
    if (!LittleFS.rename(ACTIVE_PATH, finalPath)) { unresolvedSessionFile = true; return; }
    HistoryParseResult parsed;
    if (!loadParsed(finalPath, parsed) || parsed.validBytes < HISTORY_HEADER_FRAME_BYTES) { unresolvedSessionFile = true; return; }
    File file = LittleFS.open(finalPath, "a");
    if (!file) { unresolvedSessionFile = true; return; }
    uint8_t frame[HISTORY_COMPLETION_FRAME_BYTES];
    const size_t length = encodeHistoryCompletion(frame, sizeof(frame), pendingResult,
        parsed.summary.recordCount == 0 ? 0 : parsed.summary.durationSeconds,
        parsed.summary.recordCount, sampleTruncated);
    if (!writeFrame(file, frame, length)) { file.close(); unresolvedSessionFile = true; return; }
    file.close();
    char completedPath[22]; pathFor(activeId, completedPath, sizeof(completedPath));
    if (!LittleFS.rename(finalPath, completedPath)) { unresolvedSessionFile = true; return; }
    hasPendingResult = false;
    recordingEnabled = false;
    unresolvedSessionFile = false;
}
}

void historySetup() {
    LittleFS.mkdir("/history");
    // A torn active file is retained as interrupted only when its valid prefix can be parsed.
    if (LittleFS.exists(ACTIVE_PATH)) { unresolvedSessionFile = true; }
}

bool historyStartSession(HistorySessionKind kind, const Recipe &recipe, uint32_t nowMs, float actualC,
                         int targetC, bool heaterOn, uint8_t mode, uint8_t step) {
    if (workflowLatched) return recordingEnabled;
    workflowLatched = true;
    EncoderTimerGuard guard;
    if (unresolvedSessionFile || historyRetentionNeedsEviction(scanRetainedBytes(), 0, HISTORY_HEADER_FRAME_BYTES, 1)) return false;
    uint32_t greatest = 0;
    Dir dir = LittleFS.openDir("/history");
    while (dir.next()) { unsigned long id = 0; if (sscanf(dir.fileName().c_str(), "/history/S%8lx.bkh", &id) == 1 && id > greatest) greatest = id; }
    if (greatest == UINT32_MAX) return false;
    activeId = greatest + 1;
    HistoryObservation initial{};
    if (!historyObservationFromFloat(actualC, targetC, heaterOn, mode, step, initial)) return false;
    uint8_t frame[HISTORY_HEADER_FRAME_BYTES];
    const size_t length = encodeHistoryHeader(frame, sizeof(frame), activeId, kind, recipe);
    File file = LittleFS.open(ACTIVE_PATH, "w");
    if (!file || !writeFrame(file, frame, length)) { if (file) file.close(); LittleFS.remove(ACTIVE_PATH); return false; }
    file.close();
    startHistoryRecorder(recorder, initial, nowMs);
    recordingEnabled = true;
    sampleTruncated = false;
    return true;
}

void historyTick(uint32_t nowMs, float actualC, int targetC, bool heaterOn, uint8_t mode, uint8_t step) {
    if (!recordingEnabled || hasPendingResult) return;
    HistoryObservation observation{};
    if (!historyObservationFromFloat(actualC, targetC, heaterOn, mode, step, observation)) return;
    HistoryAggregate aggregate{};
    if (observeHistory(recorder, observation, nowMs, aggregate) && !appendAggregate(aggregate)) sampleTruncated = true;
}

void historyFinishSession(HistoryResult result, uint32_t nowMs) {
    if (!recordingEnabled || hasPendingResult) return;
    HistoryAggregate aggregate{};
    if (finishHistoryRecorder(recorder, nowMs, aggregate) && !appendAggregate(aggregate)) sampleTruncated = true;
    pendingResult = result;
    hasPendingResult = true;
    finalizePending();
}
void historyEndWorkflow() { finalizePending(); workflowLatched = false; }
uint32_t historyRetainedBytes() {
    EncoderTimerGuard guard;
    return scanRetainedBytes();
}

HistoryStoreResult historyListSessionIds(uint32_t *ids, uint8_t capacity, uint8_t &count) {
    count = 0;
    if (ids == nullptr) return HistoryStoreResult::Invalid;
    EncoderTimerGuard guard;
    Dir dir = LittleFS.openDir("/history");
    while (dir.next()) {
        unsigned long id = 0;
        if (sscanf(dir.fileName().c_str(), "/history/S%8lx.bkh", &id) != 1) continue;
        HistoryParseResult parsed;
        if (!loadParsed(dir.fileName().c_str(), parsed) || !parsed.valid) continue;
        if (count < capacity) ids[count++] = uint32_t(id);
    }
    for (uint8_t i = 0; i < count; ++i) for (uint8_t j = i + 1; j < count; ++j) if (ids[j] > ids[i]) { const uint32_t value = ids[i]; ids[i] = ids[j]; ids[j] = value; }
    return HistoryStoreResult::Ok;
}
HistoryStoreResult historyReadSessionSummary(uint32_t id, HistorySessionSummary &out) {
    char path[22]; pathFor(id, path, sizeof(path));
    EncoderTimerGuard guard;
    if (!LittleFS.exists(path)) return HistoryStoreResult::NotFound;
    HistoryParseResult parsed; if (!loadParsed(path, parsed)) return HistoryStoreResult::IoError;
    if (!parsed.valid) return HistoryStoreResult::Invalid;
    out = parsed.summary; return HistoryStoreResult::Ok;
}
HistoryStoreResult historyVisitSessionRecords(uint32_t id, HistoryRecordVisitor visitor, void *context) {
    if (visitor == nullptr) return HistoryStoreResult::Invalid;
    char path[22]; pathFor(id, path, sizeof(path));
    EncoderTimerGuard guard;
    if (!LittleFS.exists(path)) return HistoryStoreResult::NotFound;
    HistoryParseResult parsed; if (!loadParsed(path, parsed, visitor, context)) return HistoryStoreResult::IoError;
    return parsed.valid ? HistoryStoreResult::Ok : HistoryStoreResult::Invalid;
}
