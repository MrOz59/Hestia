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
};

} // namespace ClientTransport
