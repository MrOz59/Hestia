#include <QtTest>

#include "backend/hestiacapabilities.h"
#include "settings/streamingpreferences.h"
#include "streaming/audio/audiotelemetry.h"
#include "streaming/audio/bufferpolicy.h"
#include "streaming/audio/receiver/audioreceiver.h"
#include "streaming/connectivity/connectivityagent.h"
#include "streaming/hestianegotiation.h"
#include "streaming/input/sender/inputsender.h"
#include "streaming/protocol/hostprotocol.h"
#include "streaming/telemetry/sessiontelemetry.h"
#include "streaming/transport/clienttransport.h"
#include "streaming/video/decoder/decoder.h"
#include "streaming/video/ffmpeg-renderers/pacer/pacingpolicy.h"
#include "streaming/video/pipelinetelemetry.h"
#include "streaming/video/receiver/videoreceiver.h"
#include "streaming/video/scheduler/renderscheduler.h"
#include "streaming/video/statsdiagnostics.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QTranslator>

namespace {

VIDEO_STATS healthyStats()
{
    VIDEO_STATS stats {};
    stats.receivedFrames = 60;
    stats.decodedFrames = 60;
    stats.renderedFrames = 60;
    stats.totalFrames = 60;
    stats.receivedFps = 60;
    stats.decodedFps = 60;
    stats.renderedFps = 60;
    stats.totalDecodeTimeUs = 60ULL * 2000;
    stats.totalPacerTimeUs = 60ULL * 16000;
    stats.totalRenderTimeUs = 60ULL * 16600;
    stats.lastRtt = 8;
    stats.lastRttVariance = 1;
    return stats;
}

QJsonObject validCapabilities()
{
    return {
        {"ok", true},
        {"server_name", "Hermes"},
        {"base", "Apollo"},
        {"hestia_protocol", 1},
        {"min_client_protocol", 1},
        {"max_client_protocol", 1},
        {"server_version", "1.0.0"},
        {"compatibility", QJsonObject {
            {"gamestream", true},
            {"moonlight", true},
            {"sunshine", true},
        }},
        {"features", QJsonObject {
            {"virtual_display", true},
            {"multi_user_sessions", true},
            {"hermes_kms_isolated_sessions", QJsonObject {
                {"supported", true},
                {"enabled", true},
                {"ready", true},
            }},
            {"virtual_display_backend", QJsonArray {"hermes_kms"}},
            {"kde_kscreen", true},
            {"display_recovery", true},
            {"client_resolution_matching", true},
            {"client_fps_matching", true},
            {"hdr_mode_control", true},
            {"scale_factor", true},
            {"gamescope_session", true},
            {"server_commands", true},
            {"clipboard_sync", true},
            {"permission_system", true},
        }},
        {"limits", QJsonObject {
            {"max_width", 3840},
            {"max_height", 2160},
            {"max_fps", 120},
            {"supported_fps", QJsonArray {30, 60, 120}},
            {"supported_codecs", QJsonArray {"h264", "hevc", "av1"}},
        }},
    };
}

class FakeHostProtocol final : public HostProtocol::IHostProtocol
{
public:
    HostProtocol::Mode mode() const noexcept override
    {
        return HostProtocol::Mode::GameStream;
    }

    bool prepareSession(const HostProtocol::SessionRequest& request,
                        QString* sessionId) override
    {
        prepareCalled = true;
        preparedRequest = request;
        if (sessionId != nullptr) {
            *sessionId = QStringLiteral("fake-session");
        }
        return true;
    }

    QString launchSession(
            const HostProtocol::LaunchRequest& request) override
    {
        launchCalled = true;
        launchRequest = request;
        return QStringLiteral("rtsp://fake/session");
    }

    bool stopSession(const QString& sessionId) override
    {
        stopCalled = true;
        stoppedSessionId = sessionId;
        return true;
    }

    bool prepareCalled = false;
    bool launchCalled = false;
    bool stopCalled = false;
    HostProtocol::SessionRequest preparedRequest;
    HostProtocol::LaunchRequest launchRequest;
    QString stoppedSessionId;
};

class FakeClientTransport final : public ClientTransport::IClientTransport
{
public:
    ClientTransport::Mode mode() const noexcept override
    {
        return ClientTransport::Mode::GameStream;
    }

    ClientTransport::StartResult start() override
    {
        startCalls++;
        return nextStartResult;
    }

    void interrupt() noexcept override
    {
        interruptCalls++;
    }

    void stop() noexcept override
    {
        stopCalls++;
    }

    ClientTransport::StartResult nextStartResult {true, 0};
    int startCalls = 0;
    int interruptCalls = 0;
    int stopCalls = 0;
};

class FakeConnectivityAgent final : public Connectivity::IConnectivityAgent
{
public:
    Connectivity::SelectedPath selectPath() override
    {
        selectCalls++;
        return selectedPath;
    }

    Connectivity::SelectedPath selectedPath;
    int selectCalls = 0;
};

class FakeVideoReceiver final : public VideoReceiver::IVideoReceiver
{
public:
    VideoReceiver::Mode mode() const noexcept override
    {
        return VideoReceiver::Mode::GameStream;
    }

    VideoReceiver::Capabilities capabilities() const noexcept override
    {
        return configuredCapabilities;
    }

    void configure(
            VideoReceiver::Capabilities capabilities) noexcept override
    {
        configureCalls++;
        configuredCapabilities = capabilities;
    }

    VideoReceiver::Capabilities configuredCapabilities;
    int configureCalls = 0;
};

class FakeAudioReceiver final : public AudioReceiver::IAudioReceiver
{
public:
    AudioReceiver::Mode mode() const noexcept override
    {
        return AudioReceiver::Mode::GameStream;
    }

    AudioReceiver::Capabilities capabilities() const noexcept override
    {
        return receiverCapabilities;
    }

    bool testConfiguration(
            const AudioReceiver::Configuration& configuration) override
    {
        testCalls++;
        testedConfiguration = configuration;
        return nextTestResult;
    }

    void setBufferingProfile(
            AudioBuffer::Profile profile) noexcept override
    {
        profileCalls++;
        activeProfile = profile;
    }

    AudioBuffer::Profile
    bufferingProfile() const noexcept override
    {
        return activeProfile;
    }

    void setMuted(bool muted) noexcept override
    {
        muteCalls++;
        mutedState = muted;
    }

    bool isMuted() const noexcept override
    {
        return mutedState;
    }

    AudioReceiver::Capabilities receiverCapabilities {
        false,
        false,
        true,
    };
    AudioReceiver::Configuration testedConfiguration;
    AudioBuffer::Profile activeProfile =
            AudioBuffer::Profile::Default;
    bool nextTestResult = true;
    bool mutedState = false;
    int testCalls = 0;
    int profileCalls = 0;
    int muteCalls = 0;
};

class FakeInputSender final : public InputSender::IInputSender
{
public:
    InputSender::Mode mode() const noexcept override
    {
        return InputSender::Mode::GameStream;
    }

    InputSender::Capabilities capabilities() const noexcept override
    {
        return senderCapabilities;
    }

