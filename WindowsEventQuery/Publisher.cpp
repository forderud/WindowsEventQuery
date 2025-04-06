#include "Publisher.hpp"
#include "Event.hpp"
#include <cassert>


// Contains the value and message string for a type, such as
// an opcode or task that the provider defines or uses. If the
// type does not specify a message string, the message member
// contains the value of the type's name attribute.
typedef struct _msgstring {
    union {
        DWORD dwValue;  // Value attribute for opcode, task, and level
        UINT64 ullMask; // Mask attribute for keyword
    };
    LPWSTR pwcsMessage; // Message string or name attribute
} MSG_STRING, * PMSG_STRING;

// Header for the block of value/message string pairs. The dwSize member
// is the size, in bytes, of the block of MSG_STRING structures to which 
// the pMessage member points.
typedef struct _messages {
    DWORD dwSize;
    PMSG_STRING pMessage;
} MESSAGES, * PMESSAGES;



Event GetPublisherMetadata(std::wstring publisherId) {
    return EvtOpenPublisherMetadata(NULL, publisherId.c_str(), NULL, /*locale*/0, 0);
}

std::vector<std::wstring> EnumeratePublishers() {
    Event providerList(EvtOpenPublisherEnum(NULL, 0));
    if (!providerList) {
        DWORD status = GetLastError();
        wprintf(L"EvtOpenPublisherEnum failed with %lu.\n", status);
        abort();
    }

    std::vector<std::wstring> result;

    std::wstring publisherId;
    while (true) {
        // grow to current capacity to reduce the number of reallocations
        publisherId.resize(publisherId.capacity());

        DWORD bufferUsed = 0;
        BOOL ok = EvtNextPublisherId(providerList, (DWORD)publisherId.size(), (wchar_t*)publisherId.data(), &bufferUsed);
        if (!ok) {
            DWORD status = GetLastError();

            if (status == ERROR_NO_MORE_ITEMS) {
                // reached the end
                break;
            } else if (status == ERROR_INSUFFICIENT_BUFFER) {
                // repeat call with larger buffer
                publisherId.resize(bufferUsed);
                ok = EvtNextPublisherId(providerList, (DWORD)publisherId.size(), (wchar_t*)publisherId.data(), &bufferUsed);
            } else {
                wprintf(L"EvtNextPublisherId failed with %lu.\n", status);
                abort();
            }
        }

        GetPublisherMetadata(publisherId);

        publisherId.resize(bufferUsed - 1); // remove null-termination
        result.push_back(publisherId);
    }

    return result;
}
