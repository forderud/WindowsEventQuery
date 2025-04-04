#include <stdio.h>
#include "Event.hpp"
#include <vector>
#include <cassert>

#pragma comment(lib, "wevtapi.lib")


DWORD PrintEventAsXML(EVT_HANDLE hEvent) {
    // determine required buffer size
    DWORD bufferSize = 0; // in bytes
    BOOL ok = EvtRender(NULL, hEvent, EvtRenderEventXml, 0, nullptr, &bufferSize, nullptr);
    if (!ok) {
        // expected to fail with ERROR_INSUFFICIENT_BUFFER
        DWORD status = GetLastError();
        assert(status == ERROR_INSUFFICIENT_BUFFER);
    }

    // render event as XML string
    std::vector<wchar_t> pRenderedContent(bufferSize/sizeof(wchar_t));
    DWORD propertyCount = 0;
    ok = EvtRender(NULL, hEvent, EvtRenderEventXml, (DWORD)(sizeof(wchar_t)*pRenderedContent.size()), pRenderedContent.data(), &bufferSize, &propertyCount);
    if (!ok) {
        DWORD status = GetLastError();
        wprintf(L"EvtRender failed with %d\n", status);
        return status;
    }

    // print XML to console
    wprintf(L"\n\n%s", pRenderedContent.data());
    return ERROR_SUCCESS;
}


// Enumerate all the events in the result set. 
DWORD PrintResults(EVT_HANDLE hResults) {
    DWORD status = ERROR_SUCCESS;
    Event events[10];

    while (true) {
        DWORD dwReturned = 0;
        // get a block of events from the result set
        if (!EvtNext(hResults, std::size(events), (EVT_HANDLE*)events, INFINITE, 0, &dwReturned)) {
            status = GetLastError();
            if (status != ERROR_NO_MORE_ITEMS)
                wprintf(L"EvtNext failed with %lu\n", status);

            break;
        }

        for (DWORD i = 0; i < dwReturned; i++) {
            // print event details to console
            status = PrintEventAsXML(events[i]);

            if (status == ERROR_SUCCESS)
                events[i].Close();
            else
                break;
        }
    }

    return status;
}


void EventQuery (std::wstring channel, std::wstring query) {
    Event results(EvtQuery(NULL, channel.c_str(), query.c_str(), EvtQueryChannelPath | EvtQueryReverseDirection));

    if (!results) {
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

    PrintResults(results);
}