    InputSender::SendResult sendKeyboard(
            const InputSender::KeyboardEvent& event) noexcept override
    {
        keyboardCalls++;
        keyboardEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendText(
            const InputSender::TextEvent& event) noexcept override
    {
        textCalls++;
        textEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendMouseButton(
            const InputSender::MouseButtonEvent& event) noexcept override
    {
        mouseButtonCalls++;
        mouseButtonEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendRelativePointer(
            const InputSender::RelativePointerEvent& event) noexcept override
    {
        relativePointerCalls++;
        relativePointerEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendAbsolutePointer(
            const InputSender::AbsolutePointerEvent& event) noexcept override
    {
        absolutePointerCalls++;
        absolutePointerEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendScroll(
            const InputSender::ScrollEvent& event) noexcept override
    {
        scrollCalls++;
        scrollEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendTouch(
            const InputSender::TouchEvent& event) noexcept override
    {
        touchCalls++;
        touchEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendPen(
            const InputSender::PenEvent& event) noexcept override
    {
        penCalls++;
        penEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendControllerState(
            const InputSender::ControllerStateEvent& event) noexcept override
    {
        controllerStateCalls++;
        controllerStateEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendControllerArrival(
            const InputSender::ControllerArrivalEvent& event) noexcept override
    {
        controllerArrivalCalls++;
        controllerArrivalEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendControllerTouch(
            const InputSender::ControllerTouchEvent& event) noexcept override
    {
        controllerTouchCalls++;
        controllerTouchEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendControllerMotion(
            const InputSender::ControllerMotionEvent& event) noexcept override
    {
        controllerMotionCalls++;
        controllerMotionEvent = event;
        return nextResult;
    }

    InputSender::SendResult sendControllerBattery(
            const InputSender::ControllerBatteryEvent& event) noexcept override
    {
        controllerBatteryCalls++;
        controllerBatteryEvent = event;
        return nextResult;
    }

    InputSender::Capabilities senderCapabilities {
        true,
        true,
    };
    InputSender::SendResult nextResult {
        InputSender::SendStatus::Accepted,
        0,
    };
    InputSender::KeyboardEvent keyboardEvent;
    InputSender::TextEvent textEvent;
    InputSender::MouseButtonEvent mouseButtonEvent;
    InputSender::RelativePointerEvent relativePointerEvent;
    InputSender::AbsolutePointerEvent absolutePointerEvent;
    InputSender::ScrollEvent scrollEvent;
    InputSender::TouchEvent touchEvent;
    InputSender::PenEvent penEvent;
    InputSender::ControllerStateEvent controllerStateEvent;
    InputSender::ControllerArrivalEvent controllerArrivalEvent;
    InputSender::ControllerTouchEvent controllerTouchEvent;
    InputSender::ControllerMotionEvent controllerMotionEvent;
    InputSender::ControllerBatteryEvent controllerBatteryEvent;
    int keyboardCalls = 0;
    int textCalls = 0;
    int mouseButtonCalls = 0;
    int relativePointerCalls = 0;
    int absolutePointerCalls = 0;
    int scrollCalls = 0;
    int touchCalls = 0;
    int penCalls = 0;
    int controllerStateCalls = 0;
    int controllerArrivalCalls = 0;
    int controllerTouchCalls = 0;
    int controllerMotionCalls = 0;
    int controllerBatteryCalls = 0;
};

class FakeDecoder final : public Decoder::IDecoder
{
public:
    const Decoder::Properties& properties() const noexcept override
    {
        return decoderProperties;
    }

    void renderFrameOnMainThread() override
    {
        renderCalls++;
    }

    void setHdrMode(bool enabled) override
    {
        hdrCalls++;
        hdrEnabled = enabled;
    }

    bool notifyWindowChanged(
            const Decoder::WindowStateChange& change) override
    {
        windowChangeCalls++;
        lastWindowChange = change;
        return handlesWindowChange;
    }

    Decoder::Properties decoderProperties;
    Decoder::WindowStateChange lastWindowChange;
    bool hdrEnabled = false;
    bool handlesWindowChange = true;
    int renderCalls = 0;
    int hdrCalls = 0;
    int windowChangeCalls = 0;
};

struct FakeNativeFrame {
    bool released = false;
};

void releaseFakeNativeFrame(void* nativeHandle) noexcept
{
    static_cast<FakeNativeFrame*>(nativeHandle)->released = true;
}

class FakeRenderScheduler final
        : public RenderScheduler::IRenderScheduler
{
public:
    bool initialize(
            const RenderScheduler::Configuration& configuration) override
    {
        initializeCalls++;
        lastConfiguration = configuration;
        return initializeResult;
    }

    RenderScheduler::Capabilities capabilities()
            const noexcept override
    {
        return schedulerCapabilities;
    }

    void submitFrame(
            RenderScheduler::DecodedFrame frame) override
    {
        submitCalls++;
        lastFrameTiming = frame.timing();
        pendingFrame = std::move(frame);
    }

    void renderOnMainThread() override
    {
        renderCalls++;
    }

    void updateNetworkConditions(
            RenderScheduler::NetworkConditions conditions) noexcept override
    {
        networkUpdateCalls++;
        lastNetworkConditions = conditions;
    }

    RenderScheduler::PresentationStatus status() const override
    {
        return schedulerStatus;
    }

    void releasePendingFrame()
    {
        pendingFrame.reset();
    }

    RenderScheduler::Configuration lastConfiguration;
    RenderScheduler::NetworkConditions lastNetworkConditions;
    RenderScheduler::FrameTiming lastFrameTiming;
    RenderScheduler::Capabilities schedulerCapabilities;
    RenderScheduler::PresentationStatus schedulerStatus;
    RenderScheduler::DecodedFrame pendingFrame;
    bool initializeResult = true;
    int initializeCalls = 0;
    int submitCalls = 0;
    int renderCalls = 0;
    int networkUpdateCalls = 0;
};

class FakeSessionTelemetry final
        : public SessionTelemetry::ISessionTelemetry
{
public:
    SessionTelemetry::Mode mode() const noexcept override
    {
        return SessionTelemetry::Mode::LegacyLocal;
    }

    bool frameTracingEnabled() const noexcept override
    {
        return traceEnabled;
    }

    void publishFrameTrace(
            const PipelineTelemetry::FrameTrace& trace) override
    {
        frameTraceCalls++;
        lastFrameTrace = trace;
    }

    void publishStageStarted(
            const SessionTelemetry::StageEvent& event) override
    {
        stageStartedCalls++;
        lastStage = event;
    }

    void publishStageFailed(
            const SessionTelemetry::StageFailure& event) override
    {
        stageFailedCalls++;
        lastStageFailure = event;
    }

    void publishConnectionQuality(
            const SessionTelemetry::ConnectionQualityEvent& event) override
    {
        connectionQualityCalls++;
        lastConnectionQuality = event;
    }

    void publishTermination(
            const SessionTelemetry::TerminationEvent& event) override
    {
        terminationCalls++;
        lastTermination = event;
    }

    void publishVideoWindow(
            const SessionTelemetry::VideoWindowReport& report) override
    {
        videoWindowCalls++;
        lastVideoWindow = report;
    }

    void publishVideoSessionSummary(
            const SessionTelemetry::VideoSessionSummary& summary) override
    {
        videoSummaryCalls++;
        lastVideoSummary = summary;
    }

    void publishAudioWindow(
            const SessionTelemetry::AudioWindowReport& report) override
    {
        audioWindowCalls++;
        lastAudioWindow = report;
    }

    void publishAudioSessionSummary(
            const SessionTelemetry::AudioSessionSummary& summary) override
    {
        audioSummaryCalls++;
        lastAudioSummary = summary;
    }

    bool traceEnabled = true;
    PipelineTelemetry::FrameTrace lastFrameTrace;
    SessionTelemetry::StageEvent lastStage;
    SessionTelemetry::StageFailure lastStageFailure;
    SessionTelemetry::ConnectionQualityEvent
            lastConnectionQuality;
    SessionTelemetry::TerminationEvent lastTermination;
    SessionTelemetry::VideoWindowReport lastVideoWindow;
    SessionTelemetry::VideoSessionSummary lastVideoSummary;
    SessionTelemetry::AudioWindowReport lastAudioWindow;
    SessionTelemetry::AudioSessionSummary lastAudioSummary;
    int frameTraceCalls = 0;
    int stageStartedCalls = 0;
    int stageFailedCalls = 0;
    int connectionQualityCalls = 0;
    int terminationCalls = 0;
    int videoWindowCalls = 0;
    int videoSummaryCalls = 0;
    int audioWindowCalls = 0;
    int audioSummaryCalls = 0;
};

} // namespace

class ClientLogicTest : public QObject
{
    Q_OBJECT

private slots:
    void sessionTelemetrySupportsFakeTypedSink()
    {
        FakeSessionTelemetry fake;
        SessionTelemetry::ISessionTelemetry& telemetry = fake;
        QCOMPARE(
            static_cast<int>(telemetry.mode()),
            static_cast<int>(
                SessionTelemetry::Mode::LegacyLocal));
        QVERIFY(telemetry.frameTracingEnabled());

        telemetry.publishStageStarted({
            3,
            QStringLiteral("Video stream"),
        });
        telemetry.publishStageFailed({
            {4, QStringLiteral("Control stream")},
            -7,
            QStringLiteral("47984"),
        });
        telemetry.publishConnectionQuality({
            1,
            SessionTelemetry::ConnectionQuality::Poor,
            25000,
            true,
            false,
        });
        telemetry.publishTermination({
            -42,
            true,
            QStringLiteral("47998"),
        });

        const PipelineTelemetry::FrameTrace trace {
            99,
            100,
            110,
            120,
            130,
            140,
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::
                RenderBacklog,
        };
        telemetry.publishFrameTrace(trace);

        SessionTelemetry::VideoWindowReport window;
        window.completedWindow.receivedFrames = 60;
        window.displayWindow.renderedFrames = 118;
        window.stream.codec =
                SessionTelemetry::VideoCodec::Av1;
        window.stream.tenBit = true;
        window.stream.targetFrameRate = 60;
        window.stream.presentationPath =
                QStringLiteral("fake-vsync");
        telemetry.publishVideoWindow(window);

        SessionTelemetry::VideoSessionSummary summary;
        summary.aggregate.totalFrames = 3600;
        summary.stream = window.stream;
        telemetry.publishVideoSessionSummary(summary);

        SessionTelemetry::AudioWindowReport audioWindow;
        audioWindow.completedWindow.pipeline.decodedPackets = 200;
        audioWindow.completedWindow.renderer
            .currentQueuedDurationMs = 15;
        audioWindow.completedWindow
            .estimatedPresentationDelayMs = 25;
        audioWindow.stream.sampleRate = 48000;
        audioWindow.stream.samplesPerFrame = 240;
        audioWindow.stream.channelCount = 2;
        audioWindow.stream.rendererName =
                QStringLiteral("fake-audio");
        audioWindow.stream.bufferingProfileName =
                QStringLiteral("Low latency");
        telemetry.publishAudioWindow(audioWindow);

        SessionTelemetry::AudioSessionSummary audioSummary;
        audioSummary.aggregate.pipeline.callbackPackets = 12000;
        audioSummary.stream = audioWindow.stream;
        telemetry.publishAudioSessionSummary(audioSummary);

        QCOMPARE(fake.stageStartedCalls, 1);
        QCOMPARE(fake.lastStage.nativeStage, 3);
        QCOMPARE(fake.lastStage.name,
                 QStringLiteral("Video stream"));
        QCOMPARE(fake.stageFailedCalls, 1);
        QCOMPARE(fake.lastStageFailure.errorCode, -7);
        QCOMPARE(fake.lastStageFailure.failingPorts,
                 QStringLiteral("47984"));
        QCOMPARE(fake.connectionQualityCalls, 1);
        QCOMPARE(
            static_cast<int>(
                fake.lastConnectionQuality.quality),
            static_cast<int>(
                SessionTelemetry::ConnectionQuality::Poor));
        QCOMPARE(fake.lastConnectionQuality.bitrateKbps,
                 uint32_t {25000});
        QCOMPARE(fake.terminationCalls, 1);
        QVERIFY(fake.lastTermination.unexpected);
        QCOMPARE(fake.frameTraceCalls, 1);
        QCOMPARE(fake.lastFrameTrace.frameNumber, int64_t {99});
        QCOMPARE(fake.videoWindowCalls, 1);
        QCOMPARE(fake.lastVideoWindow.displayWindow.renderedFrames,
                 uint32_t {118});
        QCOMPARE(fake.lastVideoWindow.stream.presentationPath,
                 QStringLiteral("fake-vsync"));
        QCOMPARE(fake.videoSummaryCalls, 1);
        QCOMPARE(fake.lastVideoSummary.aggregate.totalFrames,
                 uint32_t {3600});
        QCOMPARE(fake.audioWindowCalls, 1);
        QCOMPARE(
            fake.lastAudioWindow.completedWindow
                .pipeline.decodedPackets,
            uint64_t {200});
        QCOMPARE(
            fake.lastAudioWindow.stream.packetDurationMs(),
            uint32_t {5});
        QCOMPARE(
            fake.lastAudioWindow.stream.bufferingProfileName,
            QStringLiteral("Low latency"));
        QVERIFY(!fake.lastAudioWindow.avSyncOffsetAvailable);
        QCOMPARE(fake.audioSummaryCalls, 1);
        QCOMPARE(
            fake.lastAudioSummary.aggregate.pipeline
                .callbackPackets,
            uint64_t {12000});

        SessionTelemetry::ISessionTelemetry& nullTelemetry =
                SessionTelemetry::nullSessionTelemetry();
        QCOMPARE(
            static_cast<int>(nullTelemetry.mode()),
            static_cast<int>(SessionTelemetry::Mode::Null));
        QVERIFY(!nullTelemetry.frameTracingEnabled());
    }

    void audioBufferPolicyPreservesLegacyDefaults()
    {
        const AudioBuffer::StreamFormat format {
            48000,
            240,
            2,
        };
        const AudioBuffer::Policy sdl =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::Default,
                    AudioBuffer::Backend::Sdl,
                    format);
        QCOMPARE(sdl.deviceBufferSamples, uint32_t {720});
        QCOMPARE(sdl.playbackQueueLimitMs, uint32_t {50});
        QCOMPARE(sdl.upstreamBackpressureLimitMs,
                 uint32_t {30});
        QCOMPARE(sdl.initialPrebufferMs,
                 uint32_t {10});
        QCOMPARE(sdl.underrunRecoveryStepMs,
                 uint32_t {5});
        QCOMPARE(sdl.maximumPrebufferMs,
                 uint32_t {30});

        const AudioBuffer::Policy legacyNonOpusRate =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::Default,
                    AudioBuffer::Backend::Sdl,
                    {
                        44100,
                        120,
                        2,
                    });
        QCOMPARE(
            legacyNonOpusRate.deviceBufferSamples,
            uint32_t {480});

        const AudioBuffer::Policy slAudio =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::Default,
                    AudioBuffer::Backend::SlAudio,
                    {
                        48000,
                        240,
                        6,
                    });
        QCOMPARE(slAudio.deviceBufferSamples, uint32_t {0});
        QCOMPARE(slAudio.playbackQueueLimitMs,
                 uint32_t {120});
        QCOMPARE(slAudio.upstreamBackpressureLimitMs,
                 uint32_t {120});
    }

    void audioBufferProfilesAreDeterministicAndBounded()
    {
        const AudioBuffer::StreamFormat format {
            48000,
            240,
            2,
        };
        const AudioBuffer::Policy lowLatency =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::LowLatency,
                    AudioBuffer::Backend::Sdl,
                    format);
        QCOMPARE(lowLatency.deviceBufferSamples,
                 uint32_t {480});
        QCOMPARE(lowLatency.playbackQueueLimitMs,
                 uint32_t {30});
        QCOMPARE(lowLatency.upstreamBackpressureLimitMs,
                 uint32_t {20});

        const AudioBuffer::Policy smooth =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::SmoothPlayback,
                    AudioBuffer::Backend::Sdl,
                    format);
        QCOMPARE(smooth.deviceBufferSamples, uint32_t {960});
        QCOMPARE(smooth.playbackQueueLimitMs, uint32_t {80});
        QCOMPARE(smooth.upstreamBackpressureLimitMs,
                 uint32_t {50});
        QCOMPARE(smooth.initialPrebufferMs,
                 uint32_t {20});
        QCOMPARE(smooth.underrunRecoveryStepMs,
                 uint32_t {10});
        QCOMPARE(smooth.maximumPrebufferMs,
                 uint32_t {50});
        QCOMPARE(
            AudioBuffer::nextRecoveryPrebufferMs(
                smooth,
                smooth.initialPrebufferMs),
            uint32_t {30});
        QCOMPARE(
            AudioBuffer::nextRecoveryPrebufferMs(
                smooth,
                uint32_t {40}),
            uint32_t {50});
        QCOMPARE(
            AudioBuffer::nextRecoveryPrebufferMs(
                smooth,
                uint32_t {50}),
            uint32_t {50});

        const AudioBuffer::Policy slAudioLow =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::LowLatency,
                    AudioBuffer::Backend::SlAudio,
                    {
                        48000,
                        240,
                        8,
                    });
        QCOMPARE(slAudioLow.playbackQueueLimitMs,
                 uint32_t {80});

        const AudioBuffer::Policy invalid =
                AudioBuffer::calculate(
                    AudioBuffer::Profile::Default,
                    AudioBuffer::Backend::Sdl,
                    {});
        QCOMPARE(invalid.deviceBufferSamples, uint32_t {0});
        QCOMPARE(invalid.playbackQueueLimitMs, uint32_t {0});
        QCOMPARE(
            AudioBuffer::nextRecoveryPrebufferMs(
                invalid,
                uint32_t {20}),
            uint32_t {0});
        QCOMPARE(
            AudioBuffer::drainPlaybackBufferUs(
                uint64_t {20000},
                uint64_t {5000},
                uint64_t {0}),
            uint64_t {15000});
        QCOMPARE(
            AudioBuffer::drainPlaybackBufferUs(
                uint64_t {20000},
                uint64_t {25000},
                uint64_t {0}),
            uint64_t {0});
        QCOMPARE(
            AudioBuffer::drainPlaybackBufferUs(
                uint64_t {20000},
                uint64_t {25000},
                uint64_t {7000}),
            uint64_t {7000});
        QCOMPARE(
            QString::fromLatin1(
                AudioBuffer::profileName(
                    AudioBuffer::Profile::SmoothPlayback)),
            QStringLiteral("Smooth playback"));
    }

    void audioTelemetryComputesDeltasAndDiagnosesUnderrun()
    {
        const AudioTelemetry::PipelineCounters previousPipeline {
            100,
            98,
            1,
            1,
            0,
            0,
            0,
            0,
            0,
            0,
        };
        AudioTelemetry::PipelineCounters currentPipeline =
                previousPipeline;
        currentPipeline.callbackPackets += 200;
        currentPipeline.decodedPackets += 198;
        currentPipeline.concealedPackets += 2;

        const AudioTelemetry::PipelineCounters windowPipeline =
                AudioTelemetry::subtract(
                    currentPipeline,
                    previousPipeline);
        QCOMPARE(windowPipeline.callbackPackets, uint64_t {200});
        QCOMPARE(windowPipeline.decodedPackets, uint64_t {198});
        QCOMPARE(windowPipeline.concealedPackets, uint64_t {2});

        const AudioTelemetry::RendererMetrics previousRenderer {
            100,
            10000,
            1,
            0,
            0,
            500,
            1,
        };
        AudioTelemetry::RendererMetrics currentRenderer =
                previousRenderer;
        currentRenderer.submittedPackets += 195;
        currentRenderer.underruns += 1;
        currentRenderer.currentQueuedDurationMs = 12;
        currentRenderer.deviceBufferDurationMs = 10;
        currentRenderer.queueDepthObservable = true;
        currentRenderer.deviceRunning = true;

        AudioTelemetry::Snapshot snapshot;
        snapshot.pipeline = windowPipeline;
        snapshot.renderer = AudioTelemetry::subtract(
            currentRenderer,
            previousRenderer);
        snapshot.pendingNetworkDurationMs = 5;
        snapshot.estimatedPresentationDelayMs =
                snapshot.pendingNetworkDurationMs +
                snapshot.renderer.currentQueuedDurationMs +
                snapshot.renderer.deviceBufferDurationMs;

        QCOMPARE(snapshot.renderer.submittedPackets,
                 uint64_t {195});
        QCOMPARE(snapshot.renderer.underruns, uint64_t {1});
        QCOMPARE(snapshot.estimatedPresentationDelayMs,
                 uint32_t {27});
        const AudioTelemetry::Diagnosis diagnosis =
                AudioTelemetry::diagnose(snapshot);
        QCOMPARE(
            static_cast<int>(diagnosis.health),
            static_cast<int>(
                AudioTelemetry::Health::Underrunning));
        QVERIFY(diagnosis.keyMetric.contains(
            QStringLiteral("1")));
    }

    void audioTelemetryKeepsRendererTotalsAcrossReinitialization()
    {
        AudioTelemetry::RendererMetrics aggregate;
        AudioTelemetry::RendererMetrics first;
        first.submittedPackets = 100;
        first.submittedBytes = 48000;
        first.underruns = 2;
        first.highestObservedQueueDurationMs = 40;
        AudioTelemetry::accumulate(first, aggregate);

        AudioTelemetry::RendererMetrics second;
        second.submittedPackets = 50;
        second.submittedBytes = 24000;
        second.backpressureSkippedPackets = 3;
        second.highestObservedQueueDurationMs = 25;
        AudioTelemetry::accumulate(second, aggregate);

        QCOMPARE(aggregate.submittedPackets, uint64_t {150});
        QCOMPARE(aggregate.submittedBytes, uint64_t {72000});
        QCOMPARE(aggregate.underruns, uint64_t {2});
        QCOMPARE(aggregate.backpressureSkippedPackets,
                 uint64_t {3});
        QCOMPARE(aggregate.highestObservedQueueDurationMs,
                 uint32_t {40});
    }

    void diagnosticsAcceptsNormalVsyncTime()
    {
        const Diagnostics::Diagnosis diagnosis = Diagnostics::diagnose(healthyStats(), 60);
        QCOMPARE(diagnosis.dominant, Diagnostics::BOTTLENECK_NONE);
        QVERIFY(!diagnosis.summary.isEmpty());
    }

    void diagnosticsClassifiesDecode()
    {
        VIDEO_STATS stats = healthyStats();
        stats.totalDecodeTimeUs = 60ULL * 12000;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_DECODE);
    }

    void diagnosticsClassifiesNetworkLoss()
    {
        VIDEO_STATS stats = healthyStats();
        stats.networkDroppedFrames = 3;
        stats.totalFrames = 100;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_NETWORK);
    }

    void diagnosticsAttributesJitteredPacerDropsToNetwork()
    {
        VIDEO_STATS stats = healthyStats();
        stats.pacerDroppedFrames = 3;
        stats.lastRttVariance = 15;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_NETWORK);
    }

    void diagnosticsAttributesLocalBacklogToPresentation()
    {
        VIDEO_STATS stats = healthyStats();
        stats.totalPacerTimeUs = 60ULL * 40000;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_RENDER);
    }

