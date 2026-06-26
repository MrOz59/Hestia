#include "hestiacapabilities.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>

#include <limits>

namespace {

bool fail(QString* error, const QString& message)
{
    if (error != nullptr) {
        *error = message;
    }
    return false;
}

bool hasOnlyKeys(const QJsonObject& object, const QSet<QString>& expected, QString* error, const QString& objectName)
{
    if (object.size() != expected.size()) {
        return fail(error, objectName + " has missing or unknown fields");
    }

    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (!expected.contains(it.key())) {
            return fail(error, objectName + " has an unknown field: " + it.key());
        }
    }

    for (const QString& key : expected) {
        if (!object.contains(key)) {
            return fail(error, objectName + " is missing required field: " + key);
        }
    }

    return true;
}

bool readBool(const QJsonObject& object, const QString& key, bool* value, QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isBool()) {
        return fail(error, key + " must be a boolean");
    }

    *value = jsonValue.toBool();
    return true;
}

bool readPositiveInteger(const QJsonObject& object, const QString& key, int* value, QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble()) {
        return fail(error, key + " must be an integer");
    }

    const double number = jsonValue.toDouble();
    if (number < 1 || number > std::numeric_limits<int>::max() || number != static_cast<int>(number)) {
        return fail(error, key + " must be a positive integer");
    }

    *value = static_cast<int>(number);
    return true;
}

bool readFixedInteger(const QJsonObject& object, const QString& key, int expected, QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble() || jsonValue.toDouble() != expected) {
        return fail(error, key + " is incompatible");
    }
    return true;
}

bool readFixedString(const QJsonObject& object, const QString& key, const QString& expected, QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isString() || jsonValue.toString() != expected) {
        return fail(error, key + " is incompatible");
    }
    return true;
}

bool readStringArray(const QJsonObject& object,
                     const QString& key,
                     const QSet<QString>& allowedValues,
                     QStringList* values,
                     QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isArray()) {
        return fail(error, key + " must be an array");
    }

    QSet<QString> seen;
    QStringList parsedValues;
    for (const QJsonValue& item : jsonValue.toArray()) {
        if (!item.isString() || !allowedValues.contains(item.toString()) || seen.contains(item.toString())) {
            return fail(error, key + " contains an invalid value");
        }
        seen.insert(item.toString());
        parsedValues.append(item.toString());
    }

    *values = parsedValues;
    return true;
}

bool readPositiveIntegerArray(const QJsonObject& object, const QString& key, QVector<int>* values, QString* error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isArray()) {
        return fail(error, key + " must be an array");
    }

    QSet<int> seen;
    QVector<int> parsedValues;
    for (const QJsonValue& item : jsonValue.toArray()) {
        if (!item.isDouble()) {
            return fail(error, key + " contains a non-integer value");
        }

        const double number = item.toDouble();
        if (number < 1 || number > std::numeric_limits<int>::max() ||
                number != static_cast<int>(number) || seen.contains(static_cast<int>(number))) {
            return fail(error, key + " contains an invalid value");
        }

        seen.insert(static_cast<int>(number));
        parsedValues.append(static_cast<int>(number));
    }

    *values = parsedValues;
    return true;
}

} // namespace

