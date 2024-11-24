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
#define DEBOUNCE_DELAY 50   // Debounce time in milliseconds
#define LONG_PRESS_DELAY 1000 // Long-press time in milliseconds

// Player setup
PcmDecoder pcmDecoder;
Player player(pcmDecoder);

// Interrupt handler
extern "C" void I2S_IRQHandler(void) {
//    player.handleInterrupt();
}

// Volume and button variables
float volume = 1.0;
unsigned long lastDebounceTime[3] = {0, 0, 0};  // Last debounce times for each button
bool lastButtonState[3] = {HIGH, HIGH, HIGH};   // Last stable button states
bool currentButtonState[3] = {HIGH, HIGH, HIGH}; // Current button states
unsigned long pressStartTime[3] = {0, 0, 0};    // Button press start times
bool longPressHandled[3] = {false, false, false}; // Track if long press was handled

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
    if (sourceIdx >= sizeof(sources)/sizeof(sources[0])) {
        sourceIdx = 0;
    }
    pcmDecoder.setSource(sources[sourceIdx].ptr, sources[sourceIdx].len);
}

void handleShortPress(int buttonIndex) {
    switch (buttonIndex) {
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
        case 2: // Play button
            changeSource();
            break;
    }
}

void handleLongPress(int buttonIndex) {
    switch (buttonIndex) {
        case 0: // Volume Up
            volume = 1.0; // Max volume
            player.setVolume(volume);
            break;
        case 1: // Volume Down
            volume = 0.0; // Mute
            player.setVolume(volume);
            break;
        case 2: // Play button
            if (player.isPlaying()) {
                player.stop();
            } else {
                player.startPlayback();
            }
            break;
    }
}

void updateButtonState(int buttonIndex, int buttonPin, int ledPin) {
    int reading = digitalRead(buttonPin);

    // Check for state change and debounce
    if (reading != lastButtonState[buttonIndex]) {
        lastDebounceTime[buttonIndex] = millis(); // Reset debounce timer
    }

    if ((millis() - lastDebounceTime[buttonIndex]) > DEBOUNCE_DELAY) {
        if (reading != currentButtonState[buttonIndex]) {
            currentButtonState[buttonIndex] = reading;

            if (currentButtonState[buttonIndex] == LOW) { // Button pressed
                pressStartTime[buttonIndex] = millis();
                longPressHandled[buttonIndex] = false;
            } else if (currentButtonState[buttonIndex] == HIGH) { // Button released
                if (!longPressHandled[buttonIndex]) {
                    handleShortPress(buttonIndex);
                }
            }
        }

        // Handle long press
        if (currentButtonState[buttonIndex] == LOW && 
            (millis() - pressStartTime[buttonIndex] >= LONG_PRESS_DELAY) && 
            !longPressHandled[buttonIndex]) {
            longPressHandled[buttonIndex] = true;
            handleLongPress(buttonIndex);
        }
    }

    // Update LED states to reflect button state
    digitalWrite(ledPin, currentButtonState[buttonIndex]);
    lastButtonState[buttonIndex] = reading;
}

void handleButtons() {
    int buttonPins[3] = {PIN_VOL_UP, PIN_VOL_DOWN, PIN_PLAY};
    int ledPins[3] = {LED_RED, LED_GREEN, LED_BLUE};

    for (int i = 0; i < 3; i++) {
        updateButtonState(i, buttonPins[i], ledPins[i]);
    }
}

void loop() {
    handleButtons();
    player.handleInterrupt();
}
