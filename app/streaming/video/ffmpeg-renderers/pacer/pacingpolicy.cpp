#include "pacingpolicy.h"

#include <QtGlobal>

namespace PacingPolicy {

namespace {

int samplesForDuration(int displayRateMillihertz, int durationMs)
{
    const qint64 numerator = static_cast<qint64>(displayRateMillihertz) * durationMs;
    return qMax(1, static_cast<int>((numerator + 999999) / 1000000));
}

} // namespace

int displayRateMillihertzFromSdlRate(int refreshRateHz)
{
    if (refreshRateHz <= 0) {
        return 60000;
    }

    switch (refreshRateHz) {
    case 23: return 23976;
    case 29: return 29970;
    case 47: return 47952;
    case 59: return 59940;
    case 119: return 119880;
    case 143: return 143856;
    default: return refreshRateHz * 1000;
    }
}

int rendererPresentationRateMillihertz(int displayRateMillihertz, int streamFps)
{
    return qMin(qMax(1000, displayRateMillihertz),
                qMax(1, streamFps) * 1000);
}

void AdaptiveQueueDepth::configure(int displayRateMillihertz)
{
    const int safeDisplayRate = qMax(1000, displayRateMillihertz);

    // Raise quickly enough to absorb a jitter burst, then lower slowly to avoid
    // oscillating between smoothness and latency when the link is borderline.
    m_RaiseThresholdSamples = samplesForDuration(safeDisplayRate, 250);
    m_LowerThresholdSamples = samplesForDuration(safeDisplayRate, 2000);
    m_RaiseEvidence = 0;
    m_LowerEvidence = 0;
    m_TargetDepth = 2;
}

void AdaptiveQueueDepth::observeNetworkJitter(int rttVarianceMs)
{
    const int desiredDepth = desiredDepthForJitter(rttVarianceMs);
    if (desiredDepth > m_TargetDepth) {
        m_LowerEvidence = 0;
        if (++m_RaiseEvidence >= m_RaiseThresholdSamples) {
            m_TargetDepth = desiredDepth;
            m_RaiseEvidence = 0;
        }
    }
    else if (desiredDepth < m_TargetDepth) {
        m_RaiseEvidence = 0;
        if (++m_LowerEvidence >= m_LowerThresholdSamples) {
            m_TargetDepth = desiredDepth;
            m_LowerEvidence = 0;
        }
    }
    else {
        m_RaiseEvidence = 0;
        m_LowerEvidence = 0;
    }
}

int AdaptiveQueueDepth::targetDepth() const
{
    return m_TargetDepth;
}

int AdaptiveQueueDepth::desiredDepthForJitter(int rttVarianceMs)
{
    if (rttVarianceMs < 0) {
        return 2;
    }
    if (rttVarianceMs <= 4) {
        return 1;
    }
    if (rttVarianceMs <= 12) {
        return 2;
    }
    return 3;
}

} // namespace PacingPolicy
