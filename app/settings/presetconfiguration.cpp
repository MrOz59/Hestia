#include "streamingpreferences.h"

#include <QtMath>

int StreamingPreferences::getDefaultBitrate(int width, int height, int fps, bool yuv444)
{
    // Don't scale bitrate linearly beyond 60 FPS. It's definitely not a linear
    // bitrate increase for frame rate once we get to values that high.
    float frameRateFactor = (fps <= 60 ? fps : (qSqrt(fps / 60.f) * 60.f)) / 30.f;

    // These are the long-standing Shield/Moonlight defaults.
    static const struct {
        int pixels;
        int factor;
    } resTable[] {
        { 640 * 360, 1 },
        { 854 * 480, 2 },
        { 1280 * 720, 5 },
        { 1920 * 1080, 10 },
        { 2560 * 1440, 20 },
        { 3840 * 2160, 40 },
        { -1, -1 },
    };

    float resolutionFactor;
    const qint64 pixelCount = static_cast<qint64>(width) * height;
    for (int i = 0;; i++) {
        if (pixelCount == resTable[i].pixels) {
            resolutionFactor = resTable[i].factor;
            break;
        }
        if (pixelCount < resTable[i].pixels) {
            if (i == 0) {
                resolutionFactor = resTable[i].factor;
            }
            else {
                resolutionFactor =
                        (static_cast<float>(pixelCount - resTable[i - 1].pixels) /
                         (resTable[i].pixels - resTable[i - 1].pixels)) *
                        (resTable[i].factor - resTable[i - 1].factor) +
                        resTable[i - 1].factor;
            }
            break;
        }
        if (resTable[i].pixels == -1) {
            resolutionFactor = resTable[i - 1].factor;
            break;
        }
    }

    if (yuv444) {
        resolutionFactor *= 2;
    }

    return qRound(resolutionFactor * frameRateFactor) * 1000;
}

StreamingPreferences::PresetConfiguration StreamingPreferences::calculatePreset(
        StreamingPreset preset,
        int nativeWidth,
        int nativeHeight,
        int nativeFps,
        bool yuv444)
{
    if (nativeWidth <= 0 || nativeHeight <= 0) {
        nativeWidth = 1920;
        nativeHeight = 1080;
    }
    if (nativeFps <= 0) {
        nativeFps = 60;
    }

    PresetConfiguration configuration {
        nativeWidth,
        nativeHeight,
        nativeFps,
        0,
    };
    double bitrateScale = 1.0;

    // Fit inside both dimensions while preserving aspect ratio. Pixel-count
    // capping alone can exceed maxWidth on ultrawide displays.
    auto capResolution = [&](int maxWidth, int maxHeight) {
        const double scale = qMin(1.0,
                                  qMin(static_cast<double>(maxWidth) / configuration.width,
                                       static_cast<double>(maxHeight) / configuration.height));
        if (scale < 1.0) {
            configuration.width = qMax(2, qFloor(configuration.width * scale)) & ~1;
            configuration.height = qMax(2, qFloor(configuration.height * scale)) & ~1;
        }
    };

    switch (preset) {
    case PRESET_QUALITY:
        bitrateScale = 1.25;
        break;
    case PRESET_BALANCED:
        configuration.fps = qMin(nativeFps, 60);
        break;
    case PRESET_FAST:
        capResolution(1920, 1080);
        configuration.fps = qMin(nativeFps, 60);
        bitrateScale = 0.9;
        break;
    case PRESET_BATTERY:
        capResolution(1280, 720);
        configuration.fps = qMin(nativeFps, 30);
        bitrateScale = 0.7;
        break;
    case PRESET_CUSTOM:
        break;
    }

    configuration.bitrateKbps = qRound(
            getDefaultBitrate(configuration.width,
                              configuration.height,
                              configuration.fps,
                              yuv444) *
            bitrateScale);
    return configuration;
}

int StreamingPreferences::scaleBitrateForMode(int bitrateKbps,
                                              int sourceWidth,
                                              int sourceHeight,
                                              int sourceFps,
                                              int targetWidth,
                                              int targetHeight,
                                              int targetFps,
                                              bool yuv444)
{
    const int sourceDefault = getDefaultBitrate(sourceWidth, sourceHeight, sourceFps, yuv444);
    const int targetDefault = getDefaultBitrate(targetWidth, targetHeight, targetFps, yuv444);
    if (bitrateKbps <= 0 || sourceDefault <= 0 || targetDefault <= 0) {
        return bitrateKbps;
    }

    const double qualityRatio = static_cast<double>(bitrateKbps) / sourceDefault;
    return qMax(1000, qRound(targetDefault * qualityRatio));
}
