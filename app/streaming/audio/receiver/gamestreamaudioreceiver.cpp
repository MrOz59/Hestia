#include "gamestreamaudioreceiver.h"

#include "SDL_compat.h"
#include "streaming/audio/renderers/renderer.h"
#include "streaming/audio/renderers/sdl.h"
#include "streaming/telemetry/sessiontelemetry.h"

#ifdef HAVE_SLAUDIO
#include "streaming/audio/renderers/slaud.h"
#endif

#include <opus_multistream.h>

#include <QString>
#include <QtGlobal>

#define TRY_INIT_RENDERER(renderer, opusConfiguration) \
{ \
    IAudioRenderer* candidate = new renderer(m_BufferingProfile); \
    if (candidate->prepareForPlayback(opusConfiguration)) { \
        return candidate; \
    } \
    delete candidate; \
}

namespace AudioReceiver {

GameStreamAudioReceiver* GameStreamAudioReceiver::s_ActiveReceiver = nullptr;

GameStreamAudioReceiver::GameStreamAudioReceiver(
        SessionTelemetry::ISessionTelemetry* telemetry)
    : m_Callbacks {},
      m_Telemetry(
          telemetry != nullptr ?
              telemetry :
              &SessionTelemetry::nullSessionTelemetry()),
      m_BufferingProfile(AudioBuffer::Profile::Default),
      m_Muted(false),
      m_OpusDecoder(nullptr),
      m_AudioRenderer(nullptr),
      m_ActiveAudioConfiguration {},
      m_OriginalAudioConfiguration {},
      m_AudioSampleCount(0),
      m_DropAudioEndTime(0),
      m_LastRendererName(),
      m_TelemetryActive(false),
      m_TelemetryStartUs(0),
      m_WindowStartUs(0),
      m_PipelineTotals {},
      m_LastPublishedPipelineTotals {},
      m_ArchivedRendererMetrics {},
      m_LastPublishedRendererMetrics {},
      m_InitialNetworkMetrics {},
      m_LastPublishedNetworkMetrics {}
{
    m_Capabilities.arbitraryPacketDuration = true;
#ifdef STEAM_LINK
    m_Capabilities.slowOpusDecoder = true;
#endif

    LiInitializeAudioCallbacks(&m_Callbacks);
    m_Callbacks.init = initCallback;
    m_Callbacks.cleanup = cleanupCallback;
    m_Callbacks.decodeAndPlaySample = decodeAndPlaySampleCallback;
    if (m_Capabilities.directSubmit) {
        m_Callbacks.capabilities |= CAPABILITY_DIRECT_SUBMIT;
    }
    if (m_Capabilities.slowOpusDecoder) {
        m_Callbacks.capabilities |= CAPABILITY_SLOW_OPUS_DECODER;
    }
    if (m_Capabilities.arbitraryPacketDuration) {
        m_Callbacks.capabilities |=
                CAPABILITY_SUPPORTS_ARBITRARY_AUDIO_DURATION;
    }
}

GameStreamAudioReceiver::~GameStreamAudioReceiver()
{
    Q_ASSERT(s_ActiveReceiver != this);
    if (s_ActiveReceiver == this) {
        s_ActiveReceiver = nullptr;
    }
    cleanup(true);
}

Mode GameStreamAudioReceiver::mode() const noexcept
{
    return Mode::GameStream;
}

Capabilities GameStreamAudioReceiver::capabilities() const noexcept
{
    return m_Capabilities;
}

bool GameStreamAudioReceiver::testConfiguration(
        const Configuration& configuration)
{
    if (!configuration.isValid()) {
        return false;
    }

    OPUS_MULTISTREAM_CONFIGURATION opusConfiguration {};
    opusConfiguration.sampleRate = 48000;
    opusConfiguration.samplesPerFrame = 240;
    opusConfiguration.channelCount = configuration.channelCount;

    IAudioRenderer* audioRenderer =
            createAudioRenderer(&opusConfiguration);
    if (audioRenderer == nullptr) {
        return false;
    }

    delete audioRenderer;
    return true;
}

void GameStreamAudioReceiver::setBufferingProfile(
        AudioBuffer::Profile profile) noexcept
{
    Q_ASSERT(s_ActiveReceiver != this);
    if (s_ActiveReceiver == this) {
        return;
    }
    m_BufferingProfile = profile;
}

AudioBuffer::Profile
GameStreamAudioReceiver::bufferingProfile() const noexcept
{
    return m_BufferingProfile;
}

void GameStreamAudioReceiver::setMuted(bool muted) noexcept
{
    if (m_Muted != muted &&
            m_AudioRenderer != nullptr) {
        m_AudioRenderer->resetPlaybackObservation();
    }
    m_Muted = muted;
}

bool GameStreamAudioReceiver::isMuted() const noexcept
{
    return m_Muted;
}

bool GameStreamAudioReceiver::activate() noexcept
{
    if (s_ActiveReceiver != nullptr && s_ActiveReceiver != this) {
        return false;
    }

    s_ActiveReceiver = this;
    return true;
}

void GameStreamAudioReceiver::deactivate() noexcept
{
    if (s_ActiveReceiver == this) {
        s_ActiveReceiver = nullptr;
    }
}

PAUDIO_RENDERER_CALLBACKS
GameStreamAudioReceiver::callbacks() noexcept
{
    return &m_Callbacks;
}

int GameStreamAudioReceiver::initCallback(
        int,
        const POPUS_MULTISTREAM_CONFIGURATION opusConfiguration,
        void*,
        int)
{
    Q_ASSERT(s_ActiveReceiver != nullptr);
    if (s_ActiveReceiver == nullptr) {
        return -1;
    }

    SDL_memcpy(
            &s_ActiveReceiver->m_OriginalAudioConfiguration,
            opusConfiguration,
            sizeof(*opusConfiguration));
    s_ActiveReceiver->m_AudioSampleCount = 0;
    s_ActiveReceiver->m_DropAudioEndTime = 0;
    s_ActiveReceiver->resetTelemetry();
    s_ActiveReceiver->initializeAudioRenderer();
    return 0;
}

void GameStreamAudioReceiver::cleanupCallback()
{
    Q_ASSERT(s_ActiveReceiver != nullptr);
    if (s_ActiveReceiver != nullptr) {
        s_ActiveReceiver->cleanup(true);
    }
}

void GameStreamAudioReceiver::decodeAndPlaySampleCallback(
        char* sampleData,
        int sampleLength)
{
    Q_ASSERT(s_ActiveReceiver != nullptr);
    if (s_ActiveReceiver != nullptr) {
        s_ActiveReceiver->decodeAndPlaySample(
                sampleData,
                sampleLength);
    }
}

IAudioRenderer* GameStreamAudioReceiver::createAudioRenderer(
        const POPUS_MULTISTREAM_CONFIGURATION opusConfiguration)
{
    const QString requestedAudioBackend =
            qgetenv("ML_AUDIO").toLower();
    if (requestedAudioBackend == QStringLiteral("sdl")) {
        TRY_INIT_RENDERER(SdlAudioRenderer, opusConfiguration)
        return nullptr;
    }
#if defined(HAVE_SLAUDIO)
    if (requestedAudioBackend == QStringLiteral("slaudio")) {
        TRY_INIT_RENDERER(SLAudioRenderer, opusConfiguration)
        return nullptr;
    }
#endif
    if (!requestedAudioBackend.isEmpty()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Unknown audio backend: %s",
                     SDL_getenv("ML_AUDIO"));
        return nullptr;
    }

#if defined(HAVE_SLAUDIO)
    TRY_INIT_RENDERER(SLAudioRenderer, opusConfiguration)
#endif
    TRY_INIT_RENDERER(SdlAudioRenderer, opusConfiguration)
    return nullptr;
}

