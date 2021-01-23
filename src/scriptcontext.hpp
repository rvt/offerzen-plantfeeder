#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
public:
    bool m_pump;
    bool m_moreWaterRequired;
    uint16_t m_dryThreshold;
    uint16_t m_wetThreshold;
    uint16_t m_currentValue;
    ScriptContext(const char* script) : 
        PlainTextContext512{script}, 
        m_pump(false),
        m_moreWaterRequired(true),
        m_dryThreshold(800),
        m_wetThreshold(500),
        m_currentValue(1024) {
    }

    /**
    * Decide if we need to give the pot a little bit of water or not
    * returns true if more water is required
    */
    bool requireMoreWater() {
        if ( m_currentValue >= m_dryThreshold) {
            m_moreWaterRequired=true;
        } else if ( m_currentValue <= m_wetThreshold) {
            m_moreWaterRequired=false;
        }
        return m_moreWaterRequired;
    }

    bool isBelowWet() {
        return m_currentValue <= m_wetThreshold;
    };

    bool pump() {
        return m_currentValue > 1020?false:m_pump;
    }
};
