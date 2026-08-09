#include "history_export.h"

#include <stdio.h>
#include <string.h>

namespace {
const char *kindName(HistorySessionKind value) { return value == HistorySessionKind::Mash ? "mash" : "boil"; }
const char *resultName(HistoryResult value) { switch (value) { case HistoryResult::Success: return "success"; case HistoryResult::Aborted: return "aborted"; case HistoryResult::SensorFault: return "sensor_fault"; default: return "interrupted"; } }
void centi(char *out, size_t capacity, int16_t value) { snprintf(out, capacity, "%d.%02d", value / 100, value < 0 ? -(value % 100) : value % 100); }
}
HistoryExportWriter::HistoryExportWriter(HistoryExportFormat f, HistoryTextSink s) : format(f), sink(s), selected(nullptr) {}
bool HistoryExportWriter::emit(const char *text, size_t length) { return sink.write != nullptr && sink.write(text, length, sink.context); }
bool HistoryExportWriter::emitText(const char *text) { return emit(text, strlen(text)); }
bool HistoryExportWriter::emitJsonString(const char *text) {
    if (!emitText("\"")) return false;
    for (size_t i = 0; text[i] != '\0'; ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == '"' || c == '\\') { char pair[] = {'\\', char(c)}; if (!emit(pair, 2)) return false; }
        else if (c == '\n') { if (!emitText("\\n")) return false; }
        else if (c == '\r') { if (!emitText("\\r")) return false; }
        else if (c < 0x20) { char escaped[7]; snprintf(escaped, sizeof(escaped), "\\u%04x", unsigned(c)); if (!emitText(escaped)) return false; }
        else if (!emit(reinterpret_cast<const char *>(&c), 1)) return false;
    }
    return emitText("\"");
}
bool HistoryExportWriter::emitCsvField(const char *text) {
    bool formula = text[0] == '=' || text[0] == '+' || text[0] == '-' || text[0] == '@' || text[0] == '\t' || text[0] == '\r';
    if (!emitText("\"")) return false;
    if (formula && !emitText("'")) return false;
    for (size_t i = 0; text[i] != '\0'; ++i) { const char c = text[i] == '\r' || text[i] == '\n' ? ' ' : text[i]; if (!emit(&c, 1)) return false; if (c == '"' && !emit("\"", 1)) return false; }
    return emitText("\"");
}
bool HistoryExportWriter::begin(uint32_t budgetBytes, uint32_t retainedBytes, const HistorySessionSummary *session) {
    if (started || (format == HistoryExportFormat::SessionListJson) != (session == nullptr)) return false;
    started = true; selected = session;
    char header[80];
    if (format == HistoryExportFormat::SessionListJson) { snprintf(header, sizeof(header), "{\"schema_version\":1,\"budget_bytes\":%lu,\"retained_bytes\":%lu,\"sessions\":[", static_cast<unsigned long>(budgetBytes), static_cast<unsigned long>(retainedBytes)); return emitText(header); }
    if (format == HistoryExportFormat::SessionJson) return emitText("{\"schema_version\":1,\"id\":") && (snprintf(header, sizeof(header), "%lu", static_cast<unsigned long>(session->id)), emitText(header)) && emitText(",\"records\":[");
    return emitText("session_id,kind,result,recipe_name,duration_seconds,truncated,elapsed_seconds,temp_min_c,temp_avg_c,temp_max_c,target_c,heater_on_percent,mode,step,observations\r\n");
}
bool HistoryExportWriter::writeSession(const HistorySessionSummary &session) {
    if (!started || finished || format != HistoryExportFormat::SessionListJson) return false;
    if (!first && !emitText(",")) return false;
    first = false;
    char numeric[160]; snprintf(numeric, sizeof(numeric), "{\"id\":%lu,\"kind\":\"%s\",\"result\":\"%s\",\"recipe_name\":", static_cast<unsigned long>(session.id), kindName(session.kind), resultName(session.result));
    return emitText(numeric) && emitJsonString(session.recipe.name) && (snprintf(numeric, sizeof(numeric), ",\"duration_seconds\":%lu,\"record_count\":%u,\"truncated\":%s}", static_cast<unsigned long>(session.durationSeconds), static_cast<unsigned>(session.recordCount), session.truncated ? "true" : "false"), emitText(numeric));
}
bool HistoryExportWriter::writeRecord(const HistoryAggregate &record) {
    if (!started || finished || format == HistoryExportFormat::SessionListJson || selected == nullptr) return false;
    char min[16], avg[16], max[16], target[16], output[256]; centi(min,sizeof(min),record.actualMinCentiC); centi(avg,sizeof(avg),record.actualMeanCentiC); centi(max,sizeof(max),record.actualMaxCentiC); centi(target,sizeof(target),record.targetCentiC);
    if (format == HistoryExportFormat::SessionJson) { if (!first && !emitText(",")) return false; first = false; snprintf(output,sizeof(output),"{\"elapsed_seconds\":%lu,\"temp_min_c\":%s,\"temp_avg_c\":%s,\"temp_max_c\":%s,\"target_c\":%s,\"heater_on_percent\":%u.%u,\"mode\":%u,\"step\":%u,\"observations\":%u}",static_cast<unsigned long>(record.elapsedSeconds),min,avg,max,target,static_cast<unsigned>(record.heaterOnPermille/10),static_cast<unsigned>(record.heaterOnPermille%10),static_cast<unsigned>(record.mode),static_cast<unsigned>(record.step),static_cast<unsigned>(record.observations)); wroteRecord = true; return emitText(output); }
    char prefix[64]; snprintf(prefix,sizeof(prefix),"%lu,%s,%s,",static_cast<unsigned long>(selected->id),kindName(selected->kind),resultName(selected->result)); if (!emitText(prefix) || !emitCsvField(selected->recipe.name)) return false; snprintf(output,sizeof(output),",%lu,%s,%lu,%s,%s,%s,%s,%u.%u,%u,%u,%u\r\n",static_cast<unsigned long>(selected->durationSeconds),selected->truncated?"true":"false",static_cast<unsigned long>(record.elapsedSeconds),min,avg,max,target,static_cast<unsigned>(record.heaterOnPermille/10),static_cast<unsigned>(record.heaterOnPermille%10),static_cast<unsigned>(record.mode),static_cast<unsigned>(record.step),static_cast<unsigned>(record.observations)); wroteRecord=true; return emitText(output);
}
bool HistoryExportWriter::finish() { if (!started || finished) return false; finished=true; if (format == HistoryExportFormat::SessionListJson) return emitText("]}"); if (format == HistoryExportFormat::SessionJson) return emitText("]}"); if (wroteRecord) return true; char prefix[64]; snprintf(prefix,sizeof(prefix),"%lu,%s,%s,",static_cast<unsigned long>(selected->id),kindName(selected->kind),resultName(selected->result)); if (!emitText(prefix)||!emitCsvField(selected->recipe.name)) return false; char tail[64]; snprintf(tail,sizeof(tail),",%lu,%s,,,,,,,,,\r\n",static_cast<unsigned long>(selected->durationSeconds),selected->truncated?"true":"false"); return emitText(tail); }
