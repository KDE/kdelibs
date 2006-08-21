/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <iostream>

#include <qfile.h>
#include <qicon.h>

#ifdef Q_WS_X11
/*
    FIXME: how are we supposed to handle stuff like this that is so
    integrated with QX11Embed on alternate platforms?
*/
#include <QX11EmbedWidget>
#endif

#include <QVBoxLayout>

#include <QtDBus/QtDBus>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kcmdlineargs.h>
#include <kcmoduleinfo.h>
#include <kcmoduleloader.h>
#include <kcmoduleproxy.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kservicetypetrader.h>
#include <kstartupinfo.h>
#include <kwin.h>
#include <kglobal.h>

#include "main.h"
#include <kicon.h>
#include "main.moc"

using namespace std;

KService::List m_modules;

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List all possible modules"), 0},
    { "+module", I18N_NOOP("Configuration module to open"), 0 },
    { "lang <language>", I18N_NOOP("Specify a particular language"), 0 },
    { "silent", I18N_NOOP("Do not display main window"), 0 },
    KCmdLineLastOption
};


static void listModules()
{
  const KService::List services = KServiceTypeTrader::self()->query( "KCModule", "[X-KDE-ParentApp] == 'kcontrol' or [X-KDE-ParentApp] == 'kinfocenter'" );
  for( KService::List::const_iterator it = services.begin();
       it != services.end(); it++)
  {
      const KService::Ptr s = (*it);
      if (!KAuthorized::authorizeControlModule(s->menuId()))
          continue;
      m_modules.append(s);
  }
}

static KService::Ptr locateModule(const QByteArray& module)
{
    QString path = QFile::decodeName(module);

    if (!path.endsWith(".desktop"))
        path += ".desktop";

    KService::Ptr service = KService::serviceByStorageId( path );
    if (!service)
    {
        kWarning(780) << "Could not find module '" << module << "'." << endl;
        return KService::Ptr();
    }

    if ( service->noDisplay() )
    {
        kDebug(780) << module << " should not be loaded." << endl;
        return KService::Ptr();
    }

    return service;
}

bool KCMShell::isRunning()
{
    QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(m_serviceName);
    if( owner == QDBusConnection::sessionBus().baseService() )
        return false; // We are the one and only.

    kDebug(780) << "kcmshell with modules '" <<
        m_serviceName << "' is already running." << endl;

    QDBusInterface iface(m_serviceName, "/KCModule/dialog", "org.kde.KCMShellMultiDialog");
    QDBusReply<void> reply = iface.call("activate", kapp->startupId());
    if (!reply.isValid())
    {
        kDebug(780) << "Calling D-Bus function dialog::activate() failed." << endl;
        return false; // Error, we have to do it ourselves.
    }

    return true;
}

KCMShellMultiDialog::KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(dialogFace);
    setModal(true);

    QDBusConnection::sessionBus().registerObject("/KCModule/dialog", this, QDBusConnection::ExportScriptableSlots);
}

void KCMShellMultiDialog::activate( const QByteArray& asn_id )
{
    kDebug(780) << k_funcinfo << endl;

#ifdef Q_WS_X11
    KStartupInfo::setNewStartupId( this, asn_id );
#endif
}

void KCMShell::setServiceName(const QString &dbusName )
{
    m_serviceName = QLatin1String( "org.kde.kcmshell_" ) + dbusName;
    QDBusConnection::sessionBus().registerService(m_serviceName);
}

void KCMShell::waitForExit()
{
    kDebug(780) << k_funcinfo << endl;

    connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            SLOT(appExit(QString,QString,QString)));
    exec();
}

void KCMShell::appExit(const QString &appId, const QString &oldName, const QString &newName)
{
    Q_UNUSED(newName);
    kDebug(780) << k_funcinfo << endl;

    if( appId == m_serviceName && !oldName.isEmpty() )
    {
        kDebug(780) << "'" << appId << "' closed, dereferencing." << endl;
        KGlobal::deref();
    }
}

extern "C" KDE_EXPORT int kdemain(int _argc, char *_argv[])
{
    KAboutData aboutData( "kcmshell", I18N_NOOP("KDE Control Module"),
                          0,
                          I18N_NOOP("A tool to start single KDE control modules"),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 1999-2004, The KDE Developers") );

    aboutData.addAuthor("Frans Englich", I18N_NOOP("Maintainer"), "frans.englich@kde.org");
    aboutData.addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
    aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");

    KGlobal::locale()->setMainCatalog("kcmshell");

    KCmdLineArgs::init(_argc, _argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KCMShell app;

    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    const QByteArray lang = args->getOption("lang");
    if( !lang.isNull() )
        KGlobal::locale()->setLanguage(lang);

    if (args->isSet("list"))
    {
        cout << i18n("The following modules are available:").toLocal8Bit().data() << endl;

        listModules();

        int maxLen=0;

        for( KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            int len = (*it)->desktopEntryName().length();
            if (len > maxLen)
                maxLen = len;
        }

        for( KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            QString entry("%1 - %2");

            entry = entry.arg((*it)->desktopEntryName().leftJustified(maxLen, ' '))
                         .arg(!(*it)->comment().isEmpty() ? (*it)->comment()
                                 : i18n("No description available"));

            cout << entry.toLocal8Bit().data() << endl;
        }
        return 0;
    }

    if (args->count() < 1)
    {
        args->usage();
        return -1;
    }

    QString serviceName;
    KService::List modules;
    for (int i = 0; i < args->count(); i++)
    {
        KService::Ptr service = locateModule(args->arg(i));
        if( service )
        {
            modules.append(service);
            if( !serviceName.isEmpty() )
                serviceName += '_';

            serviceName += args->arg(i);
        }
    }

    /* Check if this particular module combination is already running */
    app.setServiceName(serviceName);
    if( app.isRunning() ) {
        app.waitForExit();
        return 0;
    }

    KPageDialog::FaceType ftype = KPageDialog::Plain;

    if (modules.count() < 1) {
        return 0;
    } else if (modules.count() > 1) {
        ftype = KPageDialog::List;
    }

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog(ftype);
    KCmdLineArgs *kdeargs = KCmdLineArgs::parsedArgs("kde");
    if (kdeargs && kdeargs->isSet("caption")) {
        dlg->setCaption(QString());
    } else if (modules.count() == 1) {
        dlg->setCaption(modules.first()->name());
    }

    for (KService::List::ConstIterator it = modules.begin(); it != modules.end(); ++it)
        dlg->addModule(*it);

    if ( !args->isSet( "icon" ) && modules.count() == 1)
    {
        QString iconName = KCModuleInfo(modules.first()).icon();
        dlg->setWindowIcon( KIcon(iconName) );
    }

    dlg->exec();
    delete dlg;

    return 0;
}
// vim: sw=4 et sts=4
