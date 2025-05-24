#include <windows.h>
#include <stdio.h>
#include <evntprov.h>
#include <stdexcept>
#include <vector>
#include "MyLogSchema.h"  // Generated from manifest

#define SUNDAY     0X1
#define MONDAY     0X2
#define TUESDAY    0X4
#define WEDNESDAY  0X8
#define THURSDAY   0X10
#define FRIDAY     0X20
#define SATURDAY   0X40

enum TRANSFER_TYPE {
    Download = 1,
    Upload,
    UploadReply
};

/** ETW event provider registration wrapper class. */
class EventHandle {
public:
    EventHandle(const GUID* providerGuid) {
        DWORD status = EventRegister(
            providerGuid, // GUID that identifies the provider
            NULL,         // Callback not used
            NULL,         // Context noot used
            &m_provider   // Used when calling EventWrite and EventUnregister
        );
        if (ERROR_SUCCESS != status) {
            wprintf(L"EventRegister failed with %lu\n", status);
            throw std::runtime_error("EventRegister failed");
        }
    }

    EventHandle(EventHandle&& obj) {
        std::swap(m_provider, obj.m_provider);
    }

    ~EventHandle() {
        EventUnregister(m_provider);
        m_provider = 0;
    }

    void Write(const EVENT_DESCRIPTOR* EventDescriptor, ULONG UserDataCount, EVENT_DATA_DESCRIPTOR* UserData) {
        DWORD status = EventWrite(m_provider, EventDescriptor, UserDataCount, UserData);
        if (status != ERROR_SUCCESS) {
            wprintf(L"EventWrite failed with 0x%x", status);
            throw std::runtime_error("EventWrite failed");
        }
    }

private:
    REGHANDLE m_provider = 0;
};

static EVENT_DATA_DESCRIPTOR EventDataArg(const void* DataPtr, ULONG DataSize) {
    EVENT_DATA_DESCRIPTOR desc{};
    EventDataDescCreate(&desc, DataPtr, DataSize);
    return desc;
}

int wmain(void) {
    EventHandle provider(&PROVIDER_GUID);

    // Prepare UserData parameters for the TransferEvent event. 
    // Parameter order need to match the EventData element order.
    std::vector<EVENT_DATA_DESCRIPTOR> parameters;

    USHORT scores[3] = { 45, 63, 21 };
    parameters.push_back(EventDataArg(scores, sizeof(scores)));

    parameters.push_back(EventDataArg(&PROVIDER_GUID, sizeof(PROVIDER_GUID)));

    BYTE cert[11] = { 0x2, 0x4, 0x8, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x0, 0x1 };
    parameters.push_back(EventDataArg(cert, sizeof(cert)));

    BOOL isLocal = TRUE;
    parameters.push_back(EventDataArg(&isLocal, sizeof(isLocal)));

    std::wstring Path = L"c:\\path\\folder\\file.ext";
    parameters.push_back(EventDataArg(Path.c_str(), (ULONG)(Path.length()+1)*sizeof(wchar_t)));

    DWORD days = MONDAY | TUESDAY;
    parameters.push_back(EventDataArg(&days, sizeof(days)));

    DWORD transferType = Upload;
    parameters.push_back(EventDataArg(&transferType, sizeof(transferType)));

    // Write the event. You do not have to verify if your provider is enabled before
    // writing the event. ETW will write the event to any session that enabled
    // the provider. If no session enabled the provider, the event is not written.
    provider.Write(&TransferEvent, (ULONG)parameters.size(), parameters.data());
}
