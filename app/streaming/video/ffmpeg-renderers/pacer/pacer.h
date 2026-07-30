#pragma once

#include "../../decoder.h"
#include "../renderer.h"
#include "pacingpolicy.h"
#include "streaming/telemetry/sessiontelemetry.h"

#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>

class IVsyncSource {
public:
    virtual ~IVsyncSource() {}
    virtual bool initialize(SDL_Window* window, int displayFps) = 0;

    // Asynchronous sources produce callbacks on their own, while synchronous
    // sources require calls to waitForVsync().
    virtual bool isAsync() = 0;

    virtual const char* name() const = 0;

    virtual void waitForVsync() {
        // Synchronous sources must implement waitForVsync()!
        SDL_assert(false);
    }
};

class Pacer
{
public:
    // Three queued frames, one being rendered, and one deferred free.
    static constexpr uint32_t MAX_OUTSTANDING_FRAMES = 5;

    enum class PresentationPath {
        SynchronizedSource,
        RendererVsync,
        RendererDriven,
    };

    Pacer(IFFmpegRenderer* renderer,
          PVIDEO_STATS videoStats,
          PipelineTelemetry::FrameTimeline* frameTimeline,
          SessionTelemetry::ISessionTelemetry* telemetry);

    ~Pacer();

    void submitFrame(AVFrame* frame);

    bool initialize(SDL_Window* window, int maxVideoFps, bool enablePacing);

    void signalVsync();

    void renderOnMainThread();

    void updateNetworkJitter(uint32_t rttMs, uint32_t rttVarianceMs);

    const char* getPresentationPathName() const;

    PresentationPath getPresentationPath() const;

    int getDisplayFpsMillihertz() const;

private:
    static int vsyncThread(void* context);

    static int renderThread(void* context);

    void handleVsync(int timeUntilNextVsyncMicros);

    void enqueueFrameForRenderingAndUnlock(AVFrame* frame);

    void renderFrame(AVFrame* frame);

    void dropFrameForEnqueue(QQueue<AVFrame*>& queue);

    void recordVsyncInterval(uint64_t nowUs, uint64_t targetIntervalUs);

    void recordQueueDepth(uint32_t (&buckets)[4], int depth);

    void traceTerminalFrame(AVFrame* frame,
                            uint64_t presentStartUs,
                            uint64_t terminalUs,
                            PipelineTelemetry::FrameOutcome outcome,
                            PipelineTelemetry::FrameTerminalReason reason);

    QQueue<AVFrame*> m_RenderQueue;
    QQueue<AVFrame*> m_PacingQueue;
    QQueue<int> m_RenderQueueHistory;
    QMutex m_FrameQueueLock;
    QWaitCondition m_RenderQueueNotEmpty;
    QWaitCondition m_PacingQueueNotEmpty;
    QWaitCondition m_VsyncSignalled;
    SDL_Thread* m_RenderThread;
    SDL_Thread* m_VsyncThread;
    AVFrame* m_DeferredFreeFrame;
    std::atomic_bool m_Stopping;
    std::atomic_uint32_t m_LastRtt;
    std::atomic_uint32_t m_LastRttVariance;
    int m_PendingVsyncs;
    uint64_t m_LastVsyncUs;
    bool m_PacingEnabled;

    IVsyncSource* m_VsyncSource;
    IFFmpegRenderer* m_VsyncRenderer;
    int m_MaxVideoFps;
    int m_DisplayFps;
    // Display rate in millihertz, preserving fractional refresh (e.g. 59.94 Hz).
    // Used for the vsync interval and near-equality drop decisions, where the
    // ~0.1% the integer m_DisplayFps loses actually matters.
    int m_DisplayFpsMillihz;
    PacingPolicy::AdaptiveQueueDepth m_AdaptiveQueueDepth;
    PVIDEO_STATS m_VideoStats;
    PipelineTelemetry::FrameTimeline* m_FrameTimeline;
    SessionTelemetry::ISessionTelemetry* m_Telemetry;
    int m_RendererAttributes;
};
