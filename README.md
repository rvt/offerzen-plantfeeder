# Offerzen plantfeeder

Software + box for the offsetzen platfeeder, I feel all electronics should go in a box!

Here is a link to the project I am talking about: [plant_tech_ams](https://github.com/OfferZen-Make/plant_tech_ams)

Note, I had issues wth the motor and had to connect it to `D6`

## To Build you need PlatformIO

Use `platformio run --target upload -e default` to build and uploads.

Use `platformio run --target uploadfs -e default`to upload the needed filesystem.

## To Configure

* Look into your serial terminal and note your IP address and ID of your feeder.
* Use a webbrowser and fill in your WIFI and MQTT parameters

## Top set threshold values use MQTT and issue the following commands

* Topic: `plantfeeder/XXXXXXXX/config`
* Payload: `wet=400 dry=800`

Where `XXXXXXXX`is the ID for your plantfeeder.

When the measured value is below wet, then we ar enot adding any more water.
Consequently, when the measured value is above dry we will be adding more water;
* Dry should always be higher than wet, no protection against that
* The higher the neasured value, the dryer the soil

No MQTT, No problem! Edit `data/hwCfg.conf`and fill in the values. (The L in front of the number is correct and not a typo, format is very strict, no spaces!)

## Scripting.

It uses a very little script and primitive to turn on/off the motor, see ´data/default.txt´ to change timings of the motor. If you want to add detecting of moisture levels within the script then look into
´scriptcontext.cpp´ and the ´scripting.cpp´ files or contact me and I can help.


To ensure that we don't keep the pump running on disconnect any value measured above `1020` is considered a disconnect and we won't run the engine to prevent flooding.

## Deep Sleep
The device will enter deep sleep to conserve battery. you MUST connect FPIO16 to the RST pin with one of your dupont cable or else the device won't wake up
