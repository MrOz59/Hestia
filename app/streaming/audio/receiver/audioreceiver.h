#pragma once

#include "streaming/audio/bufferpolicy.h"

#include <cstdint>

namespace AudioReceiver {

enum class Mode : uint8_t {
    GameStream,
};

struct Configuration {
    uint8_t channelCount = 0;
    uint32_t channelMask = 0;

    bool isValid() const noexcept
    {
        return channelCount != 0;
    }
};

struct Capabilities {
    bool directSubmit = false;
    bool slowOpusDecoder = false;
    bool arbitraryPacketDuration = false;
};

class IAudioReceiver {
public:
    virtual ~IAudioReceiver() = default;

    virtual Mode mode() const noexcept = 0;
    virtual Capabilities capabilities() const noexcept = 0;
    virtual bool testConfiguration(
            const Configuration& configuration) = 0;
    // Must be configured before the receiver is activated. The default
    // profile preserves the legacy renderer constants.
    virtual void setBufferingProfile(
            AudioBuffer::Profile profile) noexcept = 0;
    virtual AudioBuffer::Profile
    bufferingProfile() const noexcept = 0;
    virtual void setMuted(bool muted) noexcept = 0;
    virtual bool isMuted() const noexcept = 0;
};

} // namespace AudioReceiver