bool HestiaCapabilities::fromJson(const QJsonObject& response, HestiaCapabilities* capabilities, QString* error)
{
    if (capabilities == nullptr) {
        return fail(error, "Capabilities destination is null");
    }

    const QSet<QString> rootKeys = {
        "ok", "server_name", "base", "hestia_protocol", "min_client_protocol", "max_client_protocol",
        "server_version", "compatibility", "features", "limits"
    };
    if (!hasOnlyKeys(response, rootKeys, error, "Capabilities response")) {
        return false;
    }

    if (!response.value("ok").isBool() || !response.value("ok").toBool()) {
        return fail(error, "ok must be true");
    }

    if (!readFixedString(response, "server_name", "Hermes", error) ||
            !readFixedString(response, "base", "Apollo", error) ||
            !readFixedInteger(response, "hestia_protocol", 1, error) ||
            !readFixedInteger(response, "min_client_protocol", 1, error) ||
            !readFixedInteger(response, "max_client_protocol", 1, error)) {
        return false;
    }

    if (!response.value("server_version").isString() || response.value("server_version").toString().isEmpty()) {
        return fail(error, "server_version must be a non-empty string");
    }

    if (!response.value("compatibility").isObject() || !response.value("features").isObject() || !response.value("limits").isObject()) {
        return fail(error, "Capabilities response contains an invalid object");
    }

    HestiaCapabilities parsed;
    parsed.supportsProtocolV1 = true;
    parsed.serverName = response.value("server_name").toString();
    parsed.base = response.value("base").toString();
    parsed.hestiaProtocol = 1;
    parsed.minClientProtocol = 1;
    parsed.maxClientProtocol = 1;
    parsed.serverVersion = response.value("server_version").toString();

    const QJsonObject compatibility = response.value("compatibility").toObject();
    const QSet<QString> compatibilityKeys = {"gamestream", "moonlight", "sunshine"};
    if (!hasOnlyKeys(compatibility, compatibilityKeys, error, "compatibility") ||
            !readBool(compatibility, "gamestream", &parsed.compatibility.gamestream, error) ||
            !readBool(compatibility, "moonlight", &parsed.compatibility.moonlight, error) ||
            !readBool(compatibility, "sunshine", &parsed.compatibility.sunshine, error)) {
        return false;
    }

    const QJsonObject features = response.value("features").toObject();
    const QSet<QString> featureKeys = {
        "virtual_display", "virtual_display_backend", "kde_kscreen", "display_recovery",
        "client_resolution_matching", "client_fps_matching", "hdr_mode_control", "scale_factor",
        "gamescope_session", "server_commands", "clipboard_sync", "permission_system"
    };
    if (!hasOnlyKeys(features, featureKeys, error, "features") ||
            !readBool(features, "virtual_display", &parsed.features.virtualDisplay, error) ||
            !readStringArray(features, "virtual_display_backend", {"evdi", "hermes_kms"}, &parsed.features.virtualDisplayBackend, error) ||
            !readBool(features, "kde_kscreen", &parsed.features.kdeKscreen, error) ||
            !readBool(features, "display_recovery", &parsed.features.displayRecovery, error) ||
            !readBool(features, "client_resolution_matching", &parsed.features.clientResolutionMatching, error) ||
            !readBool(features, "client_fps_matching", &parsed.features.clientFpsMatching, error) ||
            !readBool(features, "hdr_mode_control", &parsed.features.hdrModeControl, error) ||
            !readBool(features, "scale_factor", &parsed.features.scaleFactor, error) ||
            !readBool(features, "gamescope_session", &parsed.features.gamescopeSession, error) ||
            !readBool(features, "server_commands", &parsed.features.serverCommands, error) ||
            !readBool(features, "clipboard_sync", &parsed.features.clipboardSync, error) ||
            !readBool(features, "permission_system", &parsed.features.permissionSystem, error)) {
        return false;
    }

    const QJsonObject limits = response.value("limits").toObject();
    const QSet<QString> limitKeys = {"max_width", "max_height", "max_fps", "supported_fps", "supported_codecs"};
    if (!hasOnlyKeys(limits, limitKeys, error, "limits") ||
            !readPositiveInteger(limits, "max_width", &parsed.limits.maxWidth, error) ||
            !readPositiveInteger(limits, "max_height", &parsed.limits.maxHeight, error) ||
            !readPositiveInteger(limits, "max_fps", &parsed.limits.maxFps, error) ||
            !readPositiveIntegerArray(limits, "supported_fps", &parsed.limits.supportedFps, error) ||
            !readStringArray(limits, "supported_codecs", {"h264", "hevc", "av1"}, &parsed.limits.supportedCodecs, error)) {
        return false;
    }

    *capabilities = parsed;
    return true;
}
