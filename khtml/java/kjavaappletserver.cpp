/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "kjavaappletserver.h"
#include "kjavaappletcontext.h"
#include "kjavaprocess.h"
#include "kjavadownloader.h"

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kparts/browserextension.h>
#include <kapplication.h>
#include <kstandarddirs.h>

#include <kio/job.h>
#include <kio/kprotocolmanager.h>

#include <qtimer.h>
#include <qguardedptr.h>
#include <qvaluelist.h>
#include <qdir.h>
#include <qeventloop.h>

#include <stdlib.h>
#include <assert.h>

#define KJAS_CREATE_CONTEXT    (char)1
#define KJAS_DESTROY_CONTEXT   (char)2
#define KJAS_CREATE_APPLET     (char)3
#define KJAS_DESTROY_APPLET    (char)4
#define KJAS_START_APPLET      (char)5
#define KJAS_STOP_APPLET       (char)6
#define KJAS_INIT_APPLET       (char)7
#define KJAS_SHOW_DOCUMENT     (char)8
#define KJAS_SHOW_URLINFRAME   (char)9
#define KJAS_SHOW_STATUS       (char)10
#define KJAS_RESIZE_APPLET     (char)11
#define KJAS_GET_URLDATA       (char)12
#define KJAS_URLDATA           (char)13
#define KJAS_SHUTDOWN_SERVER   (char)14
#define KJAS_JAVASCRIPT_EVENT   (char)15
#define KJAS_GET_MEMBER        (char)16
#define KJAS_CALL_MEMBER       (char)17
#define KJAS_PUT_MEMBER        (char)18
#define KJAS_DEREF_OBJECT      (char)19
#define KJAS_AUDIOCLIP_PLAY    (char)20
#define KJAS_AUDIOCLIP_LOOP    (char)21
#define KJAS_AUDIOCLIP_STOP    (char)22
#define KJAS_APPLET_STATE      (char)23
#define KJAS_APPLET_FAILED     (char)24
#define KJAS_DATA_COMMAND      (char)25
#define KJAS_PUT_URLDATA       (char)26
#define KJAS_PUT_DATA          (char)27


class JSStackFrame;

typedef QMap< int, KJavaKIOJob* > KIOJobMap;
typedef QMap< int, JSStackFrame* > JSStack;

class JSStackFrame {
public:
    JSStackFrame(JSStack & stack, QStringList & a)
    : jsstack(stack), args(a), ticket(counter++), ready(false), exit (false) {
        jsstack.insert( ticket, this );
    }
    ~JSStackFrame() {
        jsstack.erase( ticket );
    }
    JSStack & jsstack;
    QStringList & args;
    int ticket;
    bool ready : 1;
    bool exit : 1;
    static int counter;
};

int JSStackFrame::counter = 0;

class KJavaAppletServerPrivate
{
friend class KJavaAppletServer;
private:
   //KJavaAppletServerPrivate() : locked_context(-1) {}
   int counter;
   QMap< int, QGuardedPtr<KJavaAppletContext> > contexts;
   QString appletLabel;
   JSStack jsstack;
   KIOJobMap kiojobs;
   bool javaProcessFailed;
   bool useKIO;
   //int locked_context;
   //QValueList<QByteArray> java_requests;
};

static KJavaAppletServer* self = 0;

KJavaAppletServer::KJavaAppletServer()
{
    d = new KJavaAppletServerPrivate;
    process = new KJavaProcess();

    connect( process, SIGNAL(received(const QByteArray&)),
             this,    SLOT(slotJavaRequest(const QByteArray&)) );

    setupJava( process );

    if( process->startJava() ) {
        d->appletLabel = i18n( "Loading Applet" );
        d->javaProcessFailed = false;
    }
    else {
        d->appletLabel = i18n( "Error: java executable not found" );
        d->javaProcessFailed = true;
    }
}

KJavaAppletServer::~KJavaAppletServer()
{
    quit();

    delete process;
    delete d;
}

QString KJavaAppletServer::getAppletLabel()
{
    if( self )
        return self->appletLabel();
    else
        return QString::null;
}

QString KJavaAppletServer::appletLabel()
{
    return d->appletLabel;
}

