#include "Channel.hpp"
#include "Event.hpp"
#include "Publisher.hpp"


int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        wprintf(L"Usage:\n");
        wprintf(L"  XPath query: WindowsEventQuery.exe qe <channel> <query>\n");
        wprintf(L"  List all channels: WindowsEventQuery.exe channels\n");
        wprintf(L"  List all publishers: WindowsEventQuery.exe publishers\n");
        return 1;
    }

    std::wstring mode = argv[1];

    if (mode == L"channels") {
        wprintf(L"List of Channels:\n");
        std::vector<std::wstring> channels = EnumerateChannels();
        for (const std::wstring& channel : channels) {
            wprintf(L"  %s\n", channel.c_str());
#if 0
            wprintf(L"\n");
            PrintChannelProperties(channel);
            wprintf(L"\n");
#endif
        }
    } else if (mode == L"publishers") {
        wprintf(L"List of publishers:\n");
        std::vector<std::wstring> publishers = EnumeratePublishers();
        for (const std::wstring& publisher : publishers)
            wprintf(L"  %s\n", publisher.c_str());

    } else if (mode == L"qe") {
        if (argc < 4) {
            wprintf(L"ERROR: Channel and query arguments missing.\n");
            return -1;
        }

        // perform event query
        std::wstring channel = argv[2]; // L"System";
        std::wstring query = argv[3]; // L"*/System[(Level>=1) and (Level<=2)]";
        EventQuery(channel, query, 10);
    } else {
        wprintf(L"ERROR: Incorrect arguments.\n");
        return -1;
    }

    return 0;
}
