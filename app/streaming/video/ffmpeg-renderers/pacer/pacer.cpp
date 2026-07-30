#include "pacer.h"
#include "streaming/streamutils.h"

#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "dxvsyncsource.h"
#endif

#ifdef HAS_WAYLAND
#include "waylandvsyncsource.h"
#endif

#include <SDL_syswm.h>

// Limit the number of queued frames to prevent excessive memory consumption
// if the V-Sync source or renderer is blocked for a while. It's important
// that the sum of all queued frames between both pacing and rendering queues
// must not exceed the number buffer pool size to avoid running the decoder
// out of available decoding surfaces.
#define MAX_QUEUED_FRAMES 3
static_assert(
        Pacer::MAX_OUTSTANDING_FRAMES ==
            MAX_QUEUED_FRAMES + 2,
        "Scheduler outstanding-frame limit and Pacer queues must agree");

// We may be woken up slightly late so don't go all the way
// up to the next V-sync since we may accidentally step into
// the next V-sync period. It also takes some amount of time
// to do the render itself, so we can't render right before
// V-sync happens.
#define TIMER_SLACK_MS 3

Pacer::Pacer(IFFmpegRenderer* renderer,
             PVIDEO_STATS videoStats,
             PipelineTelemetry::FrameTimeline* frameTimeline,
             SessionTelemetry::ISessionTelemetry* telemetry) :
    m_RenderThread(nullptr),
    m_VsyncThread(nullptr),
    m_DeferredFreeFrame(nullptr),
    m_Stopping(false),
    m_LastRtt(0),
    m_LastRttVariance(0),
    m_PendingVsyncs(0),
    m_LastVsyncUs(0),
    m_PacingEnabled(false),
    m_VsyncSource(nullptr),
    m_VsyncRenderer(renderer),
    m_MaxVideoFps(0),
    m_DisplayFps(0),
    m_DisplayFpsMillihz(0),
    m_VideoStats(videoStats),
    m_FrameTimeline(frameTimeline),
    m_Telemetry(
        telemetry != nullptr ?
            telemetry :
            &SessionTelemetry::nullSessionTelemetry())
{

}

Pacer::~Pacer()
{
    m_Stopping = true;

    // Stop the V-sync thread
    if (m_VsyncThread != nullptr) {
        m_PacingQueueNotEmpty.wakeAll();
        m_VsyncSignalled.wakeAll();
        SDL_WaitThread(m_VsyncThread, nullptr);
    }

    // Stop V-sync callbacks
    delete m_VsyncSource;
    m_VsyncSource = nullptr;

    // Stop the render thread
    if (m_RenderThread != nullptr) {
        m_RenderQueueNotEmpty.wakeAll();
        SDL_WaitThread(m_RenderThread, nullptr);
    }
    else {
        // Notify the renderer that it is being destroyed soon
        // NB: This must happen on the same thread that calls renderFrame().
        m_VsyncRenderer->cleanupRenderContext();
    }

    // Delete any remaining unconsumed frames
    while (!m_RenderQueue.isEmpty()) {
        AVFrame* frame = m_RenderQueue.dequeue();
        traceTerminalFrame(
            frame,
            0,
            LiGetMicroseconds(),
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::Shutdown);
        av_frame_free(&frame);
    }
    while (!m_PacingQueue.isEmpty()) {
        AVFrame* frame = m_PacingQueue.dequeue();
        traceTerminalFrame(
            frame,
            0,
            LiGetMicroseconds(),
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::Shutdown);
        av_frame_free(&frame);
    }
    av_frame_free(&m_DeferredFreeFrame);
}

void Pacer::renderOnMainThread()
{
    // Ignore this call for renderers that work on a dedicated render thread
    if (m_RenderThread != nullptr) {
        return;
    }

    m_FrameQueueLock.lock();

    if (!m_RenderQueue.isEmpty()) {
        AVFrame* frame = m_RenderQueue.dequeue();
        m_FrameQueueLock.unlock();

        renderFrame(frame);
    }
    else {
        m_FrameQueueLock.unlock();
    }
}

