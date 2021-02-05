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
constexpr uint8_t SCRIPT_LINE_SIZE_MAX=32;

static ScriptContext* scriptContext{nullptr};
static ScriptRunner<ScriptContext>* scriptRunner = nullptr;
static char scriptContextFileToLoad[32]; // See note for handleScriptContext()

using namespace rvt::scriptrunner;

bool getBoolValue(const char* value, uint8_t pos) {
    bool rValue = false;
    OptParser::get<SCRIPT_LINE_SIZE_MAX>(value, ',', [&](const OptValue & parsed) {
        if (parsed.pos() == pos) {
            rValue = (bool)parsed;
        }
    });
    return rValue;
}

void scripting_init() {
    std::vector<Command<ScriptContext>*> commands;

    commands.push_back(new Command<ScriptContext> {"serial", [](const char* value, ScriptContext & context) {
        Serial.println(value);
        Serial.flush();
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"pump", [](const char* value, ScriptContext & context) {
        context.m_pump = getBoolValue(value, 0);
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"decideAboveDry", [&](const char* value, ScriptContext & context) {
        if (!context.isAboveDry()) {
            context.jump(value);
        }
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"decideBelowWet", [&](const char* value, ScriptContext & context) {
        if (!context.isBelowWet()) {
            context.jump(value);
        }

        return true;
    }
                                                        });
    
    commands.push_back(new Command<ScriptContext> {"wateringCycle", [&](const char* value, ScriptContext & context) {
        if (!context.wateringCycle()) {
            context.jump(value);
        }
        return true;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"sleepSec", [&](const char* value, ScriptContext & context) {
        scriptContext->m_deepSleepSec = atol(value);
        return false;
    }
                                                        });

    commands.push_back(new Command<ScriptContext> {"load", [&](const char* value, ScriptContext & context) {
        strncpy(scriptContextFileToLoad, value, sizeof(scriptContextFileToLoad));
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
        scriptContext->m_wateringCycle = (bool)hwConfig.get("wateringCycle");
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