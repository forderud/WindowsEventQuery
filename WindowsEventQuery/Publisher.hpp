#pragma once
#include <windows.h>
#include <sddl.h>
#include <winevt.h>
#include <string>
#include <vector>


std::vector<std::wstring> GetPublisherList(EVT_HANDLE session);
