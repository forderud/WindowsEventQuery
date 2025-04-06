#include "Publisher.hpp"
#include "Event.hpp"
#include <cassert>


// Contains the value and message string for a type, such as
// an opcode or task that the provider defines or uses. If the
// type does not specify a message string, the message member
// contains the value of the type's name attribute.
struct MSG_STRING {
    MSG_STRING() {
        dwValue = 0;
    }

    ~MSG_STRING() {
    }

    union {
        DWORD dwValue;  // Value attribute for opcode, task, and level
        UINT64 ullMask; // Mask attribute for keyword
    };
    LPWSTR pwcsMessage = nullptr; // Message string or name attribute
};

// Header for the block of value/message string pairs. The dwSize member
// is the size, in bytes, of the block of MSG_STRING structures to which 
// the pMessage member points.
struct MESSAGES {
    MESSAGES() {
    }
    ~MESSAGES() {
        Clear();
    }

    // Free the memory for each message string in the messages block
    // and then free the messages block.
    void Clear() {
        if (pMessage) {
            DWORD dwCount = dwSize/sizeof(MSG_STRING);
            for (DWORD i = 0; i < dwCount; i++) {
                free(((pMessage) + i)->pwcsMessage);
                ((pMessage) + i)->pwcsMessage = NULL;
            }

            free(pMessage);
            pMessage = nullptr;
        }

        dwSize = 0;
    }

    DWORD dwSize = 0;
    MSG_STRING* pMessage = nullptr;
};


void  PrintPublisherMetadata(std::wstring publisherId) {
    Event metadata(EvtOpenPublisherMetadata(NULL, publisherId.c_str(), NULL, /*locale*/0, 0));
    if (!metadata) {
        wprintf(L"EvtOpenPublisherMetadata failed with %d\n", GetLastError());
        return;
    }
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

        PrintPublisherMetadata(publisherId);

        publisherId.resize(bufferUsed - 1); // remove null-termination
        result.push_back(publisherId);
    }

    return result;
}
