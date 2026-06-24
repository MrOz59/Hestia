# Hestia

Hestia is a Moonlight-Qt based desktop client focused on compatibility with
Hermes, Apollo, Sunshine, and standard GameStream-compatible hosts.

Hermes is an Apollo-compatible host with extra Hestia protocol extensions.
Hestia detects those extensions when they are available, while preserving the
normal Moonlight/Apollo/Sunshine connection path for hosts that do not provide
them.

## Features

- Hardware accelerated video decoding on Windows, macOS, and Linux.
- H.264, HEVC, and AV1 codec support where supported by the host and GPU.
- HDR streaming support where supported by the host and platform.
- Surround sound audio support.
- Gamepad support, including force feedback and motion controls where supported.
- Pointer capture and direct mouse control modes.
- Support for passing system-wide keyboard shortcuts to the host.
- Apollo/Sunshine-compatible streaming fallback.
- Hermes protocol support for host capability discovery, session preparation,
  session stop, diagnostics, and optional clipboard sync.

## Hermes compatibility

When connected to Hermes, Hestia can use the host's Hestia protocol v1 API for:

- Capability detection.
- Session preparation before streaming.
- Session cleanup after streaming.
- Host diagnostics display.
- Optional clipboard synchronization.
- Optional host-side display/session preferences.

Hestia does not assume Hermes-only features exist on every host. If a host does
not expose a capability, Hestia falls back to the standard Moonlight-compatible
behavior.

Some Hermes-focused features are based on ideas from ClassicOldSong's Android
Moonlight fork, especially the Apollo-oriented client behavior that is useful
for Hermes as an Apollo-derived host:

- https://github.com/ClassicOldSong/moonlight-android

## Building on Ubuntu/Lubuntu

Install the base build dependencies:

```bash
sudo apt update
sudo apt install \
  build-essential git pkg-config qmake6 qt6-base-dev qt6-declarative-dev \
  libqt6svg6-dev qt6-wayland qml6-module-qtquick qml6-module-qtquick-controls \
  qml6-module-qtquick-templates qml6-module-qtquick-layouts \
  qml6-module-qtquick-window libegl1-mesa-dev libgl1-mesa-dev libopus-dev \
  libsdl2-dev libsdl2-ttf-dev libssl-dev libavcodec-dev libavformat-dev \
  libswscale-dev libva-dev libvdpau-dev libxkbcommon-dev wayland-protocols \
  libdrm-dev
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
