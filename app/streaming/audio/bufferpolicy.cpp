#include "bufferpolicy.h"

#include <algorithm>

namespace AudioBuffer {

namespace {

uint32_t durationSamples(
        uint32_t sampleRate,
        uint32_t durationMs) noexcept
{
    return static_cast<uint32_t>(
        (static_cast<uint64_t>(sampleRate) *
         durationMs +
         999) /
        1000);
}

Policy sdlPolicy(
        Profile profile,
        const StreamFormat& format) noexcept
{
    uint32_t minimumDeviceBufferSamples;
    uint32_t deviceBufferPacketMultiplier;
    Policy result;

    switch (profile) {
    case Profile::LowLatency:
        minimumDeviceBufferSamples =
                durationSamples(
                    format.sampleRate,
                    10);
        deviceBufferPacketMultiplier = 2;
        result.playbackQueueLimitMs = 30;
        result.upstreamBackpressureLimitMs = 20;
        break;
    case Profile::SmoothPlayback:
        minimumDeviceBufferSamples =
                durationSamples(
                    format.sampleRate,
                    15);
        deviceBufferPacketMultiplier = 4;
        result.playbackQueueLimitMs = 80;
        result.upstreamBackpressureLimitMs = 50;
        break;
    case Profile::Default:
    default:
        // These are the historical renderer constants. Keep this branch
        // behaviorally identical for existing users.
        minimumDeviceBufferSamples = 480;
        deviceBufferPacketMultiplier = 3;
        result.playbackQueueLimitMs = 50;
        result.upstreamBackpressureLimitMs = 30;
        break;
    }

    result.deviceBufferSamples = std::max(
        minimumDeviceBufferSamples,
        format.samplesPerFrame *
            deviceBufferPacketMultiplier);
    return result;
}

Policy slAudioPolicy(
        Profile profile,
        const StreamFormat& format) noexcept
{
    uint32_t queueMsPerStereoPair;
    switch (profile) {
    case Profile::LowLatency:
        queueMsPerStereoPair = 20;
        break;
    case Profile::SmoothPlayback:
        queueMsPerStereoPair = 60;
        break;
    case Profile::Default:
    default:
        // Preserve SLAudio's historical 40 ms per stereo pair.
        queueMsPerStereoPair = 40;
        break;
    }

    Policy result;
    result.playbackQueueLimitMs =
            queueMsPerStereoPair *
            format.channelCount /
            2;
    result.upstreamBackpressureLimitMs =
            result.playbackQueueLimitMs;
    return result;
}

} // namespace

Policy calculate(
        Profile profile,
        Backend backend,
        const StreamFormat& format) noexcept
{
    if (!format.isValid()) {
        return {};
    }

    switch (backend) {
    case Backend::Sdl:
        return sdlPolicy(profile, format);
    case Backend::SlAudio:
        return slAudioPolicy(profile, format);
    }
    return {};
}

const char* profileName(Profile profile) noexcept
{
    switch (profile) {
    case Profile::Default:
        return "Default";
    case Profile::LowLatency:
        return "Low latency";
    case Profile::SmoothPlayback:
        return "Smooth playback";
    }
    return "Default";
}

} // namespace AudioBuffer
