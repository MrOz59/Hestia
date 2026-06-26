#include "deeplinkparser.h"

#include <QUrl>
#include <QUrlQuery>

QStringList DeepLinkParser::supportedSchemes()
{
    return { "hestia", "moonlight" };
}

bool DeepLinkParser::isDeepLink(const QString& arg)
{
    for (const QString& scheme : supportedSchemes()) {
        if (arg.startsWith(scheme + "://", Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QStringList DeepLinkParser::rewriteArguments(const QStringList& args)
{
    if (args.isEmpty()) {
        return args;
    }

    // Find the first argument that is a deeplink. The OS typically passes it as
    // argv[1], but scan defensively in case other arguments precede it.
    int linkIndex = -1;
    for (int i = 1; i < args.size(); i++) {
        if (isDeepLink(args.at(i))) {
            linkIndex = i;
            break;
        }
    }

    if (linkIndex < 0) {
        return args;
    }

    const QUrl url(args.at(linkIndex));
    const QString action = url.host().toLower();
    const QUrlQuery query(url);

    // Always preserve the program name. We intentionally drop any other
    // arguments so a malicious or malformed deeplink cannot smuggle in extra
    // CLI options beyond what we translate below.
    QStringList rewritten;
    rewritten.append(args.first());

    const QString host = query.queryItemValue("host", QUrl::FullyDecoded);

    if (action == "pair") {
        rewritten.append("pair");
        if (!host.isEmpty()) {
            rewritten.append(host);
        }
        const QString pin = query.queryItemValue("pin", QUrl::FullyDecoded);
        if (!pin.isEmpty()) {
            rewritten.append("--pin");
            rewritten.append(pin);
        }
        const QString passphrase = query.queryItemValue("passphrase", QUrl::FullyDecoded);
        if (!passphrase.isEmpty()) {
            rewritten.append("--passphrase");
            rewritten.append(passphrase);
        }
    }
    else if (action == "stream") {
        rewritten.append("stream");
        if (!host.isEmpty()) {
            rewritten.append(host);
        }
        const QString appName = query.queryItemValue("app", QUrl::FullyDecoded);
        if (!appName.isEmpty()) {
            rewritten.append(appName);
        }
    }
    else if (action == "quit") {
        rewritten.append("quit");
        if (!host.isEmpty()) {
            rewritten.append(host);
        }
    }
    else {
        // Unknown action: fall back to a normal start rather than the deeplink,
        // which the CLI parser would otherwise reject as an invalid action.
        return { args.first() };
    }

    return rewritten;
}
