#include "hestiahoststatus.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QStringList>

#include <algorithm>
#include <utility>

namespace {

using Level = HestiaHostFinding::Level;
using Code = HestiaHostFinding::Code;

// A session end is only worth pointing out for this long after it happened.
constexpr double kRecentSessionEndMs = 10 * 60 * 1000;

QString tr(const char* text)
{
    return QCoreApplication::translate("HestiaHostStatus", text);
}

int rank(Level level)
{
    // Passed checks read better before neutral notes.
    switch (level) {
    case Level::Error:
        return 3;
    case Level::Warning:
        return 2;
    case Level::Ok:
        return 1;
    case Level::Info:
    default:
        return 0;
    }
}

QString inputNames(const QString& detail)
{
    QStringList names;
    for (const QString& input : detail.split(',', Qt::SkipEmptyParts)) {
        if (input == QStringLiteral("keyboard")) {
            names.append(tr("keyboard"));
        }
        else if (input == QStringLiteral("mouse")) {
            names.append(tr("mouse"));
        }
        else if (input == QStringLiteral("gamepad")) {
            names.append(tr("gamepad"));
        }
        else {
            names.append(input);
        }
    }
    return names.join(QStringLiteral(", "));
}

QString checkLabel(const QString& id)
{
    if (id == QStringLiteral("encoder")) {
        return tr("Encoder");
    }
    if (id == QStringLiteral("display")) {
        return tr("Display");
    }
    if (id == QStringLiteral("virtual_display")) {
        return tr("Virtual display");
    }
    if (id == QStringLiteral("session_environment")) {
        return tr("Session");
    }
    if (id == QStringLiteral("preflight")) {
        return tr("Readiness");
    }
    return id;
}

QString terminationReason(const QString& reason)
{
    if (reason == QStringLiteral("handshake_failed")) {
        return tr("the video/audio handshake never completed (check the UDP ports and firewall)");
    }
    if (reason == QStringLiteral("protocol_error")) {
        return tr("the control stream failed");
    }
    if (reason == QStringLiteral("permission_revoked")) {
        return tr("this client's \"View Streams\" permission was revoked");
    }
    if (reason == QStringLiteral("client_lost")) {
        return tr("the connection to the client was lost");
    }
    return reason;
}

QString clipboardHint(const QString& diagnostic)
{
    if (diagnostic == QStringLiteral("wl_clipboard_missing")) {
        return tr("install wl-clipboard on the host");
    }
    if (diagnostic == QStringLiteral("xclip_missing")) {
        return tr("install xclip on the host");
    }
    if (diagnostic == QStringLiteral("desktop_session_unavailable")) {
        return tr("no desktop session");
    }
    return diagnostic.isEmpty() ? tr("no response") : diagnostic;
}

class Builder
{
public:
    explicit Builder(HestiaHostStatus* status)
        : m_Status(status)
    {
    }

    void add(Level level, Code code, const QString& checkId = QString(), const QString& detail = QString())
    {
        m_Status->findings.append({level, code, checkId, detail});
    }

    HestiaHostStatus finish()
    {
        const bool needsAttention = std::any_of(
                m_Status->findings.cbegin(), m_Status->findings.cend(),
                [](const HestiaHostFinding& finding) {
                    return finding.level == Level::Warning || finding.level == Level::Error;
                });
        if (!needsAttention) {
            // First, so the stable sort keeps it as the headline.
            m_Status->findings.prepend({Level::Ok, Code::Ready, QString(), QString()});
        }
        // Most severe first; within a level the host's own order holds.
        std::stable_sort(m_Status->findings.begin(), m_Status->findings.end(),
                         [](const HestiaHostFinding& a, const HestiaHostFinding& b) {
                             return rank(a.level) > rank(b.level);
                         });
        return *m_Status;
    }

private:
    HestiaHostStatus* m_Status;
};

void evaluatePermissions(Builder& builder, const QJsonObject& permissions)
{
    const bool view = permissions.value("view_streams").toBool();
    const bool launch = permissions.value("launch_apps").toBool();
    if (!launch && !view) {
        builder.add(Level::Error, Code::NoStreamPermission);
    }
    else if (!launch) {
        builder.add(Level::Warning, Code::NoLaunchPermission);
    }
    if (!permissions.value("list_apps").toBool()) {
        builder.add(Level::Warning, Code::NoListPermission);
    }

    QStringList missingInputs;
    if (!permissions.value("keyboard_input").toBool()) {
        missingInputs.append(QStringLiteral("keyboard"));
    }
    if (!permissions.value("mouse_input").toBool()) {
        missingInputs.append(QStringLiteral("mouse"));
    }
    if (!permissions.value("gamepad_input").toBool()) {
        missingInputs.append(QStringLiteral("gamepad"));
    }
    if (!missingInputs.isEmpty()) {
        builder.add(Level::Warning, Code::NoInputPermission, QString(), missingInputs.join(','));
    }
}

}

