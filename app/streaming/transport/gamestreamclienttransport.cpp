#include "gamestreamclienttransport.h"

#include "streaming/connectivity/connectivityagent.h"
#include "streaming/audio/receiver/gamestreamaudioreceiver.h"
#include "streaming/video/receiver/gamestreamvideoreceiver.h"

#include <QByteArray>
#include <QtGlobal>

namespace ClientTransport {

GameStreamClientTransport::GameStreamClientTransport(
        GameStreamContext context)
    : m_Context(context),
      m_ActiveVideoReceiver(nullptr),
      m_ActiveAudioReceiver(nullptr)
{
    Q_ASSERT(m_Context.connectivityAgent != nullptr);
    Q_ASSERT(m_Context.rtspSessionUrl != nullptr);
    Q_ASSERT(m_Context.streamConfiguration != nullptr);
    Q_ASSERT(m_Context.videoReceiver != nullptr);
    Q_ASSERT(m_Context.audioReceiver != nullptr);
}

Mode GameStreamClientTransport::mode() const noexcept
{
    return Mode::GameStream;
}

StartResult GameStreamClientTransport::start()
{
    auto* videoReceiver =
            dynamic_cast<VideoReceiver::GameStreamVideoReceiver*>(
                m_Context.videoReceiver);
    auto* audioReceiver =
            dynamic_cast<AudioReceiver::GameStreamAudioReceiver*>(
                m_Context.audioReceiver);
    if (videoReceiver == nullptr || audioReceiver == nullptr ||
            !videoReceiver->activate()) {
        return {
            false,
            LI_ERR_UNSUPPORTED,
        };
    }
    if (!audioReceiver->activate()) {
        videoReceiver->deactivate();
        return {
            false,
            LI_ERR_UNSUPPORTED,
        };
    }
    m_ActiveVideoReceiver = videoReceiver;
    m_ActiveAudioReceiver = audioReceiver;

    const Connectivity::SelectedPath selectedPath =
            m_Context.connectivityAgent->selectPath();
    QByteArray address = selectedPath.address.toUtf8();
    QByteArray appVersion = m_Context.appVersion.toUtf8();
    QByteArray gfeVersion = m_Context.gfeVersion.toUtf8();
    QByteArray rtspSessionUrl = m_Context.rtspSessionUrl->toUtf8();

    SERVER_INFORMATION hostInfo {};
    hostInfo.address = address.data();
    hostInfo.serverInfoAppVersion = appVersion.data();
    hostInfo.serverCodecModeSupport = m_Context.serverCodecModeSupport;
    if (!gfeVersion.isEmpty()) {
        hostInfo.serverInfoGfeVersion = gfeVersion.data();
    }
    if (!rtspSessionUrl.isEmpty()) {
        hostInfo.rtspSessionUrl = rtspSessionUrl.data();
    }

    const int errorCode = LiStartConnection(
            &hostInfo,
            m_Context.streamConfiguration,
            m_Context.connectionCallbacks,
            videoReceiver->callbacks(),
            audioReceiver->callbacks(),
            m_Context.renderContext,
            m_Context.rendererFlags,
            m_Context.audioContext,
            m_Context.audioFlags);
    if (errorCode != 0) {
        m_ActiveAudioReceiver->deactivate();
        m_ActiveAudioReceiver = nullptr;
        m_ActiveVideoReceiver->deactivate();
        m_ActiveVideoReceiver = nullptr;
    }
    return {
        errorCode == 0,
        errorCode,
    };
}

void GameStreamClientTransport::interrupt() noexcept
{
    LiInterruptConnection();
}

void GameStreamClientTransport::stop() noexcept
{
    LiStopConnection();
    if (m_ActiveAudioReceiver != nullptr) {
        m_ActiveAudioReceiver->deactivate();
        m_ActiveAudioReceiver = nullptr;
    }
    if (m_ActiveVideoReceiver != nullptr) {
        m_ActiveVideoReceiver->deactivate();
        m_ActiveVideoReceiver = nullptr;
    }
}

} // namespace ClientTransport
