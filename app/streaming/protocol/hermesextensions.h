#pragma once

#include <cstdint>

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVector>

namespace HermesExtensions {

// One Hermes protocol extension, identified by name and an independent
// version. Versions move separately from the Hestia protocol version and from
// each other, so a client and a host can be upgraded in either order.
struct Extension
{
    QString name;
    uint32_t version = 0;

    bool operator==(const Extension& other) const
    {
        return name == other.name && version == other.version;
    }
};

// Extension names this client knows how to act on. Declared once so the
// announcement, the negotiated set, and the code that uses them cannot drift
// apart through repeated string literals.
namespace Name {
inline const QString PacketFeedback = QStringLiteral("packet_feedback");
inline const QString CongestionReport = QStringLiteral("congestion_report");
} // namespace Name

// What this build can honour. An extension belongs here only once the client
// actually implements it: announcing one it cannot serve would have the host
// enable a path that silently does nothing.
QVector<Extension> supported();

// Extensions advertised by a host, parsed from the capabilities response.
// Absent or malformed is not an error -- a host that predates extensions simply
// offers none, and must keep working exactly as before.
QVector<Extension> parseAdvertised(const QJsonObject& capabilities);

// What to announce for a session: everything this client supports that the
// host also advertises. Announcing the rest would be harmless (the host drops
// unknown names) but pointless, and it keeps the request honest about what the
// two ends could actually agree on.
QVector<Extension> announcementFor(const QVector<Extension>& advertised);

// Serialize an announcement for session/prepare.
QJsonArray toJson(const QVector<Extension>& extensions);

// Read the set the host reports as in force from a session/prepare response.
// The host is the authority here: names the client announced may be missing,
// and the client must use this rather than assume its announcement was taken
// whole.
QMap<QString, uint32_t> parseNegotiated(const QJsonObject& prepareResponse);

// Whether a negotiated set contains an extension at exactly this version.
bool isActive(const QMap<QString, uint32_t>& negotiated,
              const QString& name,
              uint32_t version);

} // namespace HermesExtensions
