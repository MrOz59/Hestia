#include "hostprotocol.h"

namespace HostProtocol {

namespace {

QString codecName(VideoCodec codec)
{
    switch (codec) {
    case VideoCodec::H264:
        return QStringLiteral("h264");
    case VideoCodec::Hevc:
        return QStringLiteral("hevc");
    case VideoCodec::Av1:
        return QStringLiteral("av1");
    }

    return QStringLiteral("h264");
}

QString launchModeName(LaunchMode mode)
{
    switch (mode) {
    case LaunchMode::Normal:
        return QStringLiteral("normal");
    case LaunchMode::Gamescope:
        return QStringLiteral("gamescope");
    }

    return QStringLiteral("normal");
}

QString isolationName(SessionIsolation isolation)
{
    switch (isolation) {
    case SessionIsolation::Shared:
        return QStringLiteral("shared");
    case SessionIsolation::Required:
        return QStringLiteral("required");
    case SessionIsolation::Unspecified:
        break;
    }

    return {};
}

} // namespace

QJsonObject toHestiaPreparePayload(const SessionRequest& request)
{
    const QJsonObject client {
        {"name", request.clientName},
        {"version", request.clientVersion},
        {"platform", request.clientPlatform},
        {"display_width", request.displayWidth},
        {"display_height", request.displayHeight},
        {"refresh_rate", request.displayRefreshRate},
        {"hdr", request.displayHdr},
    };
    const QJsonObject stream {
        {"requested_width", request.streamWidth},
        {"requested_height", request.streamHeight},
        {"requested_fps", request.streamFrameRate},
        {"codec", codecName(request.streamCodec)},
        {"bitrate_kbps", request.streamBitrateKbps},
        {"hdr_mode", request.displayHdr ? "hdr" : "sdr"},
        {"scale_factor", request.streamScaleFactor},
    };
    const QJsonObject virtualDisplay {
        {"enabled", request.virtualDisplay},
        {"backend", "auto"},
        {"desktop_integration", "auto"},
        {"recover_physical_monitor", request.recoverPhysicalMonitor},
    };
    const QJsonObject app {
        {"id", QString::number(request.applicationId)},
        {"launch_mode", launchModeName(request.launchMode)},
    };

    QJsonObject payload {
        {"client", client},
        {"stream", stream},
        {"virtual_display", virtualDisplay},
        {"app", app},
    };

    if (!request.extensions.isEmpty()) {
        payload.insert("extensions", HermesExtensions::toJson(request.extensions));
    }

    const QString isolation = isolationName(request.isolation);
    if (!isolation.isEmpty()) {
        payload.insert("session", QJsonObject {
            {"isolation", isolation},
        });
    }

    return payload;
}

} // namespace HostProtocol
