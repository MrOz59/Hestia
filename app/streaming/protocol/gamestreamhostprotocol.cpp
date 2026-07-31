#include "gamestreamhostprotocol.h"

#include "backend/nvhttp.h"

#include <Limelight.h>

#include <algorithm>

namespace HostProtocol {

namespace {

int legacyVideoFormat(VideoCodec codec, bool tenBit)
{
    switch (codec) {
    case VideoCodec::H264:
        return VIDEO_FORMAT_H264;
    case VideoCodec::Hevc:
        return tenBit ? VIDEO_FORMAT_H265_MAIN10 : VIDEO_FORMAT_H265;
    case VideoCodec::Av1:
        return tenBit ? VIDEO_FORMAT_AV1_MAIN10 : VIDEO_FORMAT_AV1_MAIN8;
    }

    return VIDEO_FORMAT_H264;
}

} // namespace

GameStreamHostProtocol::GameStreamHostProtocol(NvComputer* computer)
    : m_Computer(computer)
{
}

Mode GameStreamHostProtocol::mode() const noexcept
{
    return Mode::GameStream;
}

bool GameStreamHostProtocol::prepareSession(const SessionRequest& request,
                                            QString* sessionId,
                                            QMap<QString, uint32_t>* negotiatedExtensions)
{
    NvHTTP http(m_Computer);
    return http.prepareHestiaSession(
            toHestiaPreparePayload(request),
            sessionId,
            negotiatedExtensions);
}

QString GameStreamHostProtocol::launchSession(const LaunchRequest& request)
{
    STREAM_CONFIGURATION streamConfig {};
    streamConfig.width = request.width;
    streamConfig.height = request.height;
    streamConfig.fps = request.frameRate;
    streamConfig.audioConfiguration =
            MAKE_AUDIO_CONFIGURATION(
                request.audioChannelCount,
                request.audioChannelMask);
    streamConfig.supportedVideoFormats =
            legacyVideoFormat(request.codec, request.tenBit);
    std::copy(request.remoteInputAesKey.cbegin(),
              request.remoteInputAesKey.cend(),
              streamConfig.remoteInputAesKey);
    std::copy(request.remoteInputAesIv.cbegin(),
              request.remoteInputAesIv.cend(),
              streamConfig.remoteInputAesIv);

    QString sessionUrl;
    NvHTTP http(m_Computer);
    http.startApp(
            request.action == LaunchAction::Resume ? "resume" : "launch",
            request.nvidiaServerSoftware,
            request.applicationId,
            &streamConfig,
            request.enableGameOptimizations,
            request.playAudioOnHost,
            static_cast<int>(request.gamepadMask),
            request.persistGameControllers,
            sessionUrl);
    return sessionUrl;
}

bool GameStreamHostProtocol::stopSession(const QString& sessionId)
{
    NvHTTP http(m_Computer);
    return http.stopHestiaSession(sessionId);
}

} // namespace HostProtocol
