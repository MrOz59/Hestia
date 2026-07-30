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
};

Policy calculate(
        Profile profile,
        Backend backend,
        const StreamFormat& format) noexcept;

const char* profileName(Profile profile) noexcept;

} // namespace AudioBuffer
