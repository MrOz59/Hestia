#pragma once

#include <cstdint>

namespace AudioBuffer {

enum class Profile : uint8_t {
    Default,
    LowLatency,
    SmoothPlayback,
};

enum class Backend : uint8_t {
    Sdl,
    SlAudio,
};

struct StreamFormat {
    uint32_t sampleRate = 0;
    uint32_t samplesPerFrame = 0;
    uint8_t channelCount = 0;

    bool isValid() const noexcept
    {
        return sampleRate != 0 &&
                samplesPerFrame != 0 &&
                channelCount != 0;
    }
};

struct Policy {
    uint32_t deviceBufferSamples = 0;
    uint32_t playbackQueueLimitMs = 0;
    uint32_t upstreamBackpressureLimitMs = 0;
    uint32_t initialPrebufferMs = 0;
    uint32_t underrunRecoveryStepMs = 0;
    uint32_t maximumPrebufferMs = 0;
};

Policy calculate(
        Profile profile,
        Backend backend,
        const StreamFormat& format) noexcept;

uint32_t nextRecoveryPrebufferMs(
        const Policy& policy,
        uint32_t currentPrebufferMs) noexcept;

uint64_t drainPlaybackBufferUs(
        uint64_t bufferedAudioUs,
        uint64_t elapsedPlaybackUs,
        uint64_t queuedAudioLowerBoundUs) noexcept;

const char* profileName(Profile profile) noexcept;

} // namespace AudioBuffer
