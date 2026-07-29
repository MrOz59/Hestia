#include "hestianegotiation.h"

#include "settings/streamingpreferences.h"

#include <QtMath>

namespace HestiaNegotiation {

Result negotiateStreamMode(const StreamMode& requested,
                           const HestiaLimits& limits,
                           bool adjustPresetBitrate,
                           bool yuv444)
{
    Result result;
    result.requested = requested;
    result.effective = requested;

    if (limits.maxWidth > 0 && limits.maxHeight > 0 &&
            (requested.width > limits.maxWidth || requested.height > limits.maxHeight)) {
        const double scale = qMin(static_cast<double>(limits.maxWidth) / requested.width,
                                  static_cast<double>(limits.maxHeight) / requested.height);
        result.effective.width = qMax(2, qFloor(requested.width * scale)) & ~1;
        result.effective.height = qMax(2, qFloor(requested.height * scale)) & ~1;
        result.resolutionAdjusted = true;
    }

    int highestSupportedFps = 0;
    int lowestSupportedFpsAboveRequest = 0;
    for (const int supportedFps : limits.supportedFps) {
        if (limits.maxFps > 0 && supportedFps > limits.maxFps) {
            continue;
        }

        if (supportedFps <= requested.fps) {
            highestSupportedFps = qMax(highestSupportedFps, supportedFps);
        }
        else if (lowestSupportedFpsAboveRequest == 0 ||
                 supportedFps < lowestSupportedFpsAboveRequest) {
            lowestSupportedFpsAboveRequest = supportedFps;
        }
    }

    if (highestSupportedFps > 0) {
        result.effective.fps = highestSupportedFps;
    }
    else if (lowestSupportedFpsAboveRequest > 0) {
        result.effective.fps = lowestSupportedFpsAboveRequest;
    }
    else if (limits.maxFps > 0) {
        result.effective.fps = qMin(requested.fps, limits.maxFps);
    }
    result.fpsAdjusted = result.effective.fps != requested.fps;

    if (adjustPresetBitrate && (result.resolutionAdjusted || result.fpsAdjusted)) {
        result.effective.bitrateKbps = StreamingPreferences::scaleBitrateForMode(
                requested.bitrateKbps,
                requested.width,
                requested.height,
                requested.fps,
                result.effective.width,
                result.effective.height,
                result.effective.fps,
                yuv444);
        result.bitrateAdjusted =
                result.effective.bitrateKbps != requested.bitrateKbps;
    }

    return result;
}

} // namespace HestiaNegotiation