QString HestiaHostFinding::describe(bool displayRecoveryOffered) const
{
    switch (code) {
    case Code::Ready:
        return tr("Everything looks good: this host is ready to stream.");
    case Code::ApiUnreachable:
        return tr("The Hermes API isn't responding. Check that Hermes is running and that its web/API port (47990 by default) isn't blocked by a firewall.");
    case Code::ApiError:
        return tr("The Hermes API returned an error: %1").arg(detail.isEmpty() ? tr("no response") : detail);
    case Code::CertificateRejected:
        return tr("Hermes no longer recognizes this client's pairing. Delete the PC and pair it again.");
    case Code::NoStreamPermission:
        return tr("This client has neither the \"Launch Apps\" nor the \"View Streams\" permission, so it can't stream. Grant them in the Hermes web UI under PIN → Device Management.");
    case Code::NoLaunchPermission:
        return tr("This client can't start apps or create a virtual display (missing the \"Launch Apps\" permission). Grant it in the Hermes web UI under PIN → Device Management.");
    case Code::NoListPermission:
        return tr("This client can't list apps (missing the \"List Apps\" permission).");
    case Code::NoInputPermission:
        return tr("Input is blocked for: %1. Grant the matching input permissions in the Hermes web UI under PIN → Device Management.").arg(inputNames(detail));
    case Code::NoViewPermission:
        return tr("Host diagnostics are hidden because this client lacks the \"View Streams\" permission.");
    case Code::DiagnosticsUnavailable:
        return tr("Couldn't read the host's %1.")
                .arg(checkId == QStringLiteral("display") ? tr("display status") : tr("diagnostics"));
    case Code::PreflightCheck:
        return tr("%1: %2").arg(checkLabel(checkId),
                                detail.isEmpty() ? tr("the host reports it isn't ready to stream") : detail);
    case Code::DisplayLeftDisabled:
        return displayRecoveryOffered
                ? tr("The host's physical monitor seems to be off even though nothing is streaming. Use \"Recover Physical Monitor\" in the PC's menu to turn it back on.")
                : tr("The host's physical monitor seems to be off even though nothing is streaming.");
    case Code::LastSessionEnded:
        return tr("The last session ended unexpectedly: %1.").arg(terminationReason(detail));
    case Code::Streaming:
        return detail == QStringLiteral("1")
                ? tr("1 session is streaming right now.")
                : tr("%1 sessions are streaming right now.").arg(detail);
    case Code::AwaitingReconnect:
        return tr("An app is still running on the host, waiting for a client to resume it.");
    case Code::VirtualDisplayActive:
        return tr("Virtual display active: %1").arg(detail);
    case Code::ClipboardUnavailable:
        return tr("Clipboard sync is unavailable on the host (%1).").arg(clipboardHint(detail));
    }
    return detail;
}

HestiaHostFinding::Level HestiaHostStatus::level() const
{
    Level overall = Level::Ok;
    for (const HestiaHostFinding& finding : findings) {
        if (rank(finding.level) > rank(overall)) {
            overall = finding.level;
        }
    }
    return overall;
}

bool HestiaHostStatus::needsAttention() const
{
    const Level overall = level();
    return valid && (overall == Level::Warning || overall == Level::Error);
}

QString HestiaHostStatus::details() const
{
    QStringList lines;
    for (const HestiaHostFinding& finding : findings) {
        lines.append(finding.describe(displayRecoveryOffered));
    }
    return lines.join('\n');
}

