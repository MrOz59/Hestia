#include "nvcomputer.h"
#include <Limelight.h>

#include <QDebug>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QXmlStreamReader>
#include <QSslKey>
#include <QImageReader>
#include <QtEndian>
#include <QNetworkProxy>

#define FAST_FAIL_TIMEOUT_MS 2000
#define REQUEST_TIMEOUT_MS 5000
#define LAUNCH_TIMEOUT_MS 120000
#define RESUME_TIMEOUT_MS 30000
#define QUIT_TIMEOUT_MS 30000
#define HESTIA_CAPABILITIES_TIMEOUT_MS 2000
#define HESTIA_SESSION_PREPARE_TIMEOUT_MS 5000
#define HESTIA_CLIPBOARD_TIMEOUT_MS 1000
#define HESTIA_DIAGNOSTICS_TIMEOUT_MS 2000

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define XML_NAME_EQUALS(x, y) ((x) == (y))
#else
#define XML_NAME_EQUALS(x, y) ((x) == (u##y))
#endif

NvHTTP::NvHTTP(NvAddress address, uint16_t httpsPort, QSslCertificate serverCert, bool useTrueUid, QNetworkAccessManager* nam) :
    m_Nam(nam ? nam : new QNetworkAccessManager(this)),
    m_ServerCert(serverCert),
    m_UseTrueUid(useTrueUid)
{
    m_BaseUrlHttp.setScheme("http");
    m_BaseUrlHttps.setScheme("https");

    setAddress(address);
    setHttpsPort(httpsPort);

    // Never use a proxy server
    QNetworkProxy noProxy(QNetworkProxy::NoProxy);
    m_Nam->setProxy(noProxy);
}

NvHTTP::NvHTTP(NvComputer* computer, QNetworkAccessManager* nam) :
    NvHTTP(computer->activeAddress, computer->activeHttpsPort, computer->serverCert, !computer->isNvidiaServerSoftware, nam)
{
}

void NvHTTP::setServerCert(QSslCertificate serverCert)
{
    m_ServerCert = serverCert;
}

void NvHTTP::setAddress(NvAddress address)
{
    Q_ASSERT(!address.isNull());

    m_Address = address;

    m_BaseUrlHttp.setHost(address.address());
    m_BaseUrlHttps.setHost(address.address());

    m_BaseUrlHttp.setPort(address.port());
}

void NvHTTP::setHttpsPort(uint16_t port)
{
    m_BaseUrlHttps.setPort(port);
}

void NvHTTP::setTrueUid(bool useTrueUid)
{
    m_UseTrueUid = useTrueUid;
}

NvAddress NvHTTP::address()
{
    return m_Address;
}

QSslCertificate NvHTTP::serverCert()
{
    return m_ServerCert;
}

uint16_t NvHTTP::httpPort()
{
    return m_BaseUrlHttp.port();
}

uint16_t NvHTTP::httpsPort()
{
    return m_BaseUrlHttps.port();
}

QVector<int>
NvHTTP::parseQuad(QString quad)
{
    QVector<int> ret;

    // Return an empty vector for old GFE versions
    // that were missing GfeVersion.
    if (quad.isEmpty()) {
        return ret;
    }

    QStringList parts = quad.split(".");
    ret.reserve(parts.length());
    for (int i = 0; i < parts.length(); i++)
    {
        ret.append(parts.at(i).toInt());
    }

    return ret;
}

int
NvHTTP::getCurrentGame(QString serverInfo)
{
    // GFE 2.8 started keeping currentgame set to the last game played. As a result, it no longer
    // has the semantics that its name would indicate. To contain the effects of this change as much
    // as possible, we'll force the current game to zero if the server isn't in a streaming session.
    QString serverState = getXmlString(serverInfo, "state");
    if (serverState.endsWith("_SERVER_BUSY"))
    {
        return getXmlString(serverInfo, "currentgame").toInt();
    }
    else
    {
        return 0;
    }
}

