/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>

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

// C headers
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// C++ headers
#include <string>

// aRts headers
#include <connect.h>
#include <dispatcher.h>
#include <flowsystem.h>
#include <soundserver.h>

// QT headers
#include <qfile.h>
#include <qfileinfo.h>
#include <qiomanager.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtextstream.h>

// KDE headers
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kcrash.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>

#include "knotify.h"
#include "knotify.moc"

class KNotifyPrivate
{
public:
    KConfig* globalEvents;
    KConfig* globalConfig;
    QMap<QString, KConfig*> events;
    QMap<QString, KConfig*> configs;
    QString externalPlayer;
    KProcess *externalPlayerProc;

    Arts::SoundServerV2 soundServer;
    Arts::PlayObjectFactory playObjectFactory;
    QValueList<Arts::PlayObject> playObjects;

    bool useExternal;
    int volume;
    QTimer *playTimer;
};


int main(int argc, char **argv)
{
    KAboutData aboutdata("knotify", I18N_NOOP("KNotify"),
                         "3.0", I18N_NOOP("KDE Notification Server"),
                         KAboutData::License_GPL, "(C) 1997-2002, KDE Developers");
    aboutdata.addAuthor("Christian Esken",0,"esken@kde.org");
    aboutdata.addAuthor("Stefan Westerfeld",I18N_NOOP("Sound support"),"stefan@space.twc.de");
    aboutdata.addAuthor("Charles Samuels",I18N_NOOP("Current Maintainer"),"charles@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    KUniqueApplication::addCmdLineOptions();


    // initialize application
    if ( !KUniqueApplication::start() ) {
        kdDebug() << "Running knotify found" << endl;
        return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();

    // setup mcop communication
    Arts::Dispatcher dispatcher;

    // start notify service
    KNotify notify;
    app.dcopClient()->setDefaultObject( "Notify" );
    app.dcopClient()->setDaemonMode( true );
    // kdDebug() << "knotify starting" << endl;

    return app.exec();
}


KNotify::KNotify()
    : QObject(), DCOPObject("Notify")
{
    d = new KNotifyPrivate;
    d->soundServer = Arts::SoundServerV2::null();
    d->globalEvents = new KConfig("knotify/eventsrc", true, false, "data");
    d->globalConfig = new KConfig("knotify.eventsrc", true, false);
    d->externalPlayerProc = 0;

    d->volume = 100;

    d->playTimer = 0;

    loadConfig();
}

KNotify::~KNotify()
{
    reconfigure();

    d->playObjects.clear();

    delete d->globalEvents;
    delete d->globalConfig;
    delete d->externalPlayerProc;
    delete d;
}


void KNotify::loadConfig() {
    // load external player settings
    KConfig *kc = kapp->config();
    kc->setGroup("Misc");
    d->useExternal = kc->readBoolEntry( "Use external player", false );
    d->externalPlayer = kc->readEntry("External player");

    // try to locate a suitable player if none is configured
    if ( d->externalPlayer.isEmpty() ) {
	QStringList players;
	players << "wavplay" << "aplay" << "auplay";
	QStringList::Iterator it = players.begin();
	while ( d->externalPlayer.isEmpty() && it != players.end() ) {
	    d->externalPlayer = KStandardDirs::findExe( *it );
	    ++it;
	}
    }

    // load default volume
    d->volume = kc->readNumEntry( "Volume", 100 );
}


void KNotify::reconfigure()
{
    kapp->config()->reparseConfiguration();
    loadConfig();

    // clear loaded config files
    d->globalConfig->reparseConfiguration();
    for ( QMapIterator<QString,KConfig*> it = d->configs.begin(); it != d->configs.end(); ++it )
        delete it.data();
    d->configs.clear();
}


void KNotify::notify(const QString &event, const QString &fromApp,
                     const QString &text, QString sound, QString file,
                     int present, int level)
{
    // kdDebug() << "event=" << event << " fromApp=" << fromApp << " text=" << text << " sound=" << sound <<
    //    " file=" << file << " present=" << present << " level=" << level << endl;

    QString commandline;

    // check for valid events
    if ( event.length()>0 )     {

        // get config file
        KConfig *eventsFile;
        KConfig *configFile;
	if ( d->events.contains( fromApp ) ) {
	    eventsFile = d->events[fromApp];
	} else {
	    eventsFile=new KConfig(locate("data", fromApp+"/eventsrc"),true,false);
	    d->events.insert( fromApp, eventsFile );
        }
        if ( d->configs.contains( fromApp) ) {
            configFile = d->configs[fromApp];
        } else {
	    configFile=new KConfig(fromApp+".eventsrc",true,false);
	    d->configs.insert( fromApp, configFile );
	}

	if ( !eventsFile->hasGroup( event ) && isGlobal(event) )
        {
	    eventsFile = d->globalEvents;
            configFile = d->globalConfig;
        }

        eventsFile->setGroup( event );
        configFile->setGroup( event );

        // get event presentation
        if ( present==-1 )
            present = configFile->readNumEntry( "presentation", -1 );
        if ( present==-1 )
            present = eventsFile->readNumEntry( "default_presentation", 0 );

        // get sound file name
        if( present & KNotifyClient::Sound ) {
            sound = configFile->readEntry( "soundfile" );
            if ( sound.length()==0 )
                sound = eventsFile->readEntry( "default_sound" );
        }

        // get log file name
        if( present & KNotifyClient::Logfile ) {
            file = configFile->readEntry( "logfile" );
            if ( file.length()==0 )
                file = eventsFile->readEntry( "default_logfile" );
        }

        // get default event level
        if( present & KNotifyClient::Messagebox )
            level = eventsFile->readNumEntry( "level", 0 );

	// get command line
	if (present & KNotifyClient::Execute ) {
	    commandline = configFile->readEntry( "commandline" );
	    if ( commandline.length()==0 )
		commandline = eventsFile->readEntry( "default_commandline" );
	}
    }

    // emit event
    if ( present & KNotifyClient::Sound ) // && QFile(sound).isReadable()
        notifyBySound( sound );

    if ( present & KNotifyClient::PassivePopup )
        notifyByPassivePopup( text, fromApp);

    else if ( present & KNotifyClient::Messagebox )
        notifyByMessagebox( text, level );

    if ( present & KNotifyClient::Logfile ) // && QFile(file).isWritable()
        notifyByLogfile( text, file );

    if ( present & KNotifyClient::Stderr )
        notifyByStderr( text );

    if ( present & KNotifyClient::Execute )
	notifyByExecute( commandline );
}


bool KNotify::notifyBySound( const QString &sound )
{
    if (sound.isEmpty()) {
	return false;
    }
    bool external = d->useExternal && !d->externalPlayer.isEmpty();
    // get file name
    QString soundFile(sound);
    if ( QFileInfo(sound).isRelative() )
	soundFile = locate( "sound", sound );
    if ( soundFile.isEmpty() )
        return false;

    // Oh dear! we seem to have lost our connection to artsd!
    if( !external && (d->soundServer.isNull() || d->soundServer.error()) )
        connectSoundServer();

    // kdDebug() << "KNotify::notifyBySound - trying to play file " << soundFile << endl;

    if (!external && !d->soundServer.isNull() && !d->soundServer.error()) {
        // play sound finally
        while( d->playObjects.count()>5 )
            d->playObjects.remove( d->playObjects.begin() );

        if (d->soundServer.audioDevice() == "null")
            return false;

        Arts::PlayObject player =
            d->playObjectFactory.createPlayObject( QFile::encodeName(soundFile).data() );
		if (player.isNull())
			return false;

        if ( d->volume != 100 )
        {
            Arts::Synth_BUS_UPLINK uplink = Arts::DynamicCast(player._getChild( "uplink" ));

            uplink.stop();
            player._node()->stop();
            Arts::disconnect( player, "left", uplink, "left" );
            Arts::disconnect( player, "right", uplink, "right" );

            Arts::StereoVolumeControl volumeControl = Arts::DynamicCast(d->soundServer.createObject("Arts::StereoVolumeControl"));
            player._addChild( volumeControl, "volume" );
            uplink.start();
            volumeControl.start();
            player._node()->start();

            Arts::connect(player,"left",volumeControl,"inleft");
            Arts::connect(player,"right",volumeControl,"inright");

            Arts::connect(volumeControl,"outleft",uplink,"left");
            Arts::connect(volumeControl,"outright",uplink,"right");

            volumeControl.scaleFactor( d->volume/100.0 );
        }

        player.play();
        d->playObjects.append( player );
        if ( !d->playTimer )
        {
            d->playTimer = new QTimer( this );
            connect( d->playTimer, SIGNAL( timeout() ), SLOT( playTimeout() ) );
        }
        if ( !d->playTimer->isActive() )
            d->playTimer->start( 1000 );

        return true;

    } else if(!d->externalPlayer.isEmpty()) {
        // use an external player to play the sound
        KProcess *proc = d->externalPlayerProc;
        if (!proc)
        {
           proc = d->externalPlayerProc = new KProcess;
        }
        if (proc->isRunning())
           return false; // Skip
        proc->clearArguments();
        (*proc) << d->externalPlayer << soundFile;
        proc->start(KProcess::DontCare);
        return true;
    }
    return false;
}

bool KNotify::notifyByMessagebox(const QString &text, int level)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return false;

    // display message box for specified event level
    switch( level ) {
    default:
    case KNotifyClient::Notification:
        KMessageBox::information( 0, text, i18n("Notification"), 0, false );
        break;
    case KNotifyClient::Warning:
        KMessageBox::sorry( 0, text, i18n("Warning"), false );
        break;
    case KNotifyClient::Error:
        KMessageBox::error( 0, text, i18n("Error"), false );
        break;
    case KNotifyClient::Catastrophe:
        KMessageBox::error( 0, text, i18n("Catastrophe!"), false );
        break;
    }

    return true;
}

bool KNotify::notifyByPassivePopup( const QString &text, const QString &appName )
{
    QCString senderId = kapp->dcopClient()->senderId();
    int senderWinId = 0;
    QCString compare = (appName + "-mainwindow").latin1();
    int len = compare.length();
    // kdDebug() << "notifyByPassivePopup: appName=" << appName << " sender=" << senderId << endl;
    QCStringList objs = kapp->dcopClient()->remoteObjects( senderId );
    for (QCStringList::ConstIterator it = objs.begin(); it != objs.end(); it++ ) {
        QCString obj( *it );
        if ( obj.left(len) == compare) {
            // kdDebug( ) << "found " << obj << endl;
            QCString replyType;
            QByteArray data, replyData;
            
            if ( kapp->dcopClient()->call(senderId, obj, "getWinID()", data, replyType, replyData) ) {
		QDataStream answer(replyData, IO_ReadOnly);
		if (replyType == "int") {
		    answer >> senderWinId;
		    // kdDebug() << "SUCCESS, found getWinID(): type='" << QString(replyType) 
                    //      << "' senderWinId=" << senderWinId << endl;
		}
            }
            
        }
    }
    if (senderWinId != 0) {
	KIconLoader iconLoader( appName );
	QPixmap icon = iconLoader.loadIcon( appName, KIcon::Desktop );
	KPassivePopup::message(appName, text, icon, senderWinId);
	return true;
    }
    return false;
}

bool KNotify::notifyByExecute(const QString &command) {
    if (!command.isEmpty()) {
	// kdDebug() << "executing command '" << command << "'" << endl;
	KProcess p;
	p.setUseShell(true);
	p << command;
	p.start(KProcess::DontCare);
	return true;
    }
    return false;
}


bool KNotify::notifyByLogfile(const QString &text, const QString &file)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;
    
    // open file in append mode
    QFile logFile(file);
    if ( !logFile.open(IO_WriteOnly | IO_Append) ) 
        return false;

    // append msg
    QTextStream strm( &logFile );
    strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    // close file
    logFile.close();
    return true;
}

