#include "legacysessiontelemetry.h"

#include "SDL_compat.h"
#include "streaming/video/overlaymanager.h"

#include <cinttypes>
#include <cstdio>

namespace SessionTelemetry {

LegacySessionTelemetry::LegacySessionTelemetry(
        Overlay::OverlayManager* overlayManager)
    : m_OverlayManager(overlayManager)
{
}

Mode LegacySessionTelemetry::mode() const noexcept
{
    return Mode::LegacyLocal;
}

bool LegacySessionTelemetry::frameTracingEnabled() const noexcept
{
    return PipelineTelemetry::isFrameTraceEnabled();
}

void LegacySessionTelemetry::publishFrameTrace(
        const PipelineTelemetry::FrameTrace& trace)
{
    PipelineTelemetry::emitFrameTrace(trace);
}

void LegacySessionTelemetry::publishStageStarted(
        const StageEvent& event)
{
    Q_UNUSED(event);
}

void LegacySessionTelemetry::publishStageFailed(
        const StageFailure& event)
{
    Q_UNUSED(event);
}

void LegacySessionTelemetry::publishTermination(
        const TerminationEvent& event)
{
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "Connection terminated: %d",
        event.errorCode);
}

void LegacySessionTelemetry::publishConnectionQuality(
        const ConnectionQualityEvent& event)
{
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "Connection status update: %d",
        event.nativeStatus);

    if (m_OverlayManager == nullptr ||
            !event.warningsEnabled ||
            event.overlaySuppressed) {
        return;
    }

    switch (event.quality) {
    case ConnectionQuality::Poor:
        m_OverlayManager->updateOverlayText(
            Overlay::OverlayStatusUpdate,
            event.bitrateKbps > 5000 ?
                "Slow connection to PC\nReduce your bitrate" :
                "Poor connection to PC");
        m_OverlayManager->setOverlayState(
            Overlay::OverlayStatusUpdate,
            true);
        break;
    case ConnectionQuality::Okay:
        m_OverlayManager->setOverlayState(
            Overlay::OverlayStatusUpdate,
            false);
        break;
    case ConnectionQuality::Unknown:
        break;
    }
}

void LegacySessionTelemetry::publishVideoWindow(
        const VideoWindowReport& report)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_SpikeHistory.record(
        Diagnostics::diagnose(
            report.completedWindow,
            report.stream.targetFrameRate));

    if (m_OverlayManager == nullptr ||
            !m_OverlayManager->isOverlayEnabled(
                Overlay::OverlayDebug)) {
        return;
    }

    char* overlayText = m_OverlayManager->getOverlayText(
        Overlay::OverlayDebug);
    const int overlayLength =
            m_OverlayManager->getOverlayMaxTextLength();
    stringifyVideoStats(
        report.displayWindow,
        report.stream,
        overlayText,
        overlayLength);
    appendDiagnosisText(
        Diagnostics::diagnose(
            report.displayWindow,
            report.stream.targetFrameRate),
        m_SpikeHistory,
        overlayText,
        overlayLength);
    if (m_LastAudioReport) {
        appendAudioStats(
            *m_LastAudioReport,
            overlayText,
            overlayLength);
    }
    m_OverlayManager->setOverlayTextUpdated(
        Overlay::OverlayDebug);
}

void LegacySessionTelemetry::publishVideoSessionSummary(
        const VideoSessionSummary& summary)
{
    if (summary.aggregate.renderedFps <= 0 &&
            summary.aggregate.renderedFrames == 0) {
        return;
    }

    char videoStats[2048];
    stringifyVideoStats(
        summary.aggregate,
        summary.stream,
        videoStats,
        sizeof(videoStats));
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "\nGlobal video stats\n------------------\n%s",
        videoStats);
}

void LegacySessionTelemetry::publishAudioWindow(
        const AudioWindowReport& report)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_LastAudioReport = report;
}

