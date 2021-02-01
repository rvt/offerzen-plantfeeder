/*
 */
#include <memory>
#include <cstring>
#include <vector>

#include "network.hpp"
extern "C" {
#include <crc16.h>
}

#include <FS.h>
#include <scripting.hpp>
#define WIFI_getChipId() ESP.getChipId()

#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin()

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <propertyutils.hpp>
#include <optparser.hpp>
#include <utils.h>

#include <config.hpp>

#include <StreamUtils.h>
typedef PropertyValue PV;

// of transitions
uint32_t counter50TimesSec = 1;

// Number calls per second we will be handling
constexpr uint8_t FRAMES_PER_SECOND        = 50;
constexpr uint16_t EFFECT_PERIOD_CALLBACK = (1000 / FRAMES_PER_SECOND);
constexpr uint8_t LINE_BUFFER_SIZE = 128;
constexpr uint8_t PARAMETER_SIZE = 16;

// Keep track when the last time we ran the effect state changes
uint32_t effectPeriodStartMillis = 0;

// WiFI Manager
WiFiManager wm;
#define MQTT_SERVER_LENGTH 40
#define MQTT_PORT_LENGTH 5
#define MQTT_USERNAME_LENGTH 18
#define MQTT_PASSWORD_LENGTH 18
WiFiManagerParameter wm_mqtt_server("server", "mqtt server", "", MQTT_SERVER_LENGTH);
WiFiManagerParameter wm_mqtt_port("port", "mqtt port", "", MQTT_PORT_LENGTH);
WiFiManagerParameter wm_mqtt_user("user", "mqtt username", "", MQTT_USERNAME_LENGTH);

const char _customHtml_hidden[] = "type=\"password\"";
WiFiManagerParameter wm_mqtt_password("input", "mqtt password", "", MQTT_PASSWORD_LENGTH, _customHtml_hidden, WFM_LABEL_AFTER);


Properties controllerConfig;
Properties hwConfig;

bool controllerConfigModified = false;
bool hwConfigModified = false;

// CRC value of last update to MQTT
uint16_t lastMeasurementCRC = 0;
uint32_t shouldRestart = 0;        // Indicate that a service requested an restart. Set to millies() of current time and it will restart 5000ms later

///////////////////////////////////////////////////////////////////////////
//  Loading/Saving of Properties
///////////////////////////////////////////////////////////////////////////
bool loadConfig(const char* filename, Properties& properties, bool serial) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        Serial.println("mounted file system");

        if (FileSystemFS.exists(filename)) {
            //file exists, reading and loading
            File configFile = FileSystemFS.open(filename, "r");

            if (configFile) {
                Serial.print(F("Loading config : "));
                Serial.println(filename);
                deserializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
                if (serial) serializeProperties<LINE_BUFFER_SIZE>(Serial, properties);
            }

            configFile.close();
        } else {
            Serial.print(F("File not found: "));
            Serial.println(filename);
        }

        // FileSystemFS.end();
    } else {
        Serial.print(F("Failed to begin FileSystemFS"));
    }

    return ret;
}

/**
 * Store custom oarameter configuration in FileSystemFS
 */
bool saveConfig(const char* filename, Properties& properties, bool serial) {
    bool ret = false;

    if (FileSystemFSBegin()) {
        FileSystemFS.remove(filename);
        File configFile = FileSystemFS.open(filename, "w");

        if (configFile) {
            Serial.print(F("Saving config : "));
            Serial.println(filename);
            serializeProperties<LINE_BUFFER_SIZE>(configFile, properties);
            if (serial)
            serializeProperties<LINE_BUFFER_SIZE>(Serial, properties, false);
            ret = true;
        } else {
            Serial.print(F("Failed to write file"));
            Serial.println(filename);
        }

        configFile.close();
        //    FileSystemFS.end();
    }

    return ret;
}

void publishStatusToMqtt();
void deep_sleep(uint32_t time_us) {
    if (hwConfigModified) {
        saveConfig(CONFIG_FILENAME, hwConfig, true);
    }
    if (controllerConfigModified) {
        saveConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig, false);
    }
    publishStatusToMqtt();
    network_shutdown();
    Serial.print("Good Night, see you in ");
    Serial.print(time_us/1000000);
    Serial.println(" seconds");
    delay(10);
    ESP.deepSleep(time_us);
}

///////////////////////////////////////////////////////////////////////////
//  MQTT
///////////////////////////////////////////////////////////////////////////
/*
* Publish current status
*/
void publishStatusToMqtt() {
    const char format[] = "pump=%i hygro=%d wet=%d dry=%d cycle=%d";
    char buffer[sizeof(format) + 2 + 2 + 2 + 2 -1 + 8]; 

    sprintf(buffer,
            format,
            scripting_context()->m_pump,
            scripting_context()->m_currentValue,
            scripting_context()->m_wetThreshold,
            scripting_context()->m_dryThreshold,
            scripting_context()->m_wateringCycle
           );


    // Quick hack to only update when data actually changed
    uint16_t thisCrc = crc16((uint8_t*)buffer, std::strlen(buffer));

    if (thisCrc != lastMeasurementCRC) {
        network_publishToMQTT("status", buffer);
        lastMeasurementCRC = thisCrc;
    }
}

