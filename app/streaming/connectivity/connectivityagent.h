#pragma once

#include <cstdint>

#include <QString>

namespace Connectivity {

enum class PathType : uint8_t {
    Unknown,
    DirectLan,
    Vpn,
};

struct SelectedPath {
    QString address;
    PathType type = PathType::Unknown;

    bool isUsable() const noexcept
    {
        return !address.isEmpty();
    }
};

class IConnectivityAgent {
public:
    virtual ~IConnectivityAgent() = default;

    // The selected path must remain stable for the lifetime of the current
    // connection attempt. Future path migration will use a separate contract.
    virtual SelectedPath selectPath() = 0;
};

} // namespace Connectivity
