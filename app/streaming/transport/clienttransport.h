#pragma once

#include <cstdint>

namespace ClientTransport {

enum class Mode : uint8_t {
    GameStream,
};

struct StartResult {
    bool started = false;
    int errorCode = 0;

    explicit operator bool() const noexcept
    {
        return started;
    }
};

class IClientTransport {
public:
    virtual ~IClientTransport() = default;

    virtual Mode mode() const noexcept = 0;
    virtual StartResult start() = 0;
    virtual void interrupt() noexcept = 0;
    virtual void stop() noexcept = 0;

    // Turn on a Hermes protocol extension the host negotiated. Called before
    // start(); a transport that cannot speak the extension ignores it, which
    // is the same outcome as never negotiating it.
    virtual void setPacketFeedbackEnabled(bool) noexcept
    {
    }
};

} // namespace ClientTransport
