#include "Publisher.hpp"
#include "Event.hpp"
#include <cassert>


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

        publisherId.resize(bufferUsed - 1); // remove null-termination
        result.push_back(publisherId);
    }

    return result;
}
