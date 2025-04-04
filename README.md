Sample code for querying the Windows event logs. Based on Microsoft's [Consuming Events (Windows Event Log)](https://learn.microsoft.com/en-us/windows/win32/wes/consuming-events) article.

## Examples of logged events

#### Disk failures
Disk and filesystem problems are automaticaly logged:  
![image](https://github.com/user-attachments/assets/b148863a-3d67-4b81-a64f-4bb66384e136)  
![image](https://github.com/user-attachments/assets/8011024f-be1d-4dd7-8a46-c9b9023a25ec)

#### HW device failures
HW device failures are automatically logged:  
![image](https://github.com/user-attachments/assets/5afba146-c45d-406c-be31-d7031a236b73)  
![image](https://github.com/user-attachments/assets/feef2cd2-43f1-4b77-84e3-bf74db077977)

### GPU problems
GPU watchdog recovery:  
![image](https://github.com/user-attachments/assets/ef34e983-696f-4397-9d4c-0d1f1ec73d2c)

High resource usage:  
![image](https://github.com/user-attachments/assets/ea8aaa33-fb50-4640-8506-5c7f01570471)

#### Firewall blocking network traffic
TODO: Add screenshot.

#### Application whitelising (WDAC) problems
TODO: Add screenshot.


## Documentation
* [Event Logging](https://learn.microsoft.com/en-us/windows/win32/eventlog/event-logging) (old APIs)
* [Windows Event Log](https://learn.microsoft.com/en-us/windows/win32/wes/windows-event-log) (new APIs)
