#include <QtTest>
#include <QJsonArray>
#include <QJsonObject>
#include "backend/hestiacapabilities.h"
#include "backend/hestiahoststatus.h"

// Isolated Qt unit tests: no game-streaming engine or experimental dev refactor.
namespace {

HestiaCapabilities hermesCapabilities()
{
    HestiaCapabilities capabilities;
    capabilities.supportsProtocolV1 = true;
    capabilities.features.permissionSystem = true;
    capabilities.features.displayRecovery = true;
    return capabilities;
}

QJsonObject allPermissions()
{
    QJsonObject permissions;
    for (const char* name : {"view_streams", "list_apps", "launch_apps", "keyboard_input",
                             "mouse_input", "gamepad_input", "display_recovery"}) {
        permissions.insert(QLatin1String(name), true);
    }
    return QJsonObject {{"ok", true}, {"client_id", "client"}, {"permissions", permissions}};
}

QJsonObject readyDiagnostics(const QJsonObject& sessions = QJsonObject {{"active", 0}})
{
    return QJsonObject {
        {"ok", true},
        {"dependencies", QJsonObject {}},
        {"preflight", QJsonObject {
             {"ready", true},
             {"checks", QJsonArray {QJsonObject {{"id", "encoder"}, {"status", "ok"}, {"message", "VAAPI"}}}},
         }},
        {"runtime", QJsonObject {{"sessions", sessions}}},
    };
}

HestiaHostProbe healthyProbe()
{
    HestiaHostProbe probe;
    probe.permissionsOk = true;
    probe.permissionsHttpStatus = 200;
    probe.permissions = allPermissions();
    probe.diagnosticsFetched = true;
    probe.diagnosticsOk = true;
    probe.diagnostics = readyDiagnostics();
    probe.displayFetched = true;
    probe.displayOk = true;
    probe.display = QJsonObject {{"ok", true}};
    return probe;
}

bool hasFinding(const HestiaHostStatus& status, HestiaHostFinding::Code code,
                HestiaHostFinding::Level level)
{
    for (const HestiaHostFinding& finding : status.findings) {
        if (finding.code == code && finding.level == level) {
            return true;
        }
    }
    return false;
}


}

class HestiaHostStatusTest : public QObject
{
    Q_OBJECT
private slots:
    void hostStatusIgnoresHostsWithoutTheHestiaApi()
    {
        const HestiaHostStatus status = HestiaHostStatus::evaluate(HestiaCapabilities {}, healthyProbe());
        QVERIFY(!status.valid);
        QVERIFY(!status.needsAttention());
        QVERIFY(status.findings.isEmpty());
    }

