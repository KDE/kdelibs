
#include "kjavaappletserver.h"
#include "kjavaappletcontext.h"
#include "kjavaprocess.h"

#include <kconfig.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtimer.h>
#include <qguardedptr.h>
#include <qdir.h>

#include <stdlib.h>

#define CREATE_CONTEXT    (char)1
#define DESTROY_CONTEXT   (char)2
#define CREATE_APPLET     (char)3
#define DESTROY_APPLET    (char)4
#define START_APPLET      (char)5
#define STOP_APPLET       (char)6
#define INIT_APPLET       (char)7

#define SHUTDOWN_SERVER   (char)9

#define SHOW_DOCUMENT     (char)12
#define SHOW_URLINFRAME   (char)13
#define SHOW_STATUS       (char)14
#define RESIZE_APPLET     (char)15

// For future expansion
struct KJavaAppletServerPrivate
{
   int counter;
   QMap< int, QGuardedPtr<KJavaAppletContext> > contexts;
   QString appletLabel;
};

static KJavaAppletServer* self = 0;

KJavaAppletServer::KJavaAppletServer()
{
    d = new KJavaAppletServerPrivate;
    process = new KJavaProcess();

    connect( process, SIGNAL(received(const QByteArray&)),
             this,    SLOT(received(const QByteArray&)) );

    setupJava( process );

    if( process->startJava() )
        d->appletLabel = i18n( "Loading Applet" );
    else
        d->appletLabel = i18n( "Error: java executable not found" );

}

KJavaAppletServer::~KJavaAppletServer()
{
//    kdDebug(6100) << "KJavaAppletServer::~KJavaAppletServer" << endl;
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
            jvm_path = jPath + "/bin/java";
        else if( QFile::exists(jPath) ) //check here to see if they entered the whole path the java exe
            jvm_path = jPath;
    }

    //check to see if jvm_path is valid and set d->appletLabel accordingly
    p->setJVMPath( jvm_path );

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

    p->setMainClass( "org.kde.kjas.server.Main" );

    // Prepare classpath
    QString kjava_classes = locate("data", "kjava/kjava-classes.zip");
    if( kjava_classes.isNull() ) // Should not happen
        return;

    QString new_classpath = "CLASSPATH=" +  kjava_classes;

    char *classpath = getenv("CLASSPATH");
    if(classpath)
    {
        new_classpath += ":";
        new_classpath += classpath;
    }

    // Need strdup() to prevent freeing the memory we provide to putenv
    putenv( strdup(new_classpath.latin1()) );
}

void KJavaAppletServer::createContext( int contextId, KJavaAppletContext* context )
{
//    kdDebug(6100) << "createContext: " << contextId << endl;
    d->contexts.insert( contextId, context );

    QStringList args;
    args.append( QString::number( contextId ) );
    process->send( CREATE_CONTEXT, args );
}

void KJavaAppletServer::destroyContext( int contextId )
{
//    kdDebug(6100) << "destroyContext: " << contextId << endl;
    d->contexts.remove( contextId );

    QStringList args;
    args.append( QString::number( contextId ) );
    process->send( DESTROY_CONTEXT, args );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
                                      const QString name, const QString clazzName,
                                      const QString baseURL, const QString codeBase,
                                      const QString jarFile, QSize size,
                                      const QMap<QString,QString>& params,
                                      const QString windowTitle )
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

    QStringList args;
    args.append( QString::number( contextId ) );
    args.append( QString::number( appletId ) );

    //it's ok if these are empty strings, I take care of it later...
    args.append( name );
    args.append( clazzName );
    args.append( baseURL );
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

    process->send( CREATE_APPLET, args );
}

void KJavaAppletServer::initApplet( int contextId, int appletId )
{
    QStringList args;
    args.append( QString::number( contextId ) );
    args.append( QString::number( appletId ) );

    process->send( INIT_APPLET, args );
}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( DESTROY_APPLET, args );
}

void KJavaAppletServer::startApplet( int contextId, int appletId )
{
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( START_APPLET, args );
}

void KJavaAppletServer::stopApplet( int contextId, int appletId )
{
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( STOP_APPLET, args );
}

void KJavaAppletServer::quit()
{
    QStringList args;

    process->send( SHUTDOWN_SERVER, args );
}

void KJavaAppletServer::received( const QByteArray& qb )
{
    // qb should be one command only without the length string,
    // we parse out the command and it's meaning here...
//    kdDebug(6100) << "begin KJavaAppletServer::received buffer of length = " << qb.count() << endl;
    QString buff;
    int qb_count = (int)qb.count();
    for( int i = 0; i < qb_count; i++ )
    {
        if( qb[i] == 0 )
            buff += "<SEP>";
        else
        if( qb[i] > 0 && qb[i] < 16 )
            buff += "<CMD " + QString::number(qb[i]) + ">";
        else
            buff += qb[i];
    }
//    kdDebug(6100) << "buffer = >>" << buff << "<<" << endl;

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

    //parse out the args
    while( index < qb_size )
    {
        QString tmp;
        while( qb[index] != 0 )
            tmp += qb[ index++ ];

        args.append( tmp );

        ++index; //skip the sep
    }

    //here I should find the context and call the method directly
    //instead of emitting signals
    switch( cmd_code )
    {
        case SHOW_DOCUMENT:
            cmd = QString::fromLatin1( "showdocument" );
            break;

        case SHOW_URLINFRAME:
            cmd = QString::fromLatin1( "showurlinframe" );
            break;

        case SHOW_STATUS:
            cmd = QString::fromLatin1( "showstatus" );
            break;

        case RESIZE_APPLET:
            cmd = QString::fromLatin1( "resizeapplet" );
            break;

        default:
            break;
    }

    bool ok;
    int contextID_num = contextID.toInt( &ok );

    if( !ok )
    {
        kdError(6100) << "could not parse out contextID to call command on" << endl;
        return;
    }

    KJavaAppletContext* tmp = d->contexts[ contextID_num ];
    if( tmp )
        tmp->processCmd( cmd, args );
    else
        kdError(6100) << "no context object for this id" << endl;
}

#include "kjavaappletserver.moc"