HestiaHostStatus HestiaHostStatus::evaluate(const HestiaCapabilities& capabilities,
                                            const HestiaHostProbe& probe)
{
    HestiaHostStatus status;
    if (!capabilities.supportsProtocolV1) {
        return status;
    }
    status.valid = true;
    Builder builder(&status);

    bool canRecoverDisplay = capabilities.features.displayRecovery;
    if (capabilities.features.permissionSystem) {
        if (probe.permissionsHttpStatus == 401) {
            builder.add(Level::Error, Code::CertificateRejected);
            return builder.finish();
        }
        if (!probe.permissionsOk) {
            if (probe.permissionsHttpStatus == 0) {
                builder.add(Level::Error, Code::ApiUnreachable);
            }
            else if (probe.permissionsHttpStatus >= 200 && probe.permissionsHttpStatus < 300) {
                builder.add(Level::Error, Code::ApiError, QStringLiteral("permissions"),
                            tr("couldn't read this client's permissions"));
            }
            else {
                builder.add(Level::Error, Code::ApiError, QStringLiteral("permissions"),
                            QStringLiteral("HTTP %1").arg(probe.permissionsHttpStatus));
            }
            return builder.finish();
        }

        const QJsonObject permissions = probe.permissions.value("permissions").toObject();
        evaluatePermissions(builder, permissions);
        canRecoverDisplay = canRecoverDisplay && permissions.value("display_recovery").toBool();
        if (!permissions.value("view_streams").toBool()) {
            builder.add(Level::Info, Code::NoViewPermission);
            return builder.finish();
        }
    }

    bool diagnosticsUsable = false;
    int activeSessions = 0;
    bool awaitingReconnect = false;
    if (probe.diagnosticsFetched && !probe.diagnosticsOk) {
        builder.add(Level::Warning, Code::DiagnosticsUnavailable, QStringLiteral("diagnostics"));
    }
    else if (probe.diagnosticsFetched) {
        diagnosticsUsable = true;

        HestiaPreflight preflight;
        QString error;
        if (HestiaPreflight::fromDiagnosticsJson(probe.diagnostics, &preflight, &error) && preflight.valid) {
            bool sawFailure = false;
            for (const HestiaPreflightCheck& check : std::as_const(preflight.checks)) {
                if (check.isFail()) {
                    builder.add(Level::Error, Code::PreflightCheck, check.id, check.message);
                    sawFailure = true;
                }
                else if (check.isWarn()) {
                    builder.add(Level::Warning, Code::PreflightCheck, check.id, check.message);
                }
            }
            if (!preflight.ready && !sawFailure) {
                builder.add(Level::Error, Code::PreflightCheck, QStringLiteral("preflight"));
            }
        }

        const QJsonObject sessions =
                probe.diagnostics.value("runtime").toObject().value("sessions").toObject();
        activeSessions = qMax(0, sessions.value("active").toInt());
        awaitingReconnect = sessions.value("awaiting_reconnect").toBool();
        if (activeSessions > 0) {
            builder.add(Level::Info, Code::Streaming, QString(), QString::number(activeSessions));
        }
        else if (awaitingReconnect) {
            builder.add(Level::Info, Code::AwaitingReconnect);
        }

        const QString reason = sessions.value("last_termination").toString();
        const double sinceLastEnd = sessions.value("ms_since_last_end").toDouble();
        if (activeSessions == 0 && sessions.value("total_ended").toDouble() > 0 &&
                sinceLastEnd > 0 && sinceLastEnd < kRecentSessionEndMs) {
            if (reason == QStringLiteral("handshake_failed") ||
                    reason == QStringLiteral("protocol_error") ||
                    reason == QStringLiteral("permission_revoked")) {
                builder.add(Level::Warning, Code::LastSessionEnded, QString(), reason);
            }
            else if (reason == QStringLiteral("client_lost")) {
                builder.add(Level::Info, Code::LastSessionEnded, QString(), reason);
            }
            // client_quit, server_stopped and unknown are ordinary endings.
        }

        const QJsonObject clipboard =
                probe.diagnostics.value("dependencies").toObject().value("clipboard").toObject();
        if (clipboard.value("available").isBool() && !clipboard.value("available").toBool()) {
            builder.add(Level::Info, Code::ClipboardUnavailable, QString(),
                        clipboard.value("diagnostic").toString());
        }
    }

    if (probe.displayFetched && !probe.displayOk) {
        builder.add(Level::Warning, Code::DiagnosticsUnavailable, QStringLiteral("display"));
    }
    else if (probe.displayFetched) {
        const QJsonObject virtualDisplay = probe.display.value("virtual_display").toObject();
        if (probe.display.value("active").toBool() || virtualDisplay.value("enabled").toBool()) {
            const int width = virtualDisplay.value("width").toInt();
            const int height = virtualDisplay.value("height").toInt();
            const int fps = virtualDisplay.value("fps").toInt();
            QString mode;
            if (width > 0 && height > 0) {
                mode = QStringLiteral("%1x%2").arg(width).arg(height);
                if (fps > 0) {
                    mode += QStringLiteral("@%1").arg(fps);
                }
            }
            builder.add(Level::Info, Code::VirtualDisplayActive, QString(),
                        (mode + QLatin1Char(' ') + virtualDisplay.value("backend").toString()).trimmed());
        }

        // Only a display nothing is using counts as left off: during a stream,
        // or while an app waits for its client, the monitors are meant to be off.
        const QJsonObject physical = probe.display.value("physical_display").toObject();
        if (physical.value("disabled_during_stream").toBool() && diagnosticsUsable &&
                activeSessions == 0 && !awaitingReconnect) {
            builder.add(Level::Warning, Code::DisplayLeftDisabled);
            status.displayRecoveryOffered = canRecoverDisplay;
        }
    }

    return builder.finish();
}