    void diagnosticsClassifiesLateVsync()
    {
        VIDEO_STATS stats = healthyStats();
        stats.vsyncIntervals = 60;
        stats.lateVsyncIntervals = 3;
        const Diagnostics::Diagnosis diagnosis = Diagnostics::diagnose(stats, 60);
        QCOMPARE(diagnosis.dominant, Diagnostics::BOTTLENECK_RENDER);
        QVERIFY(diagnosis.keyMetric.contains(QStringLiteral("V-Sync")));
    }

    void diagnosticsClassifiesHost()
    {
        VIDEO_STATS stats = healthyStats();
        stats.framesWithHostProcessingLatency = 60;
        stats.totalHostProcessingLatency = 60 * 400;
        QCOMPARE(Diagnostics::diagnose(stats, 60).dominant,
                 Diagnostics::BOTTLENECK_HOST);
    }

    void diagnosticsRequiresEnoughFrames()
    {
        VIDEO_STATS stats = healthyStats();
        stats.renderedFrames = 5;
        const Diagnostics::Diagnosis diagnosis = Diagnostics::diagnose(stats, 60);
        QCOMPARE(diagnosis.dominant, Diagnostics::BOTTLENECK_NONE);
        QVERIFY(diagnosis.summary.isEmpty());
    }

