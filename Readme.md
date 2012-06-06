
========================================
About Exosite Arduino Library
========================================
This project is an simple example of using an Arduino board equipped with Ethernet Shield to send and receive data to/from the cloud by using Exosite's Cloud Data Platform.  The example cloud_read_write code reads data from the cloud, adds 100 to it and sends it back to the cloud. 

License is BSD, Copyright 2012, Exosite LLC (see LICENSE file)

Tested with Arduino 1.01

========================================
Quick Start
========================================
1) Download the Arduino toolchain and development environment

 * http://www.arduino.cc/en/Main/software

2) Go to "Sketch->Show Sketch Folder", your Sketch folder will then be displayed. Create a directory named "libraries" if it does not exist.

3) Copy the Exosite library folder to the directory sketchbook-location\"libraries". You should then see "File->Examples->Exosite"

4) Open "File->Examples->Exosite->cloud_read_write"

5) Edit the "PUTYOURCIKHERE" value in cloud.cpp to match your CIK value

  * HINT: Obtain a CIK by signing up for a free account at https://portals.exosite.com. After activating your account, login and navigate to https://portals.exosite.com/manage/devices and click the +Add Device link

6) Edit the MAC address values in cloud.cpp if you have a valid MAC address, or you can just use the default value for testing purposes

7) In Portals (https://portals.exosite.com), add two Data Sources to match the data resources (aliases) the code is using

  * HINT: Go to https://portals.exosite.com/manage/data and click +Add Data Source

  * HINT: Ensure the "Resource:" values are set to "1" and "onoff" respectively to match the code

  * HINT: Add an "On/Off Switch" widget to your dashboard to control data source "onoff"

8) Go to "Tools->Board" to select the corresponding Arduino board type
 
9) In the Arduino software, compile and verify there are no errors

10) Go to "Tools->Serial" to select the serial port your Arduino board is connected to

11) Go to File->Upload to I/O Board to upload the program

12) When "Done uploading" is displayed, go to https://portals.exosite.com to see your data in the cloud!

  * HINT: Your Arduino board must be connected to the Internet via the RJ-45 ethernet jack

For more information on this project and other examples, checkout our Exosite Garage github page at http://exosite-garage.github.com

========================================
Release Info
========================================
**Release 2011-06-05**
 - initial version
