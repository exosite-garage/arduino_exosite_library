
========================================
About Exosite Arduino Library
========================================
This project is a simple example of using an Arduino board equipped with Ethernet Shield to send and receive data to/from the cloud by using Exosite's Cloud Data Platform.  The example cloud_read_write code reads data from the cloud, adds 100 to it and sends it back to the cloud. 

License is BSD, Copyright 2013, Exosite LLC (see LICENSE file)

Tested with Arduino 1.0.5

========================================
Release Info
========================================
**Release 2011-06-05**
 - initial version

**Release 2013-07-29**
 - Major rewrite to both read and write multiple datasources in one HTTP call.
 - Removed all use of Strings due to stability issues (except for manipulating string object passed to existing functions).
 - Existing `sendToCloud()` and `readFromCloud()` changed to use new call internally. External Interface Unchanged

**Release 2013-10-18**
 - Simplified interface to use character strings or Arduino Strings instead of arrays of character arrays. User must now URL encode and decode their own data.
 - Made compatible with Arduino WiFi shield (and anything that similarly subclasses the Client class).
 - Updated examples to use new interface.