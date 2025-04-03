#include <windows.h>
#include <stdio.h>
#include <winevt.h>
#include <cassert>
#include "Channel.hpp"

#pragma comment(lib, "wevtapi.lib")


/** RAII wrapper to avoid goto. */
class EvtChannelEnum {
public:
    EvtChannelEnum() {
        m_channels = EvtOpenChannelEnum(NULL, 0);
        assert(m_channels);
    }

    ~EvtChannelEnum() {
        if (m_channels)
            EvtClose(m_channels);

    }

    operator EVT_HANDLE() {
        return m_channels;
    }

private:
    EVT_HANDLE m_channels = 0;
};


std::vector<std::wstring> EnumerateChannels()
{
    // Get a handle to an enumerator that contains all the names of the 
    // channels registered on the computer.
    EvtChannelEnum channels;

    std::wstring buffer;
    std::vector<std::wstring> result;

    // Enumerate through the list of channel names.
    // To get the configuration information for a channel, call the EvtOpenChannelConfig function.
    while (true) {
        DWORD dwBufferUsed = 0;
        if (!EvtNextChannelPath(channels, (DWORD)buffer.size(), (wchar_t*)buffer.data(), &dwBufferUsed)) {
            DWORD status = GetLastError();

            if (ERROR_NO_MORE_ITEMS == status) {
                // reached the end
                break;
            } else if (ERROR_INSUFFICIENT_BUFFER == status) {
                // repeat call with larger buffer
                buffer.resize(dwBufferUsed);
                EvtNextChannelPath(channels, (DWORD)buffer.size(), (wchar_t*)buffer.data(), &dwBufferUsed);
            } else {
                wprintf(L"EvtNextChannelPath failed with %lu.\n", status);
                abort();
            }
        }

        result.push_back(buffer);
    }

    return result;
}
