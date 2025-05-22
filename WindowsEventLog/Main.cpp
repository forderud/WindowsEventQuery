#include <Windows.h>
#include <Shlobj.h> // for IsUserAnAdmin
#include <comdef.h>
#include <cassert>
#include <iostream>


class EventLog {
public:
    EventLog(const wchar_t* source) {
        m_log = RegisterEventSourceW(NULL, source);
        if (!m_log) {
            _com_error err(GetLastError());
            wprintf(L"ERROR: RegisterEventSourceW failed (%s)\n", err.ErrorMessage());
            abort();
        }
    }

    ~EventLog() {
        DeregisterEventSource(m_log);
        m_log = 0;
    }

    void WriteLog(WORD type, WORD category, DWORD eventId, WORD stringCount, const wchar_t* messages[]) {
        BOOL ok = ReportEventW(m_log, type, category, eventId, NULL, stringCount, /*raw data bytes*/0, messages, /*raw data*/NULL);
        if (!ok) {
            _com_error err(GetLastError());
            wprintf(L"ERROR: ReportEventW failed (%s)\n", err.ErrorMessage());
            abort();
        }
    }

private:
    HANDLE m_log = 0;
};


int main() {
#if 0
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges missing.\n");
        return -1;
    }
#endif

    // open log
    EventLog log (L"Application"); // or L"System"

    const wchar_t* messages[] = {L"Hello event log!"};
    WORD type     = EVENTLOG_SUCCESS;
    WORD category = 0; // source-specific category
    DWORD eventId = 0; // entry in the message file associated with the event source

    wprintf(L"Adding a '%s' log entry to the log..\n", messages[0]);
    log.WriteLog(type, category, eventId, std::size(messages), messages);
    printf("[done]\n");
}