QString
NvHTTP::getServerInfo(NvLogLevel logLevel, bool fastFail)
{
    QString serverInfo;

    // Check if we have a pinned cert and HTTPS port for this host yet
    if (!m_ServerCert.isNull() && httpsPort() != 0)
    {
        try
        {
            // Always try HTTPS first, since it properly reports
            // pairing status (and a few other attributes).
            serverInfo = openConnectionToString(m_BaseUrlHttps,
                                                "serverinfo",
                                                nullptr,
                                                fastFail ? FAST_FAIL_TIMEOUT_MS : REQUEST_TIMEOUT_MS,
                                                logLevel);
            // Throws if the request failed
            verifyResponseStatus(serverInfo);
        }
        catch (const GfeHttpResponseException& e)
        {
            if (e.getStatusCode() == 401)
            {
                // Certificate validation error, fallback to HTTP
                serverInfo = openConnectionToString(m_BaseUrlHttp,
                                                    "serverinfo",
                                                    nullptr,
                                                    fastFail ? FAST_FAIL_TIMEOUT_MS : REQUEST_TIMEOUT_MS,
                                                    logLevel);
                verifyResponseStatus(serverInfo);
            }
            else
            {
                // Rethrow real errors
                throw e;
            }
        }
    }
    else
    {
        // Only use HTTP prior to pairing or fetching HTTPS port
        serverInfo = openConnectionToString(m_BaseUrlHttp,
                                            "serverinfo",
                                            nullptr,
                                            fastFail ? FAST_FAIL_TIMEOUT_MS : REQUEST_TIMEOUT_MS,
                                            logLevel);
        verifyResponseStatus(serverInfo);

        // Populate the HTTPS port
        uint16_t httpsPort = getXmlString(serverInfo, "HttpsPort").toUShort();
        if (httpsPort == 0) {
            httpsPort = DEFAULT_HTTPS_PORT;
        }
        setHttpsPort(httpsPort);

        // If we just needed to determine the HTTPS port, we'll try again over
        // HTTPS now that we have the port number
        if (!m_ServerCert.isNull()) {
            return getServerInfo(logLevel, fastFail);
        }
    }

    return serverInfo;
}

bool NvHTTP::probeHestiaCapabilities(HestiaCapabilities* capabilities)
{
    Q_ASSERT(capabilities != nullptr);

    if (m_ServerCert.isNull()) {
        qDebug() << "[Hestia] Host does not support Hestia API; using standard mode (host is not paired)";
        return false;
    }

    // Hermes serves its optional API on the documented Apollo/Sunshine web port.
    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/capabilities");

    qDebug().noquote() << "[Hestia] Probing enhanced capabilities at" << url.toString();

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif

    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &loop, &QEventLoop::quit);
    timeoutTimer.start(HESTIA_CAPABILITIES_TIMEOUT_MS);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    if (!reply->isFinished()) {
        reply->abort();
        qDebug() << "[Hestia] Host does not support Hestia API; using standard mode (capabilities request timed out)";
        delete reply;
        return false;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
        qDebug() << "[Hestia] Host does not support Hestia API; using standard mode"
                 << "(status" << statusCode << ", network error" << reply->error() << ")";
        delete reply;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    delete reply;
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        qDebug() << "[Hestia] Host does not support Hestia API; using standard mode (malformed capabilities response)";
        return false;
    }

    HestiaCapabilities parsedCapabilities;
    QString validationError;
    if (!HestiaCapabilities::fromJson(document.object(), &parsedCapabilities, &validationError)) {
        qDebug().noquote() << "[Hestia] Host does not support Hestia API; using standard mode (" + validationError + ")";
        return false;
    }

    *capabilities = parsedCapabilities;
    qDebug() << "[Hestia] Host supports Hestia protocol v1";
    return true;
}