    void spikeHistoryCountsEvents()
    {
        Diagnostics::SpikeHistory history;
        Diagnostics::Diagnosis decode;
        decode.dominant = Diagnostics::BOTTLENECK_DECODE;
        Diagnostics::Diagnosis healthy;
        Diagnostics::Diagnosis network;
        network.dominant = Diagnostics::BOTTLENECK_NETWORK;
        network.summary = QStringLiteral("Network problem");
        network.keyMetric = QStringLiteral("15 ms");

        history.record(decode);
        history.record(decode);
        history.record(healthy);
        history.record(network);

        QCOMPARE(history.spikeCount(), 2);
        QVERIFY(history.summarize().contains(QStringLiteral("2")));
        const QString overlay = Diagnostics::formatOverlayText(network, history);
        QVERIFY(overlay.contains(QStringLiteral("Diagnosis:")));
        QVERIFY(overlay.contains(QStringLiteral("15 ms")));
        QVERIFY(overlay.contains(QStringLiteral("2")));
    }

    void pipelineLatencyPercentilesAreDeterministic()
    {
        PipelineTelemetry::LatencyHistogram histogram;
        for (int milliseconds = 1; milliseconds <= 100; milliseconds++) {
            PipelineTelemetry::recordLatency(
                    histogram,
                    std::chrono::milliseconds {milliseconds});
        }

        const auto percentiles =
                PipelineTelemetry::summarizeLatency(histogram);
        QCOMPARE(percentiles.p50, std::chrono::milliseconds {50});
        QCOMPARE(percentiles.p95, std::chrono::milliseconds {95});
        QCOMPARE(percentiles.p99, std::chrono::milliseconds {99});
        QCOMPARE(percentiles.sampleCount, uint64_t {100});
    }

