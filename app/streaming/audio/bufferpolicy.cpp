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
        result.initialPrebufferMs = 5;
        result.underrunRecoveryStepMs = 5;
        result.maximumPrebufferMs = 20;
        break;
    case Profile::SmoothPlayback:
        minimumDeviceBufferSamples =
                durationSamples(
                    format.sampleRate,
                    15);
        deviceBufferPacketMultiplier = 4;
        result.playbackQueueLimitMs = 80;
        result.upstreamBackpressureLimitMs = 50;
        result.initialPrebufferMs = 20;
        result.underrunRecoveryStepMs = 10;
        result.maximumPrebufferMs = 50;
        break;
    case Profile::Default:
    default:
        // Retain the historical device and queue sizing. The startup reserve
        // below is a separate underrun fix shared by all SDL profiles.
        minimumDeviceBufferSamples = 480;
        deviceBufferPacketMultiplier = 3;
        result.playbackQueueLimitMs = 50;
        result.upstreamBackpressureLimitMs = 30;
        result.initialPrebufferMs = 10;
        result.underrunRecoveryStepMs = 5;
        result.maximumPrebufferMs = 30;
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

uint32_t nextRecoveryPrebufferMs(
        const Policy& policy,
        uint32_t currentPrebufferMs) noexcept
{
    if (policy.maximumPrebufferMs == 0) {
        return 0;
    }

    const uint32_t baseline = std::max(
        currentPrebufferMs,
        policy.initialPrebufferMs);
    if (baseline >= policy.maximumPrebufferMs) {
        return policy.maximumPrebufferMs;
    }

    return std::min(
        policy.maximumPrebufferMs,
        baseline + policy.underrunRecoveryStepMs);
}

uint64_t drainPlaybackBufferUs(
        uint64_t bufferedAudioUs,
        uint64_t elapsedPlaybackUs,
        uint64_t queuedAudioLowerBoundUs) noexcept
{
    const uint64_t remaining =
            elapsedPlaybackUs >= bufferedAudioUs ?
                0 :
                bufferedAudioUs - elapsedPlaybackUs;
    return std::max(
        remaining,
        queuedAudioLowerBoundUs);
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
