#pragma once
#include <windows.h>
#include <sddl.h>
#include <winevt.h>
#include <string>


/** RAII wrapper to avoid goto. */
class Event {
public:
    Event() = default;

    Event(EVT_HANDLE event) : m_event(event) {
    }

    ~Event() {
        Close();
    }

    void Close() {
        if (m_event) {
            EvtClose(m_event);
            m_event = 0;
        }
    }

    operator EVT_HANDLE& () {
        return m_event;
    }

protected:
    EVT_HANDLE m_event = 0;
};
static_assert(sizeof(Event) == sizeof(EVT_HANDLE), "Event size mismatch");


void EventQuery(std::wstring channel, std::wstring query, std::wstring publisherName);