    void pipelineLatencyHistogramsMergeAcrossWindows()
    {
        PipelineTelemetry::LatencyHistogram first;
        PipelineTelemetry::LatencyHistogram second;
        for (int milliseconds = 1; milliseconds <= 50; milliseconds++) {
            PipelineTelemetry::recordLatency(
                    first,
                    std::chrono::milliseconds {milliseconds});
        }
        for (int milliseconds = 51; milliseconds <= 100; milliseconds++) {
            PipelineTelemetry::recordLatency(
                    second,
                    std::chrono::milliseconds {milliseconds});
        }

        PipelineTelemetry::mergeLatency(second, first);
        const auto percentiles =
                PipelineTelemetry::summarizeLatency(first);
        QCOMPARE(percentiles.p50, std::chrono::milliseconds {50});
        QCOMPARE(percentiles.p95, std::chrono::milliseconds {95});
        QCOMPARE(percentiles.p99, std::chrono::milliseconds {99});
        QCOMPARE(percentiles.sampleCount, uint64_t {100});
    }

    void pipelineLatencyCapsOutliers()
    {
        PipelineTelemetry::LatencyHistogram histogram;
        PipelineTelemetry::recordLatency(
                histogram,
                std::chrono::seconds {1});

        const auto percentiles =
                PipelineTelemetry::summarizeLatency(histogram);
        QCOMPARE(percentiles.p99,
                 PipelineTelemetry::MAX_REPORTED_LATENCY);
    }

    void rtpQueueDepthPercentilesAreBoundedAndDeterministic()
    {
        PipelineTelemetry::QueueDepthHistogram histogram;
        for (uint32_t depth = 1; depth <= 100; depth++) {
            PipelineTelemetry::recordQueueDepth(histogram, depth);
        }

        const auto percentiles =
                PipelineTelemetry::summarizeQueueDepth(histogram);
        QCOMPARE(percentiles.p50, uint32_t {64});
        QCOMPARE(percentiles.p95, uint32_t {128});
        QCOMPARE(percentiles.p99, uint32_t {128});
        QCOMPARE(percentiles.max, uint32_t {100});
        QCOMPARE(percentiles.sampleCount, uint64_t {100});
    }

    void rtpQueueDepthHistogramsMergeAcrossWindows()
    {
        PipelineTelemetry::QueueDepthHistogram first;
        PipelineTelemetry::QueueDepthHistogram second;
        for (uint32_t depth = 1; depth <= 50; depth++) {
            PipelineTelemetry::recordQueueDepth(first, depth);
        }
        for (uint32_t depth = 51; depth <= 100; depth++) {
            PipelineTelemetry::recordQueueDepth(second, depth);
        }

        PipelineTelemetry::mergeQueueDepth(second, first);
        const auto percentiles =
                PipelineTelemetry::summarizeQueueDepth(first);
        QCOMPARE(percentiles.p50, uint32_t {64});
        QCOMPARE(percentiles.p95, uint32_t {128});
        QCOMPARE(percentiles.p99, uint32_t {128});
        QCOMPARE(percentiles.max, uint32_t {100});
        QCOMPARE(percentiles.sampleCount, uint64_t {100});
    }

    void frameTimelineCorrelatesTerminalFrame()
    {
        PipelineTelemetry::FrameTimeline timeline;
        timeline.recordReceived(42, 100, 150);
        QVERIFY(timeline.recordDecoded(42, 250));

        const auto trace = timeline.recordTerminal(
                42,
                300,
                325,
                PipelineTelemetry::FrameOutcome::Presented,
                PipelineTelemetry::FrameTerminalReason::Presented);
        QVERIFY(trace.has_value());
        QCOMPARE(trace->frameNumber, int64_t {42});
        QCOMPARE(trace->receiveUs, uint64_t {100});
        QCOMPARE(trace->assembleUs, uint64_t {150});
        QCOMPARE(trace->decodeUs, uint64_t {250});
        QCOMPARE(trace->presentStartUs, uint64_t {300});
        QCOMPARE(trace->terminalUs, uint64_t {325});
        QCOMPARE(trace->outcome, PipelineTelemetry::FrameOutcome::Presented);

        QVERIFY(!timeline.recordTerminal(
                42,
                400,
                425,
                PipelineTelemetry::FrameOutcome::Dropped,
                PipelineTelemetry::FrameTerminalReason::Shutdown));
    }

    void frameTimelineRejectsEvictedAndUnknownFrames()
    {
        PipelineTelemetry::FrameTimeline timeline;
        timeline.recordReceived(1, 10, 20);
        timeline.recordReceived(
                1 + PipelineTelemetry::FrameTimeline::MAX_ACTIVE_FRAMES,
                30,
                40);

        QVERIFY(!timeline.recordDecoded(1, 50));
        QVERIFY(!timeline.recordTerminal(
                1,
                0,
                60,
                PipelineTelemetry::FrameOutcome::Dropped,
                PipelineTelemetry::FrameTerminalReason::Shutdown));
        QVERIFY(timeline.recordDecoded(
                1 + PipelineTelemetry::FrameTimeline::MAX_ACTIVE_FRAMES,
                50));
    }

    void frameIdTagRoundTripsWithoutAllocation()
    {
        const auto tag = PipelineTelemetry::frameIdTag(123456);
        QVERIFY(tag != nullptr);
        const auto frameNumber = PipelineTelemetry::frameIdFromTag(tag);
        QVERIFY(frameNumber.has_value());
        QCOMPARE(*frameNumber, int64_t {123456});
        QVERIFY(!PipelineTelemetry::frameIdFromTag(nullptr));
        QCOMPARE(PipelineTelemetry::frameIdTag(-1), nullptr);
    }

    void hostProtocolSerializesTypedPrepareRequest()
    {
        HostProtocol::SessionRequest request;
        request.clientName = QStringLiteral("Hestia");
        request.clientVersion = QStringLiteral("1.2.3");
        request.clientPlatform = QStringLiteral("linux");
        request.displayWidth = 2560;
        request.displayHeight = 1440;
        request.displayRefreshRate = 165;
        request.displayHdr = true;
        request.streamWidth = 1920;
        request.streamHeight = 1080;
        request.streamFrameRate = 120;
        request.streamBitrateKbps = 50000;
        request.streamCodec = HostProtocol::VideoCodec::Av1;
        request.streamScaleFactor = 125;
        request.virtualDisplay = true;
        request.recoverPhysicalMonitor = true;
        request.applicationId = 42;
        request.launchMode = HostProtocol::LaunchMode::Gamescope;
        request.isolation = HostProtocol::SessionIsolation::Required;

        const QJsonObject payload =
                HostProtocol::toHestiaPreparePayload(request);
        const QJsonObject client = payload.value("client").toObject();
        const QJsonObject stream = payload.value("stream").toObject();
        const QJsonObject virtualDisplay =
                payload.value("virtual_display").toObject();
        const QJsonObject app = payload.value("app").toObject();
        const QJsonObject session = payload.value("session").toObject();

        QCOMPARE(client.value("name").toString(), QStringLiteral("Hestia"));
        QCOMPARE(client.value("display_width").toInt(), 2560);
        QCOMPARE(client.value("refresh_rate").toInt(), 165);
        QVERIFY(client.value("hdr").toBool());
        QCOMPARE(stream.value("codec").toString(), QStringLiteral("av1"));
        QCOMPARE(stream.value("hdr_mode").toString(), QStringLiteral("hdr"));
        QCOMPARE(stream.value("scale_factor").toInt(), 125);
        QVERIFY(virtualDisplay.value("enabled").toBool());
        QVERIFY(virtualDisplay.value("recover_physical_monitor").toBool());
        QCOMPARE(app.value("id").toString(), QStringLiteral("42"));
        QCOMPARE(app.value("launch_mode").toString(),
                 QStringLiteral("gamescope"));
        QCOMPARE(session.value("isolation").toString(),
                 QStringLiteral("required"));

        request.isolation = HostProtocol::SessionIsolation::Unspecified;
        QVERIFY(!HostProtocol::toHestiaPreparePayload(request)
                         .contains("session"));
    }

