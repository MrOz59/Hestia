#pragma once

#include "audioreceiver.h"
#include "streaming/audio/audiotelemetry.h"

#include <Limelight.h>
#include <QString>

class IAudioRenderer;
struct OpusMSDecoder;

namespace SessionTelemetry {
class ISessionTelemetry;
struct AudioStreamContext;
}

namespace AudioReceiver {

class GameStreamAudioReceiver final : public IAudioReceiver {
public:
    explicit GameStreamAudioReceiver(
            SessionTelemetry::ISessionTelemetry*
                    telemetry = nullptr);
    ~GameStreamAudioReceiver() override;

    Mode mode() const noexcept override;
    Capabilities capabilities() const noexcept override;
    bool testConfiguration(
            const Configuration& configuration) override;
    void setBufferingProfile(
            AudioBuffer::Profile profile) noexcept override;
    AudioBuffer::Profile
    bufferingProfile() const noexcept override;
    void setMuted(bool muted) noexcept override;
    bool isMuted() const noexcept override;

    bool activate() noexcept;
    void deactivate() noexcept;
    PAUDIO_RENDERER_CALLBACKS callbacks() noexcept;

private:
    static int initCallback(
            int audioConfiguration,
            const POPUS_MULTISTREAM_CONFIGURATION opusConfiguration,
            void* context,
            int flags);
    static void cleanupCallback();
    static void decodeAndPlaySampleCallback(
            char* sampleData,
            int sampleLength);

    IAudioRenderer* createAudioRenderer(
            const POPUS_MULTISTREAM_CONFIGURATION opusConfiguration);
    bool initializeAudioRenderer();
    void cleanup(bool publishTelemetry) noexcept;
    void decodeAndPlaySample(char* sampleData, int sampleLength);
    void resetTelemetry() noexcept;
    void archiveRendererMetrics() noexcept;
    AudioTelemetry::RendererMetrics
    rendererMetrics() const noexcept;
    AudioTelemetry::NetworkMetrics
    networkMetrics() const noexcept;
    SessionTelemetry::AudioStreamContext
    audioStreamContext() const;
    void publishAudioWindow(
            uint64_t nowUs,
            bool force = false);
    void publishAudioSummary(uint64_t nowUs);

    Capabilities m_Capabilities;
    AUDIO_RENDERER_CALLBACKS m_Callbacks;
    SessionTelemetry::ISessionTelemetry* m_Telemetry;
    AudioBuffer::Profile m_BufferingProfile;
    bool m_Muted;
    OpusMSDecoder* m_OpusDecoder;
    IAudioRenderer* m_AudioRenderer;
    OPUS_MULTISTREAM_CONFIGURATION m_ActiveAudioConfiguration;
    OPUS_MULTISTREAM_CONFIGURATION m_OriginalAudioConfiguration;
    uint32_t m_AudioSampleCount;
    uint32_t m_DropAudioEndTime;
    QString m_LastRendererName;

    bool m_TelemetryActive;
    uint64_t m_TelemetryStartUs;
    uint64_t m_WindowStartUs;
    AudioTelemetry::PipelineCounters m_PipelineTotals;
    AudioTelemetry::PipelineCounters
            m_LastPublishedPipelineTotals;
    AudioTelemetry::RendererMetrics
            m_ArchivedRendererMetrics;
    AudioTelemetry::RendererMetrics
            m_LastPublishedRendererMetrics;
    AudioTelemetry::NetworkMetrics m_InitialNetworkMetrics;
    AudioTelemetry::NetworkMetrics
            m_LastPublishedNetworkMetrics;

    static GameStreamAudioReceiver* s_ActiveReceiver;
};

} // namespace AudioReceiver
