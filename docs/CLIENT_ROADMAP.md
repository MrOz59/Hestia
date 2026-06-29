# Hestia Client-Side Roadmap

Status: **draft for review** · Last updated: 2026-06-26

## Scope

Hestia is a **desktop** Moonlight-Qt fork (C++/Qt) targeting Windows, Linux,
macOS, **and Linux handhelds** (Steam Deck and similar — they run the desktop
build). This roadmap deliberately **excludes** mobile/TV-only concerns that do
not apply to a desktop client:

- Touch controls / on-screen gamepad / per-game touch layouts
- Virtual keyboard, soft-keyboard switching
- Android-decoder-vendor variance, Android background-kill, Android permissions
- "Console mode" UI for a 10-foot TV experience as a primary target

Where a mobile dor has a desktop analogue (e.g. "battery/thermal" → handheld
power profiles; "TV UX" → gamepad navigation polish), it is folded into a
desktop-shaped item below.

The ordering below is **technical**, not a copy of the source priority list: we
start with low-risk, high-leverage work that also produces the telemetry later
phases depend on, then move into the sensitive pipeline code, then convenience.

## Current-state anchors (verified in the tree)

These already exist and the roadmap builds on them rather than reinventing:

- Rich per-frame stats already collected in `_VIDEO_STATS`
  ([decoder.h:11](../app/streaming/video/decoder.h#L11)): decode/pacer/render/
  reassembly times (µs), RTT + RTT variance (jitter), and dropped frames split
  by cause (`networkDroppedFrames` vs `pacerDroppedFrames`). **The data for
  automatic diagnosis is already there — today it is only shown as raw numbers.**
- A real frame pacer with per-platform vsync sources
  ([ffmpeg-renderers/pacer/](../app/streaming/video/ffmpeg-renderers/pacer/):
  `dxvsyncsource`, `waylandvsyncsource`).
- Stats overlay plumbing ([overlaymanager.cpp](../app/streaming/video/overlaymanager.cpp))
  and `stringifyVideoStats` / `logVideoStats` in ffmpeg.cpp.
- Hermes capability negotiation (codecs, fps, resolution, HDR control) already
  wired — auto-config can lean on it when present.

---

## Phase 0 — Telemetry foundation & diagnosis engine  ✅ DONE

**Why first:** the single biggest desktop dor is *"stutter even when everything
looks fine,"* and *"stats show numbers but no diagnosis."* The raw data already
exists; we just don't interpret it. This phase is **read-only over the
pipeline** (low risk) and produces the signal every later phase needs to prove
it helped.

Implemented in `streaming/video/statsdiagnostics.{h,cpp}`, wired into the
per-second stats flip in `ffmpeg.cpp`.

- **0.1 — Diagnosis classifier.** ✅ A pure function `Diagnostics::diagnose()`
  takes a `VIDEO_STATS` window + target FPS and classifies the dominant
  bottleneck by normalized severity score: `BOTTLENECK_DECODE` (decode time per
  frame vs frame budget, or decoded-FPS shortfall), `BOTTLENECK_RENDER` (pacer
  drops or render+vsync time), `BOTTLENECK_NETWORK` (network-dropped frames),
  `BOTTLENECK_HOST` (host processing latency). Output is one translated sentence
  plus the single driving metric.
- **0.2 — Plain-language overlay.** ✅ When the debug overlay is enabled, the
  verdict + key metric are appended beneath the existing raw numbers
  (`appendDiagnosisText`). The detailed overlay is untouched.
- **0.3 — Spike history ring buffer.** ✅ `Diagnostics::SpikeHistory` keeps ~5
  min of per-second diagnoses and summarizes them ("3 spike(s) in last ~5 min
  (mostly network)") in the overlay.

Classifier logic validated standalone against synthetic windows for every
bottleneck class (decode-heavy, decode-shortfall, network, pacing, host,
too-few-frames, and the spike summary). Pipeline runtime validation still
pending a working local build (Qt6 link env).

**Acceptance:** with a deliberately under-powered decode setting, the overlay
names "decode-bound"; with an artificially jittery link (tc/netem), it names
"network-bound." No measurable cost to the render thread.

**Risk:** low. New code paths, no changes to decode/pacing/audio behavior.

---

## Phase 1 — Auto-config & presets (Fast / Balanced / Quality / Battery)  🔶 PARTIAL

**Why second:** *"manual config of bitrate/resolution/FPS/codec is confusing"*
and *"no presets."* Depends on Phase 0 to validate that a chosen preset actually
behaves, and on a one-time decode probe.

- **1.1 — Decode capability probe.** ✅ `SystemProperties.probeCodecAvailability`
  reuses the existing `testOnly` decoder path via `Session::getDecoderAvailability`
  to report "hardware"/"software"/"none" per codec at the target res/fps, cached
  per parameter set. NB: this is a *capability* probe (does HW decode initialize),
  not a sustained-FPS benchmark — the embedded test frame is a single fixed-size
  frame, so true throughput at 4K/120 can't be measured cheaply. The HW-vs-SW
  signal already covers most of dor #14 ("claims support but performs badly" ≈
  silently falls back to software).
- **1.2 — Presets.** ✅ A "Quality preset" selector (Fast / Balanced / Quality /
  Battery Saver / Custom) in video settings. `StreamingPreferences::applyPreset`
  derives res/fps from the native display mode and bitrate from
  `getDefaultBitrate` scaled per preset; the GUI picks the best HW-decodable
  codec via the 1.1 probe. Reverts to "Custom" on any manual edit. (Hermes
  `limits` not yet consulted — see below.)
- **1.3 — Handheld power profile.** 🔶 The "Battery Saver" preset exists (720p30,
  reduced bitrate). Not yet done: preferring the lowest-*decode-cost* codec
  (we can't measure decode cost cheaply, see 1.1), and hiding the preset on
  non-handheld desktop builds.
- **1.4 — Per-device profile memory.** ⬜ Not started. Persist the chosen
  preset/probe result keyed by machine, so a laptop and a Deck remember
  different sweet spots.

Preset resolution/fps/bitrate math validated standalone across 4K/120, 1080p60,
Steam Deck 1280x800/90, and 1440p/144. Runtime validation of the GUI flow still
pending a working local build (Qt6 link env).

**Acceptance:** fresh install → one preset click yields a stream Phase 0 rates
as healthy; the probe correctly rejects a codec that software-falls-back.

**Risk:** low–medium. Mostly settings + a reused test path; no live-pipeline
changes.

---

## Phase 2 — Frame pacing & smooth presentation

**Why third (first deep-pipeline phase):** *"stutter / micro-stutter with low
latency,"* *"doesn't sync to refresh,"* *"problems on 90/120/144 Hz and
fractional refresh."* High value, but it touches sensitive shared code, so it
goes after we have Phase 0 to measure regressions objectively.

- **2.1 — Fractional / high-refresh handling.** ✅ The pacer no longer paces off
  a rounded integer refresh rate. `StreamUtils::getDisplayRefreshRateMillihertz`
  reconstructs the NTSC-derived fractional rates SDL2 truncates (59.94, 119.88,
  …) and the pacer carries `m_DisplayFpsMillihz`: the vsync interval is computed
  in microseconds from it, and the "stream ≥ display" drop decision compares in
  millihertz so a 120 FPS stream on a 119.88 Hz panel isn't misclassified into
  the aggressive drop path. (SDL2 exposes only an integer rate, so non-NTSC
  fractional modes still fall back to the integer value — SDL3 would remove that
  limit.) Validated by compile; frame-time validation pending a runtime.
- **2.2 — Frame-queue depth tuning.** Investigate the "smooth for 10s then a
  hitch" pattern (dor #2): is the queue holding a frame too long? Make depth
  adaptive to measured jitter rather than fixed.
- **2.3 — Linux presentation correctness.** Wayland vs X11 differences,
  fullscreen + refresh-rate match (overlaps Phase 6). Validate
  `waylandvsyncsource` against direct-scanout / VRR where available.

**Acceptance:** on a 120 Hz and a 59.94 Hz panel, frame-time graph is flat (no
periodic pacer drops) over a 10-min run; Phase 0 reports no pacing-bound spikes.

**Risk:** medium–high. Per-platform, hardware-dependent, regression-prone. Each
sub-item should land behind validation on real displays.

---

## Phase 3 — Audio: stutter and A/V sync

**Why fourth:** *"audio stutter,"* *"audio delayed vs video,"* recent upstream
reports of dropouts on Intel N-series mini PCs. Self-contained subsystem
([audio/](../app/streaming/audio/)), so it can proceed in parallel with Phase 2.

- **3.1 — Buffer/underrun audit.** Characterize dropouts in `sdlaud.cpp`; expose
  the buffer-size↔latency tradeoff as a setting instead of a hidden constant,
  and detect underruns into the Phase 0 diagnosis ("audio underrunning").
- **3.2 — A/V sync offset control.** A user-tunable audio delay (ms), persisted
  per device — directly addresses the "audio 300 ms late" class of report. The
  desktop client currently offers no fine audio-delay adjustment.
- **3.3 — Output-device robustness.** Handle device changes / driver stalls
  without taking down the stream (feeds Phase 5 reconnect).

**Acceptance:** a 30-min stream with no audible dropouts on a reference setup;
A/V offset adjustable and audibly correct.

**Risk:** medium. Audio buffer changes are easy to regress; keep defaults intact
and make new behavior opt-in until validated.

---

## Phase 4 — Input latency & reliable gamepad mapping

**Why fifth:** *"input feels floaty,"* *"controller mis-mapped,"* especially on
handhelds with built-in controllers and on Windows where Steam Input duplicates
pads. Desktop-relevant subset of dors #7–#8.

- **4.1 — Input latency accounting.** Surface collect→packetize→send time so
  Phase 0 can say whether "lag" is input-side vs decode/network/display.
- **4.2 — Handheld / built-in controller mapping.** Reliable mapping for
  Deck-class built-in pads; deterministic ordering for multiple controllers
  (dor: "controllers duplicated or out of order").
- **4.3 — Windows Steam Input de-duplication.** Detect and avoid the
  double-controller case.
- **4.4 — Mouse mode feel.** Absolute vs relative mode polish for the
  "floaty mouse" complaint.

**Acceptance:** Deck built-in pad maps correctly out of the box; two pads keep
stable order; Phase 0 can attribute input latency separately.

**Risk:** medium. SDL handles much of this; risk is in platform edge cases.

---

## Phase 5 — Clean reconnection

**Why sixth:** *"reconnect returns without audio/input,"* *"black screen,"*
*"app dies,"* *"loses controller on network switch."* Cross-cutting, so it lands
after audio/input/pacing exist to reconnect *into* cleanly.

- **5.1 — State teardown/rebuild audit.** Ensure a reconnect fully re-inits
  audio + input + video, not a partial restore.
- **5.2 — Network-change resilience.** Survive Wi-Fi↔ethernet / interface
  changes (relevant on laptops/handhelds) without killing the session.
- **5.3 — Graceful black-screen recovery & user feedback.** Clear "reconnecting"
  state instead of a frozen/black window.

**Acceptance:** forced mid-stream network drop reconnects with audio, video, and
input all live; interface switch does not crash the client.

**Risk:** medium–high. Touches session lifecycle.

---

## Phase 6 — HDR, color, scaling & fullscreen correctness

**Why seventh:** *"HDR washed out,"* *"stretched image / unexpected black bars,"*
*"cursor misaligned,"* *"native-res handheld with odd resolution."* Premium-setup
and handheld-panel polish; lower frequency than the above but high annoyance.

- **6.1 — HDR/tonemapping correctness** across monitor/TV/handheld-OLED; correct
  SDR↔HDR transitions. (libplacebo path already exists; this is validation +
  fixes.)
- **6.2 — Aspect-ratio / scaling modes** (fit/fill/stretch) with correct handling
  of non-16:9 handheld panels and desktop DPI scaling; fix cursor alignment.
- **6.3 — Fullscreen + refresh-rate match** consistency across Win/X11/Wayland
  (coordinated with 2.3).

**Acceptance:** HDR content matches a reference on OLED; no unintended bars on a
16:10 handheld; cursor aligned at all scale factors.

**Risk:** medium. Color/scaling regressions are very visible.

---

## Phase 7 — Host discovery & connection clarity

**Why last:** mostly UX over an already-working mechanism. *"Can't find PC,"*
*"shows offline when on,"* *"manual IP,"* *"don't know if I need
IP/Tailscale/VPN outside LAN."*

- **7.1 — Discovery robustness** (mDNS failures, duplicate/ghost hosts,
  offline-but-on).
- **7.2 — Connection guidance** distinguishing LAN / manual-IP /
  overlay-network (Tailscale/ZeroTier) cases with actionable hints, building on
  the existing port-test (`LiTestClientConnectivity`).

**Acceptance:** ghost/duplicate hosts eliminated; an off-LAN failure yields a
specific, correct next step.

**Risk:** low.

---

## Cross-cutting notes

- **Every pipeline phase (2–6) must be regression-gated by Phase 0 telemetry** —
  no "feels smoother" merges without a frame-time / dropped-frame delta.
- **Local build currently fails at link** (Qt6 ABI env issue, pre-existing and
  unrelated to source). Pipeline phases need a working runtime to validate;
  fixing the build env is an implicit prerequisite for Phases 2–6.
- Items already shipped this cycle (OTP/DeepLink pairing, clipboard sync,
  capabilities forward-compat) are **not** in this roadmap; it is forward-looking
  only.

## Progress

- ✅ **Phase 0** — Telemetry foundation & diagnosis engine (0.1, 0.2, 0.3).
- 🔶 **Phase 1** — Auto-config & presets. Done: 1.1 (capability probe), 1.2
  (preset selector), 1.3 partial (Battery preset). Remaining: 1.4 (per-device
  profile memory), Hermes `limits` as a preset ceiling, and hiding the battery
  preset on non-handheld builds.
- ⏭️ **Next:** either finish Phase 1 (1.4 + Hermes limits) or move to **Phase 2**
  (frame pacing) — the first deep-pipeline phase, which really wants a working
  runtime to validate.