bool GameStreamAudioReceiver::initializeAudioRenderer()
{
    int error;

    SDL_assert(m_OriginalAudioConfiguration.channelCount > 0);
    SDL_assert(m_AudioRenderer == nullptr);
    SDL_assert(m_OpusDecoder == nullptr);

    m_AudioRenderer =
            createAudioRenderer(&m_OriginalAudioConfiguration);
    if (m_AudioRenderer == nullptr) {
        return false;
    }

    m_ActiveAudioConfiguration = m_OriginalAudioConfiguration;
    m_AudioRenderer->remapChannels(&m_ActiveAudioConfiguration);
    m_LastRendererName =
            QString::fromUtf8(
                m_AudioRenderer->rendererName());

    m_OpusDecoder = opus_multistream_decoder_create(
            m_ActiveAudioConfiguration.sampleRate,
            m_ActiveAudioConfiguration.channelCount,
            m_ActiveAudioConfiguration.streams,
            m_ActiveAudioConfiguration.coupledStreams,
            m_ActiveAudioConfiguration.mapping,
            &error);
    if (m_OpusDecoder == nullptr) {
        delete m_AudioRenderer;
        m_AudioRenderer = nullptr;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to create decoder: %d",
                     error);
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Audio stream has %d channels",
                m_ActiveAudioConfiguration.channelCount);
    return true;
}

