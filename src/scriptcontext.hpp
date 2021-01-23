#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
public:
    bool m_pump;

    uint16_t m_maxThreshold;
    uint16_t m_minThreshold;
    uint16_t m_currentValue;
    bool m_watering;
    ScriptContext(const char* script) : 
        PlainTextContext512{script}, 
        m_pump(false),
        m_maxThreshold(0),
        m_minThreshold(0),
        m_currentValue(1024)
    {
    }

    bool decide() {
        // Protection against loose connections
        if (m_currentValue > 1020) {
            m_pump=false;
        }
        // Set watering mode
        else if ( m_currentValue >= m_maxThreshold) {
            m_pump=true;
        }
        // remove watering mode
        else if ( m_currentValue <= m_minThreshold) {
            m_pump=false;
        }
        
        return m_pump;
    }
};
