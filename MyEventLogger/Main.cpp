#include <stdio.h>
#include <vector>
#include "EventHandle.hpp"
#include "../MyEventProvider/MyEventProvider.h"  // Generated from manifest


int wmain(void) {
    EventHandle provider(&MyMoviePlayer);

    {
        // generate WarningMessage log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        DWORD val = 41;
        params.push_back(EventData(&val, sizeof(val)));

        wprintf(L"Writing log entry...\n");
        provider.Write(&WarningMessage, params.size(), params.data());
    }

    {
        // generate ErrorMessage log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        std::wstring path = L"Something fucked up...";
        params.push_back(EventData(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        wprintf(L"Writing log entry...\n");
        provider.Write(&ErrorMessage, params.size(), params.data());
    }

    {
        // generate InteractiveInfoEvent log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        std::wstring path = L"User clicked on X...";
        params.push_back(EventData(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        wprintf(L"Writing log entry...\n");
        provider.Write(&InteractiveInfoEvent, params.size(), params.data());
    }

    wprintf(L"[done]\n");
}