void GameStreamAudioReceiver::cleanup(
        bool publishTelemetry) noexcept
{
    if (publishTelemetry && m_TelemetryActive) {
        const uint64_t nowUs = LiGetMicroseconds();
        publishAudioWindow(nowUs, true);
        publishAudioSummary(nowUs);
        m_TelemetryActive = false;
    }

    archiveRendererMetrics();
    delete m_AudioRenderer;
    m_AudioRenderer = nullptr;

    if (m_OpusDecoder != nullptr) {
        opus_multistream_decoder_destroy(m_OpusDecoder);
        m_OpusDecoder = nullptr;
    }
}

void GameStreamAudioReceiver::decodeAndPlaySample(
        char* sampleData,
        int sampleLength)
{
    const uint64_t nowUs = LiGetMicroseconds();
    m_PipelineTotals.callbackPackets++;
#ifndef STEAM_LINK
    if (m_AudioSampleCount == 0 &&
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) < 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Unable to set audio thread to high priority: %s",
                    SDL_GetError());
    }
#endif

    if (m_DropAudioEndTime != 0) {
        if (SDL_TICKS_PASSED(SDL_GetTicks(), m_DropAudioEndTime)) {
            m_DropAudioEndTime = 0;
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                        "Audio drop window has ended");
        }
        else {
            m_PipelineTotals.recoveryDroppedPackets++;
            publishAudioWindow(nowUs);
            return;
        }
    }

    m_AudioSampleCount++;
    if (m_Muted) {
        m_PipelineTotals.mutedPackets++;
        publishAudioWindow(nowUs);
        return;
    }

    if (m_AudioRenderer != nullptr) {
        const int sampleSize =
                m_AudioRenderer->getAudioBufferSampleSize();
        const int frameSize =
                sampleSize * m_ActiveAudioConfiguration.channelCount;
        int desiredBufferSize =
                frameSize *
                m_ActiveAudioConfiguration.samplesPerFrame;
        void* buffer =
                m_AudioRenderer->getAudioBuffer(&desiredBufferSize);
        if (buffer == nullptr) {
            m_PipelineTotals.rendererBufferFailures++;
            publishAudioWindow(nowUs);
            return;
        }

        int samplesDecoded;
        if (m_AudioRenderer->getAudioBufferFormat() ==
                IAudioRenderer::AudioFormat::Float32NE) {
            samplesDecoded = opus_multistream_decode_float(
                    m_OpusDecoder,
                    reinterpret_cast<unsigned char*>(sampleData),
                    sampleLength,
                    static_cast<float*>(buffer),
                    desiredBufferSize / frameSize,
                    0);
        }
        else {
            samplesDecoded = opus_multistream_decode(
                    m_OpusDecoder,
                    reinterpret_cast<unsigned char*>(sampleData),
                    sampleLength,
                    static_cast<short*>(buffer),
                    desiredBufferSize / frameSize,
                    0);
        }

        if (samplesDecoded > 0) {
            m_PipelineTotals.decodedPackets++;
            if (sampleData == nullptr &&
                    sampleLength == 0) {
                m_PipelineTotals.concealedPackets++;
            }
            SDL_assert(
                    desiredBufferSize >= frameSize * samplesDecoded);
            desiredBufferSize = frameSize * samplesDecoded;
        }
        else {
            m_PipelineTotals.decodeFailures++;
            desiredBufferSize = 0;
        }

        if (!m_AudioRenderer->submitAudio(desiredBufferSize)) {
            m_PipelineTotals.rendererFailures++;
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Reinitializing audio renderer after failure");
            cleanup(false);
        }
    }
    else {
        m_PipelineTotals.rendererUnavailablePackets++;
    }

    // Retry once per second to avoid thrashing if the device is unavailable.
    if (m_AudioRenderer == nullptr &&
            (m_AudioSampleCount % 200) == 0) {
        const uint32_t reinitStartTime = SDL_GetTicks();
        if (initializeAudioRenderer()) {
            m_PipelineTotals.rendererReinitializations++;
            const uint32_t reinitStopTime = SDL_GetTicks();
            m_DropAudioEndTime =
                    reinitStopTime +
                    (reinitStopTime - reinitStartTime);
            SDL_LogInfo(
                    SDL_LOG_CATEGORY_APPLICATION,
                    "Audio reinitialization took %d ms - starting drop window",
                    reinitStopTime - reinitStartTime);
        }
    }

    publishAudioWindow(nowUs);
}

