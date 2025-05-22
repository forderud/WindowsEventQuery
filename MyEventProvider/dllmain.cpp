#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <atlbase.h>
#include <cassert>


/** DLL entry point. */
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


// Adds entries to the system registry
STDAPI DllRegisterServer() {
    CRegKey parent;
    LSTATUS res = parent.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application", KEY_READ | KEY_WRITE);
    assert(res == ERROR_SUCCESS);
    if (res != ERROR_SUCCESS)
        return E_UNEXPECTED;

    CRegKey reg;
    res = reg.Create(parent, L"MyEventProvider");
    assert(res == ERROR_SUCCESS);
    if (res != ERROR_SUCCESS)
        return E_UNEXPECTED;

    {
        res = reg.SetDWORDValue(L"CategoryCount", 3);
        assert(res == ERROR_SUCCESS);

        res = reg.SetStringValue(L"CategoryMessageFile", L"C:\\Dev\\WindowsEventQuery\\x64\\Debug\\MyEventProvider.dll");
        assert(res == ERROR_SUCCESS);

        res = reg.SetStringValue(L"EventMessageFile", L"C:\\Dev\\WindowsEventQuery\\x64\\Debug\\MyEventProvider.dll");
        assert(res == ERROR_SUCCESS);

        res = reg.SetStringValue(L"ParameterMessageFile", L"C:\\Dev\\WindowsEventQuery\\x64\\Debug\\MyEventProvider.dll");
        assert(res == ERROR_SUCCESS);


        res = reg.SetDWORDValue(L"TypesSupported", 7);
        assert(res == ERROR_SUCCESS);
    }

    return NOERROR;
}


// Removes entries from the system registry
STDAPI DllUnregisterServer() {
    CRegKey parent;
    LSTATUS res = parent.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application", KEY_READ | KEY_WRITE);
    assert(res == ERROR_SUCCESS);
    if (res != ERROR_SUCCESS)
        return E_UNEXPECTED;

    res = parent.DeleteSubKey(L"MyEventProvider");
    assert(res == ERROR_SUCCESS);
    if (res != ERROR_SUCCESS)
        return E_UNEXPECTED;

    return NOERROR;
}
