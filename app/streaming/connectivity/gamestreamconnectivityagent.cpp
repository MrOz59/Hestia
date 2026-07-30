#include "gamestreamconnectivityagent.h"

#include "backend/nvcomputer.h"

#include <QtGlobal>

namespace Connectivity {

GameStreamConnectivityAgent::GameStreamConnectivityAgent(
        NvComputer* computer)
    : m_Computer(computer)
{
    Q_ASSERT(m_Computer != nullptr);
}

SelectedPath GameStreamConnectivityAgent::selectPath()
{
    if (m_SelectedPath.has_value()) {
        return *m_SelectedPath;
    }

    SelectedPath selectedPath;
    selectedPath.address = m_Computer->activeAddress.address();

    switch (m_Computer->getActiveAddressReachability()) {
    case NvComputer::RI_LAN:
        selectedPath.type = PathType::DirectLan;
        break;
    case NvComputer::RI_VPN:
        selectedPath.type = PathType::Vpn;
        break;
    case NvComputer::RI_UNKNOWN:
        selectedPath.type = PathType::Unknown;
        break;
    }

    m_SelectedPath = selectedPath;
    return selectedPath;
}

} // namespace Connectivity
