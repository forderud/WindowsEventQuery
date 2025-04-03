#include "LegactEventAPI.hpp"
#include "Channel.hpp"


int wmain() {
    std::vector<std::wstring> channels = EnumerateChannels();

    wprintf(L"List of Channels\n\n");
    for (const std::wstring channel : channels)
        wprintf(L"  %s\n", channel.c_str());


    //return LegacyEventQuery();
}
