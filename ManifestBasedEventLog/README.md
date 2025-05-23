Manifest-based log provider sample


Based on [Writing Manifest-based Events](https://learn.microsoft.com/en-us/windows/win32/etw/writing-manifest-based-events).

### How to install
From an admin command prompt:
```
wevtutil.exe im MyLogSchema.man
``

### How to uninstall
From an admin command prompt:
```
wevtutil.exe um MyLogSchema.man
```
