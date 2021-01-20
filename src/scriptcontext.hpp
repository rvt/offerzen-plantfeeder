#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
public:
    bool m_pump;
    uint16_t m_lastValue;

    uint16_t m_maxThreshold;
    uint16_t m_minThreshold;
    uint16_t m_currentValue;
    ScriptContext(const char* script) : 
        PlainTextContext512{script}, 
        m_pump(false), 
        m_lastValue(0),
        m_maxThreshold(0),
        m_minThreshold(0),
        m_currentValue(0) {
    }

    bool decide() {
        // We come from < min to > max, we should add water
        if ( m_currentValue >= m_maxThreshold && m_lastValue <= m_minThreshold) {
            m_lastValue = m_currentValue;
            return true;
        }
        // We come from > max to > max, we should add more water
        if ( m_currentValue >= m_maxThreshold && m_lastValue >= m_maxThreshold) {
            m_lastValue = m_currentValue;
            return true;
        }

        // We come from > max to < min, we store last value
        if ( m_currentValue <= m_minThreshold && m_lastValue >= m_maxThreshold) {
            m_lastValue = m_currentValue;
        }
        return false;
    }
};
