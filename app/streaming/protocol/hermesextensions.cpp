#include "hermesextensions.h"

#include <QJsonValue>

namespace HermesExtensions {

namespace {

constexpr uint32_t k_MaxReasonableVersion = 1000;

bool isValidName(const QString& name)
{
    return !name.isEmpty() && name.size() <= 64;
}

} // namespace

QVector<Extension> supported()
{
    return {
        {Name::CongestionReport, 1},
        {Name::PacketFeedback, 1},
    };
}

QVector<Extension> parseAdvertised(const QJsonObject& capabilities)
{
    QVector<Extension> advertised;

    const QJsonValue value = capabilities.value(QStringLiteral("extensions"));
    if (!value.isArray()) {
        // A Hermes build from before extensions existed. Not an error, and not
        // a reason to treat the host as less capable than it is.
        return advertised;
    }

    for (const QJsonValue& entry : value.toArray()) {
        if (!entry.isObject()) {
            continue;
        }

        const QJsonObject object = entry.toObject();
        const QString name = object.value(QStringLiteral("name")).toString();
        const QJsonValue version = object.value(QStringLiteral("version"));
        if (!isValidName(name) || !version.isDouble()) {
            continue;
        }

        const int parsedVersion = version.toInt();
        if (parsedVersion <= 0 ||
                static_cast<uint32_t>(parsedVersion) > k_MaxReasonableVersion) {
            continue;
        }

        advertised.append({name, static_cast<uint32_t>(parsedVersion)});
    }

    return advertised;
}

QVector<Extension> announcementFor(const QVector<Extension>& advertised)
{
    QVector<Extension> announcement;

    for (const Extension& candidate : supported()) {
        if (advertised.contains(candidate)) {
            announcement.append(candidate);
        }
    }

    return announcement;
}

QJsonArray toJson(const QVector<Extension>& extensions)
{
    QJsonArray array;

    for (const Extension& extension : extensions) {
        array.append(QJsonObject {
            {QStringLiteral("name"), extension.name},
            {QStringLiteral("version"), static_cast<int>(extension.version)},
        });
    }

    return array;
}

QMap<QString, uint32_t> parseNegotiated(const QJsonObject& prepareResponse)
{
    QMap<QString, uint32_t> negotiated;

    const QJsonValue value =
            prepareResponse.value(QStringLiteral("extensions"));
    if (!value.isArray()) {
        return negotiated;
    }

    for (const QJsonValue& entry : value.toArray()) {
        if (!entry.isObject()) {
            continue;
        }

        const QJsonObject object = entry.toObject();
        const QString name = object.value(QStringLiteral("name")).toString();
        const QJsonValue version = object.value(QStringLiteral("version"));
        if (!isValidName(name) || !version.isDouble()) {
            continue;
        }

        const int parsedVersion = version.toInt();
        if (parsedVersion <= 0) {
            continue;
        }

        negotiated.insert(name, static_cast<uint32_t>(parsedVersion));
    }

    return negotiated;
}

bool isActive(const QMap<QString, uint32_t>& negotiated,
              const QString& name,
              uint32_t version)
{
    const auto entry = negotiated.constFind(name);
    return entry != negotiated.constEnd() && entry.value() == version;
}

} // namespace HermesExtensions
