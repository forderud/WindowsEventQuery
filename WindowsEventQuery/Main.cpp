#include "LegacyEventAPI.hpp"
#include "Channel.hpp"
#include "Event.hpp"


int wmain() {
#if 0
    wprintf(L"List of Channels\n\n");
    std::vector<std::wstring> channels = EnumerateChannels();
    for (const std::wstring& channel : channels) {
        wprintf(L"  %s\n", channel.c_str());

        //PrintChannelProperties(channel);
        //wprintf(L"\n");
    }
#endif

    // perform event query
    std::wstring channel = L"System";
    std::wstring query = L"Event/System[EventID=7023]";
    EventQuery(L"System", query);

    //return LegacyEventQuery();
}
