#pragma once

#include "backend/hestiacapabilities.h"

namespace HestiaNegotiation {

struct StreamMode
{
    int width = 0;
    int height = 0;
    int fps = 0;
    int bitrateKbps = 0;
};

struct Result
{
    StreamMode requested;
    StreamMode effective;
    bool resolutionAdjusted = false;
    bool fpsAdjusted = false;
    bool bitrateAdjusted = false;
};

// Applies the host's advertised limits without mutating session state. When
// adjustPresetBitrate is true, bitrate is scaled to preserve the preset's
// quality ratio after a resolution/FPS adjustment.
Result negotiateStreamMode(const StreamMode& requested,
                           const HestiaLimits& limits,
                           bool adjustPresetBitrate,
                           bool yuv444);

} // namespace HestiaNegotiation
