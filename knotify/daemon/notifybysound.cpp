/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Allan Sandfeld Jensen <kde@carewolf.com>
                 2005-2006 by Olivier Goffart <ogoffart at kde.org>

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "notifybysound.h"
#include "knotifyconfig.h"

#include <kdebug.h>



// C headers
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// QT headers
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qhash.h>

// KDE headers
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kmacroexpander.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <kwin.h>
#include <kurl.h>

#include <config.h>

// #include <kdemm/factory.h>
#if defined(HAVE_AKODE)
#include <string>
#include <akode/player.h>
#endif

class SoundManager;

class NotifyBySound::Private 
{
	public:
		QString externalPlayer;
		KProcess *externalPlayerProc;

		int externalPlayerEventId;

#if defined(HAVE_AKODE)
		aKode::Player player;
		QString sink;
		SoundManager *manager;
		int akodePlayerEventId;
#endif
		bool useExternal;
		bool useKDEMM;
		int volume;
		QTimer *playTimer;
		QString startupEvents;
};

#if defined(HAVE_AKODE)
class SoundManager : public aKode::Player::Manager {
	NotifyBySound *d;
	public:
		SoundManager(NotifyBySound* p) : d(p) {};
		virtual ~SoundManager(){}
		void eofEvent() {
			QApplication::postEvent( d, new QCustomEvent( 3001 ) );
		}
		void errorEvent() {
			QApplication::postEvent( d, new QCustomEvent( 3002 ) );
		}
};
#endif


NotifyBySound::NotifyBySound(QObject *parent) : KNotifyPlugin(parent),d(new Private)
{
	d->externalPlayerProc = 0;
#if defined(HAVE_AKODE)
	d->manager = 0;
#endif
	d->useKDEMM = true;
	d->playTimer = 0;
	loadConfig();
}


NotifyBySound::~NotifyBySound()
{
#if defined(HAVE_AKODE)
    d->player.close();
    delete d->manager;
#endif
	delete d->externalPlayerProc;
	delete d;
}


void NotifyBySound::loadConfig() 
{
    // load external player settings
	KConfig *kc = KGlobal::config();
	kc->setGroup("Misc");
	d->useExternal = kc->readEntry( "Use external player", false );
	d->externalPlayer = kc->readPathEntry("External player");

	// try to locate a suitable player if none is configured
	if ( d->externalPlayer.isEmpty() ) {
		QStringList players;
		players << "wavplay" << "aplay" << "auplay" << "artsplay" << "akodeplay";
		QStringList::Iterator it = players.begin();
		while ( d->externalPlayer.isEmpty() && it != players.end() ) {
			d->externalPlayer = KStandardDirs::findExe( *it );
			++it;
		}
	}
#if defined(HAVE_AKODE)
    else
	{
		d->manager = new SoundManager(this);
		d->player.setManager(d->manager);
		// try to open suitable sink
		d->sink = kc->readPathEntry("aKode sink", "auto");
		d->useKDEMM = d->player.open(d->sink.toAscii().data());
	}
#endif
	// load default volume
	d->volume = kc->readEntry( "Volume", 100 );
}




void NotifyBySound::notify( int eventId, KNotifyConfig * config )
{
	kDebug() << k_funcinfo << endl;
	
	QString soundFile = config->readEntry( "sound" , true );
	if (soundFile.isEmpty())
	{
		finish( eventId );
		return;
	}

	bool external = d->useExternal && !d->externalPlayer.isEmpty();
    // get file name
	if ( QFileInfo(soundFile).isRelative() )
	{
		QString search = QString("%1/sounds/%2").arg(config->appname).arg(soundFile);
		search = KGlobal::instance()->dirs()->findResource("data", search);
		if ( search.isEmpty() )
			soundFile = locate( "sound", soundFile );
		else
			soundFile = search;
	}
	if ( soundFile.isEmpty() )
	{
		finish( eventId );
		return;
	}

//     kDebug() << "KNotify::notifyBySound - trying to play file " << soundFile << endl;

	if (!external)
	{
        //If we disabled audio, just return,
		if (!d->useKDEMM)
		{
			finish( eventId );
			return;
		}

		KUrl soundURL;
		soundURL.setPath(soundFile);
#if defined(HAVE_AKODE)
		if (d->player.state() != aKode::Player::Open) 
		{
			finish( eventId );
			return;
		}

		if (d->player.load(soundFile.toLocal8Bit().data())) {
			d->player.play();
			d->akodePlayerEventId = eventId;
			return;
		}
#endif
        finish( eventId );
        return;
//	return KDE::Multimedia::Factory::self()->playSoundEvent(soundFile);

	} else if(!d->externalPlayer.isEmpty()) {
        // use an external player to play the sound
		KProcess *proc = d->externalPlayerProc;
		if (!proc)
		{
			proc = d->externalPlayerProc = new KProcess;
			connect( proc, SIGNAL( processExited( KProcess * )),
					 SLOT( slotPlayerProcessExited( KProcess * )));
		}
		if (proc->isRunning())
		{
			finish( eventId );
			return ; // Skip
		}
		proc->clearArguments();
		(*proc) << d->externalPlayer << QFile::encodeName( soundFile );
		d->externalPlayerEventId = eventId;
		proc->start(KProcess::NotifyOnExit);
		return;
	}

	finish( eventId );
}


void NotifyBySound::setVolume( int volume )
{
	if ( volume<0 ) volume=0;
	if ( volume>=100 ) volume=100;
	d->volume = volume;
}

void NotifyBySound::slotPlayerProcessExited( KProcess * )
{
	finish(  d->externalPlayerEventId );
}

bool NotifyBySound::event( QEvent *e )
{
#if defined(HAVE_AKODE)
	switch( e->type() )
	{
		case 3001: // eof
		case 3002: // error
			d->player.stop();
			d->player.unload();
			finish( d->akodePlayerEventId );
			return true;
		default:
			break;
	}
#endif
	return QObject::event(e);
}





















#include "notifybysound.moc"
