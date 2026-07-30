#include "legacypaceradapter.h"

#include "SDL_compat.h"
#include "streaming/video/ffmpeg-renderers/pacer/pacer.h"

namespace RenderScheduler {

LegacyPacerAdapter::LegacyPacerAdapter(
        IFFmpegRenderer* renderer,
        _VIDEO_STATS* videoStats,
        PipelineTelemetry::FrameTimeline* frameTimeline,
        SessionTelemetry::ISessionTelemetry* telemetry)
    : m_Pacer(
          std::make_unique<Pacer>(
              renderer,
              videoStats,
              frameTimeline,
              telemetry))
{
}

LegacyPacerAdapter::~LegacyPacerAdapter() = default;

bool LegacyPacerAdapter::initialize(
        const Configuration& configuration)
{
    return m_Pacer->initialize(
        static_cast<SDL_Window*>(configuration.nativeWindow),
        static_cast<int>(configuration.maximumFrameRate),
        configuration.framePacing);
}

Capabilities LegacyPacerAdapter::capabilities() const noexcept
{
    return {
        Pacer::MAX_OUTSTANDING_FRAMES,
    };
}

void LegacyPacerAdapter::submitFrame(DecodedFrame frame)
{
    if (!frame) {
        return;
    }

    auto* avFrame =
            static_cast<AVFrame*>(frame.takeNativeHandle());
    m_Pacer->submitFrame(avFrame);
}

void LegacyPacerAdapter::renderOnMainThread()
{
    m_Pacer->renderOnMainThread();
}

void LegacyPacerAdapter::updateNetworkConditions(
        NetworkConditions conditions) noexcept
{
    m_Pacer->updateNetworkJitter(
        conditions.roundTripTimeMs,
        conditions.roundTripTimeVarianceMs);
}

PresentationStatus LegacyPacerAdapter::status() const
{
    PresentationStatus status;
    switch (m_Pacer->getPresentationPath()) {
    case Pacer::PresentationPath::SynchronizedSource:
        status.path = PresentationPath::SynchronizedSource;
        break;
    case Pacer::PresentationPath::RendererVsync:
        status.path = PresentationPath::RendererVsync;
        break;
    case Pacer::PresentationPath::RendererDriven:
        status.path = PresentationPath::RendererDriven;
        break;
    }
    status.pathName =
            QString::fromUtf8(m_Pacer->getPresentationPathName());
    status.displayRefreshRateMillihertz =
            static_cast<uint32_t>(
                m_Pacer->getDisplayFpsMillihertz());
    return status;
}

} // namespace RenderScheduler
