#pragma once

#include "sessiontelemetry.h"
#include "streaming/video/statsdiagnostics.h"

#include <mutex>
#include <optional>

namespace Overlay {
class OverlayManager;
}

namespace SessionTelemetry {

// Preserves the existing SDL log and in-stream overlay behavior while keeping
// those presentation mechanisms outside the streaming pipeline.
class LegacySessionTelemetry final : public ISessionTelemetry
{
public:
    explicit LegacySessionTelemetry(
            Overlay::OverlayManager* overlayManager);

    Mode mode() const noexcept override;

    bool frameTracingEnabled() const noexcept override;
    void publishFrameTrace(
            const PipelineTelemetry::FrameTrace& trace) override;

    void publishStageStarted(const StageEvent& event) override;
    void publishStageFailed(const StageFailure& event) override;
    void publishConnectionQuality(
            const ConnectionQualityEvent& event) override;
    void publishTermination(
            const TerminationEvent& event) override;

    void publishVideoWindow(
            const VideoWindowReport& report) override;
    void publishVideoSessionSummary(
            const VideoSessionSummary& summary) override;
    void publishAudioWindow(
            const AudioWindowReport& report) override;
    void publishAudioSessionSummary(
            const AudioSessionSummary& summary) override;

private:
    static void stringifyVideoStats(
            const VIDEO_STATS& stats,
            const VideoStreamContext& stream,
            char* output,
            int length);
    static void appendDiagnosisText(
            const Diagnostics::Diagnosis& diagnosis,
            const Diagnostics::SpikeHistory& history,
            char* output,
            int length);
    static const char* codecName(
            const VideoStreamContext& stream) noexcept;
    static void appendAudioStats(
            const AudioWindowReport& report,
            char* output,
            int length);
    static void stringifyAudioStats(
            const AudioTelemetry::Snapshot& stats,
            const AudioStreamContext& stream,
            char* output,
            int length);

    Overlay::OverlayManager* m_OverlayManager;
    std::mutex m_Mutex;
    Diagnostics::SpikeHistory m_SpikeHistory;
    std::optional<AudioWindowReport> m_LastAudioReport;
};

} // namespace SessionTelemetry
