#include "audiotelemetry.h"

#include <QCoreApplication>

namespace AudioTelemetry {

namespace {

class Translation
{
    Q_DECLARE_TR_FUNCTIONS(HestiaAudioDiagnostics)
};

template<typename T>
T counterDelta(T current, T previous) noexcept
{
    return current >= previous ?
        current - previous :
        current;
}

} // namespace

PipelineCounters subtract(
        const PipelineCounters& current,
        const PipelineCounters& previous) noexcept
{
    return {
        counterDelta(
            current.callbackPackets,
            previous.callbackPackets),
        counterDelta(
            current.decodedPackets,
            previous.decodedPackets),
        counterDelta(
            current.concealedPackets,
            previous.concealedPackets),
        counterDelta(
            current.decodeFailures,
            previous.decodeFailures),
        counterDelta(
            current.mutedPackets,
            previous.mutedPackets),
        counterDelta(
            current.recoveryDroppedPackets,
            previous.recoveryDroppedPackets),
        counterDelta(
            current.rendererUnavailablePackets,
            previous.rendererUnavailablePackets),
        counterDelta(
            current.rendererBufferFailures,
            previous.rendererBufferFailures),
        counterDelta(
            current.rendererFailures,
            previous.rendererFailures),
        counterDelta(
            current.rendererReinitializations,
            previous.rendererReinitializations),
    };
}

RendererMetrics subtract(
        const RendererMetrics& current,
        const RendererMetrics& previous) noexcept
{
    RendererMetrics result {
        counterDelta(
            current.submittedPackets,
            previous.submittedPackets),
        counterDelta(
            current.submittedBytes,
            previous.submittedBytes),
        counterDelta(
            current.backpressureSkippedPackets,
            previous.backpressureSkippedPackets),
        counterDelta(
            current.underruns,
            previous.underruns),
        counterDelta(
            current.queueFailures,
            previous.queueFailures),
        counterDelta(
            current.queueWaitTimeUs,
            previous.queueWaitTimeUs),
        counterDelta(
            current.queueWaitEvents,
            previous.queueWaitEvents),
    };
    result.currentQueuedDurationMs =
            current.currentQueuedDurationMs;
    result.highestObservedQueueDurationMs =
            current.highestObservedQueueDurationMs;
    result.deviceBufferDurationMs =
            current.deviceBufferDurationMs;
    result.queueLimitMs = current.queueLimitMs;
    result.upstreamBackpressureLimitMs =
            current.upstreamBackpressureLimitMs;
    result.queueDepthObservable =
            current.queueDepthObservable;
    result.deviceRunning = current.deviceRunning;
    return result;
}

NetworkMetrics subtract(
        const NetworkMetrics& current,
        const NetworkMetrics& previous) noexcept
{
    return {
        counterDelta(
            current.audioPackets,
            previous.audioPackets),
        counterDelta(
            current.fecPackets,
            previous.fecPackets),
        counterDelta(
            current.fecRecoveredPackets,
            previous.fecRecoveredPackets),
        counterDelta(
            current.fecFailedPackets,
            previous.fecFailedPackets),
        counterDelta(
            current.outOfSequencePackets,
            previous.outOfSequencePackets),
        counterDelta(
            current.invalidPackets,
            previous.invalidPackets),
        counterDelta(
            current.invalidFecPackets,
            previous.invalidFecPackets),
    };
}

void accumulate(
        const RendererMetrics& source,
        RendererMetrics& destination) noexcept
{
    destination.submittedPackets +=
            source.submittedPackets;
    destination.submittedBytes +=
            source.submittedBytes;
    destination.backpressureSkippedPackets +=
            source.backpressureSkippedPackets;
    destination.underruns += source.underruns;
    destination.queueFailures += source.queueFailures;
    destination.queueWaitTimeUs +=
            source.queueWaitTimeUs;
    destination.queueWaitEvents +=
            source.queueWaitEvents;
    if (source.highestObservedQueueDurationMs >
            destination.highestObservedQueueDurationMs) {
        destination.highestObservedQueueDurationMs =
                source.highestObservedQueueDurationMs;
    }
}

Diagnosis diagnose(const Snapshot& snapshot)
{
    Diagnosis result;

    if (snapshot.pipeline.rendererUnavailablePackets > 0 ||
            snapshot.pipeline.rendererBufferFailures > 0 ||
            snapshot.pipeline.rendererFailures > 0 ||
            snapshot.renderer.queueFailures > 0) {
        result.health = Health::RendererFailure;
        result.summary = Translation::tr(
            "The audio output device stopped or rejected data.");
        result.keyMetric = Translation::tr(
            "%1 renderer issue(s)")
                .arg(
                    snapshot.pipeline.rendererFailures +
                    snapshot.pipeline.rendererBufferFailures +
                    snapshot.pipeline.rendererUnavailablePackets +
                    snapshot.renderer.queueFailures);
        return result;
    }

    if (snapshot.renderer.underruns > 0) {
        result.health = Health::Underrunning;
        result.summary = Translation::tr(
            "The audio output buffer ran empty. Audio may stutter.");
        result.keyMetric = Translation::tr(
            "%1 underrun(s)")
                .arg(snapshot.renderer.underruns);
        return result;
    }

    if (snapshot.pipeline.concealedPackets > 0 ||
            snapshot.network.fecFailedPackets > 0) {
        result.health = Health::NetworkLoss;
        result.summary = Translation::tr(
            "Audio packets were lost and concealed.");
        result.keyMetric = Translation::tr(
            "%1 concealed packet(s)")
                .arg(snapshot.pipeline.concealedPackets);
        return result;
    }

    if (snapshot.renderer.backpressureSkippedPackets > 0) {
        result.health = Health::Backpressured;
        result.summary = Translation::tr(
            "Audio was discarded to prevent the playback queue from growing.");
        result.keyMetric = Translation::tr(
            "%1 backpressure discard(s)")
                .arg(
                    snapshot.renderer
                        .backpressureSkippedPackets);
    }

    return result;
}

} // namespace AudioTelemetry