KJavaAppletServer* KJavaAppletServer::allocateJavaServer()
{
   if( self == 0 )
   {
      self = new KJavaAppletServer();
      self->d->counter = 0;
   }

   self->d->counter++;
   return self;
}

void KJavaAppletServer::freeJavaServer()
{
    self->d->counter--;

    if( self->d->counter == 0 )
    {
        //instead of immediately quitting here, set a timer to kill us
        //if there are still no servers- give us one minute
        //this is to prevent repeated loading and unloading of the jvm
        KConfig config( "konquerorrc", true );
        config.setGroup( "Java/JavaScript Settings" );
        if( config.readBoolEntry( "ShutdownAppletServer", true )  )
        {
            int value = config.readNumEntry( "AppletServerTimeout", 60 );
            QTimer::singleShot( value*1000, self, SLOT( checkShutdown() ) );
        }
    }
}

void KJavaAppletServer::checkShutdown()
{
    if( self->d->counter == 0 )
    {
        delete self;
        self = 0;
    }
}

void KJavaAppletServer::setupJava( KJavaProcess *p )
{
    KConfig config ( "konquerorrc", true );
    config.setGroup( "Java/JavaScript Settings" );

    QString jvm_path = "java";

    QString jPath = config.readPathEntry( "JavaPath" );
    if ( !jPath.isEmpty() && jPath != "java" )
    {
        // Cut off trailing slash if any
        if( jPath[jPath.length()-1] == '/' )
            jPath.remove(jPath.length()-1, 1);
            
        QDir dir( jPath );
        if( dir.exists( "bin/java" ) )
        {
            jvm_path = jPath + "/bin/java";
        } 
        else if (dir.exists( "/jre/bin/java" ) )
        { 
            jvm_path = jPath + "/jre/bin/java";
        } 
        else if( QFile::exists(jPath) )
        {
            //check here to see if they entered the whole path the java exe
            jvm_path = jPath;
        }
    }

    //check to see if jvm_path is valid and set d->appletLabel accordingly
    p->setJVMPath( jvm_path );

    // Prepare classpath variable
    QString kjava_class = locate("data", "kjava/kjava.jar");
    kdDebug(6100) << "kjava_class = " << kjava_class << endl;
    if( kjava_class.isNull() ) // Should not happen
        return;

    QDir dir( kjava_class );
    dir.cdUp();
    kdDebug(6100) << "dir = " << dir.absPath() << endl;

    QStringList entries = dir.entryList( "*.jar" );
    kdDebug(6100) << "entries = " << entries.join( ":" ) << endl;

    QString classes;
    for( QStringList::Iterator it = entries.begin();
         it != entries.end(); it++ )
    {
        if( !classes.isEmpty() )
            classes += ":";
        classes += dir.absFilePath( *it );
    }
    p->setClasspath( classes );

    // Fix all the extra arguments
    QString extraArgs = config.readEntry( "JavaArgs" );
    p->setExtraArgs( extraArgs );

    if( config.readBoolEntry( "ShowJavaConsole", false) )
    {
        p->setSystemProperty( "kjas.showConsole", QString::null );
    }

    if( config.readBoolEntry( "UseSecurityManager", true ) )
    {
        QString class_file = locate( "data", "kjava/kjava.policy" );
        p->setSystemProperty( "java.security.policy", class_file );

        p->setSystemProperty( "java.security.manager",
                              "org.kde.kjas.server.KJASSecurityManager" );
    }

    d->useKIO = config.readBoolEntry( "UseKio", false);
    if( d->useKIO )
    {
        p->setSystemProperty( "kjas.useKio", QString::null );
    }

    //check for http proxies...
    if( KProtocolManager::useProxy() )
    {
        // only proxyForURL honors automatic proxy scripts
        // we do not know the applet url here so we just use a dummy url
        // this is a workaround for now
        // FIXME
        KURL dummyURL( "http://www.kde.org/" );
        QString httpProxy = KProtocolManager::proxyForURL(dummyURL);
        kdDebug(6100) << "httpProxy is " << httpProxy << endl;

        KURL url( httpProxy );
        p->setSystemProperty( "http.proxyHost", url.host() );
        p->setSystemProperty( "http.proxyPort", QString::number( url.port() ) );
    }

    //set the main class to run
    p->setMainClass( "org.kde.kjas.server.Main" );
}

