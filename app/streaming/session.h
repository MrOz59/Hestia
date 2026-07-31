#pragma once

#include <QSemaphore>
#include <QQuickWindow>
#include <QString>
#include <QScopedPointer>

#include <memory>

#include <Limelight.h>
#include "settings/streamingpreferences.h"
#include "audio/receiver/audioreceiver.h"
#include "connectivity/connectivityagent.h"
#include "input/input.h"
#include "input/sender/inputsender.h"
#include "protocol/hostprotocol.h"
#include "telemetry/sessiontelemetry.h"
#include "transport/clienttransport.h"
#include "video/decoder/decoder.h"
#include "video/receiver/videoreceiver.h"
#include "video/overlaymanager.h"

class NvHTTP;

class SupportedVideoFormatList : public QList<int>
{
public:
    operator int() const
    {
        int value = 0;

        for (const int v : *this) {
            value |= v;
        }

        return value;
    }

    void
    removeByMask(int mask)
    {
        int i = 0;
        while (i < this->length()) {
            if (this->value(i) & mask) {
                this->removeAt(i);
            }
            else {
                i++;
            }
        }
    }

    void
    deprioritizeByMask(int mask)
    {
        QList<int> deprioritizedList;

        int i = 0;
        while (i < this->length()) {
            if (this->value(i) & mask) {
                deprioritizedList.append(this->takeAt(i));
            }
            else {
                i++;
            }
        }

        this->append(std::move(deprioritizedList));
    }

    int maskByServerCodecModes(int serverCodecModes)
    {
        int mask = 0;

        const QMap<int, int> mapping = {
            {SCM_H264, VIDEO_FORMAT_H264},
            {SCM_H264_HIGH8_444, VIDEO_FORMAT_H264_HIGH8_444},
            {SCM_HEVC, VIDEO_FORMAT_H265},
            {SCM_HEVC_MAIN10, VIDEO_FORMAT_H265_MAIN10},
            {SCM_HEVC_REXT8_444, VIDEO_FORMAT_H265_REXT8_444},
            {SCM_HEVC_REXT10_444, VIDEO_FORMAT_H265_REXT10_444},
            {SCM_AV1_MAIN8, VIDEO_FORMAT_AV1_MAIN8},
            {SCM_AV1_MAIN10, VIDEO_FORMAT_AV1_MAIN10},
            {SCM_AV1_HIGH8_444, VIDEO_FORMAT_AV1_HIGH8_444},
            {SCM_AV1_HIGH10_444, VIDEO_FORMAT_AV1_HIGH10_444},
        };

        for (QMap<int, int>::const_iterator it = mapping.cbegin(); it != mapping.cend(); ++it) {
            if (serverCodecModes & it.key()) {
                mask |= it.value();
                serverCodecModes &= ~it.key();
            }
        }

        // Make sure nobody forgets to update this for new SCM values
        SDL_assert(serverCodecModes == 0);

        int val = *this;
        return val & mask;
    }
};

class Session : public QObject
{
    Q_OBJECT

    friend class SdlInputHandler;
    friend class DeferredSessionCleanupTask;
    friend class AsyncConnectionStartThread;

public:
    explicit Session(
            NvComputer* computer,
            NvApp& app,
            StreamingPreferences* preferences = nullptr,
            std::unique_ptr<HostProtocol::IHostProtocol> hostProtocol = {},
            std::unique_ptr<ClientTransport::IClientTransport>
                    clientTransport = {},
            std::unique_ptr<Connectivity::IConnectivityAgent>
                    connectivityAgent = {},
            std::unique_ptr<VideoReceiver::IVideoReceiver>
                    videoReceiver = {},
            std::unique_ptr<AudioReceiver::IAudioReceiver>
                    audioReceiver = {},
            std::unique_ptr<InputSender::IInputSender>
                    inputSender = {},
            std::unique_ptr<SessionTelemetry::ISessionTelemetry>
                    sessionTelemetry = {});
    virtual ~Session();

    Q_INVOKABLE bool initialize(QQuickWindow* qtWindow);
    Q_INVOKABLE void start();
    Q_INVOKABLE void interrupt();
    Q_PROPERTY(QStringList launchWarnings MEMBER m_LaunchWarnings NOTIFY launchWarningsChanged);

    static
    void getDecoderInfo(SDL_Window* window,
                        bool& isHardwareAccelerated, bool& isFullScreenOnly,
                        bool& isHdrSupported, QSize& maxResolution);

    enum class DecoderAvailability {
        None,
        Software,
        Hardware
    };

    static
    DecoderAvailability getDecoderAvailability(SDL_Window* window,
                                               StreamingPreferences::VideoDecoderSelection vds,
                                               int videoFormat, int width, int height, int frameRate);

    static Session* get()
    {
        return s_ActiveSession;
    }

    Overlay::OverlayManager& getOverlayManager()
    {
        return m_OverlayManager;
    }

    void flushWindowEvents();

    void setShouldExit(bool quitHostApp = false);

signals:
    void stageStarting(QString stage);

