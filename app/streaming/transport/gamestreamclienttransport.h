#pragma once

#include "clienttransport.h"

#include <Limelight.h>
#include <QString>

namespace Connectivity {
class IConnectivityAgent;
}

namespace AudioReceiver {
class GameStreamAudioReceiver;
class IAudioReceiver;
}

namespace VideoReceiver {
class GameStreamVideoReceiver;
class IVideoReceiver;
}

namespace ClientTransport {

// Non-owning legacy dependencies remain confined to this adapter. Session owns
// every referenced object for the complete transport lifetime.
struct GameStreamContext {
    Connectivity::IConnectivityAgent* connectivityAgent = nullptr;
    QString* rtspSessionUrl = nullptr;
    QString appVersion;
    QString gfeVersion;
    int serverCodecModeSupport = 0;
    PSTREAM_CONFIGURATION streamConfiguration = nullptr;
    PCONNECTION_LISTENER_CALLBACKS connectionCallbacks = nullptr;
    VideoReceiver::IVideoReceiver* videoReceiver = nullptr;
    AudioReceiver::IAudioReceiver* audioReceiver = nullptr;
    void* renderContext = nullptr;
    int rendererFlags = 0;
    void* audioContext = nullptr;
    int audioFlags = 0;
    // Hermes packet_feedback extension, enabled only when the host negotiated
    // it. Off against every other host, which is every host that would not
    // understand the message.
    bool packetFeedback = false;
};

class GameStreamClientTransport final : public IClientTransport {
public:
    explicit GameStreamClientTransport(GameStreamContext context);

    Mode mode() const noexcept override;
    StartResult start() override;
    void interrupt() noexcept override;
    void setPacketFeedbackEnabled(bool enabled) noexcept override;
    void stop() noexcept override;

private:
    GameStreamContext m_Context;
    VideoReceiver::GameStreamVideoReceiver* m_ActiveVideoReceiver;
    AudioReceiver::GameStreamAudioReceiver* m_ActiveAudioReceiver;
};

} // namespace ClientTransport
