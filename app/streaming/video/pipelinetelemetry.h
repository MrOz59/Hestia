#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <type_traits>

namespace PipelineTelemetry {

using Microseconds = std::chrono::microseconds;

// 0.25 ms buckets retain useful low-latency resolution while covering up to
// 128 ms. The final bucket also contains larger outliers.
constexpr std::size_t LATENCY_BUCKET_COUNT = 512;
constexpr Microseconds LATENCY_BUCKET_WIDTH {250};
constexpr Microseconds MAX_REPORTED_LATENCY {
    LATENCY_BUCKET_COUNT * LATENCY_BUCKET_WIDTH.count()
};

struct LatencyHistogram {
    std::array<uint32_t, LATENCY_BUCKET_COUNT> buckets {};
    uint64_t sampleCount = 0;
};

static_assert(std::is_trivially_copyable<LatencyHistogram>::value,
              "VIDEO_STATS is zeroed and copied as a trivial type");

struct LatencyPercentiles {
    Microseconds p50 {0};
    Microseconds p95 {0};
    Microseconds p99 {0};
    uint64_t sampleCount = 0;
};

// Packet queue depths use power-of-two upper bounds because a video frame may
// contain hundreds of RTP/FEC shards. The final bucket contains larger values.
constexpr std::size_t QUEUE_DEPTH_BUCKET_COUNT = 16;
constexpr std::array<uint32_t, QUEUE_DEPTH_BUCKET_COUNT>
        QUEUE_DEPTH_UPPER_BOUNDS {
            0, 1, 2, 4, 8, 16, 32, 64,
            128, 256, 512, 1024, 2048, 4096, 8192, 0xFFFFFFFFu,
        };

struct QueueDepthHistogram {
    std::array<uint32_t, QUEUE_DEPTH_BUCKET_COUNT> buckets {};
    uint64_t sampleCount = 0;
    uint32_t maxDepth = 0;
};

static_assert(std::is_trivially_copyable<QueueDepthHistogram>::value,
              "VIDEO_STATS is zeroed and copied as a trivial type");

struct QueueDepthPercentiles {
    uint32_t p50 = 0;
    uint32_t p95 = 0;
    uint32_t p99 = 0;
    uint32_t max = 0;
    uint64_t sampleCount = 0;
};

// O(1), allocation-free recording for latency-sensitive pipeline threads.
void recordLatency(LatencyHistogram& histogram, Microseconds latency);

// Histogram buckets are mergeable, allowing the existing one/two-second and
// whole-session statistics windows to retain valid percentile distributions.
void mergeLatency(const LatencyHistogram& source, LatencyHistogram& destination);

LatencyPercentiles summarizeLatency(const LatencyHistogram& histogram);

// Fixed-work, allocation-free recording for per-frame RTP/FEC queue peaks.
void recordQueueDepth(QueueDepthHistogram& histogram, uint32_t depth);
void mergeQueueDepth(const QueueDepthHistogram& source,
                     QueueDepthHistogram& destination);
QueueDepthPercentiles summarizeQueueDepth(
        const QueueDepthHistogram& histogram);

enum class FrameOutcome : uint8_t {
    Presented,
    Dropped,
};

enum class FrameTerminalReason : uint8_t {
    Presented,
    AwaitingIdr,
    DecodeSubmitFailed,
    QueueLimit,
    PacingBacklog,
    RenderBacklog,
    Shutdown,
};

struct FrameTrace {
    int64_t frameNumber = -1;
    uint64_t receiveUs = 0;
    uint64_t assembleUs = 0;
    uint64_t decodeUs = 0;
    uint64_t presentStartUs = 0;
    uint64_t terminalUs = 0;
    FrameOutcome outcome = FrameOutcome::Dropped;
    FrameTerminalReason reason = FrameTerminalReason::Shutdown;
};

/**
 * Fixed-size correlation timeline for decoded video frames.
 *
 * Frame numbers are the GameStream frame IDs emitted by Hermes. Slots are
 * indexed directly from the ID, so recording is bounded and allocation-free.
 * A mutex protects the decoder and presentation threads.
 */
class FrameTimeline {
public:
    static constexpr std::size_t MAX_ACTIVE_FRAMES = 512;

    void reset();
    void recordReceived(int64_t frameNumber,
                        uint64_t receiveUs,
                        uint64_t assembleUs);
    bool recordDecoded(int64_t frameNumber, uint64_t decodeUs);
    std::optional<FrameTrace> recordTerminal(
            int64_t frameNumber,
            uint64_t presentStartUs,
            uint64_t terminalUs,
            FrameOutcome outcome,
            FrameTerminalReason reason);

private:
    struct Slot {
        bool active = false;
        int64_t frameNumber = -1;
        uint64_t receiveUs = 0;
        uint64_t assembleUs = 0;
        uint64_t decodeUs = 0;
    };

    static std::size_t slotForFrame(int64_t frameNumber);

    std::array<Slot, MAX_ACTIVE_FRAMES> m_Slots {};
    std::mutex m_Mutex;
};

// Frame tracing is opt-in because it emits one structured log record per
// presented or discarded frame.
bool isFrameTraceEnabled();
void emitFrameTrace(const FrameTrace& trace);

// AVFrame::opaque is application-owned. Encoding frameNumber + 1 keeps null as
// the explicit "no trace ID" value without allocating per-frame metadata.
void* frameIdTag(int64_t frameNumber);
std::optional<int64_t> frameIdFromTag(const void* tag);

} // namespace PipelineTelemetry
