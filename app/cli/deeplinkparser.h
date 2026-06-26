#pragma once

#include <QStringList>

// Translates a Hestia/Moonlight URI-scheme deeplink into the equivalent
// command-line argument list that the existing CLI parsers understand.
//
// Supported schemes: "hestia://" and "moonlight://". The URL host is treated
// as the action and query parameters map to CLI options, e.g.
//
//   hestia://pair?host=192.168.1.5&pin=1234&passphrase=secret
//     -> moonlight pair 192.168.1.5 --pin 1234 --passphrase secret
//   hestia://stream?host=192.168.1.5&app=Desktop
//     -> moonlight stream 192.168.1.5 Desktop
//   hestia://quit?host=192.168.1.5
//     -> moonlight quit 192.168.1.5
//
// If the supplied argument list does not contain a recognized deeplink, the
// original list is returned unchanged.
class DeepLinkParser
{
public:
    // Returns the schemes this build registers as URL handlers.
    static QStringList supportedSchemes();

    // Returns true if the given string looks like a deeplink we handle.
    static bool isDeepLink(const QString& arg);

    // Rewrites 'args' so that a leading deeplink argument becomes a normal CLI
    // invocation. The first element (program name) is always preserved.
    static QStringList rewriteArguments(const QStringList& args);
};
