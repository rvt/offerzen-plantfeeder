#include <vector>
#include <Arduino.h>
#include "scripting.hpp"
#include <scriptrunner.hpp>
#include "scriptcontext.hpp"
#include <propertyutils.hpp>

#include <FS.h>
#define FileSystemFS SPIFFS
#define FileSystemFSBegin() SPIFFS.begin()

/*********************
 *      EXTERNALS
 *********************/
extern Properties hwConfig;
extern bool hwConfigModified;
extern void deep_sleep(uint32_t time);

/*********************
 *      Variables
 *********************/

static ScriptContext* scriptContext{nullptr};
static ScriptRunner<ScriptContext>* scriptRunner = nullptr;
static char scriptContextFileToLoad[32]; // See note for handleScriptContext()

using namespace rvt::scriptrunner;

void scripting_init() {
    std::vector<Command<ScriptContext>*> commands;

    commands.push_back(new Command<ScriptContext> {"serial", [](const OptValue & value, ScriptContext & context) {
        Serial.println((char*)value);
        Serial.flush();
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"pump", [](const OptValue & value, ScriptContext & context) {
        context.m_pump = (bool)value;
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"decideDry", [&](const OptValue & value, ScriptContext & context) {
        return context.requireMoreWater();
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"decideBelowWet", [&](const OptValue & value, ScriptContext & context) {
        if (context.isBelowWet()) {
                context.jump((char*)value);
        }

        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"decideDryOrWet", [&](const OptValue & value, ScriptContext & context) {
        char buffer[16];
        strncpy(buffer, (char*)value, sizeof(buffer));
        const char* jmpLocationDry;
        const char* jmpLocationWet;
        OptParser::get(buffer, ',', [&](const OptValue & value) {
            if (value.pos() == 0) {
                jmpLocationDry = value.key();
            } else if (value.pos() == 1) {
                jmpLocationWet = value.key();
            }
        });

        context.jump(context.requireMoreWater()?jmpLocationDry:jmpLocationWet);

        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"sleepSec", [&](const OptValue & value, ScriptContext & context) {
        uint32_t time = (int32_t)atol((char*)value);
        hwConfigModified = (bool)hwConfig.get("hysteresisLoop") != scriptContext->m_moreWaterRequired;
        hwConfig.put("hysteresisLoop", PropertyValue(scriptContext->m_moreWaterRequired));
        deep_sleep((int32_t)atol((char*)value) * 1000000);
        return false;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"load", [&](const OptValue & value, ScriptContext & context) {
        strncpy(scriptContextFileToLoad, (char*)value, sizeof(scriptContextFileToLoad));
        return true;
    }
                                                        });

    scriptRunner = new ScriptRunner<ScriptContext> {commands};
}

void load_script() {
    char buffer[1024];
    uint16_t pos = 0;

    if (FileSystemFS.exists(scriptContextFileToLoad)) {
        //file exists, reading and loading
        File configFile = FileSystemFS.open(scriptContextFileToLoad, "r");

        while (configFile.available()) {
            buffer[pos++] = char(configFile.read());
        }

        buffer[pos++] = 0;
        configFile.close();
        Serial.print(F("Loaded : "));
        Serial.println(scriptContextFileToLoad);
        delete scriptContext;
        scriptContext = new ScriptContext{buffer};
        scriptContext->m_moreWaterRequired = (bool)hwConfig.get("hysteresisLoop");
    } else {
        Serial.print(F("File not found: "));
        Serial.println(scriptContextFileToLoad);
    }

    scriptContextFileToLoad[0] = 0;
}

void scripting_load(const char* value) {
    strncpy(scriptContextFileToLoad, value, sizeof(scriptContextFileToLoad));
}

int8_t scripting_handle() {
    if (strlen(scriptContextFileToLoad) != 0) {
        load_script();
        return 1;
    } else if (scriptContext == nullptr) { 
        return -1;
    } else if (scriptRunner->handle(*scriptContext)) {
        return 1;
    } else {
        delete scriptContext;
        scriptContext = nullptr;
        return 0;
    }
}

ScriptContext* scripting_context() {
    return scriptContext;
}