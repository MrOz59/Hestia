#include "autoupdatechecker.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSysInfo>

// Hestia's own releases. Moonlight's update manifest describes Moonlight's
// version numbers, against which every Hestia build looks out of date.
#define HESTIA_RELEASES_API "https://api.github.com/repos/MrOz59/Hestia/releases/"

AutoUpdateChecker::AutoUpdateChecker(QObject *parent) :
    QObject(parent)
{
    m_Nam = new QNetworkAccessManager(this);

    // Never communicate over HTTP
    m_Nam->setStrictTransportSecurityEnabled(true);

    // Allow HTTP redirects
    m_Nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    connect(m_Nam, &QNetworkAccessManager::finished,
            this, &AutoUpdateChecker::handleUpdateCheckRequestFinished);

    qDebug() << "Current Hestia version:" << VERSION_STR;
    m_CurrentVersion = HestiaUpdate::parse(VERSION_STR);

    // Should at least have a 1.0-style version number
    Q_ASSERT(m_CurrentVersion && m_CurrentVersion->release.size() > 1);
}

void AutoUpdateChecker::start()
{
    if (!m_Nam) {
        Q_ASSERT(m_Nam);
        return;
    }

#if defined(Q_OS_WIN32) || defined(Q_OS_DARWIN) || defined(STEAM_LINK) || defined(APP_IMAGE) // Only run update checker on platforms without auto-update
    if (!m_CurrentVersion) {
        qWarning() << "Not checking for updates: unrecognized version" << VERSION_STR;
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && QT_VERSION < QT_VERSION_CHECK(5, 15, 1) && !defined(QT_NO_BEARERMANAGEMENT)
    // HACK: Set network accessibility to work around QTBUG-80947 (introduced in Qt 5.14.0 and fixed in Qt 5.15.1)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED
    m_Nam->setNetworkAccessible(QNetworkAccessManager::Accessible);
    QT_WARNING_POP
#endif

    // A release build follows tagged releases. A nightly follows the rolling
    // "nightly" release, which "latest" never returns since it is a prerelease.
    QUrl url(m_CurrentVersion->isNightly() ? HESTIA_RELEASES_API "tags/nightly"
                                           : HESTIA_RELEASES_API "latest");
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    // The GitHub API turns away requests without a User-Agent.
    request.setRawHeader("User-Agent", "Hestia/" VERSION_STR);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
#else
    request.setAttribute(QNetworkRequest::HTTP2AllowedAttribute, true);
#endif

    // We'll get a callback when this is finished
    m_Nam->get(request);
#endif
}

// Only point at a release that has something to replace this build with.
// Hestia publishes Linux packages, so a Windows or macOS build (best-effort)
// is not sent to a release page that has nothing for it, and neither is an
// AppImage when that nightly's AppImage build failed.
bool AutoUpdateChecker::releaseHasAssetForThisBuild(const QJsonArray& assets)
{
    for (const auto& asset : assets) {
        const QString name = asset.toObject().value(QStringLiteral("name")).toString();
#if defined(APP_IMAGE)
        if (name.endsWith(QStringLiteral(".AppImage"), Qt::CaseInsensitive) &&
                name.contains(QSysInfo::buildCpuArchitecture())) {
            return true;
        }
#elif defined(STEAM_LINK)
        if (name.contains(QStringLiteral("SteamLink"), Qt::CaseInsensitive)) {
            return true;
        }
#elif defined(Q_OS_WIN32)
        if (name.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".msi"), Qt::CaseInsensitive) ||
                (name.contains(QStringLiteral("Portable"), Qt::CaseInsensitive) &&
                 name.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive))) {
            return true;
        }
#elif defined(Q_OS_DARWIN)
        if (name.endsWith(QStringLiteral(".dmg"), Qt::CaseInsensitive)) {
            return true;
        }
#else
        Q_UNUSED(name);
#endif
    }
    return false;
}

void AutoUpdateChecker::handleUpdateCheckRequestFinished(QNetworkReply* reply)
{
    Q_ASSERT(reply->isFinished());
    reply->deleteLater();

    // Delete the QNetworkAccessManager to free resources and
    // prevent the bearer plugin from polling in the background.
    m_Nam->deleteLater();
    m_Nam = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 404) {
            // Nothing published on this channel yet.
            qInfo() << "No Hestia release to check for updates against yet";
        }
        else {
            qWarning() << "Update checking failed with error:" << reply->error() << "HTTP status:" << status;
        }
        return;
    }

    QJsonParseError error;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (!jsonDoc.isObject()) {
        qWarning() << "Release metadata malformed:" << error.errorString();
        return;
    }

    const QJsonObject release = jsonDoc.object();
    const QString url = release.value(QStringLiteral("html_url")).toString();
    if (url.isEmpty() || release.value(QStringLiteral("draft")).toBool()) {
        qWarning() << "Release metadata has no published release page";
        return;
    }

    // A nightly's version, commit included, is in the release name; a
    // release's is its tag.
    const bool nightly = m_CurrentVersion->isNightly();
    const QString label = release.value(nightly ? QStringLiteral("name") : QStringLiteral("tag_name")).toString();
    const std::optional<HestiaUpdate::BuildVersion> available =
            nightly ? HestiaUpdate::versionInNightlyName(label.toStdString())
                    : HestiaUpdate::parse(label.toStdString());
    if (!available) {
        qWarning() << "Release carries no version to compare against:" << label;
        return;
    }

    const QString availableVersion = QString::fromStdString(HestiaUpdate::format(*available));
    qDebug() << "Latest Hestia" << (nightly ? "nightly" : "release") << "is:" << availableVersion;

    const bool newer = nightly ? HestiaUpdate::nightlyIsNewer(*m_CurrentVersion, *available)
                               : !release.value(QStringLiteral("prerelease")).toBool() &&
                                 HestiaUpdate::releaseIsNewer(*m_CurrentVersion, *available);
    if (!newer) {
        qDebug() << "No newer Hestia build available";
        return;
    }

    if (!releaseHasAssetForThisBuild(release.value(QStringLiteral("assets")).toArray())) {
        qInfo() << "Hestia" << availableVersion << "has no download for this platform";
        return;
    }

    qDebug() << "Update available";
    emit onUpdateAvailable(availableVersion, url);
}
