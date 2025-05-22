#include <Windows.h>
#include <Shlobj.h> // for IsUserAnAdmin
#include <comdef.h>
#include <cassert>
#include <iostream>

#include "../MyEventProvider/MyEventProvider.h" // custom event provider


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

    void WriteInsertStrings(WORD type, WORD category, DWORD eventId, WORD messageCount, const wchar_t* messages[]) {
        BOOL ok = ReportEventW(m_log, type, category, eventId, NULL, messageCount, /*raw data bytes*/0, messages, /*raw data*/NULL);
        if (!ok) {
            _com_error err(GetLastError());
            wprintf(L"ERROR: ReportEventW failed (%s)\n", err.ErrorMessage());
            abort();
        }
    }

    void WriteUserData(WORD type, WORD category, DWORD eventId, DWORD dataSize, void* data) {
        BOOL ok = ReportEventW(m_log, type, category, eventId, NULL, 0, dataSize, NULL, data);
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
#if 0
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges missing.\n");
        return -1;
    }
#endif

    // open log
    EventLog log(L"MyEventProvider"); // or L"Application" or L"System"

    // DOC: https://learn.microsoft.com/en-us/windows/win32/eventlog/reporting-an-event
    {
        WORD type = EVENTLOG_ERROR_TYPE; // or EVENTLOG_INFORMATION_TYPE or EVENTLOG_SUCCESS or other EVENTLOG_xxx types
        WORD category = UI_CATEGORY; // source-specific category
        DWORD eventId = MSG_INVALID_COMMAND; // entry in the message file associated with the event source
        const wchar_t data[] = L"The command that was not valid";

        wprintf(L"Writing log entry...\n");
        log.WriteUserData(type, category, eventId, sizeof(data), (void*)data);
        printf("[done]\n");
    }
    {
        WORD type = EVENTLOG_ERROR_TYPE; // or EVENTLOG_INFORMATION_TYPE or EVENTLOG_SUCCESS or other EVENTLOG_xxx types
        WORD category = DATABASE_CATEGORY; // source-specific category
        DWORD eventId = MSG_BAD_FILE_CONTENTS; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"somefile.txt" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
        printf("[done]\n");
    }
    {
        WORD type = EVENTLOG_WARNING_TYPE; // or EVENTLOG_INFORMATION_TYPE or EVENTLOG_SUCCESS or other EVENTLOG_xxx types
        WORD category = NETWORK_CATEGORY; // source-specific category
        DWORD eventId = MSG_RETRIES; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"25", L"zero" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
        printf("[done]\n");
    }
    {
        WORD type = EVENTLOG_INFORMATION_TYPE; // or EVENTLOG_INFORMATION_TYPE or EVENTLOG_SUCCESS or other EVENTLOG_xxx types
        WORD category = UI_CATEGORY; // source-specific category
        DWORD eventId = MSG_COMPUTE_CONVERSION; // entry in the message file associated with the event source
        const wchar_t* messages[] = { L"8", L"2" };

        wprintf(L"Writing log entry...\n");
        log.WriteInsertStrings(type, category, eventId, std::size(messages), messages);
        printf("[done]\n");
    }

    return 0;
}
