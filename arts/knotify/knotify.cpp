/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
   Copyright (c) 2000 Charles Samuels (charles@kde.org)
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <unistd.h>
#include <signal.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <soundserver.h>
#include <dispatcher.h>

#include "knotify.h"
#include "knotify.moc"

#include <qfile.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qiomanager.h>
#include <iostream.h>
#include <qtextstream.h>

/*
 * This is global because it needs to be initialized *before* the KNotify
 * DCOP object will accept requests (otherwise there may be reentrancy issues
 * and race conditions
 */

Arts::SimpleSoundServer server;

bool connectSoundServer()
{
	/*
	 * obtain an object reference to the soundserver - retry sometimes, so
	 * it will work during the startup sequence, even if artsd is started
	 * some time after the first process requests knotify to do some
	 * notifications
	 */
	for(int tries = 0; tries < 10; tries++)
	{
		server = Arts::Reference("global:Arts_SimpleSoundServer");
		if(!server.isNull()) return true;
		sleep(1);
	}

	return false;
}

int main(int argc, char **argv)
{
	KAboutData aboutdata("knotify", I18N_NOOP("KNotify"),
	                     "2.0pre", I18N_NOOP("KDE Notification Server"),
	                     KAboutData::License_GPL, "(C) 1997-2000, KDE Developers");
	aboutdata.addAuthor("Christian Esken",0,"esken@kde.org");
	aboutdata.addAuthor("Stefan Westerfeld",I18N_NOOP("Sound support"),"stefan@space.twc.de");
	aboutdata.addAuthor("Charles Samuels",I18N_NOOP("Current Maintainer"),"charles@kde.org");
	
	KCmdLineArgs::init( argc, argv, &aboutdata );
//	KCmdLineArgs::addCmdLineOptions( options );
	KUniqueApplication::addCmdLineOptions();
	
	if (!KUniqueApplication::start())
		exit(0);
	
	KUniqueApplication app;
	
	
	// setup mcop communication
	Arts::QIOManager qiomanager;
	Arts::Dispatcher dispatcher(&qiomanager);

	if(!connectSoundServer())
		cerr << "artsd is not running, there will be no sound notifications.\n";

	(void) new KNotify;

//	signal(SIGCLD, SIG_IGN); // don't wait() for fork
	return app.exec();
}

KNotify::KNotify() : QObject(), DCOPObject("Notify")
{
}

void KNotify::notify(const QString &event, const QString &fromApp,
                     const QString &text, QString sound, QString file,
                     int present, int level)
{
	static bool eventRunning=true;
	
	if (event.length())
	{
		KConfig *eventsfile;
		if (isGlobal(event))
			eventsfile=new KConfig(locate("config", "eventsrc"), true, false);
		else
			eventsfile=new KConfig(locate("data", fromApp+"/eventsrc"),true,false);
			
		eventsfile->setGroup(event);
	
		if (present==-1)
			present=eventsfile->readNumEntry("presentation", -1);
		if (present==-1)
			present=eventsfile->readNumEntry("default_presentation", 0);
		
		sound=eventsfile->readEntry("soundfile", 0);
		if (sound.isNull())
			sound=eventsfile->readEntry("default_sound", "");
			
		file=eventsfile->readEntry("logfile", 0);
		if (file.isNull())
			file=eventsfile->readEntry("default_logfile", "");
		
		level=eventsfile->readNumEntry("level", 0);
			
		delete eventsfile;
	}
	
	eventRunning=true;
	if ((present & KNotifyClient::Sound)/* && (QFile(sound).isReadable())*/)
		notifyBySound(sound);
	if (present & KNotifyClient::Messagebox)
		notifyByMessagebox(text,level);
	if (present & KNotifyClient::Logfile/* && (QFile(file).isWritable())*/)
		notifyByLogfile(text, file);
	if (present & KNotifyClient::Stderr)
		notifyByStderr(text);
	eventRunning=false;
}

bool KNotify::notifyBySound(const QString &sound)
{
	QString f(sound);
	if (QFileInfo(sound).isRelative())
		f=locate("sound", sound);

	kdDebug() << "KNotify::notifyBySound - Trying to play file " << sound << endl;
	if(!server.isNull()) server.play(QFile::encodeName(f).data());
	
	return true;
}

bool KNotify::notifyByMessagebox(const QString &text, int level)
{
	if ( text.isEmpty() )
		return false;
		
	
//	if (!fork())
	{
		switch(level)
		{
		default:
		case(KNotifyClient::Notification):
			KMessageBox::information(0, text, i18n("Notification"), 0, false);
			break;
		case(KNotifyClient::Warning):
			KMessageBox::sorry(0, text, i18n("Warning"), false);
			break;
		case(KNotifyClient::Error):
			KMessageBox::error(0, text, i18n("Error"), false);
			break;
		case(KNotifyClient::Catastrophe):
			KMessageBox::error(0, text, i18n("Catastrophe!"), false);
		};
//		_exit(0);
	}
	
	return true;
}

bool KNotify::notifyByLogfile(const QString &text, const QString &file)
{
	QFile f(file);
	if (!f.open(IO_WriteOnly | IO_Append)) return false;
	QTextStream t(&f);

	t<< "=======================================\n";
	t<< "KNotify: " << QDateTime::currentDateTime().toString() << '\n';
	t<< text<< "\n\n";
	f.close();
	return true;
}

bool KNotify::notifyByStderr(const QString &text)
{
	QTextStream t(stderr, IO_WriteOnly);

	t<< "KNotify: " << QDateTime::currentDateTime().toString() << '\n';
	t<< text<< "\n\n";
	return true;
}

bool KNotify::isGlobal(const QString &eventname)
{
	KConfig c(locate("config", "eventsrc"), true, false);
	return c.hasGroup(eventname);
}
