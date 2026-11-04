#pragma once

#include "hestiacapabilities.h"

#include <QJsonObject>
#include <QString>
#include <QVector>

// What the Hestia v1 API said about a paired Hermes host, fetched by the
// computer poller and handed to HestiaHostStatus::evaluate().
struct HestiaHostProbe
{
    // GET /client/permissions. httpStatus is 0 when the host never answered.
    bool permissionsOk = false;
    int permissionsHttpStatus = 0;
    QJsonObject permissions;

    // GET /diagnostics and GET /display/status, skipped without the
    // permission to view streams.
    bool diagnosticsFetched = false;
    bool diagnosticsOk = false;
    QJsonObject diagnostics;
    bool displayFetched = false;
    bool displayOk = false;
    QJsonObject display;
};

struct HestiaHostFinding
{
    // Ordered by severity. Info findings never change the overall level.
    enum class Level { Info, Ok, Warning, Error };

    enum class Code {
        Ready,                  // Nothing checked needs attention.
        ApiUnreachable,         // Hermes, but its API is not answering.
        ApiError,               // detail: what went wrong.
        CertificateRejected,    // The host no longer accepts this pairing.
        NoStreamPermission,     // Neither launch nor view: cannot stream.
        NoLaunchPermission,
        NoListPermission,
        NoInputPermission,      // detail: comma-separated keyboard,mouse,gamepad.
        NoViewPermission,       // Diagnostics and display state are hidden.
        DiagnosticsUnavailable, // checkId: "diagnostics" or "display".
        PreflightCheck,         // checkId: the host's check id; detail: its message.
        DisplayLeftDisabled,    // Physical monitors off while nothing streams.
        LastSessionEnded,       // detail: the host's termination reason.
        Streaming,              // detail: number of active sessions.
        AwaitingReconnect,      // An app is parked, waiting for a client.
        VirtualDisplayActive,   // detail: "WIDTHxHEIGHT@FPS backend".
        ClipboardUnavailable,   // detail: the host's clipboard diagnostic.
    };

    Level level = Level::Info;
    Code code = Code::Ready;
    QString checkId;
    QString detail;

    // The finding as a sentence for the PC tile's tooltip.
    QString describe(bool displayRecoveryOffered) const;

    bool operator==(const HestiaHostFinding& other) const
    {
        return level == other.level && code == other.code &&
               checkId == other.checkId && detail == other.detail;
    }
    bool operator!=(const HestiaHostFinding& other) const { return !(*this == other); }
};

// Health of a paired Hermes host, the way Hestia-Android's HestiaHostStatus
// reports it: capabilities, this client's permissions, the host's preflight
// checks, its sessions and its display state turned into a few findings with
// a severity. The UI maps the overall level to a badge and the findings to
// text; it never reads the JSON itself.
//
// A host without the Hestia v1 API is not valid and gets no badge, so plain
// Sunshine, Apollo and GameStream hosts look exactly as before.
struct HestiaHostStatus
{
    bool valid = false;
    QVector<HestiaHostFinding> findings; // Most severe first.
    bool displayRecoveryOffered = false;

    HestiaHostFinding::Level level() const;
    bool needsAttention() const;

    // One line per finding, most severe first.
    QString details() const;

    static HestiaHostStatus evaluate(const HestiaCapabilities& capabilities,
                                     const HestiaHostProbe& probe);

    bool operator==(const HestiaHostStatus& other) const
    {
        return valid == other.valid && findings == other.findings &&
               displayRecoveryOffered == other.displayRecoveryOffered;
    }
    bool operator!=(const HestiaHostStatus& other) const { return !(*this == other); }
};
