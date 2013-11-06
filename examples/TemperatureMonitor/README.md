TemperatureMonitor
==================

Basic Arduino Temp Monitor that reports data to Exosite IoT Platform / Web Portal 

Exosite Arduino Basic Temp Monitor - Version 3.1 

This sketch shows an example of sending data from a connected
sensor to Exosite. (http://exosite.com) Code was used from various
public examples including the Arduino Ethernet examples and the OneWire
Library examples found on the Arduino playground. 
(OneWire Lib credits to Jim Studt, Tom Pollard, Robin James, and Paul Stoffregen)

This code keeps track of how many milliseconds have passed
and after the user defined REPORT_TIMEOUT (default 60 seconds)
reports the temperature from a Dallas Semi DS18B20 1-wire temp sensor.
The code sets up the Ethernet client connection and connects / disconnects 
to the Exosite server when sending data.

Assumptions:
- Tested with Aruduino 1.0.5
- Arduino included Ethernet Library
- Arduino included SPI Library
- Using Exosite Library V2.1.1 (2013-11-04) https://github.com/exosite-garage/arduino_exosite_library
- Using OneWire Library Version 2.0 - http://www.arduino.cc/playground/Learning/OneWire
- Using Dallas Temperature Control Library - http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_372Beta.zip
- Uses Exosite's basic HTTP API, revision 1.0 https://github.com/exosite/api/tree/master/data
--- USER MUST DO THE FOLLOWING ---
- User has an Exosite account and created a device (CIK needed / https://portals.exosite.com -> Add Device)
- User has added a device to Exosite account and added a data source with alias 'temp', type 'float'


Hardware:
- Arduino Duemilanove or similiar
- Arduino Ethernet Shield
- Dallas Semiconductor DS18B20 1-Wire Temp sensor used in parasite power mode (on data pin 7, with 4.7k pull-up)

Version History:
- 1.0 - November 8, 2010 - by M. Aanenson
- 2.0 - July 6, 2012 - by M. Aanenson
- 3.0 - October 25, 2013 - by M. Aanenson - Note: Updated to use latest Exosite Arduino Library
- 3.1 - November 6, 2013 - by P. Barrett - Note: Updated to use Exosite Arduino Library V2.1.1

