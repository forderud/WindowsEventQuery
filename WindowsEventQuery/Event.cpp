#include <stdio.h>
#include "Event.hpp"
#include <vector>
#include <cassert>

#pragma comment(lib, "wevtapi.lib")


void PrintEventAsXML(EVT_HANDLE event) {
    // determine required buffer size
    DWORD bufferSize = 0; // in bytes
    BOOL ok = EvtRender(NULL, event, EvtRenderEventXml, 0, nullptr, &bufferSize, nullptr);
    if (!ok) {
        // expected to fail with ERROR_INSUFFICIENT_BUFFER
        DWORD status = GetLastError();
        assert(status == ERROR_INSUFFICIENT_BUFFER);
    }

    // render event as XML string
    std::vector<wchar_t> pRenderedContent(bufferSize/sizeof(wchar_t));
    DWORD propertyCount = 0;
    ok = EvtRender(NULL, event, EvtRenderEventXml, (DWORD)(sizeof(wchar_t)*pRenderedContent.size()), pRenderedContent.data(), &bufferSize, &propertyCount);
    if (!ok) {
        DWORD status = GetLastError();
        wprintf(L"EvtRender failed with %d\n", status);
        return;
    }

    // print XML to console
    wprintf(L"\n\n%s", pRenderedContent.data());
}


/** Gets the specified message string from the event.Returns empty string if the event does not contain the specified message. */
std::wstring GetMessageString(EVT_HANDLE hMetadata, EVT_HANDLE hEvent, EVT_FORMAT_MESSAGE_FLAGS FormatId) {
    // determine required buffer size
    DWORD bufferUsed = 0; // in characters
    if (!EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, 0, nullptr, &bufferUsed)) {
        DWORD status = GetLastError();

        if (status == ERROR_INSUFFICIENT_BUFFER) {
            // expected failure
        } else if (status == ERROR_EVT_MESSAGE_NOT_FOUND || status == ERROR_EVT_MESSAGE_ID_NOT_FOUND) {
            return L"";
        } else {
            wprintf(L"EvtFormatMessage failed with %u\n", status);
            return L"";
        }
    }

    // repeat call with larger buffer
    std::wstring pBuffer(bufferUsed, L'\0');
    EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, (DWORD)pBuffer.size(), (wchar_t*)pBuffer.data(), &bufferUsed);

    // terminate the list of strings with a second null terminator character
    if ((EvtFormatMessageKeyword == FormatId))
        pBuffer[bufferUsed - 1] = L'\0';

    return pBuffer;
}

void PrintEventStrings(EVT_HANDLE hEvent) {
    std::wstring msgXml = GetMessageString(NULL, hEvent, EvtFormatMessageXml);
    //wprintf(L"XML message string: %s\n\n", msgXml.c_str());

    Event hProviderMetadata;

    const wchar_t PROVIDER_SEARCH[] = L"<Provider Name='";
    size_t idx1 = msgXml.find(PROVIDER_SEARCH);
    if (idx1 != std::wstring::npos) {
        // Get publisher from "/Event/System/Provider@Name" in message XML
        idx1 += std::size(PROVIDER_SEARCH)-1;
        size_t idx2 = msgXml.find(L"'", idx1);
        std::wstring publisherId = msgXml.substr(idx1, idx2 - idx1);

        // Get the handle to the provider's metadata that contains the message strings.
        hProviderMetadata = Event(EvtOpenPublisherMetadata(NULL, publisherId.c_str(), NULL, 0, 0));
        if (!hProviderMetadata) {
            wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
            return;
        }
    }

    // Get the various message strings from the event.
    std::wstring pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageEvent);
    wprintf(L"Message: %s\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageLevel);
    wprintf(L"Level: %s\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageTask);
    wprintf(L"Task: %s\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageOpcode);
    wprintf(L"Opcode: %s\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageKeyword);
    {
        wprintf(L"Keyword: %s", pwsMessage.c_str());

        const wchar_t* ptemp = pwsMessage.c_str();
        while (*(ptemp += wcslen(ptemp) + 1))
            wprintf(L", %s", ptemp);

        wprintf(L"\n");
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageChannel);
    wprintf(L"Channel: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageProvider);
    wprintf(L"Provider: %s\n\n", pwsMessage.c_str());
}


// Enumerate all the events in the result set. 
DWORD PrintResults(EVT_HANDLE hResults, size_t maxCount) {
    DWORD status = ERROR_SUCCESS;
    Event events[10];

    for (size_t i = 0; i < maxCount/std::size(events) + 1; i++) {
        DWORD eventSize = std::size(events);
        if (i == maxCount/std::size(events)) {
            // special handling of last iteration
            eventSize = maxCount % std::size(events);

            if (!eventSize)
                continue;
        }

        DWORD dwReturned = 0;
        // get a block of events from the result set
        if (!EvtNext(hResults, eventSize, events[0].GetAddress(), INFINITE, 0, &dwReturned)) {
            status = GetLastError();
            if (status != ERROR_NO_MORE_ITEMS)
                wprintf(L"EvtNext failed with %lu\n", status);

            return status;
        }

        for (DWORD i = 0; i < dwReturned; i++) {
            // print event details to console
            //PrintEventAsXML(events[i]);
            PrintEventStrings(events[i]);

            events[i].Close();
        }
    }

    return ERROR_SUCCESS;
}


void EventQuery (std::wstring channel, std::wstring query, size_t maxCount) {
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

    PrintResults(results, maxCount);
}
