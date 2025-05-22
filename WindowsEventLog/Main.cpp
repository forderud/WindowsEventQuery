#include <Windows.h>
#include <Shlobj.h> // for IsUserAnAdmin
#include <cassert>
#include <iostream>


int main() {
    if (!IsUserAnAdmin()) {
        wprintf(L"ERROR: Admin privileges missing.\n");
        return -1;
    }

    // open log
    const wchar_t LOG_NAME[] = L"System";
    HANDLE h = RegisterEventSourceW(NULL, LOG_NAME);
    if (!h) {
        DWORD err = GetLastError();
        wprintf(L"ERROR: RegisterEventSourceW failed with %u\n", err);
        abort();
    }

    const wchar_t* message = L"Hello event log!";
    wprintf(L"Adding a '%s' log entry to the %s log..\n", message, LOG_NAME);
    BOOL ok = ReportEventW(h, EVENTLOG_SUCCESS, 0, 0, NULL, 1, 0, &message, NULL);
    if (!ok) {
        DWORD err = GetLastError();
        wprintf(L"ERROR: ReportEventW failed with %u\n", err);
        abort();
    }
    printf("[done]\n");

    DeregisterEventSource(h);

}