void KJavaAppletServer::createContext( int contextId, KJavaAppletContext* context )
{
//    kdDebug(6100) << "createContext: " << contextId << endl;
    if ( d->javaProcessFailed ) return;

    d->contexts.insert( contextId, context );

    QStringList args;
    args.append( QString::number( contextId ) );
    process->send( KJAS_CREATE_CONTEXT, args );
}

void KJavaAppletServer::destroyContext( int contextId )
{
//    kdDebug(6100) << "destroyContext: " << contextId << endl;
    if ( d->javaProcessFailed ) return;
    d->contexts.remove( contextId );

    QStringList args;
    args.append( QString::number( contextId ) );
    process->send( KJAS_DESTROY_CONTEXT, args );
}

bool KJavaAppletServer::createApplet( int contextId, int appletId,
                             const QString & name, const QString & clazzName,
                             const QString & baseURL, const QString & user,
                             const QString & password, const QString & authname,
                             const QString & codeBase, const QString & jarFile,
                             QSize size, const QMap<QString,QString>& params,
                             const QString & windowTitle )
{
//    kdDebug(6100) << "createApplet: contextId = " << contextId     << endl
//              << "              appletId  = " << appletId      << endl
//              << "              name      = " << name          << endl
//              << "              clazzName = " << clazzName     << endl
//              << "              baseURL   = " << baseURL       << endl
//              << "              codeBase  = " << codeBase      << endl
//              << "              jarFile   = " << jarFile       << endl
//              << "              width     = " << size.width()  << endl
//              << "              height    = " << size.height() << endl;

    if ( d->javaProcessFailed ) return false;

    QStringList args;
    args.append( QString::number( contextId ) );
    args.append( QString::number( appletId ) );

    //it's ok if these are empty strings, I take care of it later...
    args.append( name );
    args.append( clazzName );
    args.append( baseURL );
    args.append( user );
    args.append( password );
    args.append( authname );
    args.append( codeBase );
    args.append( jarFile );

    args.append( QString::number( size.width() ) );
    args.append( QString::number( size.height() ) );

    args.append( windowTitle );

    //add on the number of parameter pairs...
    int num = params.count();
    QString num_params = QString("%1").arg( num, 8 );
    args.append( num_params );

    QMap< QString, QString >::ConstIterator it;

    for( it = params.begin(); it != params.end(); ++it )
    {
        args.append( it.key() );
        args.append( it.data() );
    }

    process->send( KJAS_CREATE_APPLET, args );

    return true;
}

void KJavaAppletServer::initApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    QStringList args;
    args.append( QString::number( contextId ) );
    args.append( QString::number( appletId ) );

    process->send( KJAS_INIT_APPLET, args );
}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( KJAS_DESTROY_APPLET, args );
}

void KJavaAppletServer::startApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( KJAS_START_APPLET, args );
}

void KJavaAppletServer::stopApplet( int contextId, int appletId )
{
    if ( d->javaProcessFailed ) return;
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( KJAS_STOP_APPLET, args );
}

void KJavaAppletServer::sendURLData( int loaderID, int code, const QByteArray& data )
{
    QStringList args;
    args.append( QString::number(loaderID) );
    args.append( QString::number(code) );

    process->send( KJAS_URLDATA, args, data );
}

void KJavaAppletServer::removeDataJob( int loaderID )
{
    KIOJobMap::iterator it = d->kiojobs.find( loaderID );
    if (it != d->kiojobs.end()) {
        it.data()->deleteLater();
        d->kiojobs.erase( it );
    }
}

void KJavaAppletServer::quit()
{
    QStringList args;

    process->send( KJAS_SHUTDOWN_SERVER, args );
    process->flushBuffers();
    process->wait( 10 );
}

