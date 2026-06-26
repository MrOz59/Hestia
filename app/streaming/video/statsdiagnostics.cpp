#include "statsdiagnostics.h"

#include <QObject>

namespace Diagnostics {

namespace {

// A window needs at least this many rendered frames before we trust it. Avoids
// classifying noise during the first moments of a stream or a brief stall.
const uint32_t MIN_FRAMES_TO_JUDGE = 10;

// Fraction of dropped frames (of the relevant denominator) above which a drop
// cause is considered a real problem rather than incidental loss.
const double DROP_PERCENT_THRESHOLD = 2.0;

// Decode is considered to be struggling once it consumes this fraction of the
// per-frame time budget, even if no frames have been dropped yet.
const double DECODE_BUDGET_FRACTION = 0.6;

// Rendering (including vsync wait) above this fraction of the budget points at a
// presentation bottleneck.
const double RENDER_BUDGET_FRACTION = 0.9;

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

    const double networkDropPct = dropPercent(stats.networkDroppedFrames, stats.totalFrames);
    const double pacerDropPct = dropPercent(stats.pacerDroppedFrames, stats.decodedFrames);

    const double avgHostMs = stats.framesWithHostProcessingLatency > 0
            ? (stats.totalHostProcessingLatency / 10.0) / stats.framesWithHostProcessingLatency
            : 0.0;

    // Score each candidate cause; the highest score wins. Scores are normalized
    // so that 1.0 is "right at the threshold" and higher is worse, making the
    // comparison across heterogeneous metrics fair.
    double networkScore = networkDropPct / DROP_PERCENT_THRESHOLD;
    double renderScore = qMax(pacerDropPct / DROP_PERCENT_THRESHOLD,
                              renderMs / (frameBudgetMs * RENDER_BUDGET_FRACTION));
    double decodeScore = decodeMs / (frameBudgetMs * DECODE_BUDGET_FRACTION);
    double hostScore = avgHostMs / HOST_LATENCY_HIGH_MS;

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
        result.summary = QObject::tr("Stream is healthy.");
        break;
    case BOTTLENECK_DECODE:
        result.summary = QObject::tr("Your decoder is falling behind. Try a lighter codec "
                                     "(e.g. HEVC to H.264) or lower the resolution or frame rate.");
        result.keyMetric = QObject::tr("decode %1 ms/frame").arg(decodeMs, 0, 'f', 1);
        break;
    case BOTTLENECK_RENDER:
        result.summary = QObject::tr("Frames are being dropped during presentation. Check V-Sync "
                                     "and that the display refresh rate matches the stream.");
        result.keyMetric = pacerDropPct >= DROP_PERCENT_THRESHOLD
                ? QObject::tr("%1%% pacing drops").arg(pacerDropPct, 0, 'f', 1)
                : QObject::tr("render %1 ms/frame").arg(renderMs, 0, 'f', 1);
        break;
    case BOTTLENECK_NETWORK:
        result.summary = QObject::tr("Your network is dropping frames. Lower the bitrate or move "
                                     "closer to the access point / use a wired connection.");
        result.keyMetric = QObject::tr("%1%% network drops").arg(networkDropPct, 0, 'f', 1);
        break;
    case BOTTLENECK_HOST:
        result.summary = QObject::tr("The host is slow to produce frames. This is a server-side "
                                     "bottleneck, not your client.");
        result.keyMetric = QObject::tr("host %1 ms/frame").arg(avgHostMs, 0, 'f', 1);
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
    for (int i = 0; i < m_Count; i++) {
        if (m_Samples[i] != BOTTLENECK_NONE) {
            count++;
        }
    }
    return count;
}

QString SpikeHistory::summarize() const
{
    int counts[BOTTLENECK_HOST + 1] = {};
    int spikes = 0;
    for (int i = 0; i < m_Count; i++) {
        if (m_Samples[i] != BOTTLENECK_NONE) {
            counts[m_Samples[i]]++;
            spikes++;
        }
    }

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
    case BOTTLENECK_DECODE:  causeText = QObject::tr("mostly decode"); break;
    case BOTTLENECK_RENDER:  causeText = QObject::tr("mostly pacing"); break;
    case BOTTLENECK_NETWORK: causeText = QObject::tr("mostly network"); break;
    case BOTTLENECK_HOST:    causeText = QObject::tr("mostly host"); break;
    default:                 causeText = QObject::tr("mixed"); break;
    }

    // Report the window length actually covered, in minutes (rounded up).
    const int minutes = (m_Count + 59) / 60;
    return QObject::tr("%1 spike(s) in last ~%2 min (%3)").arg(spikes).arg(minutes).arg(causeText);
}

void SpikeHistory::clear()
{
    m_Count = 0;
    m_Head = 0;
}

} // namespace Diagnostics