bool KNotify::notifyByStderr(const QString &text)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;

    // open stderr for output
    QTextStream strm( stderr, IO_WriteOnly );

    // output msg
    strm << "KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    return true;
}


bool KNotify::isGlobal(const QString &eventname)
{
    return d->globalEvents->hasGroup( eventname );
}


void KNotify::connectSoundServer()
{
    static bool firstTime = true;

    /*
     * obtain an object reference to the soundserver - if we're doing it
     * for the first time, retry sometimes, so it will work during the
     * startup sequence, even if artsd is started some time after the first
     * process requests knotify to do some notifications
     */
    Arts::SoundServerV2 result;
    d->soundServer = Arts::Reference("global:Arts_SoundServerV2");
    if ( firstTime && d->soundServer.isNull() )
        for( int tries=0; tries<7; tries++ )
        {
            sleep( 1 );
            d->soundServer = Arts::Reference("global:Arts_SoundServerV2");
            if( !d->soundServer.isNull() ) break;
        }


    if( !d->soundServer.isNull() )
        // create playObjectFactory
        d->playObjectFactory=Arts::Reference("global:Arts_PlayObjectFactory");

    firstTime = false;
}


void KNotify::setVolume( int volume )
{
    if ( volume<0 ) volume=0;
    if ( volume>=100 ) volume=100;
    d->volume = volume;
}

void KNotify::playTimeout()
{
    for ( QValueList< Arts::PlayObject >::Iterator it = d->playObjects.begin();
          it != d->playObjects.end(); )
    {
        QValueList< Arts::PlayObject >::Iterator current = it++;
        if ( (*current).state() != Arts::posPlaying )
            d->playObjects.remove( current );
    }
    if ( !d->playObjects.count() )
        d->playTimer->stop();
}
