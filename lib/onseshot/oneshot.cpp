#include "oneshot.hpp"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" uint32_t millis();
#endif


OneShot::OneShot(
    const uint32_t p_delayTimeMS,
    CallbackFunction p_startCallback,
    CallbackFunction p_endCallback,
    ModifiedFunction p_modified) :
    m_delayTimeMS{p_delayTimeMS},
    m_startCallback{p_startCallback},
    m_endCallback{p_endCallback},
    m_modified{p_modified},
    m_lastStatus{false},
    m_oneShotStatus{WAIT_TRIGGER},
    m_startTime{millis()} {
}

void OneShot::handle() {
    const bool status = m_modified();

    if (status && m_oneShotStatus == WAIT_TRIGGER) {
        triggerStart();
    }

    const uint32_t currentMillis = millis();

    if (m_oneShotStatus == STARTED && (currentMillis - m_startTime >= m_delayTimeMS)) {
        triggerEnd();
    }
}

void OneShot::triggerStart() {
    m_oneShotStatus = STARTED;
    m_startTime = millis();
    m_startCallback();
}

void OneShot::triggerEnd() {
    m_oneShotStatus = ENDED;
    m_endCallback();
}

bool OneShot::lastStatus() const {
    return m_lastStatus;
}

void OneShot::reset() {
    if (m_oneShotStatus == ENDED) {
        m_oneShotStatus = STARTED;
        m_startTime = millis();
    }
}

void OneShot::start() {
    if (m_oneShotStatus == ENDED) {
        m_oneShotStatus = WAIT_TRIGGER;
    }
}

void OneShot::stop() {
    m_oneShotStatus == ENDED;
}

void OneShot::hold() {
    if (m_oneShotStatus == STARTED) {
        m_startTime = millis();
    }
}
