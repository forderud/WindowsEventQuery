#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include <cassert>
#include <vector>

#define PROVIDER_NAME           L"System"
#define MAX_TIMESTAMP_LEN       23 + 1   // mm/dd/yyyy hh:mm:ss.mmm
#define MAX_RECORD_BUFFER_SIZE  0x10000  // 64K

HANDLE GetMessageResources();
DWORD DumpRecordsInBuffer(PBYTE pBuffer, DWORD dwBytesRead);
DWORD GetEventTypeName(DWORD EventType);
std::wstring GetMessageString(DWORD Id, DWORD argc, LPWSTR args);
void GetTimestamp(const DWORD Time, WCHAR DisplayString[]);

const wchar_t* pEventTypeNames[] = {L"Error", L"Warning", L"Informational", L"Audit Success", L"Audit Failure"};

class EventLog {
public:
    EventLog(const wchar_t* source) {
        m_handle = OpenEventLogW(/*localhost*/NULL, PROVIDER_NAME);
        assert(m_handle);
    }

    ~EventLog() {
        if (m_handle)
            CloseEventLog(m_handle);

    }

    operator HANDLE () {
        return m_handle;
    }

private:
    HANDLE m_handle = 0;
};

int wmain(void)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBytesToRead = 0;
    DWORD dwBytesRead = 0;
    DWORD dwMinimumBytesToRead = 0;

    // Allocate an initial block of memory used to read event records. The number 
    // of records read into the buffer will vary depending on the size of each event.
    // The size of each event will vary based on the size of the user-defined
    // data included with each event, the number and length of insertion 
    // strings, and other data appended to the end of the event record.
    dwBytesToRead = MAX_RECORD_BUFFER_SIZE;
    std::vector<BYTE> pBuffer(dwBytesToRead);

    // The source name (provider) must exist as a subkey of Application.
    EventLog hEventLog (PROVIDER_NAME);
    if (NULL == hEventLog)
    {
        wprintf(L"OpenEventLog failed with 0x%x.\n", GetLastError());
        return -1;
    }

    // Read blocks of records until you reach the end of the log or an 
    // error occurs. The records are read from newest to oldest. If the buffer
    // is not big enough to hold a complete event record, reallocate the buffer.
    while (ERROR_SUCCESS == status)
    {
        if (!ReadEventLogW(hEventLog, 
            EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ,
            0, 
            pBuffer.data(),
            dwBytesToRead,
            &dwBytesRead,
            &dwMinimumBytesToRead))
        {
            status = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == status)
            {
                status = ERROR_SUCCESS;
                pBuffer.resize(dwMinimumBytesToRead);
                dwBytesToRead = dwMinimumBytesToRead;
            }
            else 
            {
                if (ERROR_HANDLE_EOF != status)
                {
                    wprintf(L"ReadEventLog failed with %lu.\n", status);
                    return 1;
                }
            }
        }
        else
        {
            // Print the contents of each record in the buffer.
            DumpRecordsInBuffer(pBuffer.data(), dwBytesRead);
        }
    }

    return 0;
}


// Loop through the buffer and print the contents of each record 
// in the buffer.
DWORD DumpRecordsInBuffer(PBYTE pBuffer, DWORD dwBytesRead)
{
    PBYTE pRecord = pBuffer;
    PBYTE pEndOfRecords = pBuffer + dwBytesRead;

    while (pRecord < pEndOfRecords)
    {
        WCHAR TimeStamp[MAX_TIMESTAMP_LEN];
        GetTimestamp(((PEVENTLOGRECORD)pRecord)->TimeGenerated, TimeStamp);
        wprintf(L"Time stamp: %s\n", TimeStamp);

        const wchar_t* SourceName = (LPWSTR)(pRecord + sizeof(EVENTLOGRECORD));
        wprintf(L"Source: %s\n", SourceName);

        wprintf(L"record number: %lu\n", ((PEVENTLOGRECORD)pRecord)->RecordNumber);

        wprintf(L"status code: %d\n", ((PEVENTLOGRECORD)pRecord)->EventID & 0xFFFF);

        wprintf(L"event type: %s\n", pEventTypeNames[GetEventTypeName(((PEVENTLOGRECORD)pRecord)->EventType)]);

        std::wstring pMessage = std::to_wstring(((PEVENTLOGRECORD)pRecord)->EventCategory);
        wprintf(L"event category: %s\n", pMessage.c_str());

        pMessage = GetMessageString(((PEVENTLOGRECORD)pRecord)->EventID, 
            ((PEVENTLOGRECORD)pRecord)->NumStrings, (LPWSTR)(pRecord + ((PEVENTLOGRECORD)pRecord)->StringOffset));
        wprintf(L"event message: %s\n", pMessage.c_str());

        // To write the event data, you need to know the format of the data. In
        // this example, we know that the event data is a null-terminated string.
        if (((PEVENTLOGRECORD)pRecord)->DataLength > 0)
        {
            wprintf(L"event data: %s\n", (LPWSTR)(pRecord + ((PEVENTLOGRECORD)pRecord)->DataOffset));
        }

        wprintf(L"\n");

        pRecord += ((PEVENTLOGRECORD)pRecord)->Length;
    }

    return ERROR_SUCCESS;
}


// Get an index value to the pEventTypeNames array based on 
// the event type value.
DWORD GetEventTypeName(DWORD EventType)
{
    DWORD index = 0;

    switch (EventType)
    {
        case EVENTLOG_ERROR_TYPE:
            index = 0;
            break;
        case EVENTLOG_WARNING_TYPE:
            index = 1;
            break;
        case EVENTLOG_INFORMATION_TYPE:
            index = 2;
            break;
        case EVENTLOG_AUDIT_SUCCESS:
            index = 3;
            break;
        case EVENTLOG_AUDIT_FAILURE:
            index = 4;
            break;
    }

    return index;
}


// Formats the specified message. If the message uses inserts, build
// the argument list to pass to FormatMessage.
std::wstring GetMessageString(DWORD MessageId, DWORD argc, LPWSTR argv)
{
    std::wstring pMessage;
    LPWSTR pString = argv;

    // The insertion strings appended to the end of the event record
    // are an array of strings; however, FormatMessage requires
    // an array of addresses. Create an array of DWORD_PTRs based on
    // the count of strings. Assign the address of each string
    // to an element in the array (maintaining the same order).
    for (DWORD i = 0; i < argc; i++)
    {
        pMessage += pString;
        if (i < argc-1)
            pMessage += L", ";
        pString += wcslen(pString) + 1;
    }

    return pMessage;
}

// Get a string that contains the time stamp of when the event 
// was generated.
void GetTimestamp(const DWORD Time, WCHAR DisplayString[])
{
    ULONGLONG ullTimeStamp = 0;
    ULONGLONG SecsTo1970 = 116444736000000000;
    SYSTEMTIME st;
    FILETIME ft, ftLocal;

    ullTimeStamp = Int32x32To64(Time, 10000000) + SecsTo1970;
    ft.dwHighDateTime = (DWORD)((ullTimeStamp >> 32) & 0xFFFFFFFF);
    ft.dwLowDateTime = (DWORD)(ullTimeStamp & 0xFFFFFFFF);
    
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    StringCchPrintfW(DisplayString, MAX_TIMESTAMP_LEN, L"%d/%d/%d %.2d:%.2d:%.2d", 
        st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
}
