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
#include <qdir.h>

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


class JSStackNode {
public:
    JSStackNode() : ready(false) {}
    bool ready;
    QStringList args;
    int size;
};

// For future expansion
class KJavaAppletServerPrivate
{
friend class KJavaAppletServer;
private:
   int counter;
   int ticketcounter;
   QMap< int, QGuardedPtr<KJavaAppletContext> > contexts;
   QString appletLabel;
   QMap< int, JSStackNode* > jsstack;
   bool javaProcessFailed;
};

static KJavaAppletServer* self = 0;

KJavaAppletServer::KJavaAppletServer()
{
    d = new KJavaAppletServerPrivate;
    d->ticketcounter = 0;
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

    QString jPath = config.readEntry( "JavaPath" );
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
    QString extraArgs = config.readEntry( "JavaArgs", "" );
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

    //check for http proxies...
    if( KProtocolManager::useProxy() )
    {
        // only proxyForURL honours automatic proxy scripts
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

void KJavaAppletServer::sendURLData( const QString& loaderID,
                                     const QString& url,
                                     const QByteArray& data )
{
    QStringList args;
    args.append( loaderID );
    args.append( url );

    process->send( KJAS_URLDATA, args, data );

}

void KJavaAppletServer::quit()
{
    QStringList args;

    process->send( KJAS_SHUTDOWN_SERVER, args );
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
    ++index; //skip the sep

    //now parse out the arguments
    while( index < qb_size )
    {
        QString tmp;
        while( qb[index] != 0 )
            tmp += qb[ index++ ];

        kdDebug(6100) << "KJavaAppletServer::slotJavaRequest: "<< tmp << endl;
        args.append( tmp );

        ++index; //skip the sep
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
            //here we need to get some data for a class loader and send it back...
            kdDebug(6100) << "GetURLData from classloader: "<< contextID
                          << " for url: " << args[0] << endl;
            break;
        case KJAS_JAVASCRIPT_EVENT:
            cmd = QString::fromLatin1( "JS_Event" );
            kdDebug(6100) << "Javascript request: "<< contextID
                          << " code: " << args[0] << endl;
            break;
        case KJAS_GET_MEMBER:
        case KJAS_PUT_MEMBER:
        case KJAS_CALL_MEMBER: {
            int ticket = args[0].toInt();
            QMap<int, JSStackNode*>::iterator it = d->jsstack.find(ticket);
            if (it != d->jsstack.end()) {
                kdDebug(6100) << "slotJavaRequest: " << ticket << endl; 
                args.pop_front();
                it.data()->args = args;
                it.data()->ready = true;
                process->syncCommandReceived(ticket);
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

    if( cmd_code == KJAS_GET_URLDATA )
    {
        new KJavaDownloader( contextID, args[0] );
    }
    else
    {
        bool ok;
        int contextID_num = contextID.toInt( &ok );

        if( !ok )
        {
            kdError(6100) << "could not parse out contextID to call command on" << endl;
            return;
        }

        KJavaAppletContext* context = d->contexts[ contextID_num ];
        if( context )
            context->processCmd( cmd, args );
        else if (cmd != "AppletStateNotification") 
            kdError(6100) << "no context object for this id" << endl;
    }
}

bool KJavaAppletServer::getMember(int contextId, int appletId, const unsigned long objid, const QString & name, int & type, unsigned long & rid, QString & value) {
    QStringList args;
    int ticket = d->ticketcounter++;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );
    args.append( QString::number(ticket) );
    args.append( QString::number(objid) );
    args.append( name );

    JSStackNode * frame = new JSStackNode;
    d->jsstack.insert(ticket, frame); 
    kdDebug(6100) << "KJavaAppletServer::getMember " << name << " " << ticket << endl;
    process->sendSync( ticket, KJAS_GET_MEMBER, args );

    bool retval = frame->ready;
    if (retval && frame->args.count() == 3) {
        type = frame->args[0].toInt(&retval);
        if (retval && type >= 0) {
            rid = frame->args[1].toInt(&retval);
            value = frame->args[2];
        } else
            retval = false;
    } else {
        kdError(6100) << "getMember: " << (retval ? "args ":"timeout ") << ticket << endl;
    }

    d->jsstack.erase(ticket);
    delete frame;

    return retval;
}

bool KJavaAppletServer::putMember(int contextId, int appletId, const unsigned long objid, const QString & name, const QString & value) {
    QStringList args;
    int ticket = d->ticketcounter++;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );
    args.append( QString::number(ticket) );
    args.append( QString::number(objid) );
    args.append( name );
    args.append( value );

    JSStackNode * frame = new JSStackNode;
    d->jsstack.insert(ticket, frame); 
    kdDebug(6100) << "KJavaAppletServer::putMember " << name << " " << ticket << endl;

    process->sendSync( ticket, KJAS_PUT_MEMBER, args );

    bool retval = frame->ready;
    if (retval) {
        retval = frame->args[0].toInt(&retval);
    } else {
        kdError(6100) << "putMember: timeout" << endl;
    }

    d->jsstack.erase(ticket);
    delete frame;

    return retval;
}

bool KJavaAppletServer::callMember(int contextId, int appletId, const unsigned long objid, const QString & name, const QStringList & fargs, int & type, unsigned long & rid, QString & value) {
    QStringList args;
    int ticket = d->ticketcounter++;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );
    args.append( QString::number(ticket) );
    args.append( QString::number(objid) );
    args.append( name );
    for (QStringList::const_iterator it = fargs.begin(); it != fargs.end(); it++)
        args.append(*it);

    JSStackNode * frame = new JSStackNode;
    d->jsstack.insert(ticket, frame); 

    kdDebug(6100) << "KJavaAppletServer::callMember " << name << " " << ticket << endl;
    process->sendSync( ticket, KJAS_CALL_MEMBER, args );

    bool retval = frame->ready;
    if (retval) {
        type = frame->args[0].toInt(&retval);
        if (retval && type > -1) {
            rid = frame->args[1].toInt(&retval);
            if (retval)
                value = frame->args[2];
        } else
            retval = false;
    } else {
        kdError(6100) << "callMember: timeout return data" << endl;
    }

    d->jsstack.erase(ticket);
    delete frame;

    return retval;
}

void KJavaAppletServer::derefObject(int contextId, int appletId, const unsigned long objid) {
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );
    args.append( QString::number(objid) );

    process->send( KJAS_DEREF_OBJECT, args );
}
#include "kjavaappletserver.moc"
