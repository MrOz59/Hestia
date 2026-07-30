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
    uint32_t queuedDurationMs() const noexcept;
    void observeQueueDepth(uint32_t durationMs) noexcept;

    SDL_AudioDeviceID m_AudioDevice;
    void* m_AudioBuffer;
    Uint32 m_FrameSize;
    Uint32 m_BytesPerMillisecond;
    AudioBuffer::Profile m_BufferingProfile;
    AudioTelemetry::RendererMetrics m_Metrics;
    bool m_HasQueuedAudio;
};
