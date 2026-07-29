#include "statsdiagnostics.h"

#include <QCoreApplication>

namespace Diagnostics {

namespace {

class Translation
{
    Q_DECLARE_TR_FUNCTIONS(HestiaDiagnostics)
};

// A window needs at least this many rendered frames before we trust it. Avoids
// classifying noise during the first moments of a stream or a brief stall.
const uint32_t MIN_FRAMES_TO_JUDGE = 10;

// Fraction of dropped frames (of the relevant denominator) above which a drop
// cause is considered a real problem rather than incidental loss.
const double DROP_PERCENT_THRESHOLD = 2.0;

// Decode is considered to be struggling once it consumes this fraction of the
// per-frame time budget, even if no frames have been dropped yet.
const double DECODE_BUDGET_FRACTION = 0.6;

// Rendering includes a normal V-sync wait, so one full frame budget is healthy.
// Require meaningful overrun before declaring presentation-bound.
const double RENDER_BUDGET_FRACTION = 1.15;

// More than two frame budgets in the pacing queues indicates persistent
// backlog rather than the normal one-frame presentation latency.
const double QUEUE_BUDGET_MULTIPLIER = 2.0;

// ENet reports RTT variance in milliseconds. Above this value, pacing drops
// are more likely network jitter than a local presentation problem.
const double RTT_VARIANCE_HIGH_MS = 10.0;

const double VSYNC_LATE_PERCENT_THRESHOLD = 2.0;

// Host processing latency (server side) considered high, in milliseconds.
const double HOST_LATENCY_HIGH_MS = 30.0;

double perFrameMs(uint64_t totalUs, uint32_t frames)
{
    if (frames == 0) {
        return 0.0;
    }
    return (totalUs / 1000.0) / frames;
}

double dropPercent(uint32_t dropped, uint32_t total)
{
    if (total == 0) {
        return 0.0;
    }
    return (double)dropped / total * 100.0;
}

} // namespace

Diagnosis diagnose(const VIDEO_STATS& stats, int targetFps)
{
    Diagnosis result;

    // Not enough signal to say anything meaningful.
    if (stats.renderedFrames < MIN_FRAMES_TO_JUDGE || stats.decodedFrames == 0) {
        return result;
    }

    // Per-frame time budget. Fall back to a 60 FPS budget if we weren't told the
    // target, so the fractions below stay meaningful.
    const double frameBudgetMs = 1000.0 / (targetFps > 0 ? targetFps : 60);

    const double decodeMs = perFrameMs(stats.totalDecodeTimeUs, stats.decodedFrames);
    const double renderMs = perFrameMs(stats.totalRenderTimeUs, stats.renderedFrames);
    const double queueMs = perFrameMs(stats.totalPacerTimeUs, stats.renderedFrames);

    const double networkDropPct = dropPercent(stats.networkDroppedFrames, stats.totalFrames);
    const double pacerDropPct = dropPercent(stats.pacerDroppedFrames, stats.decodedFrames);
    const double lateVsyncPct =
            dropPercent(stats.lateVsyncIntervals, stats.vsyncIntervals);

    const double avgHostMs = stats.framesWithHostProcessingLatency > 0
            ? (stats.totalHostProcessingLatency / 10.0) / stats.framesWithHostProcessingLatency
            : 0.0;

    // Score each candidate cause; the highest score wins. Scores are normalized
    // so that 1.0 is "right at the threshold" and higher is worse, making the
    // comparison across heterogeneous metrics fair.
    const double networkDropScore = networkDropPct / DROP_PERCENT_THRESHOLD;
    const double jitterScore = stats.lastRttVariance / RTT_VARIANCE_HIGH_MS;
    const double pacerDropScore = pacerDropPct / DROP_PERCENT_THRESHOLD;
    double networkScore = qMax(networkDropScore, jitterScore);
    double renderScore = qMax(renderMs / (frameBudgetMs * RENDER_BUDGET_FRACTION),
                              queueMs / (frameBudgetMs * QUEUE_BUDGET_MULTIPLIER));
    renderScore = qMax(renderScore,
                       lateVsyncPct / VSYNC_LATE_PERCENT_THRESHOLD);
    double decodeScore = decodeMs / (frameBudgetMs * DECODE_BUDGET_FRACTION);
    double hostScore = avgHostMs / HOST_LATENCY_HIGH_MS;

    // Pacer drops are ambiguous: they can be caused by bursty network delivery
    // or by a local queue/render backlog. Attribute them using the independent
    // RTT/drop signals instead of always blaming presentation.
    if (pacerDropScore >= 1.0) {
        if (jitterScore >= 1.0 || networkDropScore >= 1.0) {
            networkScore = qMax(networkScore, pacerDropScore);
        }
        else {
            renderScore = qMax(renderScore, pacerDropScore);
        }
    }

    // Decoded throughput falling short of what the network delivered is a strong
    // decode-bound signal even when individual decode times look acceptable.
    if (stats.receivedFps > 0 && stats.decodedFps > 0 &&
            stats.decodedFps < stats.receivedFps * 0.95) {
        decodeScore = qMax(decodeScore, 1.5);
    }

    // Find the dominant cause among those that crossed their threshold (score >= 1).
    Bottleneck dominant = BOTTLENECK_NONE;
    double best = 1.0;
    if (networkScore >= best) { best = networkScore; dominant = BOTTLENECK_NETWORK; }
    if (decodeScore > best)   { best = decodeScore;  dominant = BOTTLENECK_DECODE; }
    if (renderScore > best)   { best = renderScore;  dominant = BOTTLENECK_RENDER; }
    if (hostScore > best)     { best = hostScore;    dominant = BOTTLENECK_HOST; }

    result.dominant = dominant;

    switch (dominant) {
    case BOTTLENECK_NONE:
        result.summary = Translation::tr("Stream is healthy.");
        break;
    case BOTTLENECK_DECODE:
        result.summary = Translation::tr("Your decoder is falling behind. Try a lighter codec "
                                         "(e.g. HEVC to H.264) or lower the resolution or frame rate.");
        result.keyMetric = Translation::tr("decode %1 ms/frame").arg(decodeMs, 0, 'f', 1);
        break;
    case BOTTLENECK_RENDER:
        result.summary = Translation::tr("Frames are being delayed during presentation. Check V-Sync "
                                         "and that the display refresh rate matches the stream.");
        if (pacerDropPct >= DROP_PERCENT_THRESHOLD) {
            result.keyMetric = Translation::tr("%1% pacing drops").arg(pacerDropPct, 0, 'f', 1);
        }
        else if (lateVsyncPct >= VSYNC_LATE_PERCENT_THRESHOLD) {
            result.keyMetric = Translation::tr("%1% late V-Sync intervals")
                    .arg(lateVsyncPct, 0, 'f', 1);
        }
        else if (queueMs >= frameBudgetMs * QUEUE_BUDGET_MULTIPLIER) {
            result.keyMetric = Translation::tr("queue %1 ms/frame").arg(queueMs, 0, 'f', 1);
        }
        else {
            result.keyMetric = Translation::tr("render %1 ms/frame").arg(renderMs, 0, 'f', 1);
        }
        break;
    case BOTTLENECK_NETWORK:
        result.summary = Translation::tr("Your network is dropping or delaying frames. Lower the bitrate "
                                         "or move closer to the access point / use a wired connection.");
        result.keyMetric = networkDropPct >= DROP_PERCENT_THRESHOLD
                ? Translation::tr("%1% network drops").arg(networkDropPct, 0, 'f', 1)
                : Translation::tr("RTT variance %1 ms").arg(stats.lastRttVariance);
        break;
    case BOTTLENECK_HOST:
        result.summary = Translation::tr("The host is slow to produce frames. This is a server-side "
                                         "bottleneck, not your client.");
        result.keyMetric = Translation::tr("host %1 ms/frame").arg(avgHostMs, 0, 'f', 1);
        break;
    }

    return result;
}

void SpikeHistory::record(const Diagnosis& diagnosis)
{
    m_Samples[m_Head] = diagnosis.dominant;
    m_Head = (m_Head + 1) % CAPACITY;
    if (m_Count < CAPACITY) {
        m_Count++;
    }
}

int SpikeHistory::spikeCount() const
{
    int count = 0;
    Bottleneck previous = BOTTLENECK_NONE;
    for (int i = 0; i < m_Count; i++) {
        const int index = (m_Head - m_Count + i + CAPACITY) % CAPACITY;
        const Bottleneck current = m_Samples[index];
        if (current != BOTTLENECK_NONE && current != previous) {
            count++;
        }
        previous = current;
    }
    return count;
}

QString SpikeHistory::summarize() const
{
    int counts[BOTTLENECK_HOST + 1] = {};
    Bottleneck previous = BOTTLENECK_NONE;
    for (int i = 0; i < m_Count; i++) {
        const int index = (m_Head - m_Count + i + CAPACITY) % CAPACITY;
        const Bottleneck current = m_Samples[index];
        if (current != BOTTLENECK_NONE && current != previous) {
            counts[current]++;
        }
        previous = current;
    }
    const int spikes = spikeCount();

    if (spikes == 0) {
        return QString();
    }

    // Determine the most common spike cause.
    Bottleneck mostCommon = BOTTLENECK_NETWORK;
    for (int b = BOTTLENECK_DECODE; b <= BOTTLENECK_HOST; b++) {
        if (counts[b] > counts[mostCommon]) {
            mostCommon = (Bottleneck)b;
        }
    }

    QString causeText;
    switch (mostCommon) {
    case BOTTLENECK_DECODE:  causeText = Translation::tr("mostly decode"); break;
    case BOTTLENECK_RENDER:  causeText = Translation::tr("mostly pacing"); break;
    case BOTTLENECK_NETWORK: causeText = Translation::tr("mostly network"); break;
    case BOTTLENECK_HOST:    causeText = Translation::tr("mostly host"); break;
    default:                 causeText = Translation::tr("mixed"); break;
    }

    // Report the window length actually covered, in minutes (rounded up).
    const int minutes = (m_Count + 59) / 60;
    return Translation::tr("%n spike(s) in last ~%1 min (%2)", nullptr, spikes)
            .arg(minutes)
            .arg(causeText);
}

void SpikeHistory::clear()
{
    m_Count = 0;
    m_Head = 0;
}

QString formatOverlayText(const Diagnosis& diagnosis, const SpikeHistory& history)
{
    QString text;
    if (!diagnosis.summary.isEmpty()) {
        text += diagnosis.keyMetric.isEmpty()
                ? Translation::tr("Diagnosis: %1").arg(diagnosis.summary)
                : Translation::tr("Diagnosis: %1 (%2)")
                          .arg(diagnosis.summary, diagnosis.keyMetric);
        text += QLatin1Char('\n');
    }

    const QString spikes = history.summarize();
    if (!spikes.isEmpty()) {
        text += spikes;
        text += QLatin1Char('\n');
    }
    return text;
}

} // namespace Diagnostics
