#pragma once

#include "renderer.h"
#include "SDL_compat.h"
#include "streaming/audio/bufferpolicy.h"

class SdlAudioRenderer : public IAudioRenderer
{
public:
    explicit SdlAudioRenderer(
            AudioBuffer::Profile profile =
                AudioBuffer::Profile::Default);

    virtual ~SdlAudioRenderer();

    virtual bool prepareForPlayback(const OPUS_MULTISTREAM_CONFIGURATION* opusConfig);

    virtual void* getAudioBuffer(int* size);

    virtual bool submitAudio(int bytesWritten);

    const char* rendererName() const noexcept override;

    AudioTelemetry::RendererMetrics
    playbackMetrics() const noexcept override;

    void resetPlaybackObservation() noexcept override;

    virtual AudioFormat getAudioBufferFormat();

private:
    uint32_t queuedAudioBytes() const noexcept;
    uint32_t queuedDurationMs() const noexcept;
    void observeQueueDepth(uint32_t durationMs) noexcept;
    void updatePlaybackEstimate(uint64_t nowUs) noexcept;
    uint64_t audioDurationUsForBytes(int bytes) const noexcept;
    void beginRebuffering(
            bool raiseTarget,
            uint64_t nowUs) noexcept;
    void resumeIfBuffered(uint64_t nowUs) noexcept;

    SDL_AudioDeviceID m_AudioDevice;
    void* m_AudioBuffer;
    Uint32 m_FrameSize;
    Uint32 m_BytesPerMillisecond;
    AudioBuffer::Profile m_BufferingProfile;
    AudioBuffer::Policy m_BufferPolicy;
    AudioTelemetry::RendererMetrics m_Metrics;
    bool m_HasQueuedAudio;
    bool m_IsRebuffering;
    uint32_t m_PrebufferTargetMs;
    uint64_t m_EstimatedBufferedAudioUs;
    uint64_t m_LastPlaybackAccountingUs;
};