int Pacer::vsyncThread(void *context)
{
    Pacer* me = reinterpret_cast<Pacer*>(context);

#if SDL_VERSION_ATLEAST(2, 0, 9)
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_TIME_CRITICAL);
#else
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
#endif

    bool async = me->m_VsyncSource->isAsync();
    while (!me->m_Stopping) {
        if (async) {
            // Wait for an actual callback. The pending counter avoids losing a
            // callback that arrives just before wait() begins. A timeout is not
            // treated as a synthetic VSync because Wayland intentionally stops
            // frame callbacks for occluded surfaces.
            bool receivedVsync = false;
            me->m_FrameQueueLock.lock();
            while (!me->m_Stopping && me->m_PendingVsyncs == 0) {
                if (!me->m_VsyncSignalled.wait(&me->m_FrameQueueLock, 100)) {
                    break;
                }
            }
            if (me->m_PendingVsyncs > 0) {
                me->m_PendingVsyncs--;
                receivedVsync = true;
            }
            me->m_FrameQueueLock.unlock();

            if (!receivedVsync) {
                continue;
            }
        }
        else {
            // Let the VSync source wait in the context of our thread
            me->m_VsyncSource->waitForVsync();
        }

        if (me->m_Stopping) {
            break;
        }

        // Microseconds per frame from the precise (fractional) display rate:
        // 1e9 / millihertz. Keeps 59.94/119.88 Hz panels from drifting versus a
        // rounded integer interval.
        const uint64_t nowUs = LiGetMicroseconds();
        const uint64_t targetIntervalUs =
                1000000000LL / me->m_DisplayFpsMillihz;
        me->recordVsyncInterval(nowUs, targetIntervalUs);
        me->handleVsync(static_cast<int>(targetIntervalUs));
    }

    return 0;
}

int Pacer::renderThread(void* context)
{
    Pacer* me = reinterpret_cast<Pacer*>(context);

    if (SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH) < 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Unable to set render thread to high priority: %s",
                    SDL_GetError());
    }

    while (!me->m_Stopping) {
        // Wait for the renderer to be ready for the next frame
        me->m_VsyncRenderer->waitToRender();

        // Acquire the frame queue lock to protect the queue and
        // the not empty condition
        me->m_FrameQueueLock.lock();

        // Wait for a frame to be ready to render
        while (!me->m_Stopping && me->m_RenderQueue.isEmpty()) {
            me->m_RenderQueueNotEmpty.wait(&me->m_FrameQueueLock);
        }

        if (me->m_Stopping) {
            // Exit this thread
            me->m_FrameQueueLock.unlock();
            break;
        }

        AVFrame* frame = me->m_RenderQueue.dequeue();
        me->m_FrameQueueLock.unlock();

        me->renderFrame(frame);
    }

    // Notify the renderer that it is being destroyed soon
    // NB: This must happen on the same thread that calls renderFrame().
    me->m_VsyncRenderer->cleanupRenderContext();

    return 0;
}

void Pacer::enqueueFrameForRenderingAndUnlock(AVFrame *frame)
{
    dropFrameForEnqueue(m_RenderQueue);
    m_RenderQueue.enqueue(frame);

    m_FrameQueueLock.unlock();

    if (m_RenderThread != nullptr) {
        m_RenderQueueNotEmpty.wakeOne();
    }
    else {
        SDL_Event event;

        // For main thread rendering, we'll push an event to trigger a callback
        event.type = SDL_USEREVENT;
        event.user.code = SDL_CODE_FRAME_READY;
        SDL_PushEvent(&event);
    }
}

