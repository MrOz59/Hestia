#pragma once

#include "renderscheduler.h"

#include <memory>

class IFFmpegRenderer;
class Pacer;
struct _VIDEO_STATS;

namespace PipelineTelemetry {
class FrameTimeline;
}

namespace SessionTelemetry {
class ISessionTelemetry;
}

namespace RenderScheduler {

// Bridges the protocol-neutral scheduler contract to the existing FFmpeg
// Pacer without changing its zero-copy AVFrame ownership path.
class LegacyPacerAdapter final : public IRenderScheduler {
public:
    LegacyPacerAdapter(
            IFFmpegRenderer* renderer,
            _VIDEO_STATS* videoStats,
            PipelineTelemetry::FrameTimeline* frameTimeline,
            SessionTelemetry::ISessionTelemetry* telemetry);
    ~LegacyPacerAdapter() override;

    bool initialize(
            const Configuration& configuration) override;
    Capabilities capabilities() const noexcept override;
    void submitFrame(DecodedFrame frame) override;
    void renderOnMainThread() override;
    void updateNetworkConditions(
            NetworkConditions conditions) noexcept override;
    PresentationStatus status() const override;

private:
    std::unique_ptr<Pacer> m_Pacer;
};

} // namespace RenderScheduler
