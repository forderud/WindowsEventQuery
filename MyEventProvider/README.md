Custom Windows event provider. Used to enable custom log entry types throgh the legacy [Event Logging](https://learn.microsoft.com/en-us/windows/win32/eventlog/event-logging) API.


#### How to install
* Build project.
* Run `regsvr32.exe MyEventProvider.dll` from an admin command prompt to register the provider.

#### How to uninstall
* Run `regsvr32.exe /u MyEventProvider.dll` from an admin command prompt to unregister the provider.