/**
 * Handle incomming MQTT requests
 */
void handleCmd(const char* topic, const char* p_payload) {
    auto topicPos = topic + strlen(controllerConfig.get("mqttBaseTopic"));
    Serial.print(F("Handle command : "));
    Serial.print(topicPos);
    Serial.print(F(" : "));
    Serial.println(p_payload);

    // Look for a temperature setPoint topic
    char payloadBuffer[LINE_BUFFER_SIZE];
    strncpy(payloadBuffer, p_payload, sizeof(payloadBuffer));

    if (strstr(topicPos, "config") != nullptr) {
        OptParser::get(payloadBuffer, [&](OptValue v) {
            if (strcmp(v.key(), "wet") == 0) {
                hwConfig.put("wetThreshold", PV((int16_t)v));
                hwConfigModified=true;
            } else if (strcmp(v.key(), "dry") == 0) {
                hwConfig.put("dryThreshold", PV((int16_t)v));
                hwConfigModified=true;
            }
        });
    }

    if (strstr(topicPos, "load") != nullptr) {
        scripting_load(payloadBuffer);
    }

    if (strstr(topicPos, "sleep") != nullptr) {
        deep_sleep(atol(payloadBuffer));
    }

    if (strstr(topicPos, "reset") != nullptr) {
        if (strcmp(payloadBuffer, "1") == 0) {
            shouldRestart = millis();
        }
    }
}

/**
 * Initialise MQTT and variables
 */
void setupMQTTCallback() {
    network_mqtt_callback([](char* p_topic, byte * p_payload, uint16_t p_length) {
        char mqttReceiveBuffer[LINE_BUFFER_SIZE];
        // Serial.println(p_topic);

        if (p_length >= sizeof(mqttReceiveBuffer)) {
            return;
        }

        memcpy(mqttReceiveBuffer, p_payload, p_length);
        mqttReceiveBuffer[p_length] = 0;
        handleCmd(p_topic, mqttReceiveBuffer);
    });

}


///////////////////////////////////////////////////////////////////////////
//  Handle script context
///////////////////////////////////////////////////////////////////////////
void handleScriptContext() {
    if (scripting_context() != nullptr) {
        scripting_context()->m_currentValue = analogRead(MOISTA_PIN);
        scripting_context()->m_dryThreshold = (int16_t)hwConfig.get("dryThreshold");
        scripting_context()->m_wetThreshold = (int16_t)hwConfig.get("wetThreshold");
    }

    int8_t handle = scripting_handle();

    switch (handle) {
        case 0:
            digitalWrite(PUMP_PIN, false);
            Serial.println("Script ended");
            scripting_load("/default.txt");
            break;
    
        case 1:
            digitalWrite(PUMP_PIN, scripting_context()->pump());
    }

    if (scripting_context()!=nullptr && scripting_context()->m_deepSleepSec!=0) {
        hwConfigModified = (bool)hwConfig.get("wateringCycle") != scripting_context()->m_wateringCycle;
        hwConfig.put("wateringCycle", PropertyValue(scripting_context()->m_wateringCycle));
        deep_sleep(scripting_context()->m_deepSleepSec * 1000000);
    }

}


///////////////////////////////////////////////////////////////////////////
//  IOHardware
///////////////////////////////////////////////////////////////////////////
void setupIOHardware() {
    // Pump Pin
    pinMode(PUMP_PIN, OUTPUT);    

    // Moisture digital
    pinMode(MOIST_PIN, INPUT);   

}

///////////////////////////////////////////////////////////////////////////
//  Webserver/WIFIManager
///////////////////////////////////////////////////////////////////////////
void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");

    if (std::strlen(wm_mqtt_server.getValue()) > 0) {
        controllerConfig.put("mqttServer", PV(wm_mqtt_server.getValue()));
        controllerConfig.put("mqttPort", PV(std::atoi(wm_mqtt_port.getValue())));
        controllerConfig.put("mqttUsername", PV(wm_mqtt_user.getValue()));
        controllerConfig.put("mqttPassword", PV(wm_mqtt_password.getValue()));
        controllerConfigModified = true;
        // Redirect from MQTT so on the next reconnect we pickup new values
        network_mqtt_disconnect();
        // Send redirect back to param page
        wm.server->sendHeader(F("Location"), F("/param?"), true);
        wm.server->send(302, FPSTR(HTTP_HEAD_CT2), "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
        wm.server->client().stop();
    }
}

/**
 * Setup the wifimanager and configuration page
 */
