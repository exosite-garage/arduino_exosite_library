About the Exosite Arduino Library
=================================
This library allows you to quickly and easily connect your Arduino project to Exosite's Data Platform in the cloud. It connects your project using an Arduino Ethernet shield, Arduino WiFi Shield, or any other networking shield that subclasses the Client class. See the examples folder for both an Ethernet and a WiFi example.

Note: A free account on exosite portals is required: https://portals.exosite.com

License is BSD, Copyright 2013, Exosite LLC (see LICENSE file)

Tested with Arduino 1.0.5

Interface
=========
```c
boolean Exosite::readWrite(char* readString, char* writeString, char** returnString)
```

```c
boolean Exosite::readWrite(String readString, String writeString, String &returnString)
```

`readString`: This selects which datasources to read by their alias. eg. "alias1&alias2"

`writeString`: This sets the values to write to certain datasources. eg. "alias3=value3&alias4=value4"

`returnstring`: This is the string returned with the values requested in `readString`. eg. "alias1=value1&alias2=value2"


Migration from V1
=================
Version 2 no longer sets up the ethernet shield for you. In your code you'll need to replace `Exosite exosite(&Ethernet, macData, cikData);` with 

```
class EthernetClient client;
Exosite exosite(cikData, &client);
```
You will also need to remove `exosite.init();` and add `Ethernet.begin(macData);` to your setup() function.

Release Info
============
**v2.0 - Release 2013-10-18**
 - Simplified interface to use character strings or Arduino Strings instead of arrays of character arrays. User must now URL encode and decode their own data.
 - Made compatible with Arduino WiFi shield (and anything that similarly subclasses the Client class).
 - Updated examples to use new interface.

**v1.1 - Release 2013-07-29**
 - Major rewrite to both read and write multiple datasources in one HTTP call.
 - Removed all use of Strings due to stability issues (except for manipulating string object passed to existing functions).
 - Existing `sendToCloud()` and `readFromCloud()` changed to use new call internally. External Interface Unchanged

**v1.0 - Release 2011-06-05**
 - initial version
