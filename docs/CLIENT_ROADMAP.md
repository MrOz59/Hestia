# Hestia Client-Side Roadmap

Status: **active** · Last updated: 2026-07-30

## Scope

Hestia is a **desktop** Moonlight-Qt fork (C++/Qt) focused on **Linux first**,
especially Arch/CachyOS and Linux handhelds such as Steam Deck. Windows and
macOS paths remain inherited from upstream but are currently best-effort and
are not release gates. This roadmap deliberately **excludes** mobile/TV-only
concerns that do not apply to a desktop client:

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

- Rich per-frame stats collected in `_VIDEO_STATS`
  ([decoder.h:11](../app/streaming/video/decoder.h#L11)): decode/pacer/render/
  reassembly times (µs), RTT + RTT variance (jitter), and dropped frames split
  by cause (`networkDroppedFrames` vs `pacerDroppedFrames`). Phase 0 now turns
  these raw counters into a diagnosis and retains a short spike history.
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
  min of per-second diagnoses and counts transitions into a bottleneck as spike
  events, rather than incorrectly counting every affected second as a new spike.
- **0.4 — Automated regression suite.** ✅ QtTest coverage verifies healthy
  VSync timing, decode, render queue, packet loss, RTT jitter, host latency,
  insufficient samples, spike grouping, and translated overlay formatting.
  The Linux CI runs this suite on every push and pull request.
- **0.5 — Pipeline latency distributions.** ✅ Fixed, mergeable histograms now
  report p50/p95/p99 for reassembly, decode, pacer queue, and render stages.
  Recording is O(1), allocation-free, and covered by deterministic tests for
  percentile calculation, window merging, and bounded outliers. Decoder,
  pacing, and render queue depths are sampled into fixed 0/1/2/3+ buckets.
- **0.6 — Per-frame correlation.** ✅ The GameStream frame ID is retained from
  receive through decode and presentation. `HESTIA_FRAME_TRACE=1` enables one
  structured terminal record per frame with receive, assemble, decode,
  presentation, and discard timestamps/reasons. The active timeline uses 512
  fixed slots and does not allocate while recording stages; terminal logging
  remains opt-in because it can incur I/O.
- **0.7 — RTP reorder/FEC queue depth.** ✅ `moonlight-common-c` records the
  current depth, stream-lifetime maximum, sample count, and accumulated depth
  of its pending/completed RTP packet lists. Each completed decode unit carries
  the peak packet depth observed while assembling that frame. Hestia aggregates
  these peaks in a fixed power-of-two histogram and reports p50/p95/p99/max in
  the debug overlay and session log.

The Hermes H2 benchmark harness now consumes the existing
`HESTIA_FRAME_TRACE` records directly. Its importer labels one client log per
network profile, computes exact nearest-rank receive-to-present p95/p99 and
frame-ID gaps, and pairs them with the host windows without assuming that the
two monotonic clocks share an epoch. No new GameStream message or Hestia wire
capability is required. `scripts/hestia-h2-test.sh` launches the local Release
binary with tracing enabled and refuses to overwrite a profile log.

The classifier no longer treats one normal frame budget spent in render+VSync
as a presentation failure. Pacer drops are attributed to network or local
presentation using independent packet-loss/RTT-variance signals, and persistent
queue delay is scored separately. The local build and CI both pass.

**Acceptance:** with a deliberately under-powered decode setting, the overlay
names "decode-bound"; with an artificially jittery link (tc/netem), it names
"network-bound." No measurable cost to the render thread.

**Risk:** low. New code paths, no changes to decode/pacing/audio behavior.

---

## Phase 1 — Auto-config & presets (Fast / Balanced / Quality / Battery)  ✅ DONE

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
  codec via the 1.1 probe. Reverts to "Custom" on any manual edit.
- **1.3 — Handheld power profile.** ✅ The "Battery Saver" preset (720p30,
  reduced bitrate) is now hidden on non-handheld builds: `SystemProperties`
  exposes `isHandheld` (Steam Deck DMI `Jupiter`/`Galileo`, the `SteamDeck` env
  var, or a tablet/convertible/detachable SMBIOS chassis type), and the preset
  selector drops the Battery row unless `isHandheld`. Out of scope: preferring
  the lowest-*decode-cost* codec — decode cost can't be measured cheaply (1.1).
- **1.4 — Per-device profile memory.** ✅ The active preset is persisted keyed
  by a stable machine id (`QSysInfo::machineUniqueId`, SHA-1 hashed; host-name
  fallback) under `presetProfiles/<machineId>`. `StreamingPreferences`
  exposes `saveActivePreset`/`loadActivePreset`; the settings view restores the
  machine's last preset on open, saves on change, and clears to Custom on any
  manual edit. A laptop and a Deck remember different choices.
- **1.5 — Hermes-aware effective mode.** ✅ Before decoder selection, Hestia
  intersects the host's codec list and clamps preset resolution/FPS to Hermes
  `limits`. If a preset is clamped, bitrate is recalculated while preserving
  that preset's quality ratio. Custom bitrates remain untouched. A host/client
  combination with no common codec now fails clearly instead of violating the
  contract with an unconditional H.264 fallback.

Preset and negotiation math is covered for 4K/120, ultrawide, Steam Deck
1280x800/90, host clamping, custom bitrate preservation, and protocol evolution.
Preset resolution caps now fit both width and height instead of relying only on
pixel count.

**Acceptance:** fresh install → one preset click yields a stream Phase 0 rates
as healthy; the probe correctly rejects a codec that software-falls-back.

**Risk:** low–medium. Mostly settings + a reused test path; no live-pipeline
changes.

---

## Phase 2 — Frame pacing & smooth presentation  ✅ DONE

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
  limit.) Regression tests cover 59.94, 119.88, 143.856, and integer 120 Hz.
- **2.2 — Frame-queue depth tuning.** ✅ `AdaptiveQueueDepth` starts at two
  frames, raises to three after 250 ms of sustained high RTT variance, and only
  lowers to one after two seconds of stable network. Hysteresis prevents a
  borderline link from changing latency every frame. Wayland/Windows apply the
  target to the pacing queue; X11 and other renderer-driven paths apply it to
  the render queue. The existing three-frame memory/surface ceiling is retained.
- **2.3 — Linux presentation correctness.** ✅ The overlay and session logs now
  identify the presentation backend, precise refresh rate, adaptive targets,
  queue-depth distributions, and average/max/late VSync intervals. Wayland
  callbacks use a pending counter so an early callback cannot be lost, and a
  100 ms wait timeout no longer fabricates a VSync while the surface is
  occluded. Queue-overflow drops are now included in pacer statistics, and late
  VSync intervals feed the Phase 0 presentation diagnosis.

**Acceptance:** on a 120 Hz and a 59.94 Hz panel, frame-time graph is flat (no
periodic pacer drops) over a 10-min run; Phase 0 reports no pacing-bound spikes.
The automated policy/refresh suite and local X11 smoke test are release gates;
10-minute Wayland, VRR, and direct-scanout runs remain part of the hardware
release matrix because they require the target compositor and display.

**Risk:** medium–high. Per-platform, hardware-dependent, regression-prone. Each
sub-item should land behind validation on real displays.

---

## Phase 3 — Audio: stutter and A/V sync  🟡 IN PROGRESS

**Why fourth:** *"audio stutter,"* *"audio delayed vs video,"* recent upstream
reports of dropouts on Intel N-series mini PCs. Self-contained subsystem
([audio/](../app/streaming/audio/)), so it can proceed in parallel with Phase 2.

- **3.1 — Buffer/underrun audit.** ✅ The observability baseline is complete:
  the GameStream receiver now publishes bounded one-second audio windows and a
  final session summary through `ISessionTelemetry`. SDL reports playback queue
  depth, device-buffer duration, queue high-water mark, backpressure waits,
  queue failures, and underruns; SLAudio reports submissions and backpressure
  while explicitly marking queue depth as unavailable. Receiver metrics include
  Opus decode/concealment, recovery drops, renderer failures/reinitializations,
  and the existing RTP audio/FEC counters. The debug overlay now diagnoses
  underruns, network concealment, backpressure, and output-device failure. A
  persisted audio-latency profile now exposes the buffer-size↔latency tradeoff.
  Device and queue size selection remains compatible with the previous
  profiles:

  - **Default:** SDL uses a 10 ms minimum device buffer, three packets, a
    50 ms playback limit, 30 ms receiver backpressure, and a 10 ms startup
    reserve; SLAudio uses 40 ms per stereo pair.
  - **Low latency:** retains SDL's safe 10 ms floor, uses two packets, 30/20 ms
    queue limits, and a 5 ms startup reserve; SLAudio uses 20 ms per stereo
    pair.
  - **Smooth playback:** uses a 15 ms/four-packet SDL target, 80/50 ms queue
    limits, and a 20 ms startup reserve; SLAudio uses 60 ms per stereo pair.

  The pure policy is covered by deterministic tests, the effective profile and
  limits are included in session telemetry, and non-default profiles apply only
  to the next stream. SDL now starts playback only after accumulating the
  profile's bounded reserve. If that reserve is exhausted, playback pauses
  briefly while it is rebuilt; the target rises by 5–10 ms per observed
  underrun up to 20/30/50 ms for low-latency/default/smooth profiles. Remaining
  audio is accounted from submitted duration and monotonic elapsed time instead
  of treating an empty SDL application queue as proof of underrun, because the
  device may already own an unobservable buffer.
- **3.2 — A/V sync offset control.** A user-tunable audio delay (ms), persisted
  per device — directly addresses the "audio 300 ms late" class of report. The
  desktop client currently offers no fine audio-delay adjustment.
- **3.3 — Output-device robustness.** Handle device changes / driver stalls
  without taking down the stream (feeds Phase 5 reconnect).

**Acceptance:** a 30-min stream with no audible dropouts on a reference setup;
A/V offset adjustable and audibly correct.

**Risk:** medium. Audio buffer changes are easy to regress; keep the adaptive
reserve bounded and validate its latency on real output devices.

The legacy Limelight audio callback supplies an Opus payload but no media
presentation timestamp. Consequently, Phase 3 currently reports an estimated
local presentation delay (receiver queue + renderer queue + device buffer) and
marks absolute A/V offset as unavailable. A real A/V offset must wait for a
timestamped receiver, such as the future HDT path; the client must not infer one
by comparing unrelated process clocks.

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
- The local Qt6 build and Linux/Arch CI are working. Pipeline phases must keep
  the client-logic suite green and add hardware/runtime evidence where required.
- Items already shipped this cycle (OTP/DeepLink pairing, clipboard sync,
  capabilities forward-compat) are **not** in this roadmap; it is forward-looking
  only.

## Progress

- ✅ **Phase 0** — Telemetry, diagnosis, translated overlay, spike history, and
  automated regression coverage (0.1–0.4).
- ✅ **Phase 1** — Capability probe, presets, handheld Battery mode, per-device
  memory, and Hermes-aware mode/bitrate/codec negotiation (1.1–1.5).
- ✅ **Phase 2** — Fractional refresh handling, adaptive queue depth, X11/
  Wayland presentation telemetry, late-VSync diagnosis, and callback correctness
  are implemented and regression-tested (2.1–2.3).
- 🚧 **C1 architecture track** — A typed, injectable `IHostProtocol` now owns
  session prepare/launch/stop. The `GameStreamHostProtocol` adapter retains the
  legacy `NvHTTP` behavior while keeping Hermes JSON field names out of
  `Session`. A typed, injectable `IClientTransport` now owns connection
  start/interrupt/stop, while `GameStreamClientTransport` confines the legacy
  callbacks and `Li*` lifecycle calls to its adapter. An injectable
  `IConnectivityAgent` now provides a stable path selection shared by Session's
  MTU policy and the transport; `GameStreamConnectivityAgent` confines the
  legacy active-address and LAN/VPN reachability lookup. An injectable
  `IVideoReceiver` now carries typed renderer capabilities, while
  `GameStreamVideoReceiver` confines `DECODER_RENDERER_CALLBACKS`, video setup,
  push/pull selection, and synchronized `DECODE_UNIT` submission. An injectable
  `IAudioReceiver` now owns typed channel/device checks and mute state, while
  `GameStreamAudioReceiver` confines `AUDIO_RENDERER_CALLBACKS`, Opus decoding,
  renderer recovery, and post-reinitialization sample dropping. A typed
  `IDecoder` now exposes decoder properties, HDR state, main-thread rendering,
  and window changes without Limelight structures or capability bitmasks.
  `LegacyVideoDecoderAdapter` preserves the initialized FFmpeg/SLVideo decoder
  and confines `PDECODE_UNIT` submission to the GameStream path. A typed
  `IRenderScheduler` now owns move-only zero-copy decoded frames, source timing,
  local presentation deadlines, network conditions, and presentation status.
  `LegacyPacerAdapter` preserves the existing FFmpeg Pacer and VSync paths.
  A typed, injectable `IInputSender` now carries session-local sequence,
  monotonic capture timestamp, device ID, replaceability, and semantic events
  for keyboard/text, mouse, scroll, touch, pen, controllers, and sensors.
  `GameStreamInputSender` confines every `LiSend*` call, host input capability
  flag, and legacy button/capability mapping to the adapter. Fake-adapter and
  payload tests cover these boundaries. A typed, injectable
  `ISessionTelemetry` now receives lifecycle, connection quality, video-window,
  final-summary, and terminal-frame events. FFmpeg and Pacer publish through
  this sink instead of formatting the debug overlay or writing frame traces
  directly; `LegacySessionTelemetry` preserves the current SDL log, translated
  diagnosis, spike history, and overlay behavior. A null adapter keeps decoder
  probing side-effect free, and a fake sink covers the ninth C1 boundary.
- ✅ **C1 architecture track complete:** all nine planned seams are injectable
  and retain GameStream adapters.
- ⏭️ **Next implementation slice:** Phase 3 audio buffer/underrun
  instrumentation and A/V sync, gated by the Phase 0 telemetry.
