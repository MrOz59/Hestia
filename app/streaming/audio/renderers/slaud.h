#pragma once

#include "renderer.h"
#include "streaming/audio/bufferpolicy.h"
#include <SLAudio.h>

class SLAudioRenderer : public IAudioRenderer
{
public:
    explicit SLAudioRenderer(
            AudioBuffer::Profile profile =
                AudioBuffer::Profile::Default);

    virtual ~SLAudioRenderer();

    virtual bool prepareForPlayback(const OPUS_MULTISTREAM_CONFIGURATION* opusConfig);

    virtual void* getAudioBuffer(int* size);

    virtual bool submitAudio(int bytesWritten);

    const char* rendererName() const noexcept override;

    AudioTelemetry::RendererMetrics
    playbackMetrics() const noexcept override;

    virtual AudioFormat getAudioBufferFormat();

    virtual void remapChannels(POPUS_MULTISTREAM_CONFIGURATION opusConfig);

private:
    static void slLogCallback(void* context, ESLAudioLog logLevel, const char* message);

    CSLAudioContext* m_AudioContext;
    CSLAudioStream* m_AudioStream;

    void* m_AudioBuffer;
    int m_AudioBufferSize;
    int m_MaxQueuedAudioMs;
    AudioBuffer::Profile m_BufferingProfile;
    AudioTelemetry::RendererMetrics m_Metrics;
};
