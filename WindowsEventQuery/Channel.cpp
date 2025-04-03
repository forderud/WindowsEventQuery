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

    bool Next() {
        DWORD dwBufferUsed = 0;
        if (!EvtNextChannelPath(m_channels, (DWORD)m_buffer.size(), (wchar_t*)m_buffer.data(), &dwBufferUsed)) {
            DWORD status = GetLastError();

            if (status == ERROR_NO_MORE_ITEMS) {
                // reached the end
                return false;
            } else if (status == ERROR_INSUFFICIENT_BUFFER) {
                // repeat call with larger buffer
                m_buffer.resize(dwBufferUsed);
                return EvtNextChannelPath(m_channels, (DWORD)m_buffer.size(), (wchar_t*)m_buffer.data(), &dwBufferUsed);
            } else {
                wprintf(L"EvtNextChannelPath failed with %lu.\n", status);
                abort();
            }
        }

        return true;
    }

    std::wstring ChannelPath() const {
        // m_buffer might be too large so return the raw pointer to auto-shrink the string length
        return m_buffer.c_str();
    }

private:
    EVT_HANDLE m_channels = 0;
    std::wstring m_buffer;
};


std::vector<std::wstring> EnumerateChannels() {
    // Get a handle to an enumerator that contains all the names of the 
    // channels registered on the computer.
    EvtChannelEnum channels;

    std::vector<std::wstring> result;

    // Enumerate through the list of channel names.
    // To get the configuration information for a channel, call the EvtOpenChannelConfig function.
    while (channels.Next()) {
        result.push_back(channels.ChannelPath());
    }

    return result;
}
