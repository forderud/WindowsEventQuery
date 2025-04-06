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

    Event& operator = (Event&& other) {
        m_event = other.m_event;
        other.m_event = 0; // prevent double delete
        return *this;
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

    operator EVT_HANDLE () const {
        return m_event;
    }

    EVT_HANDLE* GetAddress() {
        return &m_event;
    }

private:
    EVT_HANDLE m_event = 0;
};
static_assert(sizeof(Event) == sizeof(EVT_HANDLE), "Event size mismatch");


void EventQuery(std::wstring channel, std::wstring query, std::wstring publisherId, size_t maxCount);