#pragma once

#include <stdint.h>
#include "brew_history.h"

enum class HistoryStoreResult : uint8_t { Ok = 0, NotFound = 1, Invalid = 2, IoError = 3, VisitorStopped = 4 };

void historySetup();
bool historyStartSession(HistorySessionKind kind, const Recipe &recipe, uint32_t nowMs,
                         float actualC, int targetC, bool heaterOn, uint8_t mode, uint8_t step);
void historyTick(uint32_t nowMs, float actualC, int targetC, bool heaterOn, uint8_t mode, uint8_t step);
void historyFinishSession(HistoryResult result, uint32_t nowMs);
void historyEndWorkflow();
uint32_t historyRetainedBytes();
HistoryStoreResult historyListSessionIds(uint32_t *ids, uint8_t capacity, uint8_t &count);
HistoryStoreResult historyReadSessionSummary(uint32_t id, HistorySessionSummary &out);
HistoryStoreResult historyVisitSessionRecords(uint32_t id, HistoryRecordVisitor visitor, void *context);
