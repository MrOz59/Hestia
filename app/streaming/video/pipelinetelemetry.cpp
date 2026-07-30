#include "pipelinetelemetry.h"

#include "SDL_compat.h"

#include <algorithm>
#include <cstdlib>
#include <inttypes.h>
#include <limits>

namespace PipelineTelemetry {

namespace {

std::size_t bucketForLatency(Microseconds latency)
{
    const auto latencyUs = std::max<int64_t>(latency.count(), 0);
    if (latencyUs == 0) {
        return 0;
    }

    const auto bucket =
            static_cast<uint64_t>(latencyUs - 1) /
            static_cast<uint64_t>(LATENCY_BUCKET_WIDTH.count());
    return std::min<std::size_t>(bucket, LATENCY_BUCKET_COUNT - 1);
}

Microseconds percentile(const LatencyHistogram& histogram, uint64_t numerator)
{
    if (histogram.sampleCount == 0) {
        return Microseconds {0};
    }

    const uint64_t rank =
            (numerator * histogram.sampleCount + 99) / 100;
    uint64_t cumulative = 0;
    for (std::size_t bucket = 0; bucket < histogram.buckets.size(); bucket++) {
        cumulative += histogram.buckets[bucket];
        if (cumulative >= rank) {
            return Microseconds {
                static_cast<Microseconds::rep>(
                    (bucket + 1) * LATENCY_BUCKET_WIDTH.count())
            };
        }
    }

    return MAX_REPORTED_LATENCY;
}

uint32_t queueDepthPercentile(const QueueDepthHistogram& histogram,
                              uint64_t numerator)
{
    if (histogram.sampleCount == 0) {
        return 0;
    }

    const uint64_t rank =
            (numerator * histogram.sampleCount + 99) / 100;
    uint64_t cumulative = 0;
    for (std::size_t bucket = 0; bucket < histogram.buckets.size(); bucket++) {
        cumulative += histogram.buckets[bucket];
        if (cumulative >= rank) {
            return QUEUE_DEPTH_UPPER_BOUNDS[bucket];
        }
    }

    return QUEUE_DEPTH_UPPER_BOUNDS.back();
}

} // namespace

void recordLatency(LatencyHistogram& histogram, Microseconds latency)
{
    auto& bucket = histogram.buckets[bucketForLatency(latency)];
    if (bucket < std::numeric_limits<uint32_t>::max()) {
        bucket++;
    }
    if (histogram.sampleCount < std::numeric_limits<uint64_t>::max()) {
        histogram.sampleCount++;
    }
}

void mergeLatency(const LatencyHistogram& source, LatencyHistogram& destination)
{
    for (std::size_t bucket = 0; bucket < source.buckets.size(); bucket++) {
        const uint64_t merged =
                static_cast<uint64_t>(destination.buckets[bucket]) +
                source.buckets[bucket];
        destination.buckets[bucket] =
                static_cast<uint32_t>(std::min<uint64_t>(
                    merged, std::numeric_limits<uint32_t>::max()));
    }

    if (std::numeric_limits<uint64_t>::max() - destination.sampleCount <
            source.sampleCount) {
        destination.sampleCount = std::numeric_limits<uint64_t>::max();
    }
    else {
        destination.sampleCount += source.sampleCount;
    }
}

LatencyPercentiles summarizeLatency(const LatencyHistogram& histogram)
{
    return {
        percentile(histogram, 50),
        percentile(histogram, 95),
        percentile(histogram, 99),
        histogram.sampleCount,
    };
}

void recordQueueDepth(QueueDepthHistogram& histogram, uint32_t depth)
{
    const auto bucket = std::lower_bound(
            QUEUE_DEPTH_UPPER_BOUNDS.begin(),
            QUEUE_DEPTH_UPPER_BOUNDS.end(),
            depth);
    const auto index = static_cast<std::size_t>(
            std::distance(QUEUE_DEPTH_UPPER_BOUNDS.begin(), bucket));

    auto& count = histogram.buckets[index];
    if (count < std::numeric_limits<uint32_t>::max()) {
        count++;
    }
    if (histogram.sampleCount < std::numeric_limits<uint64_t>::max()) {
        histogram.sampleCount++;
    }
    histogram.maxDepth = std::max(histogram.maxDepth, depth);
}

void mergeQueueDepth(const QueueDepthHistogram& source,
                     QueueDepthHistogram& destination)
{
    for (std::size_t bucket = 0; bucket < source.buckets.size(); bucket++) {
        const uint64_t merged =
                static_cast<uint64_t>(destination.buckets[bucket]) +
                source.buckets[bucket];
        destination.buckets[bucket] =
                static_cast<uint32_t>(std::min<uint64_t>(
                    merged, std::numeric_limits<uint32_t>::max()));
    }

    if (std::numeric_limits<uint64_t>::max() - destination.sampleCount <
            source.sampleCount) {
        destination.sampleCount = std::numeric_limits<uint64_t>::max();
    }
    else {
        destination.sampleCount += source.sampleCount;
    }
    destination.maxDepth =
            std::max(destination.maxDepth, source.maxDepth);
}

QueueDepthPercentiles summarizeQueueDepth(
        const QueueDepthHistogram& histogram)
{
    return {
        queueDepthPercentile(histogram, 50),
        queueDepthPercentile(histogram, 95),
        queueDepthPercentile(histogram, 99),
        histogram.maxDepth,
        histogram.sampleCount,
    };
}

void FrameTimeline::reset()
{
    std::lock_guard<std::mutex> lock {m_Mutex};
    m_Slots = {};
}

void FrameTimeline::recordReceived(int64_t frameNumber,
                                   uint64_t receiveUs,
                                   uint64_t assembleUs)
{
    if (frameNumber < 0) {
        return;
    }

    std::lock_guard<std::mutex> lock {m_Mutex};
    m_Slots[slotForFrame(frameNumber)] = {
        true,
        frameNumber,
        receiveUs,
        assembleUs,
        0,
    };
}

bool FrameTimeline::recordDecoded(int64_t frameNumber, uint64_t decodeUs)
{
    if (frameNumber < 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock {m_Mutex};
    auto& slot = m_Slots[slotForFrame(frameNumber)];
    if (!slot.active || slot.frameNumber != frameNumber) {
        return false;
    }

    slot.decodeUs = decodeUs;
    return true;
}

std::optional<FrameTrace> FrameTimeline::recordTerminal(
        int64_t frameNumber,
        uint64_t presentStartUs,
        uint64_t terminalUs,
        FrameOutcome outcome,
        FrameTerminalReason reason)
{
    if (frameNumber < 0) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock {m_Mutex};
    auto& slot = m_Slots[slotForFrame(frameNumber)];
    if (!slot.active || slot.frameNumber != frameNumber) {
        return std::nullopt;
    }

    const FrameTrace trace {
        slot.frameNumber,
        slot.receiveUs,
        slot.assembleUs,
        slot.decodeUs,
        presentStartUs,
        terminalUs,
        outcome,
        reason,
    };
    slot = {};
    return trace;
}

std::size_t FrameTimeline::slotForFrame(int64_t frameNumber)
{
    return static_cast<uint64_t>(frameNumber) % MAX_ACTIVE_FRAMES;
}

bool isFrameTraceEnabled()
{
    static const bool enabled = [] {
        const char* value = std::getenv("HESTIA_FRAME_TRACE");
        return value != nullptr && value[0] != '\0' &&
                !(value[0] == '0' && value[1] == '\0');
    }();
    return enabled;
}

namespace {

const char* outcomeName(FrameOutcome outcome)
{
    switch (outcome) {
    case FrameOutcome::Presented:
        return "presented";
    case FrameOutcome::Dropped:
        return "dropped";
    }
    return "unknown";
}

const char* reasonName(FrameTerminalReason reason)
{
    switch (reason) {
    case FrameTerminalReason::Presented:
        return "presented";
    case FrameTerminalReason::AwaitingIdr:
        return "awaiting_idr";
    case FrameTerminalReason::DecodeSubmitFailed:
        return "decode_submit_failed";
    case FrameTerminalReason::QueueLimit:
        return "queue_limit";
    case FrameTerminalReason::PacingBacklog:
        return "pacing_backlog";
    case FrameTerminalReason::RenderBacklog:
        return "render_backlog";
    case FrameTerminalReason::Shutdown:
        return "shutdown";
    }
    return "unknown";
}

} // namespace

void emitFrameTrace(const FrameTrace& trace)
{
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "HESTIA_FRAME_TRACE "
        "{\"schema\":1,\"frame_id\":%" PRId64
        ",\"receive_us\":%" PRIu64
        ",\"assemble_us\":%" PRIu64
        ",\"decode_us\":%" PRIu64
        ",\"present_start_us\":%" PRIu64
        ",\"terminal_us\":%" PRIu64
        ",\"outcome\":\"%s\",\"reason\":\"%s\"}",
        trace.frameNumber,
        trace.receiveUs,
        trace.assembleUs,
        trace.decodeUs,
        trace.presentStartUs,
        trace.terminalUs,
        outcomeName(trace.outcome),
        reasonName(trace.reason));
}

void* frameIdTag(int64_t frameNumber)
{
    if (frameNumber < 0) {
        return nullptr;
    }

    const auto encoded = static_cast<uintptr_t>(frameNumber) + 1;
    return reinterpret_cast<void*>(encoded);
}

std::optional<int64_t> frameIdFromTag(const void* tag)
{
    const auto encoded = reinterpret_cast<uintptr_t>(tag);
    if (encoded == 0) {
        return std::nullopt;
    }
    return static_cast<int64_t>(encoded - 1);
}

} // namespace PipelineTelemetry
