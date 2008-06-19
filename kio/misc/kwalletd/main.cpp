/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Michael Leupold <lemma@confuego.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kwalletd.h"

static bool isWalletEnabled()
{
	KConfig cfg("kwalletrc");
	KConfigGroup walletGroup(&cfg, "Wallet");
	return walletGroup.readEntry("Enabled", true);
}

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KAboutData aboutdata("kwalletd", "kdelibs4", ki18n("KDE Wallet Service"),
                         "0.1", ki18n("KDE Wallet Service"),
                         KAboutData::License_LGPL, ki18n("(C) 2002-2008 George Staikos, Michael Leupold, Thiago Maceira"));
    // STRING FREEZE!
    // aboutdata.addAuthor(ki18n("Michael Leupold"),ki18n("Maintainer"),"lemma@confuego.org");
    // aboutdata.addAuthor(ki18n("George Staikos"),ki18n("Former maintainer"),"staikos@kde.org");
    // aboutdata.addAuthor(ki18n("Thiago Maceira"),ki18n("DBus Interface"),"thiago@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    KUniqueApplication::addCmdLineOptions();
    KUniqueApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();
    app.setQuitOnLastWindowClosed( false );

    // check if kwallet is disabled
    if (!isWalletEnabled()) {
      kDebug(7024) << "kwalletd is disabled!";
      return (0);
    }

    if (!KUniqueApplication::start())
    {
      kDebug(7024) << "kwalletd is already running!";
      return (0);
    }

    KWalletD walletd;
    int res = app.exec();
    
    return res;
}
