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
