#include "Publisher.hpp"
#include "Event.hpp"


std::vector<std::wstring> GetPublisherList(EVT_HANDLE session) {
    Event providerList(EvtOpenPublisherEnum(session, 0));

    std::vector<std::wstring> result;

    std::wstring buffer;
    while (true) {
        DWORD bufferUsed = 0;
        BOOL ok = EvtNextPublisherId(providerList, (DWORD)buffer.size(), (wchar_t*)buffer.data(), &bufferUsed);
        if (!ok) {
            DWORD status = GetLastError();

            if (status == ERROR_NO_MORE_ITEMS) {
                // reached the end
                break;
            } else if (status == ERROR_INSUFFICIENT_BUFFER) {
                // repeat call with larger buffer
                buffer.resize(bufferUsed);
                ok = EvtNextPublisherId(providerList, (DWORD)buffer.size(), (wchar_t*)buffer.data(), &bufferUsed);
            } else {
                wprintf(L"EvtNextPublisherId failed with %lu.\n", status);
                abort();
            }
        }

        result.push_back(buffer);
    }

    return result;
}