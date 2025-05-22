#include <Windows.h>
#include <Shlobj.h> // for IsUserAnAdmin
#include <comdef.h>
#include <cassert>
#include <iostream>


int main() {
#if 0
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges missing.\n");
        return -1;
    }
#endif

    // open log
    const wchar_t LOG_NAME[] = L"Application"; // L"System"
    HANDLE h = RegisterEventSourceW(NULL, LOG_NAME);
    if (!h) {
        _com_error err(GetLastError());
        wprintf(L"ERROR: RegisterEventSourceW failed (%s)\n", err.ErrorMessage());
        abort();
    }

    const wchar_t* message = L"Hello event log!";
    wprintf(L"Adding a '%s' log entry to the %s log..\n", message, LOG_NAME);
    WORD type = EVENTLOG_SUCCESS;
    WORD category = 0;
    DWORD eventId = 0;
    BOOL ok = ReportEventW(h, type, category, eventId, NULL, 1, 0, &message, NULL);
    if (!ok) {
        _com_error err(GetLastError());
        wprintf(L"ERROR: ReportEventW failed (%s)\n", err.ErrorMessage());
        abort();
    }
    printf("[done]\n");

    DeregisterEventSource(h);
}
