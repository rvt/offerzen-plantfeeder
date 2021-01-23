# Offerzen plantfeeder

Software + box for the offsetzen platfeeder, I feel all electronics should go in a box!

Here is a link to the project I am talking about: [plant_tech_ams](https://github.com/OfferZen-Make/plant_tech_ams)

## To Build you need PlatformIO

Use `platformio run --target upload -e default` to build and uploads.

Use `platformio run --target uploadfs -e default`to upload the needed filesystem.

## To Configure

* Look into your serial terminal and note your IP address and ID of your feeder.
* Use a webbrowser and fill in your WIFI and MQTT parameters

## Top set thresshold values use MQTT and issue the following commands

* Topic: `plantfeeder/XXXXXXXX/config`
* Payload: `min=400 max=800`

Where `XXXXXXXX`is the ID for your plantfeeder
Note: Max should always be higher than min, no protection against that

No MQTT, No problem! Edit `data/hwCfg.conf`and fill in the values. (The L in front of the number is correct and not a typo, format is very strict, no spaces!)

## Scripting.

It uses a very little script and primitive to turn on/off the motor, see ´data/default.txt´ to change timings of the motor. If you want to add detecting of moisture levels within the script then look into
´scriptcontext.cpp´ and the ´scripting.cpp´ files or contact me and I can help.


To ensure that we don't keep the pump running on disconnect any value measured above `1020` is considered a disconnect and we won't run the engine to prevent flooding.
