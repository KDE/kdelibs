/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmmanager.h"
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KCmdLineOptions options;
    options.add("kdeconfig", ki18n("Configure KDE Print"));
    options.add("serverconfig", ki18n("Configure print server"));

    KCmdLineArgs::init(argc, argv, "kaddprinterwizard", "kdelibs4", ki18n("KAddPrinterWizard"),
                       "0.1", ki18n("Start the add printer wizard"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool doConfig = args->isSet("kdeconfig");
    bool doSrvConfig = args->isSet("serverconfig");

    if (doConfig)
        KMManager::self()->invokeOptionsDialog();
    else if (doSrvConfig)
        KMManager::self()->configureServer();
    else if (KMManager::self()->addPrinterWizard() == -1) {
        KMessageBox::error(0, KMManager::self()->errorMsg().prepend("<qt>").append("</qt>"));
    }

    return 0;
}
