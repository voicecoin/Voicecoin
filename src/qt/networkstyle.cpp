// Copyright (c) 2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "networkstyle.h"

#include "guiconstants.h"

#include <QApplication>

static const struct {
    const char *networkId;
    const char *appName;
    const char *appIcon;
    const char *titleAddText;
    const char *splashImage;
} network_styles[] = {
    {"main", QAPP_APP_NAME_DEFAULT, ":/icons/voicecoin", "", ":/images/splash"},
    {"test", QAPP_APP_NAME_TESTNET, ":/icons/voicecoin_testnet", QT_TRANSLATE_NOOP("SplashScreen", "[testnet]"), ":/images/splash_testnet"},
    {"regtest", QAPP_APP_NAME_TESTNET, ":/icons/voicecoin_testnet", "[regtest]", ":/images/splash_testnet"}
};
static const unsigned network_styles_count = sizeof(network_styles)/sizeof(*network_styles);

// titleAddText needs to be const char* for tr()
NetworkStyle::NetworkStyle(const QString &appName, const QString &appIcon, const char *titleAddText, const QString &splashImage):
    appName(appName),
    appIcon(appIcon),
    titleAddText(qApp->translate("SplashScreen", titleAddText)),
    splashImage(splashImage)
{
}

const NetworkStyle *NetworkStyle::instantiate(const QString &networkId)
{
    for (unsigned x=0; x<network_styles_count; ++x)
    {
        if (networkId == network_styles[x].networkId)
        {
            return new NetworkStyle(
                    network_styles[x].appName,
                    network_styles[x].appIcon,
                    network_styles[x].titleAddText,
                    network_styles[x].splashImage);
        }
    }
    return 0;
}
