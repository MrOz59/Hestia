# Hestia — October 2026 release candidate 1

Coordinated RC for testing before the end-of-October stable release. This is a **prerelease**, not a stable release.

- Source commit: [`94a1bc58b510f7417bd208ed70cd22d2b89ed649`](https://github.com/MrOz59/Hestia/commit/94a1bc58b510f7417bd208ed70cd22d2b89ed649)
- Validated build: [Actions run 40257911735](https://github.com/MrOz59/Hestia/actions/runs/40257911735)
- Embedded build version: `0.1.0+94a1bc5`

This is an immutable promotion of the exact successful CI artifacts. Package/APK bytes, embedded build versions, filenames and signatures are preserved; the shared `october-2026-rc1` tag identifies the coordinated candidate. `RC_MANIFEST.json` records each source/build/artifact identity, and `SHA256SUMS` covers all attached payload files.

## Included

- Optional PyroWave decoding from PR #6, with the Linux codec runtime included.
- Linux x86_64/ARM64 archives, Arch/CachyOS package and x86_64 AppImage.
- Linux/package builds and the Vulkan software-device encode/decode roundtrip passed.

Select PyroWave explicitly and connect to the matching Hermes RC. Hardware GPU performance and end-to-end latency still need real-device measurements.

## Matching candidates

- [Hermes RC1](https://github.com/MrOz59/Hermes/releases/tag/october-2026-rc1) — `964c5fa24b4992f942a59142e82b3adee8214de9`
- [Hestia RC1](https://github.com/MrOz59/Hestia/releases/tag/october-2026-rc1) — `94a1bc58b510f7417bd208ed70cd22d2b89ed649`
- [Hestia-Android RC1](https://github.com/MrOz59/Hestia-Android/releases/tag/october-2026-rc1) — `0ef14edfd152b4d220d3f2992c82c101e582ca44`
- [Hermes-KMS RC1](https://github.com/MrOz59/Hermes-KMS/releases/tag/october-2026-rc1) — `8c8aa0812783437c91e6d65ff8073fdbe57068b2`

## Remaining validation

Real-host AMD capture/encode, desktop and Android streaming, SDR/HDR, disconnect/reconnect, concurrent isolated Gamescope sessions, DS4/optional DualSense, private audio/microphone and upgrade/rollback checks remain part of RC testing. Passing CI does not record those hardware tests as completed. PyroWave HDR is outside the supported candidate path; use the existing codecs for HDR.

When reporting a problem, include this RC tag, component build version, GPU/driver/kernel, client device and codec, and logs from both ends.
