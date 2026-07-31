#pragma once

#include "hostprotocol.h"

class NvComputer;

namespace HostProtocol {

class GameStreamHostProtocol final : public IHostProtocol {
public:
    explicit GameStreamHostProtocol(NvComputer* computer);

    Mode mode() const noexcept override;
    bool prepareSession(const SessionRequest& request,
                        QString* sessionId,
                        QMap<QString, uint32_t>* negotiatedExtensions = nullptr) override;
    QString launchSession(const LaunchRequest& request) override;
    bool stopSession(const QString& sessionId) override;

private:
    NvComputer* m_Computer;
};

} // namespace HostProtocol
