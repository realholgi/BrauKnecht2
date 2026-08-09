#pragma once

#include <stddef.h>
#include <stdint.h>
#include "brew_history.h"

enum class HistoryExportFormat : uint8_t { SessionListJson, SessionJson, SessionCsv };
struct HistoryTextSink { bool (*write)(const char *, size_t, void *); void *context; };

class HistoryExportWriter {
public:
    HistoryExportWriter(HistoryExportFormat format, HistoryTextSink sink);
    bool begin(uint32_t budgetBytes, uint32_t retainedBytes, const HistorySessionSummary *session);
    bool writeSession(const HistorySessionSummary &session);
    bool writeRecord(const HistoryAggregate &record);
    bool finish();
private:
    bool emit(const char *text, size_t length);
    bool emitText(const char *text);
    bool emitJsonString(const char *text);
    bool emitCsvField(const char *text);
    HistoryExportFormat format;
    HistoryTextSink sink;
    const HistorySessionSummary *selected;
    bool started = false;
    bool finished = false;
    bool first = true;
    bool wroteRecord = false;
};
