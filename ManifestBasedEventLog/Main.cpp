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

#define MAX_NAMEDVALUES          5  // Maximum array size

struct NAMEDVALUE {
    LPCWSTR name;
    USHORT  value;
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

    // Data to load into event descriptors

    // Load the array of data descriptors for the TransferEvent event. 
    // Add the data to the array in the order of the <data> elements
    // defined in the event's template. 
    std::vector<EVENT_DATA_DESCRIPTOR> parameters;

    USHORT Scores[3] = { 45, 63, 21 };
    void* pImage = &Scores;
    parameters.push_back(EventDataArg(&pImage, sizeof(pImage)));
    parameters.push_back(EventDataArg(Scores, sizeof(Scores)));

    PBYTE Guid = (PBYTE)&PROVIDER_GUID;
    parameters.push_back(EventDataArg(Guid, sizeof(GUID)));

    BYTE Cert[11] = { 0x2, 0x4, 0x8, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x0, 0x1 };
    parameters.push_back(EventDataArg(Cert, sizeof(Cert)));

    BOOL IsLocal = TRUE;
    parameters.push_back(EventDataArg(&IsLocal, sizeof(BOOL)));

    LPCWSTR Path = L"c:\\path\\folder\\file.ext";
    parameters.push_back(EventDataArg(Path, (ULONG)(wcslen(Path) + 1) * sizeof(WCHAR)));

    USHORT ArraySize = MAX_NAMEDVALUES;
    parameters.push_back(EventDataArg(&ArraySize, sizeof(USHORT)));

    // If your event contains a structure, you should write each member
    // of the structure separately. If the structure contained integral data types
    // such as DWORDs and the data types were aligned on an 8-byte boundary, you 
    // could use the following call to write the structure, however, you are 
    // encouraged to write the members separately.
    //
    // EventDataDescCreate(&EvtData, struct, sizeof(struct));
    //
    // Because the array of structures in this example contains both strings 
    // and numbers, you must write each member of the structure separately.
    NAMEDVALUE NamedValues[MAX_NAMEDVALUES] = {
        {L"Bill", 1},
        {L"Bob", 2},
        {L"William", 3},
        {L"Robert", 4},
        {L"", 5}
    };

    for (int j = 0; j < MAX_NAMEDVALUES; j++) {
        parameters.push_back(EventDataArg(NamedValues[j].name, (ULONG)(wcslen(NamedValues[j].name) + 1) * sizeof(WCHAR)));
        parameters.push_back(EventDataArg(&(NamedValues[j].value), sizeof(USHORT)));
    }

    DWORD Day = MONDAY | TUESDAY;
    parameters.push_back(EventDataArg(&Day, sizeof(DWORD)));

    DWORD TransferType = Upload;
    parameters.push_back(EventDataArg(&TransferType, sizeof(DWORD)));

    // Write the event. You do not have to verify if your provider is enabled before
    // writing the event. ETW will write the event to any session that enabled
    // the provider. If no session enabled the provider, the event is not 
    // written.
    provider.Write(&TransferEvent, (ULONG)parameters.size(), parameters.data());
}
