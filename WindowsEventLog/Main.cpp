#include <Windows.h>
#include <comdef.h>
#include <cassert>
#include <iostream>

#include "../MyEventProvider/MyEventProvider.h" // custom event provider


/** C++ RAII wrapper of the legacy "Event Logging" API for generating log events. */
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

    /** Write log entry with insertion strings. */
    void WriteInsertStrings(WORD type, WORD category, DWORD eventId, WORD stringCount, const wchar_t* strings[]) {
        BOOL ok = ReportEventW(m_log, type, category, eventId, NULL, stringCount, /*raw data bytes*/0, strings, /*raw data*/NULL);
        if (!ok) {
            _com_error err(GetLastError());
            wprintf(L"ERROR: ReportEventW failed (%s)\n", err.ErrorMessage());
            abort();
        }
    }

    /** Write log entry with extra user-defined binary data. */
    void WriteUserData(WORD type, WORD category, DWORD eventId, DWORD rawDataSize, const BYTE* rawData) {
        BOOL ok = ReportEventW(m_log, type, category, eventId, NULL, /*strCount*/0, rawDataSize, /*strings*/NULL, (void*)rawData);
        if (!ok) {
            _com_error err(GetLastError());
            wprintf(L"ERROR: ReportEventW failed (%s)\n", err.ErrorMessage());
            abort();
        }
    }

private:
    HANDLE m_log = 0;
};


int wmain() {
    // open event provider
    EventLog log(L"MyEventProvider"); // or L"Application" or L"System"

    // generate log entries defined by the provider
    {
        WORD type = EVENTLOG_ERROR_TYPE; //  or other EVENTLOG_xxx type
        WORD category = UI_CATEGORY; // source-specific category
        DWORD eventId = MSG_INVALID_COMMAND; // entry in the message file associated with the event source
        const BYTE data[] = "Extra command details";

        wprintf(L"Writing log entry...\n");
        log.WriteUserData(type, category, eventId, sizeof(data), data);
    }
    {
        WORD type = EVENTLOG_ERROR_TYPE; //  or other EVENTLOG_xxx type
        WORD category = DATABASE_CATEGORY; // source-specific category
        DWORD eventId = MSG_BAD_FILE_CONTENTS; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"somefile.txt" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
    }
    {
        WORD type = EVENTLOG_WARNING_TYPE; // or other EVENTLOG_xxx type
        WORD category = NETWORK_CATEGORY; // source-specific category
        DWORD eventId = MSG_RETRIES; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"25", L"zero" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
    }
    {
        WORD type = EVENTLOG_INFORMATION_TYPE; //  or other EVENTLOG_xxx type
        WORD category = UI_CATEGORY; // source-specific category
        DWORD eventId = MSG_COMPUTE_CONVERSION; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"8", L"2" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
    }

    printf("[done]\n");
    return 0;
}
