#include <stdio.h>
#include "Event.hpp"
#include <vector>
#include <cassert>

#pragma comment(lib, "wevtapi.lib")


std::wstring RenderEventValue(EVT_HANDLE hEvent, const wchar_t* query) {
    const wchar_t* ppValues[] = { query };

    // Identify the components of the event that you want to render. In this case,
    // render the provider's name and channel from the system section of the event.
    // To get user data from the event, you can specify an expression such as
    // L"Event/EventData/Data[@Name=\"<data name goes here>\"]".
    Event hContext(EvtCreateRenderContext(std::size(ppValues), (LPCWSTR*)ppValues, EvtRenderContextValues));
    if (!hContext) {
        DWORD status = GetLastError();
        wprintf(L"EvtCreateRenderContext failed with %lu\n", status);
        abort();
    }

    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    EVT_VARIANT* pRenderedValues = nullptr;

    // The function returns an array of variant values for each element or attribute that
    // you want to retrieve from the event. The values are returned in the same order as 
    // you requested them.
    if (!EvtRender(hContext, hEvent, EvtRenderEventValues, 0, nullptr, &dwBufferUsed, &dwPropertyCount)) {
        DWORD status = GetLastError();
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            DWORD dwBufferSize = dwBufferUsed;
            pRenderedValues = (EVT_VARIANT*)malloc(dwBufferSize);
            EvtRender(hContext, hEvent, EvtRenderEventValues, dwBufferSize, pRenderedValues, &dwBufferUsed, &dwPropertyCount);
        }

        status = GetLastError();
        if (status != ERROR_SUCCESS) {
            wprintf(L"EvtRender failed with %d\n", GetLastError());
            abort();
        }
    }

    // Print the selected values.
    std::wstring result = pRenderedValues[0].StringVal;

    if (pRenderedValues)
        free(pRenderedValues);

    return result;
}

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
            return L"N/A";
        } else {
            wprintf(L"EvtFormatMessage failed with %u\n", status);
            return L"";
        }
    }

    // repeat call with larger buffer
    std::wstring pBuffer(bufferUsed-1, L'\0'); // exclude null-terminator
    EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, (DWORD)pBuffer.size()+1, (wchar_t*)pBuffer.data(), &bufferUsed);

    // terminate the list of strings with a second null terminator character
    if ((EvtFormatMessageKeyword == FormatId))
        pBuffer[bufferUsed - 1] = L'\0';

    return pBuffer;
}

void PrintEventStrings(EVT_HANDLE hEvent) {
    std::wstring msgXml = GetMessageString(NULL, hEvent, EvtFormatMessageXml);
    //wprintf(L"XML message string: %s\n\n", msgXml.c_str());

    std::wstring providerName = RenderEventValue(hEvent, L"Event/System/Provider/@Name");

    Event providerMetadata;
    if (providerName.size() > 0) {
        // Get the handle to the provider's metadata that contains the message strings.
        providerMetadata = Event(EvtOpenPublisherMetadata(NULL, providerName.c_str(), NULL, 0, 0));
        if (!providerMetadata) {
            wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
            return;
        }
    }

    {
        std::wstring message = RenderEventValue(hEvent, L"Event/System/Channel");
        wprintf(L"Channel: %s\n", message.c_str());
    }

    //message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageProvider);
    // using provider from XML since EvtFormatMessageProvider returns an empty string for Schannel
    wprintf(L"Provider: %s\n", providerName.c_str());

    {
        // Print date/time in "2025-04-06T16:53:45.4470000Z" format
        // TODO: Replace with API call or proper XML query
        const wchar_t TIME_SEARCH[] = L"<TimeCreated SystemTime='"; // "SystemTime" is a REQUIRED attribute
        size_t idx1 = msgXml.find(TIME_SEARCH);
        assert(idx1 != std::wstring::npos);

        idx1 += std::size(TIME_SEARCH) - 1;
        size_t idx2 = msgXml.find(L"'", idx1);

        std::wstring message = msgXml.substr(idx1, idx2 - idx1);
        wprintf(L"Date: %s\n", message.c_str());
    }

    {
        // Print Event ID
        // TODO: Replace with API call or proper XML query
        // match both "<EventID Qualifiers='49152'>7023</EventID>" and "<EventID>7023</EventID>", since "Qualifiers" is an OPTIONAL attribute
        const wchar_t EVENTID_SEARCH[] = L"</EventID>";
        size_t idx2 = msgXml.find(EVENTID_SEARCH);
        assert(idx2 != std::wstring::npos);

        size_t idx1 = msgXml.rfind(L">", idx2);
        idx1 += 1;

        std::wstring message = msgXml.substr(idx1, idx2 - idx1);
        wprintf(L"Event ID: %s\n", message.c_str());
    }

    // Get the various message strings from the event.
    std::wstring message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageTask);
    wprintf(L"Task: %s\n", message.c_str());

    message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageLevel);
    wprintf(L"Level: %s\n", message.c_str());

    message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageOpcode);
    wprintf(L"Opcode: %s\n", message.c_str());

    message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageKeyword);
    {
        wprintf(L"Keyword: %s", message.c_str());

        const wchar_t* ptemp = message.c_str();
        while (*(ptemp += wcslen(ptemp) + 1)) // safe since message has double null-termination
            wprintf(L", %s", ptemp);

        wprintf(L"\n");
    }

    message = GetMessageString(providerMetadata, hEvent, EvtFormatMessageEvent);
    wprintf(L"Description: %s\n", message.c_str());

    wprintf(L"\n");
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


    // get multiple events per API call
    Event events[10];

    // iterate over events in the result set
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
        if (!EvtNext(results, eventSize, events[0].GetAddress(), INFINITE, 0, &dwReturned)) {
            DWORD status = GetLastError();
            if (status != ERROR_NO_MORE_ITEMS)
                wprintf(L"EvtNext failed with %lu\n", status);

            return;
        }

        for (DWORD i = 0; i < dwReturned; i++) {
            // print event details to console
            PrintEventStrings(events[i]);

            events[i].Close();
        }
    }
}