void KJavaAppletServer::slotJavaRequest( const QByteArray& qb )
{
    // qb should be one command only without the length string,
    // we parse out the command and it's meaning here...
    QString cmd;
    QStringList args;
    int index = 0;
    int qb_size = qb.size();

    //get the command code
    char cmd_code = qb[ index++ ];
    ++index; //skip the next sep

    //get contextID
    QString contextID;
    while( qb[index] != 0 && index < qb_size )
    {
        contextID += qb[ index++ ];
    }
    bool ok;
    int ID_num = contextID.toInt( &ok ); // context id or kio job id
    /*if (d->locked_context > -1 && 
        ID_num != d->locked_context &&
        (cmd_code == KJAS_JAVASCRIPT_EVENT ||
         cmd_code == KJAS_APPLET_STATE ||
         cmd_code == KJAS_APPLET_FAILED))
    {
        / * Don't allow requests from other contexts if we're waiting
         * on a return value that can trigger JavaScript events
         * /
        d->java_requests.push_back(qb);
        return;
    }*/
    ++index; //skip the sep

    if (cmd_code == KJAS_PUT_DATA) {
        // rest of the data is for kio put
        if (ok) {
            KIOJobMap::iterator it = d->kiojobs.find( ID_num );
            if (ok && it != d->kiojobs.end()) {
                QByteArray qba;
                qba.setRawData(qb.data() + index, qb.size() - index - 1);
                it.data()->data(qba);
                qba.resetRawData(qb.data() + index, qb.size() - index - 1);
            }
            kdDebug(6100) << "PutData(" << ID_num << ") size=" << qb.size() - index << endl;
        } else
            kdError(6100) << "PutData error " << ok << endl;
        return;
    }
    //now parse out the arguments
    while( index < qb_size )
    {
        int sep_pos = qb.find( 0, index );
        if (sep_pos < 0) {
            kdError(6100) << "Missing separation byte" << endl;
            sep_pos = qb_size;
        }
        //kdDebug(6100) << "KJavaAppletServer::slotJavaRequest: "<< QString::fromLocal8Bit( qb.data() + index, sep_pos - index ) << endl;
        args.append( QString::fromLocal8Bit( qb.data() + index, sep_pos - index ) );
        index = sep_pos + 1; //skip the sep
    }
    //here I should find the context and call the method directly
    //instead of emitting signals
    switch( cmd_code )
    {
        case KJAS_SHOW_DOCUMENT:
            cmd = QString::fromLatin1( "showdocument" );
            break;

        case KJAS_SHOW_URLINFRAME:
            cmd = QString::fromLatin1( "showurlinframe" );
            break;

        case KJAS_SHOW_STATUS:
            cmd = QString::fromLatin1( "showstatus" );
            break;

        case KJAS_RESIZE_APPLET:
            cmd = QString::fromLatin1( "resizeapplet" );
            break;

        case KJAS_GET_URLDATA:
            if (ok && args.size () > 0) {
                d->kiojobs.insert(ID_num, new KJavaDownloader(ID_num, args[0]));
                kdDebug(6100) << "GetURLData(" << ID_num << ") url=" << args[0] << endl;
            } else
                kdError(6100) << "GetURLData error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_PUT_URLDATA:
            if (ok && args.size () > 0) {
                KJavaUploader *job = new KJavaUploader(ID_num, args[0]);
                d->kiojobs.insert(ID_num, job);
                job->start();
                kdDebug(6100) << "PutURLData(" << ID_num << ") url=" << args[0] << endl;
            } else
                kdError(6100) << "PutURLData error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_DATA_COMMAND:
            if (ok && args.size () > 0) {
                int cmd = args[0].toInt( &ok );
                KIOJobMap::iterator it = d->kiojobs.find( ID_num );
                if (ok && it != d->kiojobs.end())
                    it.data()->jobCommand( cmd );
                kdDebug(6100) << "KIO Data command: " << ID_num << " " << args[0] << endl;
            } else
                kdError(6100) << "KIO Data command error " << ok << " args:" << args.size() << endl;
            return;
        case KJAS_JAVASCRIPT_EVENT:
            cmd = QString::fromLatin1( "JS_Event" );
            kdDebug(6100) << "Javascript request: "<< contextID
                          << " code: " << args[0] << endl;
            break;
        case KJAS_GET_MEMBER:
        case KJAS_PUT_MEMBER:
        case KJAS_CALL_MEMBER: {
            int ticket = args[0].toInt();
            JSStack::iterator it = d->jsstack.find(ticket);
            if (it != d->jsstack.end()) {
                kdDebug(6100) << "slotJavaRequest: " << ticket << endl; 
                args.pop_front();
                it.data()->args.operator=(args); // just in case ..
                it.data()->ready = true;
                it.data()->exit = true;
            } else
                kdDebug(6100) << "Error: Missed return member data" << endl;
            return;
        }
        case KJAS_AUDIOCLIP_PLAY:
            cmd = QString::fromLatin1( "audioclip_play" );
            kdDebug(6100) << "Audio Play: url=" << args[0] << endl;
            break;
        case KJAS_AUDIOCLIP_LOOP:
            cmd = QString::fromLatin1( "audioclip_loop" );
            kdDebug(6100) << "Audio Loop: url=" << args[0] << endl;
            break;
        case KJAS_AUDIOCLIP_STOP:
            cmd = QString::fromLatin1( "audioclip_stop" );
            kdDebug(6100) << "Audio Stop: url=" << args[0] << endl;
            break;
        case KJAS_APPLET_STATE:
            kdDebug(6100) << "Applet State Notification for Applet " << args[0] << ". New state=" << args[1] << endl;
            cmd = QString::fromLatin1( "AppletStateNotification" );
            break;
        case KJAS_APPLET_FAILED:
            kdDebug(6100) << "Applet " << args[0] << " Failed: " << args[1] << endl;
            cmd = QString::fromLatin1( "AppletFailed" );
            break;
        default:
            return;
            break;
    }


    if( !ok )
    {
        kdError(6100) << "could not parse out contextID to call command on" << endl;
        return;
    }

    KJavaAppletContext* context = d->contexts[ ID_num ];
    if( context )
        context->processCmd( cmd, args );
    else if (cmd != "AppletStateNotification") 
        kdError(6100) << "no context object for this id" << endl;
}