    void hostStatusIsReadyWhenNothingNeedsAttention()
    {
        HestiaHostProbe probe = healthyProbe();
        probe.diagnostics = readyDiagnostics(QJsonObject {{"active", 1}});
        const HestiaHostStatus status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(status.valid);
        QVERIFY(!status.needsAttention());
        QVERIFY(status.findings.first().code == HestiaHostFinding::Code::Ready);
        // A live session is a note, not a problem.
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::Streaming, HestiaHostFinding::Level::Info));
        QVERIFY(status.level() == HestiaHostFinding::Level::Ok);
    }

    void hostStatusReportsARejectedPairing()
    {
        HestiaHostProbe probe = healthyProbe();
        probe.permissionsOk = false;
        probe.permissionsHttpStatus = 401;
        const HestiaHostStatus status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(status.level() == HestiaHostFinding::Level::Error);
        QCOMPARE(status.findings.size(), 1);
        QVERIFY(status.findings.first().code == HestiaHostFinding::Code::CertificateRejected);

        probe.permissionsHttpStatus = 0;
        QVERIFY(HestiaHostStatus::evaluate(hermesCapabilities(), probe).findings.first().code ==
                HestiaHostFinding::Code::ApiUnreachable);
    }

    void hostStatusReportsMissingPermissions()
    {
        HestiaHostProbe probe = healthyProbe();
        QJsonObject permissions = probe.permissions.value("permissions").toObject();
        permissions.insert("launch_apps", false);
        permissions.insert("gamepad_input", false);
        probe.permissions.insert("permissions", permissions);

        HestiaHostStatus status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(status.level() == HestiaHostFinding::Level::Warning);
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::NoLaunchPermission, HestiaHostFinding::Level::Warning));
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::NoInputPermission, HestiaHostFinding::Level::Warning));
        QVERIFY(status.details().contains(QStringLiteral("gamepad")));

        // Without view, nothing more can be read, and neither can stream.
        permissions.insert("view_streams", false);
        probe.permissions.insert("permissions", permissions);
        status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(status.level() == HestiaHostFinding::Level::Error);
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::NoStreamPermission, HestiaHostFinding::Level::Error));
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::NoViewPermission, HestiaHostFinding::Level::Info));
        QVERIFY(!hasFinding(status, HestiaHostFinding::Code::PreflightCheck, HestiaHostFinding::Level::Error));
    }

    void hostStatusOrdersPreflightProblemsBySeverity()
    {
        HestiaHostProbe probe = healthyProbe();
        QJsonObject diagnostics = readyDiagnostics();
        diagnostics.insert("preflight", QJsonObject {
            {"ready", false},
            {"checks", QJsonArray {
                 QJsonObject {{"id", "display"}, {"status", "warn"}, {"message", "No monitor"}},
                 QJsonObject {{"id", "encoder"}, {"status", "fail"}, {"message", "No encoder"}},
             }},
        });
        probe.diagnostics = diagnostics;

        const HestiaHostStatus status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(status.level() == HestiaHostFinding::Level::Error);
        QCOMPARE(status.findings.at(0).checkId, QStringLiteral("encoder"));
        QVERIFY(status.findings.at(0).level == HestiaHostFinding::Level::Error);
        QCOMPARE(status.findings.at(1).checkId, QStringLiteral("display"));
        QVERIFY(status.findings.at(1).level == HestiaHostFinding::Level::Warning);

        // Not ready without saying why is still not ready.
        diagnostics.insert("preflight", QJsonObject {{"ready", false}, {"checks", QJsonArray {}}});
        probe.diagnostics = diagnostics;
        QVERIFY(hasFinding(HestiaHostStatus::evaluate(hermesCapabilities(), probe),
                           HestiaHostFinding::Code::PreflightCheck, HestiaHostFinding::Level::Error));
    }

    void hostStatusFlagsADisplayLeftOffOnlyWhenIdle()
    {
        HestiaHostProbe probe = healthyProbe();
        probe.display = QJsonObject {
            {"ok", true},
            {"virtual_display", QJsonObject {{"enabled", true}, {"backend", "hermes-kms"},
                                             {"width", 1920}, {"height", 1080}, {"fps", 60}}},
            {"physical_display", QJsonObject {{"disabled_during_stream", true}}},
        };

        HestiaHostStatus status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::DisplayLeftDisabled, HestiaHostFinding::Level::Warning));
        QVERIFY(status.displayRecoveryOffered);
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::VirtualDisplayActive, HestiaHostFinding::Level::Info));
        QVERIFY(status.details().contains(QStringLiteral("1920x1080@60 hermes-kms")));

        // During a stream, or with an app parked for its client, the monitor is meant to be off.
        probe.diagnostics = readyDiagnostics(QJsonObject {{"active", 0}, {"awaiting_reconnect", true}});
        status = HestiaHostStatus::evaluate(hermesCapabilities(), probe);
        QVERIFY(!hasFinding(status, HestiaHostFinding::Code::DisplayLeftDisabled, HestiaHostFinding::Level::Warning));
        QVERIFY(hasFinding(status, HestiaHostFinding::Code::AwaitingReconnect, HestiaHostFinding::Level::Info));
        QVERIFY(!status.needsAttention());
    }

    void hostStatusReportsRecentAbnormalSessionEnds()
    {
        HestiaHostProbe probe = healthyProbe();
        probe.diagnostics = readyDiagnostics(QJsonObject {
            {"active", 0}, {"total_ended", 3}, {"ms_since_last_end", 5000},
            {"last_termination", "handshake_failed"},
        });
        QVERIFY(hasFinding(HestiaHostStatus::evaluate(hermesCapabilities(), probe),
                           HestiaHostFinding::Code::LastSessionEnded, HestiaHostFinding::Level::Warning));

        // An ordinary ending, or one long ago, is nothing to report.
        probe.diagnostics = readyDiagnostics(QJsonObject {
            {"active", 0}, {"total_ended", 3}, {"ms_since_last_end", 5000},
            {"last_termination", "client_quit"},
        });
        QVERIFY(!HestiaHostStatus::evaluate(hermesCapabilities(), probe).needsAttention());
        probe.diagnostics = readyDiagnostics(QJsonObject {
            {"active", 0}, {"total_ended", 3}, {"ms_since_last_end", 60 * 60 * 1000},
            {"last_termination", "handshake_failed"},
        });
        QVERIFY(!HestiaHostStatus::evaluate(hermesCapabilities(), probe).needsAttention());
    }

};

QTEST_GUILESS_MAIN(HestiaHostStatusTest)
#include "tst_hestiahoststatus.moc"
