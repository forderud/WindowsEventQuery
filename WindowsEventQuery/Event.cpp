#include <stdio.h>
#include "Event.hpp"
#include <vector>

#pragma comment(lib, "wevtapi.lib")


DWORD PrintEventAsXML(EVT_HANDLE hEvent) {
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    std::vector<wchar_t> pRenderedContent;

    // render event as XML string
    if (!EvtRender(NULL, hEvent, EvtRenderEventXml, (DWORD)(sizeof(wchar_t)*pRenderedContent.size()), pRenderedContent.data(), &dwBufferUsed, &dwPropertyCount)) {
        status = GetLastError();
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            // repeat query with larger buffer
            pRenderedContent.resize(dwBufferUsed/sizeof(wchar_t));
            EvtRender(NULL, hEvent, EvtRenderEventXml, (DWORD)(sizeof(wchar_t)*pRenderedContent.size()), pRenderedContent.data(), &dwBufferUsed, &dwPropertyCount);
        }

        status = GetLastError();
        if (status != ERROR_SUCCESS) {
            wprintf(L"EvtRender failed with %d\n", GetLastError());
            return status;
        }
    }

    wprintf(L"\n\n%s", pRenderedContent.data());
    return status;
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
