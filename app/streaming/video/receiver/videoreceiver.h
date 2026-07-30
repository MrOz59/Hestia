#pragma once

#include <cstdint>

namespace VideoReceiver {

enum class Mode : uint8_t {
    GameStream,
};

struct Capabilities {
    bool directSubmit = false;
    bool pullRenderer = false;
    bool referenceFrameInvalidationH264 = false;
    bool referenceFrameInvalidationHevc = false;
    bool referenceFrameInvalidationAv1 = false;
    uint8_t slicesPerFrame = 0;
};

class IVideoReceiver {
public:
    virtual ~IVideoReceiver() = default;

    virtual Mode mode() const noexcept = 0;
    virtual Capabilities capabilities() const noexcept = 0;
    virtual void configure(Capabilities capabilities) noexcept = 0;
};

} // namespace VideoReceiver
