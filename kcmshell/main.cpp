/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <iostream>
#include <stdlib.h>

#include <qfile.h>

#include <dcopclient.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdesktopfile.h>
#include <qxembed.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kaboutdata.h>
#include <kwin.h>

#include <kcmultidialog.h>
#include <kcmoduleinfo.h>
#include <kcmoduleloader.h>

#include "kcdialog.h"
#include "global.h"
#include "kcmshell.h"
#include "proxywidget.h"
#include "modules.h"

#include "version.h"

using namespace std;

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List all possible modules"), 0},
    { "+module", I18N_NOOP("Configuration module to open."), 0 },
    { "lang <language>", I18N_NOOP("Specify a particular language."), 0 },
    { "embed <id>", I18N_NOOP("Window ID to embed into."), 0 },
    { "silent", I18N_NOOP("Do not display main window."), 0 },
    KCmdLineLastOption
};

static KService::Ptr locateModule(const QCString& module)
{
    QString path = QFile::decodeName(module);

    if (!path.endsWith(".desktop"))
        path += ".desktop";

    KService::Ptr service = KService::serviceByStorageId( path );
    if (!service)
    {
        kdWarning() << "Could not find module '" << module << "'." << endl;
        return 0;
    }

    if (!kapp->authorizeControlModule(service->menuId()))
        return 0;
        
    return service;
}

void
kcmApplication::setDCOPName(const QCString &dcopName)
{
    m_dcopName = "kcmshell_"+dcopName;
    dcopClient()->registerAs(m_dcopName, false);
}

bool
kcmApplication::isRunning()
{
    if (dcopClient()->appId() == m_dcopName)
       return false; // We are the one and only.
    dcopClient()->attach(); // Reregister as anonymous

    dcopClient()->setNotifications(true);

    QByteArray data;
    QDataStream str( data, IO_WriteOnly );
    str << kapp->startupId();
    QCString replyType;
    QByteArray replyData;
    if (!dcopClient()->call(m_dcopName, "dialog", "activate(QCString)", data,
		replyType, replyData))
    {
        return false; // Error, we have to do it ourselves.
    }
    return true;
}

void
kcmApplication::waitForExit()
{
    connect(dcopClient(), SIGNAL(applicationRemoved(const QCString&)),
            this, SLOT(slotAppExit(const QCString&)));
    exec();
}

void
kcmApplication::slotAppExit(const QCString &appId)
{
    if (appId == m_dcopName)
        deref();
}

static const QString stripPath(const QString& path)
{
    int pathEnd = path.findRev('/')+1;
    int dot = path.findRev('.');
    return path.mid(pathEnd, dot-pathEnd);
}

static void setIcon(QWidget *w, const QString &iconName)
{
    QPixmap icon = DesktopIcon(iconName);
    QPixmap miniIcon = SmallIcon(iconName);
    w->setIcon( icon ); //standard X11
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    KWin::setIcons(w->winId(), icon, miniIcon );
#endif
}

