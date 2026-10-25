#pragma once

#include "updatechannel.h"

#include <QObject>
#include <QNetworkAccessManager>

#include <optional>

class QJsonArray;

class AutoUpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit AutoUpdateChecker(QObject *parent = nullptr);

    Q_INVOKABLE void start();

signals:
    void onUpdateAvailable(QString newVersion, QString url);

private slots:
    void handleUpdateCheckRequestFinished(QNetworkReply* reply);

private:
    static bool releaseHasAssetForThisBuild(const QJsonArray& assets);

    std::optional<HestiaUpdate::BuildVersion> m_CurrentVersion;
    QNetworkAccessManager* m_Nam;
};
