# Getting help with Hestia

This is the page the client's Help button and its error dialogs open. Hestia
is a Moonlight-Qt fork, so for anything the two share, Moonlight's guides
describe Hestia too; this page says where that is the case and where Hestia
differs.

## Setting up a host

- **Hermes** (Linux): follow [Installing in the Hermes README][hermes-install].
  Hestia detects Hermes on its own and turns on the Hermes features the host
  reports.
- **Apollo or Sunshine**: set the host up as its own documentation describes,
  then add it in Hestia like any GameStream host. Moonlight's
  [Setup Guide][moonlight-setup] covers ports and pairing, which work the same
  way here.

If the PC does not show up on its own, add it with the **Add PC manually**
button and its IP address.

## Connection problems

A Hermes host can usually say what is wrong itself. When it reports
something that needs attention, its tile shows a warning or error badge;
hover over it, or open **Hermes Status** in the PC's menu (right click, or the
menu button on a gamepad), which asks the host again and, when the monitor was
left off, offers to turn it back on. The menu also has:

- **Hermes Diagnostics**: the host's readiness checks, which encoder it uses,
  and its live sessions.
- **Hermes Permissions**: what this client is allowed to do. A client missing
  the launch or input permission connects but cannot start apps or control
  them; grant it in the Hermes web UI (**Open Host Web UI** in the same menu)
  under PIN → Device Management.
- **Hermes Display Status** and **Recover Physical Monitor**: when a stream
  left the host's monitor switched off.

For connection failures in general (blocked ports, firewalls, streaming over
the Internet), Moonlight's [Troubleshooting guide][moonlight-troubleshooting]
applies as written; **Test Network** in the PC's menu runs the same port test.

## Microphone

**Stream my microphone to the host**, under Audio Settings, sends this
computer's microphone to hosts that accept it. On Hermes, turn on **Stream the
client's microphone** in its audio settings; Hermes delivers it to independent
(isolated) sessions, not to the host's own desktop. When a host doesn't accept
it, the stream starts without it and says so for a few seconds.

## Video and gamepads

Decoding and controller handling are Moonlight-Qt's, unchanged:

- [Fixing hardware decoding problems][moonlight-decoding]
- [Gamepad mapping][moonlight-gamepads]

## Reporting a problem

Open an issue at [MrOz59/Hestia][issues] with:

- the Hestia version (shown at the top of the settings page; a nightly also
  names its commit, as in `0.1.0+abc1234`),
- the host and its version, and for a Hermes host what **Hermes Diagnostics**
  shows,
- the client log: on Linux, start `moonlight` from a terminal and save what it
  prints; on Windows and macOS the client writes a log file and prints where
  it put it.

Hermes problems that are not about the client belong in
[the Hermes issues][hermes-issues].

[hermes-install]: https://github.com/MrOz59/Hermes#installing
[hermes-issues]: https://github.com/MrOz59/Hermes/issues
[issues]: https://github.com/MrOz59/Hestia/issues
[moonlight-setup]: https://github.com/moonlight-stream/moonlight-docs/wiki/Setup-Guide
[moonlight-troubleshooting]: https://github.com/moonlight-stream/moonlight-docs/wiki/Troubleshooting
[moonlight-decoding]: https://github.com/moonlight-stream/moonlight-docs/wiki/Fixing-Hardware-Decoding-Problems
[moonlight-gamepads]: https://github.com/moonlight-stream/moonlight-docs/wiki/Gamepad-Mapping