    void hostProtocolSupportsFakeLifecycleAdapter()
    {
        FakeHostProtocol fake;
        HostProtocol::IHostProtocol& protocol = fake;

        HostProtocol::SessionRequest prepareRequest;
        prepareRequest.applicationId = 7;
        QString sessionId;
        QVERIFY(protocol.prepareSession(prepareRequest, &sessionId));
        QVERIFY(fake.prepareCalled);
        QCOMPARE(fake.preparedRequest.applicationId, 7);
        QCOMPARE(sessionId, QStringLiteral("fake-session"));

        HostProtocol::LaunchRequest launchRequest;
        launchRequest.action = HostProtocol::LaunchAction::Resume;
        launchRequest.applicationId = 7;
        launchRequest.width = 1920;
        launchRequest.height = 1080;
        launchRequest.frameRate = 120;
        launchRequest.codec = HostProtocol::VideoCodec::Hevc;
        launchRequest.tenBit = true;
        QCOMPARE(protocol.launchSession(launchRequest),
                 QStringLiteral("rtsp://fake/session"));
        QVERIFY(fake.launchCalled);
        QCOMPARE(fake.launchRequest.applicationId, 7);
        QCOMPARE(fake.launchRequest.frameRate, 120);
        QCOMPARE(static_cast<int>(fake.launchRequest.action),
                 static_cast<int>(HostProtocol::LaunchAction::Resume));

        QVERIFY(protocol.stopSession(sessionId));
        QVERIFY(fake.stopCalled);
        QCOMPARE(fake.stoppedSessionId, QStringLiteral("fake-session"));
    }

    void clientTransportSupportsFakeLifecycleAdapter()
    {
        FakeClientTransport fake;
        ClientTransport::IClientTransport& transport = fake;

        const ClientTransport::StartResult started = transport.start();
        QVERIFY(started);
        QCOMPARE(started.errorCode, 0);
        QCOMPARE(fake.startCalls, 1);

        transport.interrupt();
        transport.stop();
        QCOMPARE(fake.interruptCalls, 1);
        QCOMPARE(fake.stopCalls, 1);

        fake.nextStartResult = {false, -42};
        const ClientTransport::StartResult failed = transport.start();
        QVERIFY(!failed);
        QCOMPARE(failed.errorCode, -42);
        QCOMPARE(fake.startCalls, 2);
    }

    void connectivityAgentSupportsFakePathSelection()
    {
        FakeConnectivityAgent fake;
        fake.selectedPath = {
            QStringLiteral("192.0.2.10"),
            Connectivity::PathType::Vpn,
        };
        Connectivity::IConnectivityAgent& connectivity = fake;

        const Connectivity::SelectedPath selectedPath =
                connectivity.selectPath();
        QVERIFY(selectedPath.isUsable());
        QCOMPARE(selectedPath.address, QStringLiteral("192.0.2.10"));
        QCOMPARE(selectedPath.type, Connectivity::PathType::Vpn);
        QCOMPARE(fake.selectCalls, 1);
    }

    void videoReceiverSupportsFakeCapabilityConfiguration()
    {
        FakeVideoReceiver fake;
        VideoReceiver::IVideoReceiver& receiver = fake;
        VideoReceiver::Capabilities capabilities;
        capabilities.directSubmit = true;
        capabilities.referenceFrameInvalidationHevc = true;
        capabilities.slicesPerFrame = 4;

        receiver.configure(capabilities);

        QCOMPARE(receiver.mode(), VideoReceiver::Mode::GameStream);
        QCOMPARE(fake.configureCalls, 1);
        const VideoReceiver::Capabilities configured =
                receiver.capabilities();
        QVERIFY(configured.directSubmit);
        QVERIFY(configured.referenceFrameInvalidationHevc);
        QCOMPARE(configured.slicesPerFrame, uint8_t {4});
    }

    void audioReceiverSupportsFakeConfigurationAndMute()
    {
        FakeAudioReceiver fake;
        AudioReceiver::IAudioReceiver& receiver = fake;
        const AudioReceiver::Configuration configuration {
            6,
            0x3F,
        };

        QVERIFY(receiver.testConfiguration(configuration));
        QCOMPARE(receiver.mode(), AudioReceiver::Mode::GameStream);
        QCOMPARE(fake.testCalls, 1);
        QCOMPARE(fake.testedConfiguration.channelCount, uint8_t {6});
        QCOMPARE(fake.testedConfiguration.channelMask, uint32_t {0x3F});
        QVERIFY(receiver.capabilities().arbitraryPacketDuration);

        receiver.setBufferingProfile(
            AudioBuffer::Profile::LowLatency);
        QCOMPARE(
            receiver.bufferingProfile(),
            AudioBuffer::Profile::LowLatency);
        QCOMPARE(fake.profileCalls, 1);

        receiver.setMuted(true);
        QVERIFY(receiver.isMuted());
        QCOMPARE(fake.muteCalls, 1);
    }

    void inputSenderCarriesTypedMetadataAndState()
    {
        FakeInputSender fake;
        InputSender::IInputSender& sender = fake;
        const InputSender::EventMetadata critical {
            41,
            123456,
            7,
            false,
        };

        const InputSender::SendResult keyboardResult =
                sender.sendKeyboard({
                    critical,
                    0x41,
                    InputSender::KeyAction::Down,
                    static_cast<uint8_t>(
                        InputSender::KeyboardModifierControl |
                        InputSender::KeyboardModifierShift),
                    true,
                });
        QVERIFY(keyboardResult);
        QCOMPARE(fake.keyboardCalls, 1);
        QCOMPARE(fake.keyboardEvent.metadata.sequence, uint64_t {41});
        QCOMPARE(fake.keyboardEvent.metadata.timestampUs,
                 uint64_t {123456});
        QCOMPARE(fake.keyboardEvent.metadata.deviceId, uint32_t {7});
        QVERIFY(!fake.keyboardEvent.metadata.replaceable);
        QCOMPARE(fake.keyboardEvent.virtualKey, uint16_t {0x41});
        QCOMPARE(
                static_cast<int>(fake.keyboardEvent.action),
                static_cast<int>(InputSender::KeyAction::Down));
        QVERIFY(fake.keyboardEvent.nonNormalized);

        sender.sendRelativePointer({
            {42, 123500, 9, true},
            -12,
            8,
        });
        QCOMPARE(fake.relativePointerCalls, 1);
        QVERIFY(fake.relativePointerEvent.metadata.replaceable);
        QCOMPARE(fake.relativePointerEvent.deltaX, int32_t {-12});
        QCOMPARE(fake.relativePointerEvent.deltaY, int32_t {8});

        sender.sendTouch({
            {43, 123600, 3, false},
            InputSender::ContactEventType::Down,
            99,
            0.25f,
            0.75f,
            0.5f,
        });
        QCOMPARE(fake.touchCalls, 1);
        QCOMPARE(fake.touchEvent.pointerId, uint32_t {99});
        QCOMPARE(
                static_cast<int>(fake.touchEvent.type),
                static_cast<int>(
                    InputSender::ContactEventType::Down));

        sender.sendControllerState({
            {44, 123700, 2, false},
            2,
            0x0005,
            InputSender::ControllerButtonA |
                InputSender::ControllerButtonDpadRight,
            10,
            20,
            -100,
            200,
            -300,
            400,
        });
        QCOMPARE(fake.controllerStateCalls, 1);
        QCOMPARE(fake.controllerStateEvent.controllerId, uint16_t {2});
        QCOMPARE(fake.controllerStateEvent.activeControllerMask,
                 uint16_t {0x0005});
        QVERIFY(fake.controllerStateEvent.buttons &
                InputSender::ControllerButtonA);
        QVERIFY(fake.controllerStateEvent.buttons &
                InputSender::ControllerButtonDpadRight);
        QVERIFY(sender.capabilities().nativePenTouch);
        QVERIFY(sender.capabilities().controllerTouch);
        QCOMPARE(sender.mode(), InputSender::Mode::GameStream);
    }

