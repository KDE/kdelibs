#include <stdlib.h>

#include "kjavaappletserver.moc"
#include "kjavaprocess.h"
#include "kjavaappletcontext.h"
#include <kconfig.h>
#include <kstddirs.h>
#include <kdebug.h>

#define CREATE_CONTEXT   (char)1
#define DESTROY_CONTEXT  (char)2
#define CREATE_APPLET    (char)3
#define DESTROY_APPLET   (char)4
#define START_APPLET     (char)5
#define STOP_APPLET      (char)6
#define SHOW_APPLET      (char)7
#define SET_PARAMETER    (char)8
#define SHUTDOWN_SERVER  (char)9
#define SHOW_DOCUMENT    (char)12
#define SHOW_URLINFRAME  (char)13
#define SHOW_STATUS      (char)14
#define RESIZE_APPLET    (char)15

// For future expansion
struct KJavaAppletServerPrivate
{
   int counter;

   QMap<QString, KJavaAppletContext> context_cache;
};

static KJavaAppletServer *self = 0;

KJavaAppletServer::KJavaAppletServer()
{
   d = new KJavaAppletServerPrivate;
   CHECK_PTR( d );

   process = new KJavaProcess();
   CHECK_PTR( process );
   connect( process, SIGNAL(received(const QByteArray&)),
            this, SLOT(received(const QByteArray&)) );

   setupJava( process );

   process->startJava();
}

KJavaAppletServer::~KJavaAppletServer()
{
   delete process;
}

KJavaAppletServer* KJavaAppletServer::allocateJavaServer()
{
   if( self == 0 ) {
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
      self->quit();
      delete self;
      self = 0;
   }
}

void KJavaAppletServer::setupJava( KJavaProcess *p )
{
    KConfig config ( "konquerorrc", true );
    config.setGroup( "Java/JavaScript Settings" );

    if( config.readBoolEntry( "JavaAutoDetect", true) )
        p->setJVMPath( "java" );
    else
    {
        QString jPath = config.readEntry( "JavaPath", "/usr/lib/jdk" );
        // Cut off trailing slash if any
        if( jPath[jPath.length()-1] == '/' )
          jPath.remove(jPath.length()-1, 1);

        p->setJVMPath( jPath + "/bin/java");
    }
    QString extraArgs = config.readEntry( "JavaArgs", "" );
    p->setExtraArgs( extraArgs );

    if( config.readBoolEntry( "ShowJavaConsole", false) )
    {
        p->setSystemProperty( "kjas.showConsole", QString::null );
    }

    if( config.readBoolEntry( "UseJava2SecurityManager", true ) )
    {
        p->setSystemProperty( "java.security.manager", QString::null );

        QString class_file = locate( "data", "kjava/kjava.policy" );
        p->setSystemProperty( "java.security.policy", class_file );
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
    putenv(strdup(new_classpath.latin1()));
}

void KJavaAppletServer::createContext( int contextId )
{
    kdDebug() << "createContext: " << contextId << endl;

    QStringList args;
    args.append( QString::number( contextId ) );

    process->send( CREATE_CONTEXT, args );
}

void KJavaAppletServer::destroyContext( int contextId )
{
    kdDebug() << "destroyContext: " << contextId << endl;

    QStringList args;
    args.append( QString::number( contextId ) );

    process->send( DESTROY_CONTEXT, args );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
                                      const QString name,
                                      const QString clazzName,
                                      const QString baseURL,
                                      const QString codeBase,
                                      const QString jarFile,
                                      QSize size )
{
    kdDebug() << "createApplet: contextId = " << contextId     << endl
              << "              appletId  = " << appletId      << endl
              << "              name      = " << name          << endl
              << "              clazzName = " << clazzName     << endl
              << "              baseURL   = " << baseURL       << endl
              << "              codeBase  = " << codeBase      << endl
              << "              jarFile   = " << jarFile       << endl
              << "              width     = " << size.width()  << endl
              << "              height    = " << size.height() << endl;

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

    process->send( CREATE_APPLET, args );
}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );

    process->send( DESTROY_APPLET, args );
}


void KJavaAppletServer::setParameter( int contextId, int appletId,
                                      const QString name, const QString value )
{
    kdDebug() << "setParameter, contextId = " << contextId << endl
              << "              appletId  = " << appletId  << endl
              << "              name      = " << name      << endl
              << "              value     = " << value     << endl;

    QStringList args;
    args.append( QString::number( contextId ) );
    args.append( QString::number( appletId ) );

    args.append( name );
    args.append( value );

    process->send( SET_PARAMETER, args );
}

void KJavaAppletServer::showApplet( int contextId, int appletId,
                                    const QString title )
{
    QStringList args;
    args.append( QString::number(contextId) );
    args.append( QString::number(appletId) );
    args.append( title );

    process->send( SHOW_APPLET, args );
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

void KJavaAppletServer::received( const QString& s )
{
    if ( !s.isEmpty() )
    {
        QStringList cmdLine = QStringList::split( "!", s, true );
        if ( cmdLine.count()>0 && !cmdLine[0].isEmpty() )
        {
            QString cmd = cmdLine[0].lower();

            QStringList arg;
            QStringList::Iterator it=cmdLine.begin();
            for( ++it; it!=cmdLine.end(); ++it )
                arg << *it;

            emit receivedCommand( cmd, arg );
        }
    }
}

void KJavaAppletServer::received( const QByteArray& qb )
{
    // qb should be one command only without the length string,
    // we parse out the command and it's meaning here...
    kdDebug() << "begin KJavaAppletServer::received buffer of length = " << qb.count() << endl;
    QString buff;
    for( int i = 0; i < qb.count(); i++ )
    {
        if( qb[i] == 0 )
            buff += "<SEP>";
        else
        if( qb[i] > 0 && qb[i] < 16 )
            buff += "<CMD " + QString::number(qb[i]) + ">";
        else
            buff += qb[i];
    }
    kdDebug() << "buffer = >>" << buff << "<<" << endl;

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
            cmd = "showdocument";
            break;

        case SHOW_URLINFRAME:
            cmd = "showurlinframe";
            break;

        case SHOW_STATUS:
            cmd = "showstatus";
            break;

        case RESIZE_APPLET:
            cmd = "resizeapplet";
            break;

        default:
            break;
    }

    emit receivedCommand( cmd, args );
}
