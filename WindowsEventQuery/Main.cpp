#include "LegacyEventAPI.hpp"
#include "Channel.hpp"
#include "Event.hpp"


int wmain() {
    std::vector<std::wstring> channels = EnumerateChannels();

    wprintf(L"List of Channels\n\n");
    for (const std::wstring& channel : channels) {
        wprintf(L"  %s\n", channel.c_str());

        //PrintChannelProperties(channel);
        //wprintf(L"\n");
    }

    EventQuery(L"System", L"Event/System[EventID=7023]");

    //return LegacyEventQuery();
}
