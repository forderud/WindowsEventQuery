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


// Gets the specified message string from the event. If the event does not
// contain the specified message, the function returns NULL.
std::wstring GetMessageString(EVT_HANDLE hMetadata, EVT_HANDLE hEvent, EVT_FORMAT_MESSAGE_FLAGS FormatId)
{
    LPWSTR pBuffer = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD status = 0;

    if (!EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed)) {
        status = GetLastError();
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            // An event can contain one or more keywords. The function returns keywords
            // as a list of keyword strings. To process the list, you need to know the
            // size of the buffer, so you know when you have read the last string, or you
            // can terminate the list of strings with a second null terminator character 
            // as this example does.
            if ((EvtFormatMessageKeyword == FormatId))
                pBuffer[dwBufferSize - 1] = L'\0';
            else
                dwBufferSize = dwBufferUsed;

            pBuffer = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));
            {
                // repeat call with larger buffer
                EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed);

                // Add the second null terminator character.
                if ((EvtFormatMessageKeyword == FormatId))
                    pBuffer[dwBufferUsed - 1] = L'\0';
            }
        } else if (ERROR_EVT_MESSAGE_NOT_FOUND == status || ERROR_EVT_MESSAGE_ID_NOT_FOUND == status) {
            ;
        } else {
            wprintf(L"EvtFormatMessage failed with %u\n", status);
        }
    }

    std::wstring result(pBuffer);
    free(pBuffer);
    return result;
}

void PrintEventStrings(EVT_HANDLE hEvent, std::wstring publisherName) {
    // Get the handle to the provider's metadata that contains the message strings.
    Event hProviderMetadata(EvtOpenPublisherMetadata(NULL, publisherName.c_str(), NULL, 0, 0));
    if (!hProviderMetadata) {
        wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
        return;
    }

    // Get the various message strings from the event.
    std::wstring pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageEvent);
    wprintf(L"Event message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageLevel);
    wprintf(L"Level message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageTask);
    wprintf(L"Task message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageOpcode);
    wprintf(L"Opcode message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageKeyword);
    {
        wprintf(L"Keyword message string: %s", pwsMessage.c_str());

        const wchar_t* ptemp = pwsMessage.c_str();
        while (*(ptemp += wcslen(ptemp) + 1))
            wprintf(L", %s", ptemp);

        wprintf(L"\n\n");
    }

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageChannel);
    wprintf(L"Channel message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageProvider);
    wprintf(L"Provider message string: %s\n\n", pwsMessage.c_str());

    pwsMessage = GetMessageString(hProviderMetadata, hEvent, EvtFormatMessageXml);
    wprintf(L"XML message string: %s\n\n", pwsMessage.c_str());
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

            return status;
        }

        for (DWORD i = 0; i < dwReturned; i++) {
            // print event details to console
            PrintEventAsXML(events[i]);
            events[i].Close();
        }
    }

    return ERROR_SUCCESS;
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