// Called in an arbitrary thread by the IVsyncSource on V-sync
// or an event synchronized with V-sync
void Pacer::handleVsync(int timeUntilNextVsyncMicros)
{
    // The wait below is millisecond-granular; convert from the microsecond
    // interval here so the caller can pass a precise (fractional-Hz) value.
    int timeUntilNextVsyncMillis = timeUntilNextVsyncMicros / 1000;

    // Make sure initialize() has been called
    SDL_assert(m_MaxVideoFps != 0);

    m_FrameQueueLock.lock();

    const int rttVarianceMs = m_LastRtt.load(std::memory_order_acquire) != 0
            ? static_cast<int>(m_LastRttVariance.load(std::memory_order_relaxed))
            : -1;
    m_AdaptiveQueueDepth.observeNetworkJitter(rttVarianceMs);
    const int frameDropTarget = m_AdaptiveQueueDepth.targetDepth();
    m_VideoStats->pacingQueueTarget = frameDropTarget;
    recordQueueDepth(m_VideoStats->pacingQueueDepth, m_PacingQueue.count());

    // Catch up if we're several frames ahead
    while (m_PacingQueue.count() > frameDropTarget) {
        AVFrame* frame = m_PacingQueue.dequeue();

        // Drop the lock while we call av_frame_free()
        m_FrameQueueLock.unlock();
        m_VideoStats->pacerDroppedFrames++;
        traceTerminalFrame(
            frame,
            0,
            LiGetMicroseconds(),
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::PacingBacklog);
        av_frame_free(&frame);
        m_FrameQueueLock.lock();
    }

    if (m_PacingQueue.isEmpty()) {
        // Wait for a frame to arrive or our V-sync timeout to expire
        if (!m_PacingQueueNotEmpty.wait(&m_FrameQueueLock, SDL_max(timeUntilNextVsyncMillis, TIMER_SLACK_MS) - TIMER_SLACK_MS)) {
            // Wait timed out - unlock and bail
            m_FrameQueueLock.unlock();
            return;
        }

        if (m_Stopping) {
            m_FrameQueueLock.unlock();
            return;
        }
    }

    // Place the first frame on the render queue
    enqueueFrameForRenderingAndUnlock(m_PacingQueue.dequeue());
}

bool Pacer::initialize(SDL_Window* window, int maxVideoFps, bool enablePacing)
{
    m_MaxVideoFps = maxVideoFps;
    m_DisplayFps = StreamUtils::getDisplayRefreshRate(window);
    m_DisplayFpsMillihz = StreamUtils::getDisplayRefreshRateMillihertz(window);
    m_PacingEnabled = enablePacing;
    m_RendererAttributes = m_VsyncRenderer->getRendererAttributes();

    if (enablePacing) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Frame pacing: target %.3f Hz with %d FPS stream",
                    m_DisplayFpsMillihz / 1000.0,
                    m_MaxVideoFps);

        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (!SDL_GetWindowWMInfo(window, &info)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "SDL_GetWindowWMInfo() failed: %s",
                         SDL_GetError());
            return false;
        }

        switch (info.subsystem) {
    #ifdef Q_OS_WIN32
        case SDL_SYSWM_WINDOWS:
            m_VsyncSource = new DxVsyncSource(this);
            break;
    #endif

    #if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(HAS_WAYLAND)
        case SDL_SYSWM_WAYLAND:
            m_VsyncSource = new WaylandVsyncSource(this);
            break;
    #endif

        default:
            // Platforms without a VsyncSource will just render frames
            // immediately like they used to.
            break;
        }

        SDL_assert(m_VsyncSource != nullptr || !(m_RendererAttributes & RENDERER_ATTRIBUTE_FORCE_PACING));

        if (m_VsyncSource != nullptr && !m_VsyncSource->initialize(window, m_DisplayFps)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "Vsync source failed to initialize. Frame pacing will not be available!");
            delete m_VsyncSource;
            m_VsyncSource = nullptr;
        }
    }
    else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Frame pacing disabled: target %.3f Hz with %d FPS stream",
                    m_DisplayFpsMillihz / 1000.0,
                    m_MaxVideoFps);
    }

    const int adaptiveSampleRateMillihertz = m_VsyncSource != nullptr
            ? m_DisplayFpsMillihz
            : PacingPolicy::rendererPresentationRateMillihertz(
                    m_DisplayFpsMillihz, m_MaxVideoFps);
    m_AdaptiveQueueDepth.configure(adaptiveSampleRateMillihertz);

    if (m_VsyncSource != nullptr) {
        m_VsyncThread = SDL_CreateThread(Pacer::vsyncThread, "PacerVsync", this);
    }

    if (m_VsyncRenderer->isRenderThreadSupported()) {
        m_RenderThread = SDL_CreateThread(Pacer::renderThread, "PacerRender", this);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Presentation path: %s (adaptive queue starts at %d frames)",
                getPresentationPathName(),
                m_AdaptiveQueueDepth.targetDepth());

    return true;
}

