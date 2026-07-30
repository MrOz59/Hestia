#pragma once

#include "connectivityagent.h"

#include <optional>

class NvComputer;

namespace Connectivity {

class GameStreamConnectivityAgent final : public IConnectivityAgent {
public:
    explicit GameStreamConnectivityAgent(NvComputer* computer);

    SelectedPath selectPath() override;

private:
    NvComputer* m_Computer;
    std::optional<SelectedPath> m_SelectedPath;
};

} // namespace Connectivity