void GameStreamAudioReceiver::resetTelemetry() noexcept
{
    const uint64_t nowUs = LiGetMicroseconds();
    m_TelemetryActive = true;
    m_TelemetryStartUs = nowUs;
    m_WindowStartUs = nowUs;
    m_PipelineTotals = {};
    m_LastPublishedPipelineTotals = {};
    m_ArchivedRendererMetrics = {};
    m_LastPublishedRendererMetrics = {};
    m_InitialNetworkMetrics = networkMetrics();
    m_LastPublishedNetworkMetrics = {};
    m_LastRendererName.clear();
}

void GameStreamAudioReceiver::archiveRendererMetrics() noexcept
{
    if (m_AudioRenderer == nullptr) {
        return;
    }
    AudioTelemetry::accumulate(
        m_AudioRenderer->playbackMetrics(),
        m_ArchivedRendererMetrics);
}

AudioTelemetry::RendererMetrics
GameStreamAudioReceiver::rendererMetrics() const noexcept
{
    AudioTelemetry::RendererMetrics metrics =
            m_ArchivedRendererMetrics;
    if (m_AudioRenderer != nullptr) {
        const AudioTelemetry::RendererMetrics current =
                m_AudioRenderer->playbackMetrics();
        AudioTelemetry::accumulate(current, metrics);
        metrics.currentQueuedDurationMs =
                current.currentQueuedDurationMs;
        metrics.deviceBufferDurationMs =
                current.deviceBufferDurationMs;
        metrics.queueLimitMs = current.queueLimitMs;
        metrics.upstreamBackpressureLimitMs =
                current.upstreamBackpressureLimitMs;
        metrics.queueDepthObservable =
                current.queueDepthObservable;
        metrics.deviceRunning = current.deviceRunning;
    }
    return metrics;
}

