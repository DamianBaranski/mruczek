#ifndef PLAYER
#define PLAYER
#include "decoder.h"
#include <nrf_i2s.h>

#define CHUNK_SIZE 1024

class Player {
public:
    Player(Decoder& decoder)
        : mDecoder(decoder), mBufferToggle(true), mIsPlaying(false), mVolume(1.0) {}

    void initI2S(uint32_t sck_pin, uint32_t lrck_pin, uint32_t sdout_pin) {
        NRF_I2S->CONFIG.TXEN = (I2S_CONFIG_TXEN_TXEN_ENABLE << I2S_CONFIG_TXEN_TXEN_Pos);

        nrf_i2s_configure(NRF_I2S, NRF_I2S_MODE_MASTER, NRF_I2S_FORMAT_I2S,
                          NRF_I2S_ALIGN_LEFT, NRF_I2S_SWIDTH_16BIT, NRF_I2S_CHANNELS_LEFT,
                          NRF_I2S_MCK_32MDIV63, NRF_I2S_RATIO_64X);
        nrf_i2s_pins_set(NRF_I2S, sck_pin, lrck_pin, 0, sdout_pin, 0);
        nrf_i2s_enable(NRF_I2S);
        nrf_i2s_int_enable(NRF_I2S, I2S_INTENSET_TXPTRUPD_Enabled);
        //NVIC_EnableIRQ(I2S_IRQn);
    }

    void startPlayback() {
        if (mIsPlaying) return; // Already playing

        fillBuffer(mAudioBuffer1);
        fillBuffer(mAudioBuffer2);

        nrf_i2s_transfer_set(NRF_I2S, CHUNK_SIZE / 2, nullptr, (uint32_t*)mAudioBuffer1);
        nrf_i2s_task_trigger(NRF_I2S, NRF_I2S_TASK_START);

        mIsPlaying = true;
    }

    void pause() {
        if (!mIsPlaying) return;

        nrf_i2s_task_trigger(NRF_I2S, NRF_I2S_TASK_STOP); // Pause the I2S task
        mIsPlaying = false;
    }

    void stop() {
        pause();         // Stop playback
        mDecoder.rewind(); // Reset the decoder to the start
    }

    void setVolume(float volume) {
        mVolume = volume;
    }

    bool isPlaying() {
      return mIsPlaying;
    }

    void handleInterrupt() {
        if (!mIsPlaying) return; // Do nothing if paused

        if (NRF_I2S->EVENTS_TXPTRUPD) {
            if (mBufferToggle) {
                NRF_I2S->TXD.PTR = (uint32_t)mAudioBuffer2;
                NRF_I2S->EVENTS_TXPTRUPD = 0;
                fillBuffer(mAudioBuffer2);
            } else {
                NRF_I2S->TXD.PTR = (uint32_t)mAudioBuffer1;
                NRF_I2S->EVENTS_TXPTRUPD = 0;
                fillBuffer(mAudioBuffer1);
            }
            mBufferToggle = !mBufferToggle;
        }
    }

private:
    void fillBuffer(int16_t* buffer) {
      mDecoder.fillBuffer(buffer);
      for(size_t idx=0; idx<CHUNK_SIZE; idx++) {
        buffer[idx]*=mVolume;
      }
    }

    Decoder& mDecoder;
    int16_t mAudioBuffer1[CHUNK_SIZE];
    int16_t mAudioBuffer2[CHUNK_SIZE];
    volatile bool mBufferToggle;
    bool mIsPlaying;
    float mVolume;
};

#endif
