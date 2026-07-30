# Hestia

Hestia is a Moonlight-Qt based desktop client focused on compatibility with
Hermes, Apollo, Sunshine, and standard GameStream-compatible hosts.

Hermes is an Apollo-compatible host with extra Hestia protocol extensions.
Hestia detects those extensions when they are available, while preserving the
normal Moonlight/Apollo/Sunshine connection path for hosts that do not provide
them.

## Platform focus

**Hestia's current focus is Linux** (Arch/CachyOS first, with handhelds such as
the Steam Deck in mind). That's where development, packaging, and CI are
prioritized right now: continuous integration builds the client on Linux and
produces an Arch/CachyOS package on every change.

This focus may broaden in the future. The codebase is inherited from
Moonlight-Qt and still contains the Windows and macOS paths, but those are
**not a priority and are not actively tested** at the moment — their CI is
opt-in and may be broken. If you need Windows or macOS today, prefer upstream
[Moonlight-Qt](https://github.com/moonlight-stream/moonlight-qt).

## Features

- Hardware accelerated video decoding inherited across Windows, macOS, and
  Linux, with current project validation focused on Linux.
- H.264, HEVC, and AV1 codec support where supported by the host and GPU.
- HDR streaming support where supported by the host and platform.
- Surround sound audio support.
- Gamepad support, including force feedback and motion controls where supported.
- Pointer capture and direct mouse control modes.
- Support for passing system-wide keyboard shortcuts to the host.
- Per-frame pipeline telemetry, bounded latency distributions, and a
  plain-language bottleneck diagnosis in the debug overlay.
- Fast, Balanced, Quality, and handheld Battery Saver presets with
  Hermes-aware resolution/FPS/codec negotiation.
- Selectable audio buffering profiles with startup prebuffering and bounded
  underrun recovery on the SDL path.
- Apollo/Sunshine-compatible streaming fallback.
- Hermes protocol support for host capability discovery, session preparation,
  session stop, diagnostics, and optional clipboard sync.

## Hermes compatibility

When connected to Hermes, Hestia can use the host's Hestia protocol v1 API for:

- Capability detection.
- Session preparation before streaming.
- Session cleanup after streaming.
- Display status/recovery and visibility into the paired client's permissions.
- Host diagnostics display, including live runtime status: the real encoder
  in use (hardware vs software + codecs), active/streaming sessions, and live
  pipeline metrics (FPS, bitrate, encode time, capture→encode latency, and
  encoded/dropped frames) while a stream is running.
- Optional clipboard synchronization.
- Optional execution of commands explicitly configured by the host.
- Optional host-side display/session preferences.

Hestia does not assume Hermes-only features exist on every host. If a host does
not expose a capability, Hestia falls back to the standard Moonlight-compatible
behavior.

For Hermes hosts whose administrator enabled `multi_user_sessions`, Hestia
requests an independent session before the normal GameStream launch. Hestia
does not expose a client-side switch and never enables isolation on the host;
when Hermes reports it disabled, the normal shared GameStream path remains in
use. Newer hosts expose explicit `enabled` and `ready` state, which takes
priority over the legacy capability boolean. For an enabled isolated launch,
Hestia retains the opaque reservation ID and sends it when stopping, so another
client's stream and virtual display are not affected. If Hermes cannot reserve
that configured isolated runtime, Hestia aborts instead of silently falling
back to the shared host session.

Some Hermes-focused features are based on ideas from ClassicOldSong's Android
Moonlight fork, especially the Apollo-oriented client behavior that is useful
for Hermes as an Apollo-derived host:

- https://github.com/ClassicOldSong/moonlight-android

## Transition plan and the long-term HDT direction

Hestia is not switching away from GameStream in one step. All production media
streaming currently uses the compatible Moonlight/GameStream path, including
sessions where the optional Hermes protocol v1 API prepares a display or
reports diagnostics. That API is a control-layer extension, not the future
media transport.

The client transition is being built in measured stages:

- C0/C1 established per-frame telemetry and typed boundaries for host protocol,
  connectivity, transport, video, audio, decoder, presentation, input, and
  session telemetry while retaining their GameStream adapters.
- C2 and the client quality roadmap improve the existing path first: pacing,
  bounded queues, presets, diagnostics, and audio robustness. Audio
  instrumentation and adaptive SDL prebuffering are present; real-device
  validation, manual A/V offset, and output-device recovery remain active work.
- Later paired phases add optional feedback/recovery extensions, ICE and
  invites, then native identity and pairing. Each step must continue working
  when the corresponding host capability is absent.
- Only the later C7/H7 phase introduces the first experimental
  **HDT — Hermes Datagram Transport** receiver and sender.

HDT is planned as an encrypted UDP transport with frame IDs, packet indexes,
priorities, deadlines, explicit pacing, feedback, FEC/retransmission decisions,
and multiplexed media/control/input flows. Its exact wire format is not stable
or implemented today.

**This is long-term work and is expected to take substantial time.** There is
no HDT release date, several prerequisite phases are still open, and the first
version will be opt-in and disabled by default. Hestia will keep connecting to
Sunshine, Apollo, Hermes, and compatible hosts through GameStream throughout a
long stabilization period. Automatic selection may be considered only after
specification, test vectors, security review, cross-platform benchmarks, and
real-world fallback/reconnection tests exist.

## Building on Arch / CachyOS

Arch/CachyOS is the primary target. A PKGBUILD lives in
[`packaging/arch/`](packaging/arch/PKGBUILD) and builds from a local checkout:

```bash
git clone --recurse-submodules https://github.com/MrOz59/Hestia.git
cd Hestia/packaging/arch
makepkg -si
```

The installed binary is `moonlight`. The same PKGBUILD is what CI uses to
produce the Arch/CachyOS package artifact on every change.

## Building on Ubuntu/Lubuntu

Install the base build dependencies:

```bash
sudo apt update
sudo apt install \
  build-essential git pkg-config qmake6 qt6-base-dev libqt6opengl6-dev \
  qt6-declarative-dev libqt6svg6-dev qt6-wayland qml6-module-qtquick \
  qml6-module-qtquick-controls qml6-module-qtquick-templates \
  qml6-module-qtquick-layouts qml6-module-qtquick-window libegl1-mesa-dev \
  libgl1-mesa-dev libopus-dev libsdl2-dev libsdl2-ttf-dev libssl-dev \
  libavcodec-dev libavformat-dev libswscale-dev libva-dev libvdpau-dev \
  libxkbcommon-dev wayland-protocols libdrm-dev
```

Then build from the repository root:

```bash
git submodule update --init --recursive
qmake6 moonlight-qt.pro
make release
```

The development binary is produced at:

```bash
app/moonlight
```

If your Ubuntu/Lubuntu package set does not provide
`qml6-module-qtqml-workerscript`, omit it. Current Hestia builds do not require
that package on every Ubuntu release.

## Runtime notes

For standard Sunshine, Apollo, and Moonlight-compatible hosts, Hestia keeps using
the normal pairing, app list, launch, and stream flow.

For Hermes hosts, Hestia enables extra behavior only after the host reports
support for it. Clipboard support depends on the host environment. On Linux
hosts, Hermes can report missing helpers such as `wl-clipboard` or `xclip`
through diagnostics, and Hestia displays those diagnostics to the user.

## Upstream

Hestia is based on Moonlight-Qt, the open source PC client for NVIDIA
GameStream-compatible hosts and Sunshine.

Original Moonlight project links:

- Website: https://moonlight-stream.org
- Upstream Qt client: https://github.com/moonlight-stream/moonlight-qt
- Sunshine host: https://github.com/LizardByte/Sunshine