void Pacer::signalVsync()
{
    m_FrameQueueLock.lock();
    m_PendingVsyncs++;
    m_VsyncSignalled.wakeOne();
    m_FrameQueueLock.unlock();
}

void Pacer::updateNetworkJitter(uint32_t rttMs, uint32_t rttVarianceMs)
{
    m_LastRttVariance.store(rttVarianceMs, std::memory_order_relaxed);
    m_LastRtt.store(rttMs, std::memory_order_release);
}

void Pacer::renderFrame(AVFrame* frame)
{
    // Count time spent in Pacer's queues
    uint64_t beforeRender = LiGetMicroseconds();
    const auto pacerTimeUs = beforeRender - (uint64_t)frame->pkt_dts;
    m_VideoStats->totalPacerTimeUs += pacerTimeUs;
    PipelineTelemetry::recordLatency(
        m_VideoStats->pacerLatency,
        PipelineTelemetry::Microseconds {pacerTimeUs});

    // Render it
    m_VsyncRenderer->renderFrame(frame);
    uint64_t afterRender = LiGetMicroseconds();

    if (m_VsyncSource == nullptr) {
        const int presentationRateMillihertz =
                PacingPolicy::rendererPresentationRateMillihertz(
                        m_DisplayFpsMillihz, m_MaxVideoFps);
        recordVsyncInterval(afterRender,
                            1000000000LL / presentationRateMillihertz);
    }

    const auto renderTimeUs = afterRender - beforeRender;
    m_VideoStats->totalRenderTimeUs += renderTimeUs;
    PipelineTelemetry::recordLatency(
        m_VideoStats->renderLatency,
        PipelineTelemetry::Microseconds {renderTimeUs});
    m_VideoStats->renderedFrames++;
    traceTerminalFrame(
        frame,
        beforeRender,
        afterRender,
        PipelineTelemetry::FrameOutcome::Presented,
        PipelineTelemetry::FrameTerminalReason::Presented);

    // Wait until after next frame to free this one to ensure the GPU
    // doesn't stall or read garbage if the backing buffer gets returned
    // to the pool and the decoder tries to write a new frame into it
    std::swap(frame, m_DeferredFreeFrame);
    av_frame_free(&frame);

    // Drop frames if we have too many queued up for a while
    m_FrameQueueLock.lock();
    recordQueueDepth(m_VideoStats->renderQueueDepth, m_RenderQueue.count());

    int frameDropTarget;

    if (m_RendererAttributes & RENDERER_ATTRIBUTE_NO_BUFFERING) {
        // Renderers that don't buffer any frames but don't support waitToRender() need us to buffer
        // an extra frame to ensure they don't starve while waiting to present.
        frameDropTarget = 1;
    }
    else if (m_VsyncSource == nullptr && m_PacingEnabled) {
        // X11 and other renderer-driven paths have no separate pacing queue.
        // Apply the network-jitter policy to the render queue instead. Subtract
        // the frame currently being presented from the desired total depth.
        const int rttVarianceMs = m_LastRtt.load(std::memory_order_acquire) != 0
                ? static_cast<int>(m_LastRttVariance.load(std::memory_order_relaxed))
                : -1;
        m_AdaptiveQueueDepth.observeNetworkJitter(rttVarianceMs);
        frameDropTarget = qMax(0, m_AdaptiveQueueDepth.targetDepth() - 1);
    }
    else {
        frameDropTarget = 0;
        for (int queueHistoryEntry : std::as_const(m_RenderQueueHistory)) {
            if (queueHistoryEntry == 0) {
                // Be lenient as long as the queue length
                // resolves before the end of frame history
                frameDropTarget = 2;
                break;
            }
        }

        // Keep a rolling 500 ms window of render queue history
        if (m_RenderQueueHistory.count() == m_MaxVideoFps / 2) {
            m_RenderQueueHistory.dequeue();
        }

        m_RenderQueueHistory.enqueue(m_RenderQueue.count());
    }
    m_VideoStats->renderQueueTarget = frameDropTarget;

    // Catch up if we're several frames ahead
    while (m_RenderQueue.count() > frameDropTarget) {
        AVFrame* frame = m_RenderQueue.dequeue();

        // Drop the lock while we call av_frame_free()
        m_FrameQueueLock.unlock();
        m_VideoStats->pacerDroppedFrames++;
        traceTerminalFrame(
            frame,
            0,
            LiGetMicroseconds(),
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::RenderBacklog);
        av_frame_free(&frame);
        m_FrameQueueLock.lock();
    }

    m_FrameQueueLock.unlock();
}

