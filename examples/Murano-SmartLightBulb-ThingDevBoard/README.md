# Exosite Murano Example - ESP8266 Thing Dev Board as WiFi Smart Lightbulb

This example walks through setting up a ESP8266 Thing Dev Board to act as a Smart Lightbulb
with Murano.  Users can feel free to customize after walking through this guide as you'll have
created a full Product instance that supports a deployment of devices specific to that Product.
Product definition can be updated and more devices can be added.  After walking through these steps,
users will be interacting with live device data using a prototype developer dashboard tool.  
After this, users can go through the Murano Solution examples to deploy a Example consumer web application
that works with this product demo.

## Hardware Setup

###Hardware
* [Sparkfun ESP8266 Thing - Dev Board](https://www.sparkfun.com/products/13711)
* [Humidity and Temperature Sensor - RHT03 ](https://www.sparkfun.com/products/10167)
* 1 10K Ohm Resistors (for digital pin pull-up)
* 1 330 Ohm Resistor
* 1 LED (suggest green or yellow)
* Micro-USB B Cable



###Hook-up Guide

![Thing-Board Hookup](assets/esp8266-thing-dev-hookup.png)

## Software Setup
### Setup Arduino IDE and Libraries
0. Install Arduino [arduino.cc](https://www.arduino.cc/en/Main/Software)
1. Install ESP8266 Thing Dev Board support using Arduino Board Manager - [ESP8266 Thing Dev Board Install Directions ](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon)
2. Install Exosite Arduino Library (if you haven't already) - Use Arduino Library Manager, search for `Exosite`

   _More info: [Exosite Arduino Library Details](https://github.com/exosite-garage/arduino_exosite_library)_
   ![search exosite](assets/library_manager.png)
   ![search exosite](assets/search_for_exosite.png)
   
3. Install [Adafruit DHT Library](https://github.com/adafruit/DHT-sensor-library) - You can use Arduino Library Manager, search for `DHT`
4. Install [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library) - You can use Arduino Library Manager, search for `DHT`
   ![search exosite](assets/search_for_dht.png)



### NEW TO ARDUINO?
_Arduino is a powerful software platform for quickly building applications on embedded hardware.  Although typically easier than most any other software IDEs and compilers, it can still be a learning experience for new users. Once installed quickly with the necessary libraries, users will find they can customize and build applications in minutes.  Here are a few links to get an understanding of Arduino since this guide does not cover every concept of the Arduino IDE and hardware concepts (in this case the SparkFun ESP8266 Thing Dev Board) Note that Arduino supports a number of hardware platforms, not just Arduino branded boards themselves, like the ESP8266 Thing Dev Board._

* [Sparkfun ESP8266 Thing - Dev Board](https://www.sparkfun.com/products/13711)
* [Arduino Getting Started](https://www.arduino.cc/en/Guide/HomePage0