void KJavaAppletServer::endWaitForReturnData() {
    kdDebug(6100) << "KJavaAppletServer::endWaitForReturnData" << endl;
    killTimers();
    JSStack::iterator it = d->jsstack.begin();
    for (; it != d->jsstack.end(); ++it)
        it.data()->exit = true;
}

void KJavaAppletServer::timerEvent(QTimerEvent *) {
    endWaitForReturnData();
    kdDebug(6100) << "KJavaAppletServer::timerEvent timeout" << endl;
}

void KJavaAppletServer::waitForReturnData(JSStackFrame * frame) {
    kdDebug(6100) << ">KJavaAppletServer::waitForReturnData" << endl;
    killTimers();
    startTimer(15000);
    while (!frame->exit)
        kapp->eventLoop()->processEvents (QEventLoop::AllEvents | QEventLoop::WaitForMore);
    if (d->jsstack.size() <= 1)
        killTimers();
    kdDebug(6100) << "<KJavaAppletServer::waitForReturnData stacksize:" << d->jsstack.size() << endl;
}

bool KJavaAppletServer::getMember(QStringList & args, QStringList & ret_args) {
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( QString::number(frame.ticket) );

    process->send( KJAS_GET_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready;
}

bool KJavaAppletServer::putMember( QStringList & args ) {
    QStringList ret_args;
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( QString::number(frame.ticket) );

    process->send( KJAS_PUT_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready && ret_args.count() > 0 && ret_args[0].toInt();
}

bool KJavaAppletServer::callMember(QStringList & args, QStringList & ret_args) {
    JSStackFrame frame( d->jsstack, ret_args );
    args.push_front( QString::number(frame.ticket) );

    process->send( KJAS_CALL_MEMBER, args );
    waitForReturnData( &frame );

    return frame.ready;
}

void KJavaAppletServer::derefObject( QStringList & args ) {
    process->send( KJAS_DEREF_OBJECT, args );
}

bool KJavaAppletServer::usingKIO() {
    return d->useKIO;
}

#include "kjavaappletserver.moc"