    void stageFailed(QString stage, int errorCode, QString failingPorts);

    void connectionStarted();

    void displayLaunchError(QString text);

    void quitStarting();

    void sessionFinished(int portTestResult);

    // Emitted after sessionFinished() when the session is ready to be destroyed
    void readyForDeletion();

    void launchWarningsChanged();

private:
    void exec();

    bool startConnectionAsync();

    HostProtocol::SessionRequest buildHostSessionRequest() const;

    HostProtocol::LaunchRequest buildHostLaunchRequest(
            bool enableGameOptimizations) const;

    void applyHestiaHostLimits();

    void applyHestiaCodecLimits();

    bool validateLaunch(SDL_Window* testWindow);

    void emitLaunchWarning(QString text);

    void pollHestiaClipboardSync();

    bool populateDecoderProperties(SDL_Window* window);

    void getWindowDimensions(int& x, int& y,
                             int& width, int& height);

    void toggleFullscreen();

    void notifyMouseEmulationMode(bool enabled);

    void updateOptimalWindowDisplayMode();

    static
    bool chooseDecoder(StreamingPreferences::VideoDecoderSelection vds,
                       StreamingPreferences::RendererSelection renderer,
                       SDL_Window* window, int videoFormat, int width, int height,
                       int frameRate, bool enableVsync, bool enableFramePacing,
                       bool testOnly,
                       Decoder::IDecoder*& chosenDecoder,
                       SessionTelemetry::ISessionTelemetry*
                               telemetry = nullptr);

    static
    void clStageStarting(int stage);

    static
    void clStageFailed(int stage, int errorCode);

    static
    void clConnectionTerminated(int errorCode);

    static
    void clLogMessage(const char* format, ...);

    static
    void clRumble(unsigned short controllerNumber, unsigned short lowFreqMotor, unsigned short highFreqMotor);

    static
    void clConnectionStatusUpdate(int connectionStatus);

    static
    void clSetHdrMode(bool enabled);

    static
    void clRumbleTriggers(uint16_t controllerNumber, uint16_t leftTrigger, uint16_t rightTrigger);

    static
    void clSetMotionEventState(uint16_t controllerNumber, uint8_t motionType, uint16_t reportRateHz);

    static
    void clSetControllerLED(uint16_t controllerNumber, uint8_t r, uint8_t g, uint8_t b);

    static
    void clSetAdaptiveTriggers(uint16_t controllerNumber, uint8_t eventFlags, uint8_t typeLeft, uint8_t typeRight, uint8_t *left, uint8_t *right);

    StreamingPreferences* m_Preferences;
    bool m_IsFullScreen;
    SupportedVideoFormatList m_SupportedVideoFormats; // Sorted in order of descending priority
    STREAM_CONFIGURATION m_StreamConfig;
    NvComputer* m_Computer;
    std::unique_ptr<HostProtocol::IHostProtocol> m_HostProtocol;
    std::unique_ptr<Connectivity::IConnectivityAgent> m_ConnectivityAgent;
    QString m_RtspSessionUrl;
    NvApp m_App;
    SDL_Window* m_Window;
    Decoder::IDecoder* m_VideoDecoder;
    SDL_mutex* m_DecoderLock;
    Overlay::OverlayManager m_OverlayManager;
    std::unique_ptr<SessionTelemetry::ISessionTelemetry>
            m_SessionTelemetry;
    std::unique_ptr<VideoReceiver::IVideoReceiver> m_VideoReceiver;
    std::unique_ptr<AudioReceiver::IAudioReceiver> m_AudioReceiver;
    std::unique_ptr<InputSender::IInputSender> m_InputSender;
    std::unique_ptr<ClientTransport::IClientTransport> m_ClientTransport;
    Uint32 m_FullScreenFlag;
    QQuickWindow* m_QtWindow;
    bool m_UnexpectedTermination;
    SdlInputHandler* m_InputHandler;
    int m_MouseEmulationRefCount;
    int m_FlushingWindowEventsRef;
    QStringList m_LaunchWarnings;
    HostProtocol::SessionRequest m_HostSessionRequest;
    QString m_HestiaSessionId;
    // Hermes extensions the host reported as in force for this session. Empty
    // against any host that advertises none, which includes every GameStream
    // host and every Hermes build from before extensions existed.
    QMap<QString, uint32_t> m_NegotiatedExtensions;
    bool m_ShouldPrepareHestiaSession;
    Uint32 m_LastHestiaClipboardSyncCheckMs;
    QString m_LastHestiaClipboardText;
    QScopedPointer<NvHTTP> m_HestiaClipboardHttp;
    bool m_ShouldExit;

    bool m_AsyncConnectionSuccess;
    int m_PortTestResults;

    int m_ActiveVideoFormat;
    int m_ActiveVideoWidth;
    int m_ActiveVideoHeight;
    int m_ActiveVideoFrameRate;

    static CONNECTION_LISTENER_CALLBACKS k_ConnCallbacks;
    static Session* s_ActiveSession;
    static QSemaphore s_ActiveSessionSemaphore;
};
