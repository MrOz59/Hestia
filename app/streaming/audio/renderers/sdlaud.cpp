#include "sdl.h"

#include <Limelight.h>

SdlAudioRenderer::SdlAudioRenderer(
        AudioBuffer::Profile profile)
    : m_AudioDevice(0),
      m_AudioBuffer(nullptr),
      m_FrameSize(0),
      m_BytesPerMillisecond(0),
      m_BufferingProfile(profile),
      m_Metrics {},
      m_HasQueuedAudio(false)
{
    SDL_assert(!SDL_WasInit(SDL_INIT_AUDIO));

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s",
                     SDL_GetError());
        SDL_assert(SDL_WasInit(SDL_INIT_AUDIO));
    }
}

bool SdlAudioRenderer::prepareForPlayback(const OPUS_MULTISTREAM_CONFIGURATION* opusConfig)
{
    SDL_AudioSpec want, have;

    SDL_zero(want);
    want.freq = opusConfig->sampleRate;
    want.format = AUDIO_F32SYS;
    want.channels = opusConfig->channelCount;

    const AudioBuffer::Policy bufferPolicy =
            AudioBuffer::calculate(
                m_BufferingProfile,
                AudioBuffer::Backend::Sdl,
                {
                    static_cast<uint32_t>(
                        opusConfig->sampleRate),
                    static_cast<uint32_t>(
                        opusConfig->samplesPerFrame),
                    static_cast<uint8_t>(
                        opusConfig->channelCount),
                });

    // The default policy retains the historical 10 ms PulseAudio safety
    // floor and three-packet target. Other profiles remain explicit opt-ins.
    want.samples = bufferPolicy.deviceBufferSamples;

    m_FrameSize = opusConfig->samplesPerFrame *
                  opusConfig->channelCount *
                  getAudioBufferSampleSize();
    m_BytesPerMillisecond =
            opusConfig->sampleRate / 1000 *
            opusConfig->channelCount *
            getAudioBufferSampleSize();

    m_AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (m_AudioDevice == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to open audio device: %s",
                     SDL_GetError());
        return false;
    }

    m_Metrics.deviceBufferDurationMs =
            have.samples * 1000 / have.freq;
    m_Metrics.queueLimitMs =
            bufferPolicy.playbackQueueLimitMs;
    m_Metrics.upstreamBackpressureLimitMs =
            bufferPolicy.upstreamBackpressureLimitMs;
    m_Metrics.queueDepthObservable = true;
    m_Metrics.deviceRunning = true;

    m_AudioBuffer = SDL_malloc(m_FrameSize);
    if (m_AudioBuffer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to allocate audio buffer");
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Desired audio buffer: %u samples (%u bytes)",
                want.samples,
                want.samples * want.channels * getAudioBufferSampleSize());

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Obtained audio buffer: %u samples (%u bytes)",
                have.samples,
                have.size);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "SDL audio driver: %s",
                SDL_GetCurrentAudioDriver());

    // Start playback
    SDL_PauseAudioDevice(m_AudioDevice, 0);

    return true;
}

SdlAudioRenderer::~SdlAudioRenderer()
{
    if (m_AudioDevice != 0) {
        // Stop playback
        SDL_PauseAudioDevice(m_AudioDevice, 1);
        SDL_CloseAudioDevice(m_AudioDevice);
    }

    if (m_AudioBuffer != nullptr) {
        SDL_free(m_AudioBuffer);
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    SDL_assert(!SDL_WasInit(SDL_INIT_AUDIO));
}

void* SdlAudioRenderer::getAudioBuffer(int*)
{
    return m_AudioBuffer;
}

bool SdlAudioRenderer::submitAudio(int bytesWritten)
{
    if (bytesWritten == 0) {
        // Nothing to do
        return true;
    }

    const uint32_t queuedBeforeSubmit =
            queuedDurationMs();
    observeQueueDepth(queuedBeforeSubmit);
    if (m_HasQueuedAudio && queuedBeforeSubmit == 0) {
        m_Metrics.underruns++;
        m_HasQueuedAudio = false;
    }

    // Don't queue if the receiver already holds more audio than this profile
    // allows.
    if (LiGetPendingAudioDuration() >
            static_cast<int>(
                m_Metrics.upstreamBackpressureLimitMs)) {
        m_Metrics.backpressureSkippedPackets++;
        return true;
    }

    // Provide backpressure on the queue to ensure too many frames don't build up
    // in SDL's audio queue, but don't wait forever to avoid a deadlock if the
    // audio device fails.
    const uint64_t queueWaitStartedUs =
            LiGetMicroseconds();
    bool waitedForQueue = false;
    for (int i = 0; i < 100; i++) {
        // Our device may enter a permanent error status upon removal, so we need
        // to recreate the audio device to pick up the new default audio device.
        if (SDL_GetAudioDeviceStatus(m_AudioDevice) == SDL_AUDIO_STOPPED) {
            m_Metrics.deviceRunning = false;
            return false;
        }

        // Only queue more samples where there is 50 ms or less in SDL's queue
        const uint32_t queuedMs = queuedDurationMs();
        observeQueueDepth(queuedMs);
        if (queuedMs <= m_Metrics.queueLimitMs) {
            break;
        }

        waitedForQueue = true;
        SDL_Delay(1);
    }
    if (waitedForQueue) {
        m_Metrics.queueWaitEvents++;
        m_Metrics.queueWaitTimeUs +=
                LiGetMicroseconds() -
                queueWaitStartedUs;
    }

    if (SDL_QueueAudio(m_AudioDevice, m_AudioBuffer, bytesWritten) < 0) {
        m_Metrics.queueFailures++;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to queue audio sample: %s",
                     SDL_GetError());
    }
    else {
        m_Metrics.submittedPackets++;
        m_Metrics.submittedBytes +=
                static_cast<uint64_t>(bytesWritten);
        m_HasQueuedAudio = true;
        observeQueueDepth(queuedDurationMs());
    }

    return true;
}

const char* SdlAudioRenderer::rendererName() const noexcept
{
    return "SDL";
}

AudioTelemetry::RendererMetrics
SdlAudioRenderer::playbackMetrics() const noexcept
{
    AudioTelemetry::RendererMetrics metrics =
            m_Metrics;
    metrics.currentQueuedDurationMs =
            queuedDurationMs();
    metrics.deviceRunning =
            m_AudioDevice != 0 &&
            SDL_GetAudioDeviceStatus(m_AudioDevice) !=
                SDL_AUDIO_STOPPED;
    return metrics;
}

void SdlAudioRenderer::resetPlaybackObservation() noexcept
{
    m_HasQueuedAudio = false;
}

uint32_t SdlAudioRenderer::queuedDurationMs() const noexcept
{
    if (m_AudioDevice == 0 ||
            m_BytesPerMillisecond == 0) {
        return 0;
    }
    return SDL_GetQueuedAudioSize(m_AudioDevice) /
            m_BytesPerMillisecond;
}

void SdlAudioRenderer::observeQueueDepth(
        uint32_t durationMs) noexcept
{
    if (durationMs >
            m_Metrics.highestObservedQueueDurationMs) {
        m_Metrics.highestObservedQueueDurationMs =
                durationMs;
    }
}

IAudioRenderer::AudioFormat SdlAudioRenderer::getAudioBufferFormat()
{
    return AudioFormat::Float32NE;
}
