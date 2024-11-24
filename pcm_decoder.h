#ifndef PCM_DECODER
#define PCM_DECODER

#include "decoder.h"
#include <stdint.h>

class PcmDecoder: public Decoder {
public:
    PcmDecoder()
        : mAudioData(nullptr), mDataSize(0), mOffset(0) {}

    void fillBuffer(int16_t* buffer) override {
        if(mAudioData==nullptr || mDataSize<CHUNK_SIZE) {
          return;
        }
        memcpy(buffer, &mAudioData[mOffset], CHUNK_SIZE * sizeof(int16_t));
        mOffset += CHUNK_SIZE;

        if (mOffset + CHUNK_SIZE >= mDataSize) {
            mOffset = 0; // Rewind to the beginning
        }
    }

    void rewind() override {
        mOffset = 0;
    }

    void setSource(const int16_t* audioData, size_t dataSize) {
      mAudioData = audioData;
      mDataSize = dataSize;
      mOffset = 0;
    }

private:
    const int16_t* mAudioData;
    size_t mDataSize;
    size_t mOffset;
};

#endif
