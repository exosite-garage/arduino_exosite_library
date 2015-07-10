ESP8266 Occupancy Sensor
=========================

1. Download the Arduino toolchain and development environment

    1. http://arduino.cc/en/Main/Software

2. Go to "Sketch->Show Sketch Folder", your Sketch folder will then be displayed. Create a directory named "libraries" if it does not exist.

3. Copy the Exosite library folder to the directory sketchbook-location\"libraries". You should then see "File->Examples->Exosite"

4. Comment out line 503 of Exosite.cpp in the Exosite library directory. This is so that this particular board will work, you can comment it back in for other projects.

5. Open "File->Examples->arduino_exosite_library->ESP8266OccupancySensor"

6. Sign up for a free account at https://portals.exosite.com.

7. Go to the Devices page (https://portals.exosite.com/manage/devices) and add a device

	1. Click on "+ Add Device"

	2. Select "I want to create a generic device."

	3. Leave Device Type as generic, enter your timezone, and optionally add a location. You may also limit the resources your device may use.

	4. Name your device.

8. Click on the device you just created to open the Device Information window.

9. Add the two datasources "uptime" and "command". (For Each)

	1. Click "+ Add Data"

	2. Give your device a human readable name (eg. "Device Uptime"), set it to format "string", set the alias ("uptime" and "command").

10. Copy your new device's 40 character CIK from the Device Information window and paste it into line 47 of the example opened in step 4.

11. Edit the SSID and password on lines 48 and 49 to correspond to an in-range WiFi network.

12. Connect your ESP8266 to your computer using a USB cable. This program was tested using this one: https://www.sparkfun.com/products/9873

	1. The Sparkfun ESP8266 Thing was used to test this example

13. Connect the PIR Motion Sensor to the ESP8266.

	1. You may need to modify the PIR Motion Sensor as done in this community form post: https://community.exosite.com/t/esp8266-wifi-occupancy-sensor/142

14. Go to "Tools->Serial" to select the serial port your Arduino board is connected to

15. Download and install the ESP8266 library following the instructions here: https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon

16. Go to "Tools->Board" to select the Sparkfun ESP8266 Thing
 
17. In the Arduino software, compile and verify there are no errors

18. Upload the program to your board.

19. When "Done uploading" is displayed, go to https://portals.exosite.com to see your data in the cloud!

20. Create some Lua script datarule of your own design to set the amount of motions detected in an interval result in an "occupied" room. See the community form post above for an example.

For more information on this project and other examples, checkout our Exosite Garage github page at http://exosite-garage.github.com