void Pacer::dropFrameForEnqueue(QQueue<AVFrame*>& queue)
{
    SDL_assert(queue.size() <= MAX_QUEUED_FRAMES);
    if (queue.size() == MAX_QUEUED_FRAMES) {
        AVFrame* frame = queue.dequeue();
        m_VideoStats->pacerDroppedFrames++;
        traceTerminalFrame(
            frame,
            0,
            LiGetMicroseconds(),
            PipelineTelemetry::FrameOutcome::Dropped,
            PipelineTelemetry::FrameTerminalReason::QueueLimit);
        av_frame_free(&frame);
    }
}

void Pacer::recordVsyncInterval(uint64_t nowUs, uint64_t targetIntervalUs)
{
    if (m_LastVsyncUs != 0 && nowUs > m_LastVsyncUs) {
        const uint64_t intervalUs = nowUs - m_LastVsyncUs;

        m_VideoStats->vsyncIntervals++;
        m_VideoStats->totalVsyncIntervalUs += intervalUs;
        m_VideoStats->maxVsyncIntervalUs =
                qMax(m_VideoStats->maxVsyncIntervalUs,
                     static_cast<uint32_t>(qMin<uint64_t>(intervalUs, UINT32_MAX)));
        if (intervalUs > targetIntervalUs * 3 / 2) {
            m_VideoStats->lateVsyncIntervals++;
        }
    }
    m_LastVsyncUs = nowUs;
}

void Pacer::recordQueueDepth(uint32_t (&buckets)[4], int depth)
{
    buckets[qBound(0, depth, 3)]++;
}

void Pacer::traceTerminalFrame(
        AVFrame* frame,
        uint64_t presentStartUs,
        uint64_t terminalUs,
        PipelineTelemetry::FrameOutcome outcome,
        PipelineTelemetry::FrameTerminalReason reason)
{
    if (!m_Telemetry->frameTracingEnabled() ||
            m_FrameTimeline == nullptr) {
        return;
    }

    const auto frameNumber =
            PipelineTelemetry::frameIdFromTag(frame->opaque);
    if (!frameNumber) {
        return;
    }

    const auto trace = m_FrameTimeline->recordTerminal(
            *frameNumber,
            presentStartUs,
            terminalUs,
            outcome,
            reason);
    if (trace) {
        m_Telemetry->publishFrameTrace(*trace);
    }
}

const char* Pacer::getPresentationPathName() const
{
    if (m_VsyncSource != nullptr) {
        return m_VsyncSource->name();
    }
    if (!m_PacingEnabled) {
        return "renderer VSync (frame pacing disabled)";
    }
    return "renderer-driven VSync";
}

Pacer::PresentationPath Pacer::getPresentationPath() const
{
    if (m_VsyncSource != nullptr) {
        return PresentationPath::SynchronizedSource;
    }
    if (!m_PacingEnabled) {
        return PresentationPath::RendererVsync;
    }
    return PresentationPath::RendererDriven;
}

int Pacer::getDisplayFpsMillihertz() const
{
    return m_DisplayFpsMillihz;
}

void Pacer::submitFrame(AVFrame* frame)
{
    // Make sure initialize() has been called
    SDL_assert(m_MaxVideoFps != 0);

    // Queue the frame and possibly wake up the render thread
    m_FrameQueueLock.lock();
    if (m_VsyncSource != nullptr) {
        dropFrameForEnqueue(m_PacingQueue);
        m_PacingQueue.enqueue(frame);
        m_FrameQueueLock.unlock();
        m_PacingQueueNotEmpty.wakeOne();
    }
    else {
        enqueueFrameForRenderingAndUnlock(frame);
    }
}
