#include <stdio.h>
#include <vector>
#include "EventHandle.hpp"
#include "../MyEventProvider/MyEventProvider.h"  // Generated from manifest


int wmain(void) {
    EventHandle provider(&PROVIDER_GUID);

    {
        // generate WarningMessage log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        std::wstring path = L"Something strange happened...";
        params.push_back(EventDataArg(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        wprintf(L"Writing log entry...\n");
        provider.Write(&WarningMessage, params.size(), params.data());
    }

    {
        // generate ErrorMessage log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        std::wstring path = L"Something fucked up...";
        params.push_back(EventDataArg(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        wprintf(L"Writing log entry...\n");
        provider.Write(&ErrorMessage, params.size(), params.data());
    }

    {
        // generate InteractiveInfoEvent log entry
        // Parameter order need to match the EventData element order.
        std::vector<EVENT_DATA_DESCRIPTOR> params;

        USHORT scores[3] = { 45, 63, 21 };
        params.push_back(EventDataArg(scores, sizeof(scores)));

        params.push_back(EventDataArg(&PROVIDER_GUID, sizeof(PROVIDER_GUID)));

        BYTE cert[11] = { 0x2, 0x4, 0x8, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x0, 0x1 };
        params.push_back(EventDataArg(cert, sizeof(cert)));

        std::wstring path = L"C:\\SomeFolder\\SomeFile.txt";
        params.push_back(EventDataArg(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        DWORD day = 3;
        params.push_back(EventDataArg(&day, sizeof(day)));

        DWORD transfer = 99;
        params.push_back(EventDataArg(&transfer, sizeof(transfer)));

        wprintf(L"Writing log entry...\n");
        provider.Write(&InteractiveInfoEvent, params.size(), params.data());
    }

    wprintf(L"[done]\n");
}
