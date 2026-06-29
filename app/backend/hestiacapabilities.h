#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

class QJsonObject;

struct HestiaCompatibility
{
    bool gamestream = false;
    bool moonlight = false;
    bool sunshine = false;
};

struct HestiaFeatures
{
    bool virtualDisplay = false;
    QStringList virtualDisplayBackend;
    bool kdeKscreen = false;
    bool displayRecovery = false;
    bool clientResolutionMatching = false;
    bool clientFpsMatching = false;
    bool hdrModeControl = false;
    bool scaleFactor = false;
    bool gamescopeSession = false;
    bool serverCommands = false;
    bool clipboardSync = false;
    bool permissionSystem = false;
};

struct HestiaLimits
{
    int maxWidth = 0;
    int maxHeight = 0;
    int maxFps = 0;
    QVector<int> supportedFps;
    QStringList supportedCodecs;
};

struct HestiaCapabilities
{
    bool supportsProtocolV1 = false;
    QString serverName;
    QString base;
    int hestiaProtocol = 0;
    int minClientProtocol = 0;
    int maxClientProtocol = 0;
    QString serverVersion;
    HestiaCompatibility compatibility;
    HestiaFeatures features;
    HestiaLimits limits;

    static bool fromJson(const QJsonObject& response, HestiaCapabilities* capabilities, QString* error);
};

// One streaming-readiness check from the Hermes diagnostics preflight.
struct HestiaPreflightCheck
{
    QString id;
    QString status;   // "ok" | "warn" | "fail" (unknown values treated as warn)
    QString message;

    bool isFail() const { return status == QStringLiteral("fail"); }
    bool isWarn() const { return status == QStringLiteral("warn"); }
};

// Host readiness summary, parsed from diagnostics.preflight. Lets Hestia warn
// the user about a paired host before they try to start a session.
struct HestiaPreflight
{
    bool valid = false;     // True once a preflight was successfully parsed.
    bool ready = false;     // diagnostics.preflight.ready
    QVector<HestiaPreflightCheck> checks;

    // Checks that are not "ok", for surfacing in the UI (tooltip/details).
    QVector<HestiaPreflightCheck> problems() const
    {
        QVector<HestiaPreflightCheck> result;
        for (const auto& check : checks) {
            if (check.status != QStringLiteral("ok")) {
                result.append(check);
            }
        }
        return result;
    }

    static bool fromDiagnosticsJson(const QJsonObject& response, HestiaPreflight* preflight, QString* error);
};