bool NvHTTP::probeHestiaDiagnostics(HestiaPreflight* preflight)
{
    Q_ASSERT(preflight != nullptr);

    if (m_ServerCert.isNull()) {
        // Diagnostics needs a paired client certificate; skip silently.
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/diagnostics");

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif

    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &loop, &QEventLoop::quit);
    timeoutTimer.start(HESTIA_DIAGNOSTICS_TIMEOUT_MS);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    if (!reply->isFinished()) {
        reply->abort();
        delete reply;
        return false;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
        delete reply;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    delete reply;
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return false;
    }

    HestiaPreflight parsedPreflight;
    QString validationError;
    if (!HestiaPreflight::fromDiagnosticsJson(document.object(), &parsedPreflight, &validationError)) {
        qDebug().noquote() << "[Hestia] Diagnostics preflight unavailable (" + validationError + ")";
        return false;
    }

    *preflight = parsedPreflight;
    return true;
}

bool NvHTTP::prepareHestiaSession(const QJsonObject& sessionRequest)
{
    if (m_ServerCert.isNull()) {
        qDebug() << "[Hestia] Skipping session prepare; host is not paired";
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/session/prepare");

    const QJsonObject stream = sessionRequest.value("stream").toObject();
    const QJsonObject virtualDisplay = sessionRequest.value("virtual_display").toObject();
    qDebug() << "[Hestia] Preparing session with virtual_display="
             << virtualDisplay.value("enabled").toBool()
             << "width=" << stream.value("requested_width").toInt()
             << "height=" << stream.value("requested_height").toInt()
             << "fps=" << stream.value("requested_fps").toInt();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif

    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->post(request, QJsonDocument(sessionRequest).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &loop, &QEventLoop::quit);
    timeoutTimer.start(HESTIA_SESSION_PREPARE_TIMEOUT_MS);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    if (!reply->isFinished()) {
        reply->abort();
        qDebug() << "[Hestia] Session prepare unavailable; using standard Moonlight mode (request timed out)";
        delete reply;
        return false;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
        qDebug() << "[Hestia] Session prepare unavailable; using standard Moonlight mode"
                 << "(status" << statusCode << ", network error" << reply->error() << ")";
        delete reply;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    delete reply;
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        qDebug() << "[Hestia] Session prepare unavailable; using standard Moonlight mode (malformed response)";
        return false;
    }

    const QJsonObject response = document.object();
    const QJsonObject responseVirtualDisplay = response.value("virtual_display").toObject();
    const QJsonArray warnings = response.value("warnings").toArray();
    if (!response.value("ok").isBool() || !response.value("ok").toBool() ||
            !response.value("session_id").isString() || response.value("session_id").toString().isEmpty() ||
            responseVirtualDisplay.isEmpty() ||
            !responseVirtualDisplay.value("created").isBool() ||
            !responseVirtualDisplay.value("name").isString() ||
            !responseVirtualDisplay.value("backend").isString() ||
            !responseVirtualDisplay.value("width").isDouble() ||
            !responseVirtualDisplay.value("height").isDouble() ||
            !responseVirtualDisplay.value("fps").isDouble() ||
            !responseVirtualDisplay.value("hdr").isBool() ||
            !responseVirtualDisplay.value("kscreen_enabled").isBool() ||
            !response.value("warnings").isArray()) {
        qDebug() << "[Hestia] Session prepare unavailable; using standard Moonlight mode (invalid response)";
        return false;
    }

    for (const QJsonValue& warning : warnings) {
        if (!warning.isString()) {
            qDebug() << "[Hestia] Session prepare unavailable; using standard Moonlight mode (invalid warnings)";
            return false;
        }
    }

    qDebug() << "[Hestia] Session prepared:" << response.value("session_id").toString();
    return true;
}

bool NvHTTP::stopHestiaSession()
{
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/session/stop");
    QNetworkRequest request(url);
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->post(request, QByteArray());
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    return success && error.error == QJsonParseError::NoError && response.isObject() &&
            response.object().value("ok").toBool();
}

bool NvHTTP::getHestiaDisplayStatus(QJsonObject* status)
{
    Q_ASSERT(status != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/display/status");

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool()) {
        qDebug() << "[Hestia] Display status unavailable; keeping standard mode";
        return false;
    }

    *status = response.object();
    return true;
}

bool NvHTTP::recoverHestiaDisplay(bool force, QJsonObject* status)
{
    Q_ASSERT(status != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/display/recover");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->post(request, QJsonDocument(QJsonObject {{"force", force}}).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool()) {
        qDebug() << "[Hestia] Display recovery unavailable";
        return false;
    }

    *status = response.object();
    return true;
}

bool NvHTTP::getHestiaServerCommands(QJsonArray* commands)
{
    Q_ASSERT(commands != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/commands");
    QNetworkRequest request(url);
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool() || !response.object().value("commands").isArray()) {
        qDebug() << "[Hestia] Server commands unavailable";
        return false;
    }

    *commands = response.object().value("commands").toArray();
    return true;
}

bool NvHTTP::getHestiaClientPermissions(QJsonObject* permissions)
{
    Q_ASSERT(permissions != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/client/permissions");
    QNetworkRequest request(url);
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool() || !response.object().value("client_id").isString() ||
            !response.object().value("permissions").isObject()) {
        qDebug() << "[Hestia] Client permissions unavailable";
        return false;
    }

    *permissions = response.object();
    return true;
}

bool NvHTTP::getHestiaDiagnostics(QJsonObject* diagnostics)
{
    Q_ASSERT(diagnostics != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/diagnostics");
    QNetworkRequest request(url);
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool() || !response.object().value("dependencies").isObject()) {
        qDebug() << "[Hestia] Hermes diagnostics unavailable";
        return false;
    }

    *diagnostics = response.object();
    return true;
}

bool NvHTTP::getHestiaClipboard(QString* text)
{
    Q_ASSERT(text != nullptr);
    if (m_ServerCert.isNull()) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/clipboard");
    QNetworkRequest request(url);
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);
    QEventLoop loop;
    QTimer::singleShot(HESTIA_CLIPBOARD_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    if (!success || error.error != QJsonParseError::NoError || !response.isObject() ||
            !response.object().value("ok").toBool() || !response.object().value("text").isString()) {
        qDebug() << "[Hestia] Host clipboard is unavailable";
        return false;
    }

    *text = response.object().value("text").toString();
    return true;
}

bool NvHTTP::setHestiaClipboard(const QString& text)
{
    if (m_ServerCert.isNull() || text.contains(QChar::Null) || text.toUtf8().size() > 64 * 1024) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/clipboard");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->post(request, QJsonDocument(QJsonObject {{"text", text}}).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer::singleShot(HESTIA_CLIPBOARD_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    return success && error.error == QJsonParseError::NoError && response.isObject() &&
            response.object().value("ok").toBool();
}

bool NvHTTP::runHestiaServerCommand(int commandId)
{
    if (m_ServerCert.isNull() || commandId < 0) {
        return false;
    }

    QUrl url;
    url.setScheme("https");
    url.setHost(m_Address.address());
    url.setPort(m_Address.port() + 1);
    url.setPath("/api/hestia/v1/commands/run");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());
    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->post(request, QJsonDocument(QJsonObject {{"id", commandId}}).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    QTimer::singleShot(HESTIA_SESSION_PREPARE_TIMEOUT_MS, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    disconnect(sslErrorsConnection);

    const bool success = reply->isFinished() && reply->error() == QNetworkReply::NoError &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200 &&
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 300;
    QJsonParseError error;
    const QJsonDocument response = QJsonDocument::fromJson(reply->readAll(), &error);
    delete reply;
    return success && error.error == QJsonParseError::NoError && response.isObject() &&
            response.object().value("ok").toBool();
}

void
NvHTTP::startApp(QString verb,
                 bool isGfe,
                 int appId,
                 PSTREAM_CONFIGURATION streamConfig,
                 bool sops,
                 bool localAudio,
                 int gamepadMask,
                 bool persistGameControllersOnDisconnect,
                 QString& rtspSessionUrl)
{
    int riKeyId;

    memcpy(&riKeyId, streamConfig->remoteInputAesIv, sizeof(riKeyId));
    riKeyId = qFromBigEndian(riKeyId);

    QString response =
            openConnectionToString(m_BaseUrlHttps,
                                   verb,
                                   "appid="+QString::number(appId)+
                                   "&mode="+QString::number(streamConfig->width)+"x"+
                                   QString::number(streamConfig->height)+"x"+
                                   // Using an FPS value over 60 causes SOPS to default to 720p60,
                                   // so force it to 0 to ensure the correct resolution is set. We
                                   // used to use 60 here but that locked the frame rate to 60 FPS
                                   // on GFE 3.20.3. We don't need this hack for Sunshine.
                                   QString::number((streamConfig->fps > 60 && isGfe) ? 0 : streamConfig->fps)+
                                   "&additionalStates=1&sops="+QString::number(sops ? 1 : 0)+
                                   "&rikey="+QByteArray(streamConfig->remoteInputAesKey, sizeof(streamConfig->remoteInputAesKey)).toHex()+
                                   "&rikeyid="+QString::number(riKeyId)+
                                   ((streamConfig->supportedVideoFormats & VIDEO_FORMAT_MASK_10BIT) ?
                                       "&hdrMode=1&clientHdrCapVersion=0&clientHdrCapSupportedFlagsInUint32=0&clientHdrCapMetaDataId=NV_STATIC_METADATA_TYPE_1&clientHdrCapDisplayData=0x0x0x0x0x0x0x0x0x0x0" :
                                        "")+
                                   "&localAudioPlayMode="+QString::number(localAudio ? 1 : 0)+
                                   "&surroundAudioInfo="+QString::number(SURROUNDAUDIOINFO_FROM_AUDIO_CONFIGURATION(streamConfig->audioConfiguration))+
                                   "&remoteControllersBitmap="+QString::number(gamepadMask)+
                                   "&gcmap="+QString::number(gamepadMask)+
                                   "&gcpersist="+QString::number(persistGameControllersOnDisconnect ? 1 : 0)+
                                   LiGetLaunchUrlQueryParameters(),
                                   LAUNCH_TIMEOUT_MS);

    qInfo() << "Launch response:" << response;

    // Throws if the request failed
    verifyResponseStatus(response);

    rtspSessionUrl = getXmlString(response, "sessionUrl0");
}

void
NvHTTP::quitApp()
{
    QString response =
            openConnectionToString(m_BaseUrlHttps,
                                   "cancel",
                                   nullptr,
                                   QUIT_TIMEOUT_MS);

    qInfo() << "Quit response:" << response;

    // Throws if the request failed
    verifyResponseStatus(response);

    // Newer GFE versions will just return success even if quitting fails
    // if we're not the original requester.
    if (getCurrentGame(getServerInfo(NvHTTP::NVLL_ERROR)) != 0) {
        // Generate a synthetic GfeResponseException letting the caller know
        // that they can't kill someone else's stream.
        throw GfeHttpResponseException(599, "");
    }
}

QVector<NvDisplayMode>
NvHTTP::getDisplayModeList(QString serverInfo)
{
    QXmlStreamReader xmlReader(serverInfo);
    QVector<NvDisplayMode> modes;

    while (!xmlReader.atEnd()) {
        while (xmlReader.readNextStartElement()) {
            auto name = xmlReader.name();
            if (XML_NAME_EQUALS(name, "DisplayMode")) {
                modes.append(NvDisplayMode());
            }
            else if (!modes.isEmpty()) {
                if (XML_NAME_EQUALS(name, "Width")) {
                    modes.last().width = xmlReader.readElementText().toInt();
                }
                else if (XML_NAME_EQUALS(name, "Height")) {
                    modes.last().height = xmlReader.readElementText().toInt();
                }
                else if (XML_NAME_EQUALS(name, "RefreshRate")) {
                    modes.last().refreshRate = xmlReader.readElementText().toInt();
                }
            }
        }
    }

    return modes;
}

QVector<NvApp>
NvHTTP::getAppList()
{
    QString appxml = openConnectionToString(m_BaseUrlHttps,
                                            "applist",
                                            nullptr,
                                            REQUEST_TIMEOUT_MS,
                                            NvLogLevel::NVLL_ERROR);
    verifyResponseStatus(appxml);

    QXmlStreamReader xmlReader(appxml);
    QVector<NvApp> apps;
    while (!xmlReader.atEnd()) {
        while (xmlReader.readNextStartElement()) {
            auto name = xmlReader.name();
            if (XML_NAME_EQUALS(name, "App")) {
                // We must have a valid app before advancing to the next one
                if (!apps.isEmpty() && !apps.last().isInitialized()) {
                    qWarning() << "Invalid applist XML";
                    throw std::runtime_error("Invalid applist XML");
                }
                apps.append(NvApp());
            }
            else if (!apps.isEmpty()) {
                if (XML_NAME_EQUALS(name, "AppTitle")) {
                    // If an app has no name, Sunshine may send us <AppTitle/>,
                    // which readElementText() returns as a null QString.
                    // We want to treat this as an empty QString instead, so we
                    // will explicitly convert it. An empty string will satisfy
                    // NvApp's isInitialized() check.
                    QString name = xmlReader.readElementText();
                    if (name.isNull()) {
                        name = "";
                    }
                    apps.last().name = name;
                }
                else if (XML_NAME_EQUALS(name, "ID")) {
                    apps.last().id = xmlReader.readElementText().toInt();
                }
                else if (XML_NAME_EQUALS(name, "IsHdrSupported")) {
                    apps.last().hdrSupported = xmlReader.readElementText() == "1";
                }
                else if (XML_NAME_EQUALS(name, "IsAppCollectorGame")) {
                    apps.last().isAppCollectorGame = xmlReader.readElementText() == "1";
                }
            }
        }
    }

    return apps;
}

void
NvHTTP::verifyResponseStatus(QString xml)
{
    QXmlStreamReader xmlReader(xml);

    while (xmlReader.readNextStartElement())
    {
        if (XML_NAME_EQUALS(xmlReader.name(), "root"))
        {
            // Status code can be 0xFFFFFFFF in some rare cases on GFE 3.20.3, and
            // QString::toInt() will fail in that case, so use QString::toUInt()
            // and cast the result to an int instead.
            int statusCode = (int)xmlReader.attributes().value("status_code").toUInt();
            if (statusCode == 200)
            {
                // Successful
                return;
            }
            else
            {
                QString statusMessage = xmlReader.attributes().value("status_message").toString();
                if (statusCode != 401) {
                    // 401 is expected for unpaired PCs when we fetch serverinfo over HTTPS
                    qWarning() << "Request failed:" << statusCode << statusMessage;
                }
                if (statusCode == -1 && statusMessage == "Invalid") {
                    // Special case handling an audio capture error which GFE doesn't
                    // provide any useful status message for.
                    statusCode = 418;
                    statusMessage = tr("Missing audio capture device. Reinstalling GeForce Experience should resolve this error.");
                }
                throw GfeHttpResponseException(statusCode, statusMessage);
            }
        }
    }

    throw GfeHttpResponseException(-1, "Malformed XML (missing root element)");
}

QImage
NvHTTP::getBoxArt(int appId)
{
    QNetworkReply* reply = openConnection(m_BaseUrlHttps,
                                          "appasset",
                                          "appid="+QString::number(appId)+
                                          "&AssetType=2&AssetIdx=0",
                                          REQUEST_TIMEOUT_MS,
                                          NvLogLevel::NVLL_VERBOSE);
    QImage image = QImageReader(reply).read();
    delete reply;

    return image;
}

QByteArray
NvHTTP::getXmlStringFromHex(QString xml,
                            QString tagName)
{
    return QByteArray::fromHex(getXmlString(xml, tagName).toUtf8());
}

QString
NvHTTP::getXmlString(QString xml,
                     QString tagName)
{
    QXmlStreamReader xmlReader(xml);

    while (!xmlReader.atEnd())
    {
        if (xmlReader.readNext() != QXmlStreamReader::StartElement)
        {
            continue;
        }

        if (xmlReader.name() == tagName)
        {
            return xmlReader.readElementText();
        }
    }

    return QString();
}

void NvHTTP::handleSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
    bool ignoreErrors = true;

    if (m_ServerCert.isNull()) {
        // We should never make an HTTPS request without a cert
        Q_ASSERT(!m_ServerCert.isNull());
        return;
    }

    for (const QSslError& error : errors) {
        if (m_ServerCert != error.certificate()) {
            ignoreErrors = false;
            break;
        }
    }

    if (ignoreErrors) {
        reply->ignoreSslErrors(errors);
    }
}

QString
NvHTTP::openConnectionToString(QUrl baseUrl,
                               QString command,
                               QString arguments,
                               int timeoutMs,
                               NvLogLevel logLevel)
{
    QNetworkReply* reply = openConnection(baseUrl, command, arguments, timeoutMs, logLevel);
    QString ret;

    QTextStream stream(reply);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    ret = stream.readAll();
    delete reply;

    return ret;
}

QNetworkReply*
NvHTTP::openConnection(QUrl baseUrl,
                       QString command,
                       QString arguments,
                       int timeoutMs,
                       NvLogLevel logLevel)
{
    // Port must be set
    Q_ASSERT(baseUrl.port(0) != 0);

    // Build a URL for the request
    QUrl url(baseUrl);
    url.setPath("/" + command);

    // Use a placeholder UID for GFE allow them to quit games for each other.
    url.setQuery("uniqueid=" + (m_UseTrueUid ? IdentityManager::get()->getUniqueId() : "0123456789ABCDEF") +
                 "&uuid=" + QUuid::createUuid().toRfc4122().toHex() +
                 ((arguments != nullptr) ? ("&" + arguments) : ""));

    QNetworkRequest request(url);

    // Add our client certificate
    request.setSslConfiguration(IdentityManager::get()->getSslConfig());

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Disable HTTP/2 (GFE 3.22 doesn't like it) and Qt 6 enables it by default
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    // Use fine-grained idle timeouts to avoid calling QNetworkAccessManager::clearAccessCache(),
    // which tears down the NAM's global thread each time. We must not keep persistent connections
    // or GFE will puke.
    request.setAttribute(QNetworkRequest::ConnectionCacheExpiryTimeoutSecondsAttribute, 0);
#endif

    auto sslErrorsConnection = connect(m_Nam, &QNetworkAccessManager::sslErrors, this, &NvHTTP::handleSslErrors);
    QNetworkReply* reply = m_Nam->get(request);

    // Run the request with a timeout if requested
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &loop, &QEventLoop::quit);
    if (timeoutMs) {
        QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
    }
    if (logLevel >= NvLogLevel::NVLL_VERBOSE) {
        qInfo() << "Executing request:" << url.toString();
    }
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    // Abort the request if it timed out
    if (!reply->isFinished())
    {
        if (logLevel >= NvLogLevel::NVLL_ERROR) {
            qWarning() << "Aborting timed out request for" << url.toString();
        }
        reply->abort();
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
    // If we couldn't use fine-grained connection idle timeouts, kill them all now
    m_Nam->clearAccessCache();
#endif
    disconnect(sslErrorsConnection);

    // Handle error
    if (reply->error() != QNetworkReply::NoError)
    {
        if (logLevel >= NvLogLevel::NVLL_ERROR) {
            qWarning() << command << "request failed with error:" << reply->error();
        }

        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            // This will trigger falling back to HTTP for the serverinfo query
            // then pairing again to get the updated certificate.
            GfeHttpResponseException exception(401, "Server certificate mismatch");
            delete reply;
            throw exception;
        }
        else if (reply->error() == QNetworkReply::OperationCanceledError) {
            QtNetworkReplyException exception(QNetworkReply::TimeoutError, "Request timed out");
            delete reply;
            throw exception;
        }
        else {
            QtNetworkReplyException exception(reply->error(), reply->errorString());
            delete reply;
            throw exception;
        }
    }

    return reply;
}
