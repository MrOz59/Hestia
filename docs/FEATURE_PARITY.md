# Hestia feature parity: desktop vs Android

Last audit: 2026-11-05

Sources: `MrOz59/Hestia` (`master` at `c84505f`), `MrOz59/Hestia` (`dev` at `1095475`) and `MrOz59/Hestia-Android` (`moonlight-noir` at `7d7a88f`). This is a source-level inventory, **not** proof of hardware/runtime validation. `dev` is experimental and must **not** be merged wholesale into `master`.

## Shared features already in the mainline clients

| Feature | Android `moonlight-noir` | Desktop `master` | Notes |
| --- | --- | --- | --- |
| GameStream/Sunshine/Apollo connection, pairing, app list and streaming | Yes | Yes | Legacy host fallback must remain intact |
| H.264, HEVC, AV1 | Yes | Yes | Subject to device support |
| HDR10 | Yes | Yes | Device/display/host dependent |
| PyroWave experimental SDR codec | Yes | Yes | Android arm64 Vulkan only; desktop PyroWave uses a different decoder pipeline |
| Hermes capabilities and paired HTTPS v1 APIs | Yes | Yes | Never assume optional endpoints exist |
| Hermes virtual-display launch | Yes | Yes | Behavior and settings differ |
| Hermes display status/recovery | Yes | Yes | Explicit confirmation before recovery |
| Hermes permissions and runtime diagnostics | Yes | Yes | Not the same UX or status coverage |
| Local network host discovery and manual connection | Yes | Yes | Android has extra NSD/JmDNS fallbacks; desktop uses a different discovery stack |
| Text clipboard interaction | Yes | Yes | Android uses Apollo-style `actions/clipboard`; desktop uses Hermes v1 clipboard endpoints; permission and platform semantics differ |
| Host-approved command execution | Yes | Yes | Android uses GameStream server command list; desktop uses Hermes v1 endpoints; keep authorization checks |
| Controller, keyboard, mouse, touch | Yes | Yes | Device-specific implementations differ |
| Gamepad rumble and motion | Yes | Yes | Hardware and mapping support differ |
| Performance overlay | Yes | Yes | Android has lite/logging options; desktop has richer diagnosis and spike history |
| Resolution/FPS/bitrate configuration | Yes | Yes | Android offers additional rotation and metered bitrate controls |
| Stereo/5.1/7.1 output | Yes | Yes | Decoder and audio system dependent |

## Android features missing or incomplete on desktop master

| Feature | Desktop master | `dev`/PR #5 | Priority / recommendation |
| --- | --- | --- | --- |
| Complete Hermes host health badge (permissions, certificate errors, preflight, recent session, monitor status) | Partial preflight-only warning | Implemented on dev, not merged to master | **P1**: port independently |
| Client microphone passthrough to Hermes/Apollo | No | Implemented on dev, requires forked `moonlight-common-c` | **P1**: integrate common-c PyroWave + microphone carefully |
| Explicit error after Hermes declines `session/prepare` | Partial/log feedback | Implemented on dev | **P1**: adapt to master session code |
| Post-stream latency summary | No explicit post-stream toast | Implemented on dev; depends on telemetry changes | **P2**: implement against master telemetry |
| Hermes stream provenance overlay (capture backend, encoder/GPU and latency) | Not present | Not included in PR #5 | **P1**: port Android PR #28 endpoint handling |
| Fetch/share full Hermes diagnostic report | No | Not included in PR #5 | **P1**: this PR adds opt-in asynchronous copy-to-clipboard |
| Open Host Web UI | No | Implemented on dev | **P1**: this PR ports to master |
| Hestia-specific Help destinations | No | Implemented on dev | **P2**: this PR ports to master |
| Opt-out of mouse wiggle after connect | No | Implemented on dev, needs common-c | **P2**: integrate only with compatible common-c |
| Named, editable stream profiles | No; preset selector remembered per device | Android has profile manager | **P2**: useful for multiple hosts/network settings |
| Video aspect modes Fit/Fill/Stretch | No equivalent exposed selector | — | **P2**: add desktop scaling control where renderer supports it |
| In-stream pan/zoom | No equivalent | — | **P3**: useful for high-resolution desktop access |
| Per-network metered bitrate | No equivalent setting | — | **P3**: add only if mobile/handheld network handling warrants it |
| More granular Android pacing modes (Warp/Warp2) | Different pacer design | — | **P3 / research**: measure frame time before translating any policy |
| One-click performance-stat export | No equivalent to Android sharing | — | **P2**: export desktop stats and host logs with redaction |

## Desktop features missing or incomplete on Android

| Feature | Android | Priority / recommendation |
| --- | --- | --- |
| Explicit Hermes Gamescope launch-mode preference | Capability parsed, but Android `HestiaSessionRequest` sends `launch_mode=normal` | **P1**: expose Normal/Gamescope, gate by capability and permission |
| Fast/Balanced/Quality/Battery automatic streaming presets | Named manual Android profiles, but not the desktop's automatic capability-probed presets | **P2**: adopt device-appropriate presets without overwriting saved profiles |
| Decoder selection / detailed renderer control | Android MediaCodec platform-specific behavior | **P3**: only expose safe hardware-specific options |
| YUV 4:4:4 streaming toggle | Not exposed as desktop has it | **P3 / research**: verify Android device decode/presentation support |
| Desktop diagnostics bottleneck classification and five-minute spike history | Simplified overlay/logging instead | **P2**: portable diagnosis semantics, Android-native UI |
| Direct host-control/session stopping through Hestia v1 `session/stop` | Standard GameStream quit/disconnect; no parallel Hestia stop call in the Android API | **P1**: session-scoped cleanup, never stop another client's stream |
| Desktop style external keyboard shortcut capture | Android system-key restrictions | Platform-specific: follow modern Android APIs, no accessibility-service workaround |
| CLI/deep links and desktop automation | Android has shortcuts/intents | Platform-specific: Android shortcuts/intents are preferable |
| Discord Rich Presence | Not core to Android UI | **Low**: not a parity requirement |

## Intentionally platform-specific

Android TV/Google TV/Fire TV leanback UI, external display/DeX, PiP, on-screen gamepad, special-key layouts, soft keyboard, S Pen and touch gestures are Android/handheld features; they should not be copied verbatim into a desktop Qt client. Desktop multiwindow/fullscreen behavior, Steam Deck support, SDL/controller integration, desktop keyboard shortcuts and rich FFmpeg/VAAPI rendering are naturally desktop-native. Pursue equivalent capabilities, not identical menus.

## Integration rules

1. Use `master` as the base; do not merge the experimental `dev` branch.
2. Keep `moonlight-common-c` PyroWave support while adding microphone and mouse-wiggle APIs. Its `feature/pyrowave-codec` and `hestia` heads currently diverge.
3. Break changes into reviewable commits; require Linux CI, non-Hermes fallback, and real stream tests for input/audio/protocol changes.
4. Never expose a Hermes-only action before validating protocol support, pairing and permissions.
5. Keep Android and desktop settings/platform UI separate even if they share capability or diagnostics semantics.
6. Keep report exports opt-in and avoid logging diagnostic-report bodies or clipboard contents.

Related: desktop parity PR #5 (merged to **dev**), Android PR #28 (stream provenance), Android PR #29 (diagnostic reports).
