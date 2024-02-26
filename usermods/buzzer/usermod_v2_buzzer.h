#pragma once

#include "wled.h"
#include "Arduino.h"

#include <deque>

#define USERMOD_ID_BUZZER 900
#ifndef USERMOD_BUZZER_PIN
#define USERMOD_BUZZER_PIN GPIO_NUM_13
#endif

// The ESP32 has 16 channels which can generate 16 independent waveforms
// We'll just choose PWM channel 0 here
const int TONE_PWM_CHANNEL = 0;

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 *
 * Using a usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding #include "usermod_filename.h" in the top and registerUsermod(new MyUsermodClass()) in the bottom of usermods_list.cpp
 */

class BuzzerUsermod : public Usermod {
  private:
    bool enabled = false;
    unsigned long lastTime_ = 0;
    unsigned long delay_ = 0;
    bool _pinAttached = false;
    std::deque<std::pair<note_t, unsigned long>> sequence_ {};
  public:

    /**
     * Enable/Disable the usermod
     */
    inline void enable(bool enable) { enabled = enable; }

    /**
     * Get usermod enabled/disabled state
     */
    inline bool isEnabled() { return enabled; }

    void setup() {
      // Beep on startup
      sequence_.push_back({ note_t::NOTE_D, 100 });
      sequence_.push_back({ note_t::NOTE_Eb, 100 });
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      // beep on WiFi
      sequence_.push_back({ note_t::NOTE_A, 150 });
      sequence_.push_back({ note_t::NOTE_D, 80 });
    }

    /**
     * handleButton() can be used to override default button behaviour. Returning true
     * will prevent button working in a default way.
     */
    bool handleButton(uint8_t b) {
      yield();
      // ignore certain button types as they may have other consequences
      if (!enabled
       || buttonType[b] == BTN_TYPE_NONE
       || buttonType[b] == BTN_TYPE_RESERVED
       || buttonType[b] == BTN_TYPE_PIR_SENSOR
       || buttonType[b] == BTN_TYPE_ANALOG
       || buttonType[b] == BTN_TYPE_ANALOG_INVERTED) {
        return false;
      }

      bool handled = false;

      // do your button handling here
      //sequence_.push_back({ note_t::NOTE_G, 50 });

      return handled;
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     */
    void loop() {
      if (millis() - lastTime_ <= delay_) {
        return; // Wait until delay has elapsed
      } else {
        if (_pinAttached) {
          // turn off tone, write out a PWM waveform with 0 frequency
          ledcDetachPin(USERMOD_BUZZER_PIN);
          _pinAttached = false;
        }
      }
      if (sequence_.size() < 1) return; // Wait until there is a sequence

      auto event = sequence_.front();
      sequence_.pop_front();

      if (!_pinAttached) {
        ledcAttachPin(USERMOD_BUZZER_PIN, TONE_PWM_CHANNEL);
        _pinAttached = true;
      }
      uint8_t octave = 4;
      ledcWriteNote(TONE_PWM_CHANNEL, event.first, octave);

      delay_ = event.second;

      lastTime_ = millis();
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_BUZZER;
    }
};