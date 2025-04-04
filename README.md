Sample code for querying the Windows event logs. Based on Microsoft's [Consuming Events (Windows Event Log)](https://learn.microsoft.com/en-us/windows/win32/wes/consuming-events) article.

**WARNING**: Work in progress. The sample code is currently only performing a hardcode XPath query in the "System" log. The log entries are also diplayed in "raw" XML, and not yet "pretty printed".

## Examples of logged events
Some examples of system problems automatically logged by Windows. All of these events can also be accessed through the Windows Event Log APIs.

### Disk failures
Disk and filesystem problems are automaticaly logged:  
![image](https://github.com/user-attachments/assets/b148863a-3d67-4b81-a64f-4bb66384e136)  
![image](https://github.com/user-attachments/assets/8011024f-be1d-4dd7-8a46-c9b9023a25ec)

### HW device failures
HW device failures are automatically logged:  
![image](https://github.com/user-attachments/assets/723867a3-6bb5-4405-8719-967240e72781)
![image](https://github.com/user-attachments/assets/5afba146-c45d-406c-be31-d7031a236b73)

### GPU problems
GPU watchdog recovery:  
![image](https://github.com/user-attachments/assets/ef34e983-696f-4397-9d4c-0d1f1ec73d2c)

High GPU resource usage:  
![image](https://github.com/user-attachments/assets/ea8aaa33-fb50-4640-8506-5c7f01570471)

### Firewall logs
Logging of dropped packets by the firewall: (logging _not_ enabled by default)  
![image](https://github.com/user-attachments/assets/4c01cd9f-2a6b-4963-9d30-c2a2b776a5c7)

### Application whitelising (WDAC) problems
Applications not starting due to insufficient whitelisting:  
![image](https://github.com/user-attachments/assets/c034b43e-fb26-470a-903b-e1c37dc1fef9)

### Background service problems
![image](https://github.com/user-attachments/assets/75f05ab9-e45a-494f-9471-5d9f3a775eb0)

### Critical OS failure
Reason for unexpected system reboots:  
![image](https://github.com/user-attachments/assets/0164914d-c894-4347-904b-bfa60e65bda7)


### SW installation failure
TODO: Add screenshot to show how installation failures are logged.

Documentation: [Windows Installer Event Logging](https://learn.microsoft.com/en-us/windows/win32/msi/event-logging)

## Documentation
* ~~[Event Logging](https://learn.microsoft.com/en-us/windows/win32/eventlog/event-logging)~~ (old APIs)
* [Windows Event Log](https://learn.microsoft.com/en-us/windows/win32/wes/windows-event-log) (new APIs)
