#pragma once

#include <QString>

#include "decoder.h"

// Phase 0 of the client roadmap: interpret the per-frame statistics that the
// decoder already collects (see VIDEO_STATS) and turn them into a plain-language
// verdict about where a stutter is coming from. This is pure, side-effect-free
// analysis over a stats window; it does not touch the decode/render pipeline.

namespace Diagnostics {

enum Bottleneck {
    // Stream is healthy: nothing is meaningfully holding it back.
    BOTTLENECK_NONE,
    // The local decoder cannot keep up (decode time per frame too high, or
    // decoded FPS falling short of received FPS).
    BOTTLENECK_DECODE,
    // Presentation/pacing is the problem: frames are dropped by the pacer or
    // render+vsync time is excessive.
    BOTTLENECK_RENDER,
    // The network is losing or jittering frames before they reach us.
    BOTTLENECK_NETWORK,
    // The host is slow to produce frames (server-side, not the client).
    BOTTLENECK_HOST,
};

struct Diagnosis {
    Bottleneck dominant = BOTTLENECK_NONE;

    // A single human-readable sentence describing the verdict and, when there
    // is a problem, a concrete suggestion. Already translated.
    QString summary;

    // The driving metric for the verdict, so the overlay can show the one
    // number that matters instead of all of them. Empty when healthy.
    QString keyMetric;
};

// Classifies the dominant bottleneck for a one-or-two-second stats window.
// targetFps is the configured stream frame rate, used to judge whether decoded/
// rendered throughput is falling short. A window with too few frames to judge
// returns BOTTLENECK_NONE with an empty summary.
Diagnosis diagnose(const VIDEO_STATS& stats, int targetFps);

// Fixed-size ring buffer that remembers recent per-second diagnoses so that
// intermittent spikes remain visible after the fact (roadmap item 0.3). Not
// thread-safe; intended to be owned and driven by the decode thread.
class SpikeHistory {
public:
    static const int CAPACITY = 300; // ~5 minutes at one sample per second

    void record(const Diagnosis& diagnosis);

    // Number of non-healthy samples currently retained.
    int spikeCount() const;

    // A short summary like "3 spikes in last 5 min (mostly network)", or an
    // empty string if there have been no spikes. classified by dominant cause.
    QString summarize() const;

    void clear();

private:
    Bottleneck m_Samples[CAPACITY] = {};
    int m_Count = 0;   // number of valid samples (capped at CAPACITY)
    int m_Head = 0;    // next write position
};

} // namespace Diagnostics
