#include <stdio.h>
#include <vector>
#include "EventHandle.hpp"
#include "MyLogSchema.h"  // Generated from manifest



/** WARNING: Does NOT copy the data. */
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

    std::wstring path = L"c:\\path\\folder\\file.ext";
    parameters.push_back(EventDataArg(path.c_str(), (ULONG)(path.length()+1)*sizeof(wchar_t))); // incl. null-termination

    DWORD day = 3;
    parameters.push_back(EventDataArg(&day, sizeof(day)));

    DWORD transfer = 99;
    parameters.push_back(EventDataArg(&transfer, sizeof(transfer)));

    // Write the event. You do not have to verify if your provider is enabled before
    // writing the event. ETW will write the event to any session that enabled
    // the provider. If no session enabled the provider, the event is not written.
    provider.Write(&TransferEvent, (ULONG)parameters.size(), parameters.data());
}
