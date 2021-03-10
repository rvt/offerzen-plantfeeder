#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
private:
    bool m_pump;
    bool m_wateringCycle;
    bool m_probe;
    bool m_measuring;
    uint16_t m_currentValue;
    uint16_t m_dryThreshold;
    uint16_t m_wetThreshold;
public:
    int32_t m_deepSleepSec;
    ScriptContext(const char* script, bool wateringCycle, int16_t dryThreshold, int16_t wetThreshold) :
        PlainTextContext512{script},
        m_pump(false),
        m_wateringCycle(wateringCycle),
        m_probe(false),
        m_measuring(false),
        m_currentValue(0),
        m_dryThreshold((uint16_t)dryThreshold),
        m_wetThreshold((uint16_t)wetThreshold),
        m_deepSleepSec(0) {
    }

    void currentValue(uint16_t currentValue) {

        m_currentValue = currentValue;

        if (!validMeasurement()) return;

        if (m_currentValue >= m_dryThreshold) {
            m_wateringCycle = true;
        } else if (m_currentValue <= m_wetThreshold) {
            m_wateringCycle = false;
        }
    }

    uint16_t currentValue() const {
        return m_currentValue;
    }

    /**
    * Decide if we need to give the pot a little bit of water or not
    * returns true if more water is required
    */
    bool wateringCycle() const {
        return m_wateringCycle;
    }

    // returns true when current value is beliw wetThreshold, remember
    // the lower the value the wetter the soil
    bool isBelowWet() const {
        return m_currentValue <= m_wetThreshold && validMeasurement();
    };
    bool isAboveDry() const {
        return m_currentValue >= m_dryThreshold && validMeasurement();
    };

    bool validMeasurement() const {
        return m_currentValue > 5 && m_currentValue<1020;
    }

    bool pump() const {
        return validMeasurement() ? m_pump : false;
    }

    void pump(bool p) {
        m_pump = p;
    }

    bool probe() const {
        return m_probe;
    }

    void probe(bool p) {
        m_probe = p;
    }

    bool measuring() const {
        return m_measuring;
    }

    void measuring(bool v) {
        m_measuring = v;
    }
    
    uint16_t dryThreshold() const {
        return m_dryThreshold;
    }

    uint16_t wetThreshold() const {
        return m_wetThreshold;
    }
};
