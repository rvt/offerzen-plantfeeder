#pragma once

#include <scriptrunner.hpp>

using namespace rvt::scriptrunner;
typedef PlainTextContext<512> PlainTextContext512;

class ScriptContext : public PlainTextContext512 {
public:
    bool m_pump;
    bool m_hysteresisLoop;
    uint16_t m_maxThreshold;
    uint16_t m_minThreshold;
    uint16_t m_currentValue;
    ScriptContext(const char* script) : 
        PlainTextContext512{script}, 
        m_pump(false),
        m_hysteresisLoop(true),
        m_maxThreshold(800),
        m_minThreshold(500),
        m_currentValue(1024) {
    }

    /**
    Decide if we need to give the pot a little bit of water or not
    */
    bool decide() {
        if ( m_currentValue >= m_maxThreshold) {
            m_hysteresisLoop=true;
        } else if ( m_currentValue <= m_minThreshold) {
            m_hysteresisLoop=false;
        }
        return m_hysteresisLoop;
    }

    bool pump() {
        return m_currentValue > 1020?false:m_pump;
    }
};
