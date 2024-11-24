#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial
#include "data.h"
#include "data2.h"
#include "player.h"
#include "pcm_decoder.h"

// Pin definitions
#define PIN_SCK 45   // 8
#define PIN_LRCK 46  // 9
#define PIN_SDOUT 44 // 7
#define PIN_VOL_UP D0
#define PIN_VOL_DOWN D1
#define PIN_PLAY D2
#define DEBOUNCE_DELAY 50  // Debounce time in milliseconds

// Player setup
PcmDecoder pcmDecoder;
Player player(pcmDecoder);

// Interrupt handler
extern "C" void I2S_IRQHandler(void) {
//    player.handleInterrupt();
}

// Volume and button variables
float volume = 1.0;
unsigned long lastDebounceTime[3] = {0, 0, 0}; // Last debounce times for each button
bool lastButtonState[3] = {HIGH, HIGH, HIGH};  // Last stable button states
bool currentButtonState[3] = {HIGH, HIGH, HIGH}; // Current button states
struct Sources {
  const int16_t * ptr;
  size_t len;
} sources[] = {{audio_data, sizeof(audio_data)/sizeof(audio_data[0]) }, {audio_data2, sizeof(audio_data2)/sizeof(audio_data2[0])}};

int sourceIdx = 0;

void setup() {
    pinMode(PIN_VOL_UP, INPUT_PULLUP);
    pinMode(PIN_VOL_DOWN, INPUT_PULLUP);
    pinMode(PIN_PLAY, INPUT_PULLUP);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    pcmDecoder.setSource(sources[0].ptr, sources[0].len);
    player.initI2S(PIN_SCK, PIN_LRCK, PIN_SDOUT);
    player.startPlayback();
}

void changeSource() {
  sourceIdx++;
  if(sourceIdx>=sizeof(sources)/sizeof(sources[0])) {
    sourceIdx=0;
  }
  pcmDecoder.setSource(sources[sourceIdx].ptr, sources[sourceIdx].len);
}

void handleButtons() {
    int buttonPins[3] = {PIN_VOL_UP, PIN_VOL_DOWN, PIN_PLAY};
    int ledPins[3] = {LED_RED, LED_GREEN, LED_BLUE};

    for (int i = 0; i < 3; i++) {
        int reading = digitalRead(buttonPins[i]);

        // Check for state change and debounce
        if (reading != lastButtonState[i]) {
            lastDebounceTime[i] = millis(); // Reset debounce timer
        }

        if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
            // If the state has stabilized, update the current state
            if (reading != currentButtonState[i]) {
                currentButtonState[i] = reading;

                // Handle button press actions
                if (currentButtonState[i] == LOW) { // Button pressed
                    switch (i) {
                        case 0: // Volume Up
                            volume += 0.1;
                            if (volume > 1.0) volume = 1.0;
                            player.setVolume(volume);
                            break;
                        case 1: // Volume Down
                            volume -= 0.1;
                            if (volume < 0.0) volume = 0.0;
                            player.setVolume(volume);
                            break;
                        case 2: 
                            changeSource();
                            break;
                    }
                }
            }
        }

        // Update LED states to reflect button state
        digitalWrite(ledPins[i], currentButtonState[i]);

        // Save the last reading for the next loop
        lastButtonState[i] = reading;
    }
}

void loop() {
    handleButtons();
    player.handleInterrupt();
}
