#include "Channel.hpp"
#include "Event.hpp"
#include "Publisher.hpp"


int wmain() {
#if 0
    wprintf(L"List of Channels:\n");
    std::vector<std::wstring> channels = EnumerateChannels();
    for (const std::wstring& channel : channels) {
        wprintf(L"  %s\n", channel.c_str());

        PrintChannelProperties(channel);
        wprintf(L"\n");
    }
#endif

#if 0
    wprintf(L"List of publishers:\n");
    std::vector<std::wstring> publishers = EnumeratePublishers(true);
    for (const std::wstring& publisher : publishers)
        wprintf(L"  %s\n", publisher.c_str());
#endif

#if 1
    // perform event query
    std::wstring channel = L"System";
    std::wstring query = L"Event/System[EventID=7023]";
    std::wstring publisherName = L"";
    EventQuery(channel, query, publisherName, 20);
#endif
}
