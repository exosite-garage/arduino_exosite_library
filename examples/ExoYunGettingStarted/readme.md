Exosite Arduino Yún Quick Start
====================================
Note: Exosite has created a separate reference Yún device type for a couple of reasons.
The Yún actually has MAC Adddresses programmed into the device, so this can be used 
for activation instead of the Arduino Compatible Ethernet example that generates a random unique identifier and you need to put this into your sketch.  For the Yún, you do not need to do this, it just uses it's WiFi MAC Address to activate.  The second reasons is the Yún has slightly different hardware available, most notably the Yún has D13 available to control the board LED.  When you use a regular Arduino with Ethernet shield, you lose D13 so providing an option to control the LED wouldn't make sense.


1. Download the Arduino toolchain and development environment

    1. http://arduino.cc/en/Main/Software

2. Go to "Sketch->Show Sketch Folder", your Sketch folder will then be displayed. Create a directory named "libraries" if it does not exist.

3. Copy the Exosite library folder to the directory sketchbook-location\"libraries". You should then see "File->Examples->Exosite"

4. Open "File->Examples->arduino_exosite_library->ExoYunGettingStarted"

5. Sign up for a free account at https://portals.exosite.com.

6. Go to the Devices page (https://portals.exosite.com/manage/devices) and add a device

	1. Click on "+ Add Device"
	2. Select "Select a supported device below."
	3. Select "Arduino Yun Compatible" device type from drop down list
	4. Click 'Continue' button.

	3. Enter your WiFi MAC Address.  You can get this either from the Yún's web server page or if you run this sketch, it will print it out at boot time.  It should be in the format 00:11:AA:BB:22:CC (capital letters and use colons).  If you use something other than your Yún's WiFi MAC Address, the device won't activate.

	4. Name your device, anything you want.  Leave the device location field empty, it is optional.
	5. Hit 'Continue' button.
	6. Hit the 'Quit' button on the final window that says "Your Arduino Yún Compatible Device was successfully enabled with the CIK..."
	

11. Power your Arduino Yún using the USB cable connected to your computer or using a power adapter and follow the Yún instructions for connecting to your network via Ethernet or WiFi.  [Arduino Yún Platform Information](http://arduino.cc/en/Main/ArduinoBoardYun)

12. Go to "Tools->Serial" to select the serial port your Arduino Yún board is connected to

13. Go to "Tools->Board" to select the corresponding Arduino Yún board type
 
14. In the Arduino software, compile and verify there are no errors

15. Upload the program to your board.

16. When "Done uploading" is displayed, go to https://portals.exosite.com to see your data in the cloud!  

