#include <windows.h>
#include <sddl.h>
#include <stdio.h>
#include <winevt.h>
#include "Event.hpp"

#pragma comment(lib, "wevtapi.lib")

#define ARRAY_SIZE 10
#define TIMEOUT 1000  // 1 second; Set and use in place of INFINITE in EvtNext call


/** RAII wrapper to avoid goto. */
class Event {
public:
    Event() = default;

    Event(EVT_HANDLE event) : m_event(event) {
    }

    ~Event() {
        Close();
    }

    void Close() {
        if (m_event) {
            EvtClose(m_event);
            m_event = 0;
        }
    }

    operator EVT_HANDLE& () {
        return m_event;
    }

private:
    EVT_HANDLE m_event = 0;
};
static_assert(sizeof(Event) == sizeof(EVT_HANDLE), "Event size mismatch");




DWORD PrintEvent(EVT_HANDLE hEvent)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pRenderedContent = NULL;

    // The EvtRenderEventXml flag tells EvtRender to render the event as an XML string.
    if (!EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            dwBufferSize = dwBufferUsed;
            pRenderedContent = (LPWSTR)malloc(dwBufferSize);
            if (pRenderedContent)
            {
                EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
            }
            else
            {
                wprintf(L"malloc failed\n");
                status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
        }

        if (ERROR_SUCCESS != (status = GetLastError()))
        {
            wprintf(L"EvtRender failed with %d\n", GetLastError());
            goto cleanup;
        }
    }

    wprintf(L"\n\n%s", pRenderedContent);

cleanup:

    if (pRenderedContent)
        free(pRenderedContent);

    return status;
}

// Enumerate all the events in the result set. 
DWORD PrintResults(EVT_HANDLE hResults) {
    DWORD status = ERROR_SUCCESS;
    Event hEvents[ARRAY_SIZE];

    while (true) {
        DWORD dwReturned = 0;
        // Get a block of events from the result set.
        if (!EvtNext(hResults, ARRAY_SIZE, (EVT_HANDLE*)hEvents, INFINITE, 0, &dwReturned)) {
            status = GetLastError();
            if (status != ERROR_NO_MORE_ITEMS)
                wprintf(L"EvtNext failed with %lu\n", status);

            break;
        }

        // For each event, call the PrintEvent function which renders the
        // event for display. PrintEvent is shown in RenderingEvents.
        for (DWORD i = 0; i < dwReturned; i++) {
            status = PrintEvent(hEvents[i]);
            if (status == ERROR_SUCCESS)
                hEvents[i].Close();
            else
                break;
        }
    }

    return status;
}


void EventQuery (std::wstring channel, std::wstring query) {
    Event hResults(EvtQuery(NULL, channel.c_str(), query.c_str(), EvtQueryChannelPath | EvtQueryReverseDirection));

    if (!hResults) {
        DWORD status = GetLastError();

        if (status == ERROR_EVT_CHANNEL_NOT_FOUND)
            wprintf(L"The channel was not found.\n");
        else if (status == ERROR_EVT_INVALID_QUERY)
            // You can call the EvtGetExtendedStatus function to try to get 
            // additional information as to what is wrong with the query.
            wprintf(L"The query is not valid.\n");
        else
            wprintf(L"EvtQuery failed with %lu.\n", status);

        return;
    }

    PrintResults(hResults);
}
