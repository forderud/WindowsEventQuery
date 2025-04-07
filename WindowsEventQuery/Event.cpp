#include <stdio.h>
#include "Event.hpp"
#include <vector>
#include <cassert>
#include <variant>

#pragma comment(lib, "wevtapi.lib")


std::variant<std::wstring, uint16_t, FILETIME> RenderEventValue(EVT_HANDLE hEvent, const wchar_t* query) {
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
    std::vector<BYTE> buffer;
    EVT_VARIANT* values = nullptr;

    // The function returns an array of variant values for each element or attribute that
    // you want to retrieve from the event. The values are returned in the same order as 
    // you requested them.
    if (!EvtRender(hContext, hEvent, EvtRenderEventValues, 0, nullptr, &dwBufferUsed, &dwPropertyCount)) {
        DWORD status = GetLastError();
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            buffer.resize(dwBufferUsed);
            values = (EVT_VARIANT*)buffer.data();
            EvtRender(hContext, hEvent, EvtRenderEventValues, (DWORD)buffer.size(), values, &dwBufferUsed, &dwPropertyCount);
        }

        status = GetLastError();
        if (status != ERROR_SUCCESS) {
            wprintf(L"EvtRender failed with %d\n", GetLastError());
            abort();
        }
    }

    std::variant<std::wstring, uint16_t, FILETIME> result;
    if (values[0].Type == EvtVarTypeString) {
        result = values[0].StringVal;
    } else if (values[0].Type == EvtVarTypeUInt16) {
        result = values[0].UInt16Val;
    } else if (values[0].Type == EvtVarTypeFileTime) {
        FILETIME ft{};
        ft.dwHighDateTime = (DWORD)((values[0].FileTimeVal >> 32) & 0xFFFFFFFF);
        ft.dwLowDateTime = (DWORD)(values[0].FileTimeVal & 0xFFFFFFFF);
        result = ft;
    } else {
        abort();
    }
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
#if 0
    std::wstring msgXml = GetMessageString(NULL, hEvent, EvtFormatMessageXml);
    wprintf(L"XML message string: %s\n\n", msgXml.c_str());
#endif

    auto channel = std::get<std::wstring>(RenderEventValue(hEvent, L"Event/System/Channel"));
    wprintf(L"Channel: %s\n", channel.c_str());

    auto providerName = std::get<std::wstring>(RenderEventValue(hEvent, L"Event/System/Provider/@Name"));
    wprintf(L"Provider: %s\n", providerName.c_str());

    {
        // Print date/time in "2025-04-06T16:53:45.4470000Z" format
        auto ft = std::get<FILETIME>(RenderEventValue(hEvent, L"Event/System/TimeCreated/@SystemTime"));

        SYSTEMTIME st{};
        FileTimeToSystemTime(&ft, &st); // UTC time

        wprintf(L"Date: %02d-%02d-%02dT%02d:%02d:%02d.%02dZ\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }

    auto eventId = std::get<uint16_t>(RenderEventValue(hEvent, L"Event/System/EventID"));
    wprintf(L"Event ID: %u\n", eventId);

    Event providerMetadata;
    if (providerName.size() > 0) {
        // Get the handle to the provider's metadata that contains the message strings.
        providerMetadata = Event(EvtOpenPublisherMetadata(NULL, providerName.c_str(), NULL, 0, 0));
        if (!providerMetadata) {
            wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
            return;
        }
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