void setupWifiManager() {
    char port[6];
    snprintf(port, sizeof(port), "%d", (int16_t)controllerConfig.get("mqttPort"));
    wm_mqtt_port.setValue(port, MQTT_PORT_LENGTH);
    wm_mqtt_password.setValue(controllerConfig.get("mqttPassword"), MQTT_PASSWORD_LENGTH);
    wm_mqtt_user.setValue(controllerConfig.get("mqttUsername"), MQTT_USERNAME_LENGTH);
    wm_mqtt_server.setValue(controllerConfig.get("mqttServer"), MQTT_SERVER_LENGTH);

    wm.addParameter(&wm_mqtt_server);
    wm.addParameter(&wm_mqtt_port);
    wm.addParameter(&wm_mqtt_user);
    wm.addParameter(&wm_mqtt_password);

    /////////////////
    // set country
    wm.setClass("invert");
    wm.setCountry("US"); // setting wifi country seems to improve OSX soft ap connectivity, may help others as well

    // Set configuration portal
    wm.setShowStaticFields(false);
    wm.setConfigPortalBlocking(false); // Must be blocking or else AP stays active
    wm.setDebugOutput(false);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setHostname(controllerConfig.get("mqttClientID"));
    std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "param", "sep", "erase", "restart"};
    wm.setMenu(menu);

    wm.startWebPortal();
    wm.autoConnect(controllerConfig.get("mqttClientID"));
}

///////////////////////////////////////////////////////////////////////////
//  SETUP and LOOP
///////////////////////////////////////////////////////////////////////////

void setDefaultConfigurations() {
    // controllerConfig
    char chipHexBuffer[9];
    snprintf(chipHexBuffer, sizeof(chipHexBuffer), "%08X", WIFI_getChipId());

    char mqttClientID[24];
    snprintf(mqttClientID, sizeof(mqttClientID), MQTT_NAME "_%s", chipHexBuffer);

    char mqttBaseTopic[24];
    snprintf(mqttBaseTopic, sizeof(mqttBaseTopic), MQTT_NAME "/%s", chipHexBuffer);

    char mqttLastWillTopic[64];
    snprintf(mqttLastWillTopic, sizeof(mqttLastWillTopic), "%s/%s", mqttBaseTopic, MQTT_LASTWILL_TOPIC);

    controllerConfigModified |= controllerConfig.putNotContains("mqttServer", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttUsername", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPassword", PV(""));
    controllerConfigModified |= controllerConfig.putNotContains("mqttPort", PV(1883));
    controllerConfigModified |= controllerConfig.putNotContains("pauseForOTA", PV(true));

    controllerConfig.put("mqttClientID", PV(mqttClientID));
    controllerConfig.put("mqttBaseTopic", PV(mqttBaseTopic));
    controllerConfig.put("mqttLastWillTopic", PV(mqttLastWillTopic));

    // hwConfig
    hwConfigModified |= hwConfig.putNotContains("dryThreshold", PV(800));
    hwConfigModified |= hwConfig.putNotContains("wetThreshold", PV(600));
    hwConfigModified |= hwConfig.putNotContains("wateringCycle", PV(true));    
}

void setup() {
    // Enable serial port
    Serial.begin(115200);
    delay(500); // SOmething buggy in the ESP8266MOD
    while (!Serial) {
        delay(20);
    }

    // load configurations
    loadConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig, false);
    loadConfig(CONFIG_FILENAME, hwConfig, true);
    setDefaultConfigurations();

    scripting_init();
    scripting_load("/default.txt");

    setupIOHardware();
    network_init();
    setupMQTTCallback();
    setupWifiManager();
    effectPeriodStartMillis = millis();
}

constexpr uint8_t NUMBER_OF_SLOTS = 10;
uint8_t maxSlots = 255;
bool motor = false;
void loop() {
    const uint32_t currentMillis = millis();

    if (currentMillis - effectPeriodStartMillis >= EFFECT_PERIOD_CALLBACK) {
        effectPeriodStartMillis += EFFECT_PERIOD_CALLBACK;
        counter50TimesSec++;

        // once a second publish status to mqtt (if there are changes)
        if (counter50TimesSec % 50 == 0) {
            publishStatusToMqtt();
        }

        // Maintenance stuff
        uint8_t slot50 = 0;

        if (counter50TimesSec % maxSlots == slot50++) {
            network_handle();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (controllerConfigModified) {
                controllerConfigModified = false;
                saveConfig(CONTROLLER_CONFIG_FILENAME, controllerConfig, false);
            }
        } else if (counter50TimesSec % maxSlots == slot50++) {
            handleScriptContext();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (hwConfigModified) {
                hwConfigModified = false;
                saveConfig(CONFIG_FILENAME, hwConfig, true);
            }

        } else if (counter50TimesSec % maxSlots == slot50++) {
            wm.process();
        } else if (counter50TimesSec % maxSlots == slot50++) {
            if (shouldRestart != 0 && (currentMillis - shouldRestart >= 5000)) {
                shouldRestart = 0;
                ESP.restart();
            }
        } else {
            maxSlots = slot50;
        }
    }
}

