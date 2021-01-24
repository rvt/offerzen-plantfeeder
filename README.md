# Offerzen plantfeeder

This is software + box for the offerzen plantfeeder [plant_tech_ams](https://github.com/OfferZen-Make/plant_tech_ams)
Check `platform.io` for pin layout as I had to change the pin number for the motor that somehow did not work.

### Support

* Deep sleep
* sending messages over MQTT
* A little scripting language
* Web based configurating of WiFi and MQTT configurations
* Loos wire detection so the motor won't keep running if your sensor wires breaks
* Hysteresis so your motor won't turn off/on around threshold value
* STl + OpenSCAD in box directory, you might need to modify dimentions for your need 

![Total setup](box/setup.jpg)


## To Build you need PlatformIO

Use `platformio run --target upload -e default` to build and uploads.

Use `platformio run --target uploadfs -e default`to upload the needed filesystem.

## To Configure

* Look into your serial terminal and note your IP address and ID of your feeder.
* Use a webbrowser and fill in your WiFi and MQTT parameters

## Set threshold values use MQTT and issue the following commands

* Topic: `plantfeeder/XXXXXXXX/config`
* Payload: `wet=400 dry=800`

Where `XXXXXXXX`is the ID for your plantfeeder, check a tool like MQTT Spy with plantfeeder/# to see your messages

When the measured value is below wet, then we are not adding any more water.
Consequently, when the measured value is above dry we will be adding more water;
* Dry should always be higher than wet, no protection against that
* The higher the measured value, the dryer the soil

No MQTT, No problem! Edit `data/hwCfg.conf`and fill in the values. (The L in front of the number is correct and not a typo, format is very strict, no spaces and integer only!)

## Scripting.

It uses a very little script and primitive to turn on/off the motor, see ´data/default.txt´ to change timings of the motor. If you want to add detecting of moisture levels within the script then look into
´scriptcontext.cpp´ and the ´scripting.cpp´ files or contact me and I can help.

To ensure that we don't keep the pump running on disconnect any value measured above `1020` is considered a disconnect and we won't run the engine to prevent flooding.

You can send on topic `plantfeeder/XXXXXXXX/load` with payload `/pump5.txt` to start the script (it will turn on the pump for 5 seconds) after that it will load the `default.txt` again. Be ware that during deep sleep nothing will happen, I have yet to test if after deep sleep it will load the script correctly (let me know).

## Deep Sleep
The device will enter deep sleep to conserve battery. you MUST connect GPIO16 to the RST pin with one of your dupont cable or else the device won't wake up. 
I did had issues with a powerpack that would turn of because the load was so low that the powerpack turned itself of.