    void decoderSupportsTypedPropertiesAndWindowChanges()
    {
        FakeDecoder fake;
        fake.decoderProperties.capabilities.hardwareAccelerated = true;
        fake.decoderProperties.capabilities.hdr = true;
        fake.decoderProperties.capabilities.directSubmit = true;
        fake.decoderProperties.capabilities.slicesPerFrame = 4;
        fake.decoderProperties.colorSpace = Decoder::ColorSpace::Rec2020;
        fake.decoderProperties.colorRange = Decoder::ColorRange::Full;
        fake.decoderProperties.maximumResolution = QSize {3840, 2160};
        Decoder::IDecoder& decoder = fake;

        const Decoder::Properties& properties = decoder.properties();
        QVERIFY(properties.capabilities.hardwareAccelerated);
        QVERIFY(properties.capabilities.hdr);
        QVERIFY(properties.capabilities.directSubmit);
        QCOMPARE(properties.capabilities.slicesPerFrame, uint8_t {4});
        QCOMPARE(static_cast<int>(properties.colorSpace),
                 static_cast<int>(Decoder::ColorSpace::Rec2020));
        QCOMPARE(static_cast<int>(properties.colorRange),
                 static_cast<int>(Decoder::ColorRange::Full));
        QCOMPARE(properties.maximumResolution, QSize(3840, 2160));

        decoder.renderFrameOnMainThread();
        decoder.setHdrMode(true);
        QCOMPARE(fake.renderCalls, 1);
        QCOMPARE(fake.hdrCalls, 1);
        QVERIFY(fake.hdrEnabled);

        Decoder::WindowStateChange change;
        change.nativeWindow = reinterpret_cast<void*>(quintptr {0x1234});
        change.sizeChanged = true;
        change.displayChanged = true;
        change.width = 2560;
        change.height = 1440;
        change.displayIndex = 2;
        QVERIFY(decoder.notifyWindowChanged(change));
        QCOMPARE(fake.windowChangeCalls, 1);
        QCOMPARE(fake.lastWindowChange.nativeWindow, change.nativeWindow);
        QVERIFY(fake.lastWindowChange.sizeChanged);
        QVERIFY(fake.lastWindowChange.displayChanged);
        QCOMPARE(fake.lastWindowChange.width, 2560);
        QCOMPARE(fake.lastWindowChange.height, 1440);
        QCOMPARE(fake.lastWindowChange.displayIndex, 2);
    }

    void renderSchedulerOwnsFramesAndExposesTypedStatus()
    {
        FakeRenderScheduler fake;
        fake.schedulerStatus.path =
                RenderScheduler::PresentationPath::SynchronizedSource;
        fake.schedulerStatus.pathName =
                QStringLiteral("fake-vsync");
        fake.schedulerStatus.displayRefreshRateMillihertz = 119880;
        fake.schedulerCapabilities.maximumOutstandingFrames = 5;
        RenderScheduler::IRenderScheduler& scheduler = fake;

        RenderScheduler::Configuration configuration;
        configuration.nativeWindow =
                reinterpret_cast<void*>(quintptr {0x5678});
        configuration.maximumFrameRate = 120;
        configuration.framePacing = true;
        QVERIFY(scheduler.initialize(configuration));
        QCOMPARE(fake.initializeCalls, 1);
        QCOMPARE(fake.lastConfiguration.nativeWindow,
                 configuration.nativeWindow);
        QCOMPARE(fake.lastConfiguration.maximumFrameRate, uint32_t {120});
        QVERIFY(fake.lastConfiguration.framePacing);
        QCOMPARE(scheduler.capabilities().maximumOutstandingFrames,
                 uint32_t {5});

        scheduler.updateNetworkConditions({8, 2});
        QCOMPARE(fake.networkUpdateCalls, 1);
        QCOMPARE(fake.lastNetworkConditions.roundTripTimeMs,
                 uint32_t {8});
        QCOMPARE(fake.lastNetworkConditions.roundTripTimeVarianceMs,
                 uint32_t {2});

        FakeNativeFrame nativeFrame;
        RenderScheduler::FrameTiming timing;
        timing.frameNumber = 42;
        timing.readyTimeUs = 1000;
        timing.sourcePresentationTimeUs = 16000;
        timing.presentationDeadlineUs = 17000;
        timing.rtpTimestamp = 90000;
        scheduler.submitFrame(RenderScheduler::DecodedFrame {
            &nativeFrame,
            releaseFakeNativeFrame,
            timing,
        });
        QCOMPARE(fake.submitCalls, 1);
        QVERIFY(!nativeFrame.released);
        QCOMPARE(fake.lastFrameTiming.frameNumber, int64_t {42});
        QCOMPARE(fake.lastFrameTiming.readyTimeUs, uint64_t {1000});
        QCOMPARE(fake.lastFrameTiming.sourcePresentationTimeUs,
                 uint64_t {16000});
        QCOMPARE(fake.lastFrameTiming.presentationDeadlineUs,
                 uint64_t {17000});
        QCOMPARE(fake.lastFrameTiming.rtpTimestamp, uint32_t {90000});

        scheduler.renderOnMainThread();
        QCOMPARE(fake.renderCalls, 1);

        const RenderScheduler::PresentationStatus status =
                scheduler.status();
        QCOMPARE(static_cast<int>(status.path),
                 static_cast<int>(
                     RenderScheduler::PresentationPath::
                         SynchronizedSource));
        QCOMPARE(status.pathName, QStringLiteral("fake-vsync"));
        QCOMPARE(status.displayRefreshRateMillihertz,
                 uint32_t {119880});

        fake.releasePendingFrame();
        QVERIFY(nativeFrame.released);
    }

    void portugueseDiagnosticsCatalog()
    {
        QTranslator translator;
        QVERIFY(translator.load(QStringLiteral("../app/languages/qml_pt_BR.qm")));
        QCoreApplication::installTranslator(&translator);

        QCOMPARE(QCoreApplication::translate("HestiaDiagnostics", "Diagnosis: %1"),
                 QStringLiteral("Diagnóstico: %1"));
        QCOMPARE(QCoreApplication::translate("HestiaDiagnostics",
                                             "%1% late V-Sync intervals"),
                 QStringLiteral("%1% de intervalos de V-Sync atrasados"));
        QCOMPARE(
            QCoreApplication::translate(
                "HestiaAudioDiagnostics",
                "The audio output buffer ran empty. Audio may stutter."),
            QStringLiteral(
                "O buffer de saída de áudio ficou vazio. O áudio pode apresentar falhas."));
        QCOMPARE(
            QCoreApplication::translate(
                "SettingsView",
                "Audio latency profile"),
            QStringLiteral(
                "Perfil de latência do áudio"));

        QCoreApplication::removeTranslator(&translator);
    }

    void fractionalRefreshRateNormalization()
    {
        QCOMPARE(PacingPolicy::displayRateMillihertzFromSdlRate(59), 59940);
        QCOMPARE(PacingPolicy::displayRateMillihertzFromSdlRate(119), 119880);
        QCOMPARE(PacingPolicy::displayRateMillihertzFromSdlRate(143), 143856);
        QCOMPARE(PacingPolicy::displayRateMillihertzFromSdlRate(120), 120000);
        QCOMPARE(PacingPolicy::displayRateMillihertzFromSdlRate(0), 60000);
        QCOMPARE(PacingPolicy::rendererPresentationRateMillihertz(60000, 30),
                 30000);
        QCOMPARE(PacingPolicy::rendererPresentationRateMillihertz(59940, 60),
                 59940);
    }

