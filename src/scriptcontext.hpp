#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
private:
    bool m_pump;
    bool m_wateringCycle;
    bool m_probe;
    uint16_t m_currentValue;
public:
    uint16_t m_dryThreshold;
    uint16_t m_wetThreshold;
    int32_t m_deepSleepSec;
    ScriptContext(const char* script, bool wateringCycle) :
        PlainTextContext512{script},
        m_pump(false),
        m_wateringCycle(wateringCycle),
        m_probe(false),
        m_currentValue(1024),
        m_dryThreshold(800),
        m_wetThreshold(500),
        m_deepSleepSec(0) {
    }

    void currentValue(uint16_t currentValue) {
        m_currentValue = currentValue;

        if (m_currentValue >= m_dryThreshold) {
            m_wateringCycle = true;
        } else if (m_currentValue <= m_wetThreshold) {
            m_wateringCycle = false;
        }
    }

    uint16_t currentValue() {
        return m_currentValue;
    }

    /**
    * Decide if we need to give the pot a little bit of water or not
    * returns true if more water is required
    */
    bool wateringCycle() {
        return m_wateringCycle;
    }

    // returns true when current value is beliw wetThreshold, remember
    // the lower the value the wetter the soil
    bool isBelowWet() {
        return m_currentValue <= m_wetThreshold;
    };
    bool isAboveDry() {
        return m_currentValue >= m_dryThreshold;
    };

    bool pump() {
        return m_currentValue > 1020 ? false : m_pump;
    }

    void pump(bool p) {
        m_pump = p;
    }

    void probe(bool v) {
        m_probe = v;
    }
    bool probe() const {
        return m_probe;
    }
};
