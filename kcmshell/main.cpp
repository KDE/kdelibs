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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <iostream>

#include <qfile.h> 
#include <qcstring.h>

#include <dcopclient.h>

#include <qxembed.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <kcmoduleinfo.h>
#include <kcmultidialog.h>
#include <kservicegroup.h>
#include <klocale.h>
#include <kwin.h>

#include "main.h"
#include "main.moc"

using namespace std;

KService::List m_modules;

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List all possible modules"), 0},
    { "+module", I18N_NOOP("Configuration module to open."), 0 },
    { "lang <language>", I18N_NOOP("Specify a particular language."), 0 },
    { "embed <id>", I18N_NOOP("Window ID to embed into."), 0 },
    { "silent", I18N_NOOP("Do not display main window."), 0 },
    KCmdLineLastOption
};

static void listModules(const QString &baseGroup)
{

  KServiceGroup::Ptr group = KServiceGroup::group(baseGroup);

  if (!group || !group->isValid()) return;

  KServiceGroup::List list = group->entries(true, true);

  for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
  {
     KSycocaEntry *p = (*it);
     if (p->isType(KST_KService))
     {
        KService *s = static_cast<KService*>(p);
        if (!kapp->authorizeControlModule(s->menuId()))
           continue;
        m_modules.append(s);
     }
     else if (p->isType(KST_KServiceGroup))
     {
        listModules(p->entryPath());
     }
  }
}

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
                          0,
                          I18N_NOOP("A tool to start single KDE control modules"),
                          KAboutData::License_GPL,
                          "(c) 1999-2004, The KDE Developers");

    aboutData.addAuthor("Frans Englich", "Maintainer", "frans.englich@kde.org");
    aboutData.addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
    aboutData.addAuthor("Matthias Hoelzer-Kluepfel",0, "hoelzer@kde.org");
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Waldo Bastian",0, "bastian@kde.org");

    KCmdLineArgs::init(_argc, _argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    kcmApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KGlobal::locale()->setLanguage(args->getOption("lang"));

    if (args->isSet("list")) {
        cout << i18n("The following modules are available:").local8Bit() << endl;

        listModules( "Settings/" );

        int maxLen=0;

        for( KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); it++)
        {
            int len = stripPath((*it)->name()).length();
            if (len > maxLen)
                maxLen = len;
        }

        for( KService::List::ConstIterator it = m_modules.begin(); it != m_modules.end(); it++)
        {
            QString entry("%1 - %2");

            entry = entry.arg(stripPath((*it)->name()).leftJustify(maxLen, ' '));
            entry = entry.arg(!(*it)->comment().isEmpty() ? 
                              (*it)->comment() : i18n("No description available"));

            cout << entry.local8Bit() << endl;
        }
        return 0;
    }

    if (args->count() < 1) {
        args->usage();
        return -1;
    }

    KService::List modules;
    for (int i = 0; i < args->count(); i++)
    {
        KService::Ptr service = locateModule(args->arg(i));
        if (service)
            modules.append(service);
    }

    KDialogBase::DialogType dtype = KDialogBase::Plain;

    if ( modules.count() == 1 )
    {
        app.setDCOPName(args->arg(0));
        if (app.isRunning())
        {
           app.waitForExit();
           return 0;
        }
    }
    else if ( modules.count() < 1 )
        return 0;
    else
        dtype = KDialogBase::IconList;

    KCMultiDialog* dlg = new KCMultiDialog( dtype, i18n("Configure - %1").arg(kapp->caption()), 0, "", true );
    dlg->setAcceptDrops( true );
    
    for (KService::List::ConstIterator it = modules.begin(); it != modules.end(); ++it)
        dlg->addModule(KCModuleInfo(*it));


    if (kapp->iconName() != kapp->name())
        setIcon(dlg, kapp->iconName());
    else if ( modules.count() == 1 )
        setIcon(dlg, KCModuleInfo( modules.first()).icon());

    /* Should we embed? */
    QCString embed;
    embed = args->getOption( "embed" );
    if ( !embed.isEmpty() ) /* Yes */
    {
        bool ok;
        int id = embed.toInt(&ok);    
        if( ok )
        {
            QXEmbed::embedClientIntoWindow(dlg, id);
        }
    }

    dlg->exec();
    delete dlg;
    return 0;
}
// vim: sw=4 et sts=4

