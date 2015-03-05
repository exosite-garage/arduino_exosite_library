Exosite Arduino Ethernet Quick Start
====================================

1. Download the Arduino toolchain and development environment

    1. http://arduino.cc/en/Main/Software

2. Go to "Sketch->Show Sketch Folder", your Sketch folder will then be displayed. Create a directory named "libraries" if it does not exist.

3. Copy the Exosite library folder to the directory sketchbook-location\"libraries". You should then see "File->Examples->Exosite"

4. Open "File->Examples->arduino_exosite_library->ExoEthernetGettingStarted"

5. Sign up for a free account at https://portals.exosite.com.

6. Go to the Devices page (https://portals.exosite.com/manage/devices) and add a device

	1. Click on "+ Add Device"
	2. Select "Select a supported device below."
	3. Select "Arduino Compatible Device" device type from drop down list
	4. Click 'Continue' button.

	3. Leave the Unique Identifier provided in 'Enter device Unique Identifier' field.  Copy this unique identifier, which will be used in your Arduino Sketch.

	4. Name your device, anything you want.  Leave the device location field empty, it is optional.
	5. Hit 'Continue' button.
	6. Hit the 'Quit' button on the final window that says "Your Arduino Compatible Device was successfully enabled with the CIK..."
	

9. Copy your new device's **Unique Identifier** that was auto generated from the Device Information window and paste it into line of the example that says "unique_id" opened in step 4.

     char unique_id[11] = "00AA33FFDD"; 

10. Edit the MAC address value on line 40 with the MAC address printed on the white sticker on your Ethernet shield.  This is not required but if you don't, it may cause issues on your local network.

11. Connect your Arduino to your computer using a USB cable and to your router using an Ethernet cable.

12. Go to "Tools->Serial" to select the serial port your Arduino board is connected to

13. Go to "Tools->Board" to select the corresponding Arduino board type
 
14. In the Arduino software, compile and verify there are no errors

15. Upload the program to your board.

16. When "Done uploading" is displayed, go to https://portals.exosite.com to see your data in the cloud!  

For more information on this project and other examples, checkout our Exosite Garage github page at http://exosite-garage.github.com