AudioTelemetry::NetworkMetrics
GameStreamAudioReceiver::networkMetrics() const noexcept
{
    const RTP_AUDIO_STATS* stats =
            LiGetRTPAudioStats();
    if (stats == nullptr) {
        return {};
    }
    return {
        stats->packetCountAudio,
        stats->packetCountFec,
        stats->packetCountFecRecovered,
        stats->packetCountFecFailed,
        stats->packetCountOOS,
        stats->packetCountInvalid,
        stats->packetCountFecInvalid,
    };
}

SessionTelemetry::AudioStreamContext
GameStreamAudioReceiver::audioStreamContext() const
{
    const OPUS_MULTISTREAM_CONFIGURATION& configuration =
            m_ActiveAudioConfiguration.sampleRate != 0 ?
                m_ActiveAudioConfiguration :
                m_OriginalAudioConfiguration;
    return {
        static_cast<uint32_t>(
            configuration.sampleRate),
        static_cast<uint32_t>(
            configuration.samplesPerFrame),
        static_cast<uint8_t>(
            configuration.channelCount),
        m_LastRendererName,
        QString::fromLatin1(
            AudioBuffer::profileName(
                m_BufferingProfile)),
    };
}

void GameStreamAudioReceiver::publishAudioWindow(
        uint64_t nowUs,
        bool force)
{
    if (!m_TelemetryActive ||
            (!force &&
             nowUs < m_WindowStartUs + 1000000)) {
        return;
    }

    const AudioTelemetry::RendererMetrics
            currentRendererMetrics = rendererMetrics();
    const AudioTelemetry::NetworkMetrics
            cumulativeNetworkMetrics =
                AudioTelemetry::subtract(
                    networkMetrics(),
                    m_InitialNetworkMetrics);

    AudioTelemetry::Snapshot snapshot;
    snapshot.pipeline = AudioTelemetry::subtract(
        m_PipelineTotals,
        m_LastPublishedPipelineTotals);
    snapshot.renderer = AudioTelemetry::subtract(
        currentRendererMetrics,
        m_LastPublishedRendererMetrics);
    snapshot.network = AudioTelemetry::subtract(
        cumulativeNetworkMetrics,
        m_LastPublishedNetworkMetrics);
    snapshot.pendingNetworkDurationMs =
            static_cast<uint32_t>(
                qMax(0, LiGetPendingAudioDuration()));
    snapshot.estimatedPresentationDelayMs =
            snapshot.pendingNetworkDurationMs +
            snapshot.renderer.currentQueuedDurationMs +
            snapshot.renderer.deviceBufferDurationMs;
    snapshot.measurementStartUs = m_WindowStartUs;
    snapshot.measurementEndUs = nowUs;

    m_Telemetry->publishAudioWindow({
        snapshot,
        audioStreamContext(),
        false,
        0,
    });

    m_LastPublishedPipelineTotals =
            m_PipelineTotals;
    m_LastPublishedRendererMetrics =
            currentRendererMetrics;
    m_LastPublishedNetworkMetrics =
            cumulativeNetworkMetrics;
    m_WindowStartUs = nowUs;
}

void GameStreamAudioReceiver::publishAudioSummary(
        uint64_t nowUs)
{
    AudioTelemetry::Snapshot snapshot;
    snapshot.pipeline = m_PipelineTotals;
    snapshot.renderer = rendererMetrics();
    snapshot.network = AudioTelemetry::subtract(
        networkMetrics(),
        m_InitialNetworkMetrics);
    snapshot.pendingNetworkDurationMs =
            static_cast<uint32_t>(
                qMax(0, LiGetPendingAudioDuration()));
    snapshot.estimatedPresentationDelayMs =
            snapshot.pendingNetworkDurationMs +
            snapshot.renderer.currentQueuedDurationMs +
            snapshot.renderer.deviceBufferDurationMs;
    snapshot.measurementStartUs = m_TelemetryStartUs;
    snapshot.measurementEndUs = nowUs;
    m_Telemetry->publishAudioSessionSummary({
        snapshot,
        audioStreamContext(),
    });
}

} // namespace AudioReceiver

#undef TRY_INIT_RENDERER