void LegacySessionTelemetry::publishAudioSessionSummary(
        const AudioSessionSummary& summary)
{
    if (summary.aggregate.pipeline.callbackPackets == 0) {
        return;
    }

    char audioStats[1024];
    stringifyAudioStats(
        summary.aggregate,
        summary.stream,
        audioStats,
        sizeof(audioStats));
    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "\nGlobal audio stats\n------------------\n%s",
        audioStats);
}

const char* LegacySessionTelemetry::codecName(
        const VideoStreamContext& stream) noexcept
{
    switch (stream.codec) {
    case VideoCodec::H264:
        return stream.chroma444 ? "H.264 4:4:4" : "H.264";
    case VideoCodec::Hevc:
        if (stream.tenBit) {
            if (stream.hdr) {
                return stream.chroma444 ?
                    "HEVC 10-bit HDR 4:4:4" :
                    "HEVC 10-bit HDR";
            }
            return stream.chroma444 ?
                "HEVC 10-bit SDR 4:4:4" :
                "HEVC 10-bit SDR";
        }
        return stream.chroma444 ? "HEVC 4:4:4" : "HEVC";
    case VideoCodec::Av1:
        if (stream.tenBit) {
            if (stream.hdr) {
                return stream.chroma444 ?
                    "AV1 10-bit HDR 4:4:4" :
                    "AV1 10-bit HDR";
            }
            return stream.chroma444 ?
                "AV1 10-bit SDR 4:4:4" :
                "AV1 10-bit SDR";
        }
        return stream.chroma444 ? "AV1 4:4:4" : "AV1";
    case VideoCodec::Unknown:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

void LegacySessionTelemetry::stringifyVideoStats(
        const VIDEO_STATS& stats,
        const VideoStreamContext& stream,
        char* output,
        int length)
{
    int offset = 0;
    int ret;
    output[offset] = 0;

    if (stats.receivedFps > 0) {
        ret = std::snprintf(
            &output[offset],
            length - offset,
            "Video stream: %dx%d %.2f FPS (Codec: %s)\n"
#ifdef DISPLAY_BITRATE
            "Bitrate: %.1f Mbps, Peak (%us): %.1f\n"
#endif
            "Incoming frame rate from network: %.2f FPS\n"
            "Decoding frame rate: %.2f FPS\n"
            "Rendering frame rate: %.2f FPS\n",
            stream.width,
            stream.height,
            stats.totalFps,
            codecName(stream),
#ifdef DISPLAY_BITRATE
            stream.averageMegabitsPerSecond,
            stream.peakBitrateWindowSeconds,
            stream.peakMegabitsPerSecond,
#endif
            stats.receivedFps,
            stats.decodedFps,
            stats.renderedFps);
        if (ret < 0 || ret >= length - offset) {
            SDL_assert(false);
            return;
        }
        offset += ret;
    }

    if (stats.framesWithHostProcessingLatency > 0) {
        ret = std::snprintf(
            &output[offset],
            length - offset,
            "Host processing latency min/max/average: %.1f/%.1f/%.1f ms\n",
            static_cast<float>(stats.minHostProcessingLatency) / 10,
            static_cast<float>(stats.maxHostProcessingLatency) / 10,
            static_cast<float>(stats.totalHostProcessingLatency) /
                10 / stats.framesWithHostProcessingLatency);
        if (ret < 0 || ret >= length - offset) {
            SDL_assert(false);
            return;
        }
        offset += ret;
    }

    if (stats.renderedFrames == 0) {
        return;
    }

    char rttString[32];
    const auto reassemblyLatency =
            PipelineTelemetry::summarizeLatency(
                stats.reassemblyLatency);
    const auto decodeLatency =
            PipelineTelemetry::summarizeLatency(
                stats.decodeLatency);
    const auto pacerLatency =
            PipelineTelemetry::summarizeLatency(
                stats.pacerLatency);
    const auto renderLatency =
            PipelineTelemetry::summarizeLatency(
                stats.renderLatency);
    const auto rtpFecQueueDepth =
            PipelineTelemetry::summarizeQueueDepth(
                stats.rtpFecQueueDepth);
    const auto milliseconds =
            [](PipelineTelemetry::Microseconds value) {
        return value.count() / 1000.0;
    };

    if (stats.lastRtt != 0) {
        std::snprintf(
            rttString,
            sizeof(rttString),
            "%u ms (variance: %u ms)",
            stats.lastRtt,
            stats.lastRttVariance);
    }
    else {
        std::snprintf(
            rttString,
            sizeof(rttString),
            "N/A");
    }

    ret = std::snprintf(
        &output[offset],
        length - offset,
        "Frames dropped by your network connection: %.2f%%\n"
        "Frames dropped by the frame pacer: %.2f%%\n"
        "Average network latency: %s\n"
        "Average decoding time: %.2f ms\n"
        "Average frame queue delay: %.2f ms\n"
        "Average rendering time (including monitor V-sync latency): %.2f ms\n"
        "Reassembly p50/p95/p99: %.2f/%.2f/%.2f ms\n"
        "Decode p50/p95/p99: %.2f/%.2f/%.2f ms\n"
        "Frame queue p50/p95/p99: %.2f/%.2f/%.2f ms\n"
        "Render p50/p95/p99: %.2f/%.2f/%.2f ms\n"
        "RTP reorder/FEC queue peak packets p50/p95/p99/max: %u/%u/%u/%u\n",
        static_cast<float>(stats.networkDroppedFrames) /
            stats.totalFrames * 100,
        static_cast<float>(stats.pacerDroppedFrames) /
            stats.decodedFrames * 100,
        rttString,
        (stats.totalDecodeTimeUs / 1000.0) /
            stats.decodedFrames,
        (stats.totalPacerTimeUs / 1000.0) /
            stats.renderedFrames,
        (stats.totalRenderTimeUs / 1000.0) /
            stats.renderedFrames,
        milliseconds(reassemblyLatency.p50),
        milliseconds(reassemblyLatency.p95),
        milliseconds(reassemblyLatency.p99),
        milliseconds(decodeLatency.p50),
        milliseconds(decodeLatency.p95),
        milliseconds(decodeLatency.p99),
        milliseconds(pacerLatency.p50),
        milliseconds(pacerLatency.p95),
        milliseconds(pacerLatency.p99),
        milliseconds(renderLatency.p50),
        milliseconds(renderLatency.p95),
        milliseconds(renderLatency.p99),
        rtpFecQueueDepth.p50,
        rtpFecQueueDepth.p95,
        rtpFecQueueDepth.p99,
        rtpFecQueueDepth.max);
    if (ret < 0 || ret >= length - offset) {
        SDL_assert(false);
        return;
    }
    offset += ret;

    if (!stream.presentationPath.isEmpty()) {
        ret = std::snprintf(
            &output[offset],
            length - offset,
            "Presentation path: %s at %.3f Hz\n"
            "Decoder queue depth 0/1/2/3+: %u/%u/%u/%u\n"
            "Pacing queue depth 0/1/2/3+: %u/%u/%u/%u (target: %u)\n"
            "Render queue depth 0/1/2/3+: %u/%u/%u/%u (target: %u)\n",
            qPrintable(stream.presentationPath),
            stream.displayRefreshRateMillihertz / 1000.0,
            stats.decoderQueueDepth[0],
            stats.decoderQueueDepth[1],
            stats.decoderQueueDepth[2],
            stats.decoderQueueDepth[3],
            stats.pacingQueueDepth[0],
            stats.pacingQueueDepth[1],
            stats.pacingQueueDepth[2],
            stats.pacingQueueDepth[3],
            stats.pacingQueueTarget,
            stats.renderQueueDepth[0],
            stats.renderQueueDepth[1],
            stats.renderQueueDepth[2],
            stats.renderQueueDepth[3],
            stats.renderQueueTarget);
        if (ret < 0 || ret >= length - offset) {
            SDL_assert(false);
            return;
        }
        offset += ret;
    }

    if (stats.vsyncIntervals != 0) {
        ret = std::snprintf(
            &output[offset],
            length - offset,
            "V-Sync interval average/max: %.2f/%.2f ms (%u late)\n",
            (stats.totalVsyncIntervalUs / 1000.0) /
                stats.vsyncIntervals,
            stats.maxVsyncIntervalUs / 1000.0,
            stats.lateVsyncIntervals);
        if (ret < 0 || ret >= length - offset) {
            SDL_assert(false);
        }
    }
}

void LegacySessionTelemetry::appendDiagnosisText(
        const Diagnostics::Diagnosis& diagnosis,
        const Diagnostics::SpikeHistory& history,
        char* output,
        int length)
{
    const int offset = static_cast<int>(
        SDL_strlen(output));
    if (offset >= length) {
        return;
    }

    const QByteArray diagnosisText =
            (QLatin1Char('\n') +
             Diagnostics::formatOverlayText(
                 diagnosis,
                 history)).toUtf8();
    if (!diagnosisText.isEmpty()) {
        std::snprintf(
            &output[offset],
            length - offset,
            "%s",
            diagnosisText.constData());
    }
}

void LegacySessionTelemetry::appendAudioStats(
        const AudioWindowReport& report,
        char* output,
        int length)
{
    int offset = static_cast<int>(
        SDL_strlen(output));
    if (offset >= length) {
        return;
    }

    const int ret = std::snprintf(
        &output[offset],
        length - offset,
        "\n");
    if (ret < 0 || ret >= length - offset) {
        return;
    }
    offset += ret;

    stringifyAudioStats(
        report.completedWindow,
        report.stream,
        &output[offset],
        length - offset);
}

void LegacySessionTelemetry::stringifyAudioStats(
        const AudioTelemetry::Snapshot& stats,
        const AudioStreamContext& stream,
        char* output,
        int length)
{
    int offset = 0;
    output[0] = 0;

    int ret = std::snprintf(
        &output[offset],
        length - offset,
        "Audio stream: %u Hz, %u channel(s), %u ms packets (Renderer: %s, Profile: %s)\n"
        "Audio queue receiver/playback/device/estimated: %u/%u/%u/%u ms\n"
        "Audio decoded/concealed/underruns: %" PRIu64 "/%" PRIu64 "/%" PRIu64 "\n"
        "Audio backpressure/recovery drops: %" PRIu64 "/%" PRIu64 "\n",
        stream.sampleRate,
        static_cast<unsigned int>(stream.channelCount),
        stream.packetDurationMs(),
        qPrintable(stream.rendererName),
        qPrintable(stream.bufferingProfileName),
        stats.pendingNetworkDurationMs,
        stats.renderer.currentQueuedDurationMs,
        stats.renderer.deviceBufferDurationMs,
        stats.estimatedPresentationDelayMs,
        stats.pipeline.decodedPackets,
        stats.pipeline.concealedPackets,
        stats.renderer.underruns,
        stats.renderer.backpressureSkippedPackets,
        stats.pipeline.recoveryDroppedPackets);
    if (ret < 0 || ret >= length - offset) {
        SDL_assert(false);
        return;
    }
    offset += ret;

    const AudioTelemetry::Diagnosis diagnosis =
            AudioTelemetry::diagnose(stats);
    if (diagnosis.health !=
            AudioTelemetry::Health::Healthy) {
        const QByteArray summary =
                diagnosis.summary.toUtf8();
        const QByteArray metric =
                diagnosis.keyMetric.toUtf8();
        ret = std::snprintf(
            &output[offset],
            length - offset,
            "Audio diagnosis: %s (%s)\n",
            summary.constData(),
            metric.constData());
        if (ret < 0 || ret >= length - offset) {
            SDL_assert(false);
        }
    }
}

} // namespace SessionTelemetry
