ESP8266 Occupancy Sensor
=========================

1. Download the Arduino toolchain and development environment

    1. http://arduino.cc/en/Main/Software

2. Go to "Sketch->Show Sketch Folder", your Sketch folder will then be displayed. Create a directory named "libraries" if it does not exist.

3. Copy the Exosite library folder to the directory sketchbook-location\"libraries". You should then see "File->Examples->Exosite"

4. Open "File->Examples->arduino_exosite_library->ESP8266OccupancySensor"

5. Sign up for a free account at https://portals.exosite.com.

6. Go to the Devices page (https://portals.exosite.com/manage/devices) and add a device

	1. Click on "+ Add Device"

	2. Select "I want to create a generic device."

	3. Leave Device Type as generic, enter your timezone, and optionally add a location. You may also limit the resources your device may use.

	4. Name your device.

7. Click on the device you just created to open the Device Information window.

8. Add the two datasources "uptime" and "command". (For Each)

	1. Click "+ Add Data"

	2. Give your device a human readable name (eg. "Device Uptime"), set it to format "string", set the alias ("uptime" and "command").

9. Copy your new device's 40 character CIK from the Device Information window and paste it into line 47 of the example opened in step 4.

10. Edit the SSID and password on lines 48 and 49 to correspond to an in-range WiFi network.

11. Connect your ESP8266 to your computer using a USB cable

	1. The Sparkfun ESP8266 Thing was used to test this example

12. Connect the PIR Motion Sensor to the ESP8266.

	1. You may need to modify the PIR Motion Sensor as done in this community form post: [LINK HERE]

13. Go to "Tools->Serial" to select the serial port your Arduino board is connected to

14. Download and install the ESP8266 library following the instructions here: https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon

15. Go to "Tools->Board" to select the Sparkfun ESP8266 Thig
 
14. In the Arduino software, compile and verify there are no errors

15. Upload the program to your board.

16. When "Done uploading" is displayed, go to https://portals.exosite.com to see your data in the cloud!

For more information on this project and other examples, checkout our Exosite Garage github page at http://exosite-garage.github.com