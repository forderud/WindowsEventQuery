#include <stdio.h>
#include <cassert>
#include "Channel.hpp"
#include "Event.hpp"

#pragma comment(lib, "wevtapi.lib")


/** RAII wrapper to avoid goto. */
class EvtChannelEnum : public Event {
public:
    EvtChannelEnum() : Event(EvtOpenChannelEnum(NULL, 0)) {
        assert(m_event);
    }

    ~EvtChannelEnum() {
    }

    bool Next() {
        DWORD dwBufferUsed = 0;
        if (!EvtNextChannelPath(m_event, (DWORD)m_buffer.size(), (wchar_t*)m_buffer.data(), &dwBufferUsed)) {
            DWORD status = GetLastError();

            if (status == ERROR_NO_MORE_ITEMS) {
                // reached the end
                return false;
            } else if (status == ERROR_INSUFFICIENT_BUFFER) {
                // repeat call with larger buffer
                m_buffer.resize(dwBufferUsed);
                return EvtNextChannelPath(m_event, (DWORD)m_buffer.size(), (wchar_t*)m_buffer.data(), &dwBufferUsed);
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

const wchar_t* pwcsChannelTypes[] = { L"Admin", L"Operational", L"Analytic", L"Debug" };
const wchar_t* pwcsIsolationTypes[] = { L"Application", L"System", L"Custom" };
const wchar_t* pwcsClockTypes[] = { L"System", L"QPC" };


// Print the property value.
DWORD PrintChannelProperty(int Id, PEVT_VARIANT pProperty)
{
    DWORD status = ERROR_SUCCESS;
    WCHAR wszSessionGuid[50];
    LPWSTR lpws = NULL;

    switch (Id)
    {
    case EvtChannelConfigEnabled:
        wprintf(L"Enabled: %s\n", (TRUE == pProperty->BooleanVal) ? L"TRUE" : L"FALSE");
        break;

    case EvtChannelConfigIsolation:
        wprintf(L"Isolation: %s\n", pwcsIsolationTypes[pProperty->UInt32Val]);
        break;

    case EvtChannelConfigType:
        wprintf(L"Type: %s\n", pwcsChannelTypes[pProperty->UInt32Val]);
        break;

        // This will contain a value if the channel is imported.
    case EvtChannelConfigOwningPublisher:
        wprintf(L"Publisher that defined the channel: %s\n", pProperty->StringVal);
        break;

    case EvtChannelConfigClassicEventlog:
        wprintf(L"ClassicEventlog: %s\n", (TRUE == pProperty->BooleanVal) ? L"TRUE" : L"FALSE");
        break;

    case EvtChannelConfigAccess:
        wprintf(L"Access: %s\n", pProperty->StringVal);
        break;

    case EvtChannelLoggingConfigRetention:
        wprintf(L"Retention: %s\n", (TRUE == pProperty->BooleanVal) ? L"TRUE (Sequential)" : L"FALSE (Circular)");
        break;

    case EvtChannelLoggingConfigAutoBackup:
        wprintf(L"AutoBackup: %s\n", (TRUE == pProperty->BooleanVal) ? L"TRUE" : L"FALSE");
        break;

    case EvtChannelLoggingConfigMaxSize:
        wprintf(L"MaxSize: %I64u MB\n", pProperty->UInt64Val / (1024 * 1024));
        break;

    case EvtChannelLoggingConfigLogFilePath:
        wprintf(L"LogFilePath: %s\n", pProperty->StringVal);
        break;

    case EvtChannelPublishingConfigLevel:
        if (EvtVarTypeNull == pProperty->Type)
            wprintf(L"Level: \n");
        else
            wprintf(L"Level: %lu\n", pProperty->UInt32Val);

        break;

        // The upper 8 bits can contain reserved bit values, so do not include them
        // when checking to see if any keywords are set.
    case EvtChannelPublishingConfigKeywords:
        if (EvtVarTypeNull == pProperty->Type)
            wprintf(L"Keywords: \n");
        else
            wprintf(L"Keywords: %I64u\n", pProperty->UInt64Val & 0x00FFFFFFFFFFFFFF);

        break;

    case EvtChannelPublishingConfigControlGuid:
        if (EvtVarTypeNull == pProperty->Type)
            wprintf(L"ControlGuid: \n");
        else
        {
            StringFromGUID2(*(pProperty->GuidVal), wszSessionGuid, sizeof(wszSessionGuid) / sizeof(WCHAR));
            wprintf(L"ControlGuid: %s\n", wszSessionGuid);
        }

        break;

    case EvtChannelPublishingConfigBufferSize:
        wprintf(L"BufferSize: %lu KB\n", pProperty->UInt32Val);
        break;

    case EvtChannelPublishingConfigMinBuffers:
        wprintf(L"MinBuffers: %lu\n", pProperty->UInt32Val);
        break;

    case EvtChannelPublishingConfigMaxBuffers:
        wprintf(L"MaxBuffers: %lu\n", pProperty->UInt32Val);
        break;

    case EvtChannelPublishingConfigLatency:
        wprintf(L"Latency: %lu (sec)\n", pProperty->UInt32Val / 1000); // 1 ms
        break;

    case EvtChannelPublishingConfigClockType:
        wprintf(L"ClockType: %s\n", pwcsClockTypes[pProperty->UInt32Val]);
        break;

    case EvtChannelPublishingConfigSidType:
        wprintf(L"Include security ID (SID): %s\n", (EvtChannelSidTypeNone == pProperty->UInt32Val) ? L"No" : L"Yes");
        break;

    case EvtChannelPublisherList:

        wprintf(L"List of providers that import this channel: \n");
        for (DWORD i = 0; i < pProperty->Count; i++)
        {
            wprintf(L"  %s\n", pProperty->StringArr[i]);
        }

        break;

    case EvtChannelPublishingConfigFileMax:
        wprintf(L"FileMax: %lu\n", pProperty->UInt32Val);
        break;

    default:
        wprintf(L"Unknown property Id: %d\n", Id);
    }

    return status;
}

// Print the channel's configuration properties. Use the EVT_CHANNEL_CONFIG_PROPERTY_ID
// enumeration values to loop through all the properties.
void PrintChannelProperties(std::wstring channelPath) {
    Event channel(EvtOpenChannelConfig(NULL, channelPath.c_str(), 0));
    if (!channel) {// Fails with 15007 (ERROR_EVT_CHANNEL_NOT_FOUND) if the channel is not found
        wprintf(L"EvtOpenChannelConfig failed with %lu.\n", GetLastError());
        return;
    }

    std::vector<BYTE> pProperty; // EVT_VARIANT buffer that receives the property value
    DWORD dwBufferUsed = 0;
    DWORD status = ERROR_SUCCESS;

    for (int Id = 0; Id < EvtChannelConfigPropertyIdEND; Id++) {
        // Get the specified property. If the buffer is too small, reallocate it.
        if (!EvtGetChannelConfigProperty(channel, (EVT_CHANNEL_CONFIG_PROPERTY_ID)Id, 0, (DWORD)pProperty.size(), (EVT_VARIANT*)pProperty.data(), &dwBufferUsed)) {
            status = GetLastError();
            if (status == ERROR_INSUFFICIENT_BUFFER) {
                // repeat call with larger buffer
                pProperty.resize(dwBufferUsed);
                EvtGetChannelConfigProperty(channel, (EVT_CHANNEL_CONFIG_PROPERTY_ID)Id, 0, (DWORD)pProperty.size(), (EVT_VARIANT*)pProperty.data(), &dwBufferUsed);
            }

            status = GetLastError();
            if (status != ERROR_SUCCESS) {
                wprintf(L"EvtGetChannelConfigProperty failed with %d\n", GetLastError());
                return;
            }
        }

        status = PrintChannelProperty(Id, (EVT_VARIANT*)pProperty.data());
        if (status != ERROR_SUCCESS)
            break;
    }
}
