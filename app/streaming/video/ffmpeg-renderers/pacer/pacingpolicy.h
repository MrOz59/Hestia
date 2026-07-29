#pragma once

namespace PacingPolicy {

// Converts SDL2's integer refresh rate to millihertz, reconstructing the common
// NTSC-derived rates that SDL2 truncates.
int displayRateMillihertzFromSdlRate(int refreshRateHz);

// Renderer-driven paths present no faster than either the stream or display.
int rendererPresentationRateMillihertz(int displayRateMillihertz, int streamFps);

class AdaptiveQueueDepth
{
public:
    void configure(int displayRateMillihertz);
    void observeNetworkJitter(int rttVarianceMs);

    int targetDepth() const;

private:
    static int desiredDepthForJitter(int rttVarianceMs);

    int m_RaiseThresholdSamples = 1;
    int m_LowerThresholdSamples = 1;
    int m_RaiseEvidence = 0;
    int m_LowerEvidence = 0;
    int m_TargetDepth = 2;
};

} // namespace PacingPolicy
