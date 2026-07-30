#pragma once

#include <array>
#include <cstdint>

#include <QJsonObject>
#include <QString>

namespace HostProtocol {

enum class Mode : uint8_t {
    GameStream,
};

enum class VideoCodec : uint8_t {
    H264,
    Hevc,
    Av1,
};

enum class LaunchAction : uint8_t {
    Launch,
    Resume,
};

enum class LaunchMode : uint8_t {
    Normal,
    Gamescope,
};

enum class SessionIsolation : uint8_t {
    Unspecified,
    Shared,
    Required,
};

struct SessionRequest {
    QString clientName;
    QString clientVersion;
    QString clientPlatform;
    int displayWidth = 0;
    int displayHeight = 0;
    int displayRefreshRate = 0;
    bool displayHdr = false;

    int streamWidth = 0;
    int streamHeight = 0;
    int streamFrameRate = 0;
    int streamBitrateKbps = 0;
    VideoCodec streamCodec = VideoCodec::H264;
    int streamScaleFactor = 100;

    bool virtualDisplay = false;
    bool recoverPhysicalMonitor = false;
    int applicationId = 0;
    LaunchMode launchMode = LaunchMode::Normal;
    SessionIsolation isolation = SessionIsolation::Unspecified;
};

struct LaunchRequest {
    LaunchAction action = LaunchAction::Launch;
    int applicationId = 0;
    bool nvidiaServerSoftware = false;

    int width = 0;
    int height = 0;
    int frameRate = 0;
    VideoCodec codec = VideoCodec::H264;
    bool tenBit = false;
    uint8_t audioChannelCount = 2;
    uint16_t audioChannelMask = 0x3;

    std::array<char, 16> remoteInputAesKey {};
    std::array<char, 16> remoteInputAesIv {};

    bool enableGameOptimizations = false;
    bool playAudioOnHost = false;
    uint32_t gamepadMask = 0;
    bool persistGameControllers = false;
};

class IHostProtocol {
public:
    virtual ~IHostProtocol() = default;

    virtual Mode mode() const noexcept = 0;
    virtual bool prepareSession(const SessionRequest& request,
                                QString* sessionId) = 0;
    virtual QString launchSession(const LaunchRequest& request) = 0;
    virtual bool stopSession(const QString& sessionId) = 0;
};

// The Hermes extension payload remains an adapter detail. Keeping this
// conversion outside Session prevents UI/session code from depending on JSON
// field names and also makes the wire mapping deterministic to unit-test.
QJsonObject toHestiaPreparePayload(const SessionRequest& request);

} // namespace HostProtocol