    void adaptiveQueueRaisesQuicklyForJitter()
    {
        PacingPolicy::AdaptiveQueueDepth queueDepth;
        queueDepth.configure(60000);
        QCOMPARE(queueDepth.targetDepth(), 2);

        for (int i = 0; i < 14; i++) {
            queueDepth.observeNetworkJitter(20);
        }
        QCOMPARE(queueDepth.targetDepth(), 2);

        queueDepth.observeNetworkJitter(20);
        QCOMPARE(queueDepth.targetDepth(), 3);
    }

    void adaptiveQueueRejectsTransientJitter()
    {
        PacingPolicy::AdaptiveQueueDepth queueDepth;
        queueDepth.configure(120000);

        for (int i = 0; i < 29; i++) {
            queueDepth.observeNetworkJitter(20);
        }
        queueDepth.observeNetworkJitter(6);
        QCOMPARE(queueDepth.targetDepth(), 2);
    }

    void adaptiveQueueLowersSlowlyOnStableNetwork()
    {
        PacingPolicy::AdaptiveQueueDepth queueDepth;
        queueDepth.configure(60000);

        for (int i = 0; i < 119; i++) {
            queueDepth.observeNetworkJitter(2);
        }
        QCOMPARE(queueDepth.targetDepth(), 2);

        queueDepth.observeNetworkJitter(2);
        QCOMPARE(queueDepth.targetDepth(), 1);
    }

    void adaptiveQueueUsesRendererPresentationRate()
    {
        PacingPolicy::AdaptiveQueueDepth queueDepth;
        queueDepth.configure(
                PacingPolicy::rendererPresentationRateMillihertz(120000, 30));

        for (int i = 0; i < 7; i++) {
            queueDepth.observeNetworkJitter(20);
        }
        QCOMPARE(queueDepth.targetDepth(), 2);

        queueDepth.observeNetworkJitter(20);
        QCOMPARE(queueDepth.targetDepth(), 3);
    }

    void qualityPresetUsesNativeMode()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_QUALITY, 3840, 2160, 120, false);
        QCOMPARE(preset.width, 3840);
        QCOMPARE(preset.height, 2160);
        QCOMPARE(preset.fps, 120);
        QCOMPARE(preset.bitrateKbps,
                 qRound(StreamingPreferences::getDefaultBitrate(3840, 2160, 120, false) * 1.25));
    }

    void fastPresetFitsUltrawideBounds()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_FAST, 3440, 1440, 144, false);
        QCOMPARE(preset.width, 1920);
        QVERIFY(preset.height <= 1080);
        QCOMPARE(preset.height % 2, 0);
        QCOMPARE(preset.fps, 60);
    }

    void batteryPresetPreservesHandheldAspect()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_BATTERY, 1280, 800, 90, false);
        QCOMPARE(preset.width, 1152);
        QCOMPARE(preset.height, 720);
        QCOMPARE(preset.fps, 30);
    }

    void presetFallsBackToSensibleDisplayMode()
    {
        const auto preset = StreamingPreferences::calculatePreset(
                StreamingPreferences::PRESET_BALANCED, 0, 0, 0, false);
        QCOMPARE(preset.width, 1920);
        QCOMPARE(preset.height, 1080);
        QCOMPARE(preset.fps, 60);
    }

    void capabilitiesAcceptCompatibleFutureProtocol()
    {
        QJsonObject response = validCapabilities();
        response.insert("hestia_protocol", 2);
        response.insert("max_client_protocol", 2);
        response.insert("future_root_field", true);

        QJsonObject features = response.value("features").toObject();
        features.insert("virtual_display_backend",
                        QJsonArray {"hermes_kms", "future_kms"});
        features.insert("future_feature", true);
        response.insert("features", features);

        QJsonObject limits = response.value("limits").toObject();
        limits.insert("supported_codecs",
                      QJsonArray {"h264", "hevc", "av1", "future_codec"});
        response.insert("limits", limits);

        HestiaCapabilities capabilities;
        QString error;
        QVERIFY2(HestiaCapabilities::fromJson(response, &capabilities, &error),
                 qPrintable(error));
        QVERIFY(capabilities.supportsProtocolV1);
        QCOMPARE(capabilities.hestiaProtocol, 2);
        QCOMPARE(capabilities.maxClientProtocol, 2);
        QCOMPARE(capabilities.features.virtualDisplayBackend,
                 QStringList {"hermes_kms"});
        QCOMPARE(capabilities.limits.supportedCodecs,
                 QStringList({"h264", "hevc", "av1"}));
    }

    void capabilitiesRejectIncompatibleProtocolRange()
    {
        QJsonObject response = validCapabilities();
        response.insert("hestia_protocol", 2);
        response.insert("min_client_protocol", 2);
        response.insert("max_client_protocol", 3);

        HestiaCapabilities capabilities;
        QString error;
        QVERIFY(!HestiaCapabilities::fromJson(response, &capabilities, &error));
        QVERIFY(error.contains(QStringLiteral("protocol v1")));
    }

    void capabilitiesPreferHostSelectedIsolationState()
    {
        QJsonObject response = validCapabilities();
        QJsonObject features = response.value("features").toObject();
        features.insert("multi_user_sessions", true);
        features.insert("hermes_kms_isolated_sessions", QJsonObject {
            {"supported", true},
            {"enabled", false},
            {"ready", false},
        });
        response.insert("features", features);

        HestiaCapabilities capabilities;
        QString error;
        QVERIFY2(HestiaCapabilities::fromJson(response, &capabilities, &error),
                 qPrintable(error));
        QVERIFY(capabilities.features.hasExplicitMultiUserSessionState);
        QVERIFY(!capabilities.features.multiUserSessions);
        QVERIFY(!capabilities.features.multiUserSessionsReady);
    }

    void preflightTreatsUnknownStatusAsWarning()
    {
        const QJsonObject response {
            {"ok", true},
            {"preflight", QJsonObject {
                {"ready", true},
                {"checks", QJsonArray {
                    QJsonObject {
                        {"id", "future-check"},
                        {"status", "degraded"},
                        {"message", "Future warning"},
                    },
                }},
            }},
        };

        HestiaPreflight preflight;
        QString error;
        QVERIFY2(HestiaPreflight::fromDiagnosticsJson(response, &preflight, &error),
                 qPrintable(error));
        QVERIFY(preflight.valid);
        QCOMPARE(preflight.checks.size(), 1);
        QVERIFY(preflight.checks.first().isWarn());
    }

    void hermesNegotiationClampsModeAndPresetBitrate()
    {
        HestiaLimits limits;
        limits.maxWidth = 1920;
        limits.maxHeight = 1080;
        limits.maxFps = 60;
        limits.supportedFps = {30, 60};

        const HestiaNegotiation::StreamMode requested {
            3840,
            2160,
            120,
            90000,
        };
        const auto result = HestiaNegotiation::negotiateStreamMode(
                requested, limits, true, false);

        QCOMPARE(result.effective.width, 1920);
        QCOMPARE(result.effective.height, 1080);
        QCOMPARE(result.effective.fps, 60);
        QCOMPARE(result.effective.bitrateKbps,
                 StreamingPreferences::scaleBitrateForMode(
                         90000, 3840, 2160, 120, 1920, 1080, 60, false));
        QVERIFY(result.resolutionAdjusted);
        QVERIFY(result.fpsAdjusted);
        QVERIFY(result.bitrateAdjusted);
    }

    void hermesNegotiationPreservesCustomBitrate()
    {
        HestiaLimits limits;
        limits.maxWidth = 1920;
        limits.maxHeight = 1080;
        limits.maxFps = 60;
        limits.supportedFps = {60};

        const HestiaNegotiation::StreamMode requested {
            3840,
            2160,
            120,
            50000,
        };
        const auto result = HestiaNegotiation::negotiateStreamMode(
                requested, limits, false, false);
        QCOMPARE(result.effective.bitrateKbps, 50000);
        QVERIFY(!result.bitrateAdjusted);
    }
};

QTEST_GUILESS_MAIN(ClientLogicTest)

#include "tst_clientlogic.moc"
