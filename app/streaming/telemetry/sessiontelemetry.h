#pragma once

#include "streaming/audio/audiotelemetry.h"
#include "streaming/video/decoder.h"
#include "streaming/video/pipelinetelemetry.h"

#include <QString>

#include <cstdint>

namespace SessionTelemetry {

enum class Mode {
    Null,
    LegacyLocal,
};

enum class ConnectionQuality {
    Unknown,
    Okay,
    Poor,
};

enum class VideoCodec {
    Unknown,
    H264,
    Hevc,
    Av1,
};

struct StageEvent {
    int nativeStage = 0;
    QString name;
};

struct StageFailure {
    StageEvent stage;
    int errorCode = 0;
    QString failingPorts;
};

struct TerminationEvent {
    int errorCode = 0;
    bool unexpected = false;
    QString failingPorts;
};

struct ConnectionQualityEvent {
    int nativeStatus = 0;
    ConnectionQuality quality = ConnectionQuality::Unknown;
    uint32_t bitrateKbps = 0;
    bool warningsEnabled = false;
    bool overlaySuppressed = false;
};

struct VideoStreamContext {
    int width = 0;
    int height = 0;
    int targetFrameRate = 0;
    VideoCodec codec = VideoCodec::Unknown;
    bool tenBit = false;
    bool hdr = false;
    bool chroma444 = false;
    QString presentationPath;
    uint32_t displayRefreshRateMillihertz = 0;
    double averageMegabitsPerSecond = 0.0;
    double peakMegabitsPerSecond = 0.0;
    uint32_t peakBitrateWindowSeconds = 0;
};

struct VideoWindowReport {
    // The just-completed collection window, normally one second.
    VIDEO_STATS completedWindow {};

    // A smoothed view intended for live diagnostics, normally the last two
    // completed windows.
    VIDEO_STATS displayWindow {};

    VideoStreamContext stream;
};

struct VideoSessionSummary {
    VIDEO_STATS aggregate {};
    VideoStreamContext stream;
};

struct AudioStreamContext {
    uint32_t sampleRate = 0;
    uint32_t samplesPerFrame = 0;
    uint8_t channelCount = 0;
    QString rendererName;
    QString bufferingProfileName;

    uint32_t packetDurationMs() const noexcept
    {
        return sampleRate != 0 ?
            samplesPerFrame * 1000 / sampleRate :
            0;
    }
};

struct AudioWindowReport {
    AudioTelemetry::Snapshot completedWindow;
    AudioStreamContext stream;

    // The legacy GameStream callback does not expose a presentation timestamp,
    // so this remains false until a receiver can provide a real media-clock
    // comparison. estimatedPresentationDelayMs is still available above.
    bool avSyncOffsetAvailable = false;
    int64_t avSyncOffsetUs = 0;
};

struct AudioSessionSummary {
    AudioTelemetry::Snapshot aggregate;
    AudioStreamContext stream;
};

// Calls may originate from the connection, decoder, or presentation threads.
// Implementations that share state across event families must synchronize it;
// producers never transfer ownership of the referenced snapshots.
class ISessionTelemetry
{
public:
    virtual ~ISessionTelemetry() = default;

    virtual Mode mode() const noexcept = 0;

    virtual bool frameTracingEnabled() const noexcept = 0;
    virtual void publishFrameTrace(
            const PipelineTelemetry::FrameTrace& trace) = 0;

    virtual void publishStageStarted(const StageEvent& event) = 0;
    virtual void publishStageFailed(const StageFailure& event) = 0;
    virtual void publishConnectionQuality(
            const ConnectionQualityEvent& event) = 0;
    virtual void publishTermination(
            const TerminationEvent& event) = 0;

    virtual void publishVideoWindow(
            const VideoWindowReport& report) = 0;
    virtual void publishVideoSessionSummary(
            const VideoSessionSummary& summary) = 0;
    virtual void publishAudioWindow(
            const AudioWindowReport& report) = 0;
    virtual void publishAudioSessionSummary(
            const AudioSessionSummary& summary) = 0;
};

class NullSessionTelemetry final : public ISessionTelemetry
{
public:
    Mode mode() const noexcept override
    {
        return Mode::Null;
    }

    bool frameTracingEnabled() const noexcept override
    {
        return false;
    }

    void publishFrameTrace(
            const PipelineTelemetry::FrameTrace&) override
    {
    }

    void publishStageStarted(const StageEvent&) override
    {
    }

    void publishStageFailed(const StageFailure&) override
    {
    }

    void publishConnectionQuality(
            const ConnectionQualityEvent&) override
    {
    }

    void publishTermination(const TerminationEvent&) override
    {
    }

    void publishVideoWindow(const VideoWindowReport&) override
    {
    }

    void publishVideoSessionSummary(
            const VideoSessionSummary&) override
    {
    }

    void publishAudioWindow(
            const AudioWindowReport&) override
    {
    }

    void publishAudioSessionSummary(
            const AudioSessionSummary&) override
    {
    }
};

inline ISessionTelemetry& nullSessionTelemetry() noexcept
{
    static NullSessionTelemetry telemetry;
    return telemetry;
}

} // namespace SessionTelemetry
