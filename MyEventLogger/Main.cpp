#include <stdio.h>
#include <vector>
#include "EventHandle.hpp"
#include "../MyEventProvider/MyEventProvider.h"  // Generated from manifest


int wmain(void) {
    EventHandle provider(&MyMoviePlayer);

    {
        // generate CacheSizeMB log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        DWORD val = 512;
        params.push_back(EventData(&val, sizeof(val)));

        wprintf(L"Writing log entry...\n");
        provider.Write(&CacheSizeMB, params.size(), params.data());
    }

    {
        // generate Playback log entry
        std::vector<EVENT_DATA_DESCRIPTOR> params;
        std::wstring path = L"Avatar";
        params.push_back(EventData(path.c_str(), (path.length() + 1) * sizeof(wchar_t))); // incl. null-termination

        wprintf(L"Writing log entry...\n");
        provider.Write(&Play, params.size(), params.data());
    }

    wprintf(L"[done]\n");
}