extern "C" int kdemain(int _argc, char *_argv[])
{
    KAboutData aboutData( "kcmshell", I18N_NOOP("KDE Control Module"),
                          KCONTROL_VERSION,
                          I18N_NOOP("A tool to start single KDE control modules"),
                          KAboutData::License_GPL,
                          "(c) 1999-2002, The KDE Developers");

    aboutData.addAuthor("Daniel Molkentin", I18N_NOOP("Current Maintainer"), "molkentin@kde.org");
    aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");

    KCmdLineArgs::init(_argc, _argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KLocale::setMainCatalogue("kcontrol");
    kcmApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KGlobal::iconLoader()->addAppDir( "kcontrol" );

    KGlobal::locale()->setLanguage(args->getOption("lang"));

    if (args->isSet("list")) {

        cout << i18n("The following modules are available:").local8Bit() << endl;

        ConfigModuleList modules;
        modules.readDesktopEntries();
        ConfigModule *module = 0;

        int maxLen=0;

        for (module=modules.first(); module != 0; module=modules.next())
        {
            int len = stripPath(module->fileName()).length();
            if (len > maxLen) maxLen = len;
        }

        for (module=modules.first(); module != 0; module=modules.next())
        {

            QString entry("%1 - %2");

            entry = entry.arg(stripPath(module->fileName()).leftJustify(maxLen, ' '));
            entry = entry.arg(!module->comment().isEmpty() ? 
                              module->comment() : i18n("No description available"));

            cout << entry.local8Bit() << endl;
        }


        return 0;
    }

    if (args->count() < 1) {
        args->usage();
        return -1;
    }

    if (args->count() == 1) {
        app.setDCOPName(args->arg(0));
        if (app.isRunning())
        {
           app.waitForExit();
           return 0;
        }

        KService::Ptr service = locateModule(args->arg(0));
        if (!service)
           return 1; // error

        // load the module
        KCModuleInfo info(service);

        KCModule *module = KCModuleLoader::loadModule(info, false);

        if (module) {
            // create the dialog
            QCString embedStr = args->getOption("embed");
            bool embed = false;
            int id = -1;
            if (!embedStr.isEmpty())
               id = embedStr.toInt(&embed);
            if (!args->isSet("silent")) {
             if (!embed)
             {
                KCDialog * dlg = new KCDialog(module, module->buttons(), info.docPath(), 0, 0, true );
                QString caption = (kapp->caption() != i18n("KDE Control Module")) ?
                                   kapp->caption() : info.moduleName();
                if (kapp->iconName() != kapp->name())
                    setIcon(dlg, kapp->iconName());
                else
                    setIcon(dlg, info.icon());
                dlg->setPlainCaption(i18n("Configure - %1").arg(caption));

                // Needed for modules that use d'n'd (not really the right
                // solution for this though, I guess)
                dlg->setAcceptDrops(true);

                // run the dialog
                dlg->exec();
                delete dlg;
             }
             // if we are going to be embedded, embed
             else
             {
                QWidget *dlg = new ProxyWidget(module, info.moduleName(), "kcmshell", false);
                // Needed for modules that use d'n'd (not really the right
                // solution for this though, I guess)
                dlg->setAcceptDrops(true);

                QXEmbed::embedClientIntoWindow(dlg, id);
                kapp->exec();
                delete dlg;
             }
            } else {
             //Silent
             kapp->exec();
            }
            KCModuleLoader::unloadModule(info);
            return 0;
        }

       KCModuleLoader::showLastLoaderError(0L);
       return 0;
    }

    // multiple control modules
    KService::List modules;
    for (int i = 0; i < args->count(); i++) {
        KService::Ptr service = locateModule(args->arg(i));
        if (service)
	{
		modules.append(service);
		continue;
	}
    }

    if (modules.count() < 1) return -1;

    // create the dialog
    KCMultiDialog * dlg = new KCMultiDialog(0, 0, true);

    // Needed for modules that use d'n'd (not really the right
    // solution for this though, I guess)
    dlg->setAcceptDrops(true);

    // add modules
    for (KService::List::ConstIterator it = modules.begin(); it != modules.end(); ++it)
        dlg->addModule(KCModuleInfo(*it));

    // if we are going to be embedded, embed
    QCString embed = args->getOption("embed");
    if (!embed.isEmpty())
    {
        bool ok;
        int id = embed.toInt(&ok);
        if (ok)
        {
            // NOTE: This has to be changed for QT 3.0. See above!
            QXEmbed::embedClientIntoWindow(dlg, id);
            delete dlg;
          return 0;
        }
    }

    if (kapp->iconName() != kapp->name())
        setIcon(dlg, kapp->iconName());
    dlg->setPlainCaption(i18n("Configure - %1").arg(kapp->caption()));

    // run the dialog
    dlg->exec();
    delete dlg;
    return 0;
}
#include "kcmshell.moc"
