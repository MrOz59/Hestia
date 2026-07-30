#pragma once

#include <QString>

#include <cstdint>

namespace AudioTelemetry {

struct PipelineCounters {
    // Includes both payload callbacks and zero-length callbacks used for
    // packet-loss concealment. NetworkMetrics carries actual RTP arrivals.
    uint64_t callbackPackets = 0;
    uint64_t decodedPackets = 0;
    uint64_t concealedPackets = 0;
    uint64_t decodeFailures = 0;
    uint64_t mutedPackets = 0;
    uint64_t recoveryDroppedPackets = 0;
    uint64_t rendererUnavailablePackets = 0;
    uint64_t rendererBufferFailures = 0;
    uint64_t rendererFailures = 0;
    uint64_t rendererReinitializations = 0;
};

struct RendererMetrics {
    uint64_t submittedPackets = 0;
    uint64_t submittedBytes = 0;
    uint64_t backpressureSkippedPackets = 0;
    uint64_t underruns = 0;
    uint64_t queueFailures = 0;
    uint64_t queueWaitTimeUs = 0;
    uint64_t queueWaitEvents = 0;

    uint32_t currentQueuedDurationMs = 0;
    uint32_t highestObservedQueueDurationMs = 0;
    uint32_t deviceBufferDurationMs = 0;
    uint32_t queueLimitMs = 0;
    uint32_t upstreamBackpressureLimitMs = 0;
    bool queueDepthObservable = false;
    bool deviceRunning = false;
};

struct NetworkMetrics {
    uint64_t audioPackets = 0;
    uint64_t fecPackets = 0;
    uint64_t fecRecoveredPackets = 0;
    uint64_t fecFailedPackets = 0;
    uint64_t outOfSequencePackets = 0;
    uint64_t invalidPackets = 0;
    uint64_t invalidFecPackets = 0;
};

struct Snapshot {
    PipelineCounters pipeline;
    RendererMetrics renderer;
    NetworkMetrics network;

    uint32_t pendingNetworkDurationMs = 0;
    uint32_t estimatedPresentationDelayMs = 0;
    uint64_t measurementStartUs = 0;
    uint64_t measurementEndUs = 0;
};

enum class Health {
    Healthy,
    Underrunning,
    NetworkLoss,
    Backpressured,
    RendererFailure,
};

struct Diagnosis {
    Health health = Health::Healthy;
    QString summary;
    QString keyMetric;
};

PipelineCounters subtract(
        const PipelineCounters& current,
        const PipelineCounters& previous) noexcept;
RendererMetrics subtract(
        const RendererMetrics& current,
        const RendererMetrics& previous) noexcept;
NetworkMetrics subtract(
        const NetworkMetrics& current,
        const NetworkMetrics& previous) noexcept;

void accumulate(
        const RendererMetrics& source,
        RendererMetrics& destination) noexcept;

Diagnosis diagnose(const Snapshot& snapshot);

} // namespace AudioTelemetry
