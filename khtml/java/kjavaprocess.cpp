#include "kjavaprocess.moc"

typedef QMap<QString, QString> PropsMap;

const int MAX_INPUT_SIZE = 1023;

struct KJavaProcessPrivate
{
   int versionMajor;
   int versionMinor;
   int versionPatch;
   QString httpProxyHost;
   int httpProxyPort;
   QString ftpProxyHost;
   int ftpProxyPort;
   bool ok;
   QString jvmPath;
   QString mainClass;
   QString extraArgs;
   QString classArgs;
};

KJavaProcess::KJavaProcess()
    : inputBuffer(),
      systemProps()
{
   d = new KJavaProcessPrivate;
   CHECK_PTR( d );

   javaProcess = new KProcess();
   CHECK_PTR( javaProcess );

   connect( javaProcess, SIGNAL( wroteStdin( KProcess * ) ),
	    this, SLOT( wroteData() ) );
   connect( javaProcess, SIGNAL( processExited( KProcess * ) ),
	    this, SLOT( javaHasDied() ) );
   connect( javaProcess, SIGNAL( receivedStdout( KProcess *, char *, int ) ),
	    this, SLOT( receivedData( KProcess *, char *, int ) ) );

   d->jvmPath = "java";
   d->mainClass = "-help"; // This should always get overwritten
}

KJavaProcess::~KJavaProcess()
{
  if ( d->ok && isRunning() )
    stopJava();

   delete javaProcess;
   delete d;
}

bool KJavaProcess::isOK()
{
   return d->ok;
}

void KJavaProcess::javaHasDied()
{
    d->ok = false;
}

bool KJavaProcess::isRunning()
{
   return javaProcess->isRunning();
}

void KJavaProcess::startJava()
{
   invokeJVM();
}

void KJavaProcess::stopJava()
{
   killJVM();
}

void KJavaProcess::setJVMPath( const QString path )
{
   d->jvmPath = path;
}

void KJavaProcess::setJVMVersion( int major, int minor, int patch )
{
   d->versionMajor = major;
   d->versionMinor = minor;
   d->versionPatch = patch;
}

void KJavaProcess::setHTTPProxy( const QString host, int port )
{
   d->httpProxyHost = host;
   d->httpProxyPort = port;
}

void KJavaProcess::setFTPProxy( const QString host, int port )
{
   d->ftpProxyHost = host;
   d->ftpProxyPort = port;
}

void KJavaProcess::setSystemProperty( const QString name, const QString value )
{
   systemProps.insert( name, value );
}

void KJavaProcess::setMainClass( const QString clazzName )
{
   d->mainClass = clazzName;
}

void KJavaProcess::setExtraArgs( const QString args )
{
   d->extraArgs = args;
}

void KJavaProcess::setClassArgs( const QString args )
{
   d->classArgs = args;
}

void KJavaProcess::send( const QString command )
{
    inputBuffer.append( command );

warning("sendRequest: %s", command.data() );

    // If there's nothing being sent right now
    if ( inputBuffer.count() == 1 ) {
	if ( !javaProcess->writeStdin( inputBuffer.first(),
				       qstrlen( inputBuffer.first() ) ) ) {
	    warning( "Could not write %s command", command.data() );
	}
    }
}

void KJavaProcess::wroteData( )
{
    inputBuffer.removeFirst();

    if ( inputBuffer.count() >= 1 ) {
	if ( !javaProcess->writeStdin( inputBuffer.first(),
				       qstrlen( inputBuffer.first() ) ) ) {
	    warning( "Could not lazy write %s command", inputBuffer.first() );
	}
    }
}

void KJavaProcess::invokeJVM()
{
   *javaProcess << d->jvmPath;

   // For each system property
   // arg = "-D" + name + "=" + value
   PropsMap::Iterator it;
   for ( it = systemProps.begin(); it != systemProps.end(); ++it ) {
       QString s = "-D%1=%2";
       QString t = s.arg( it.key() ).arg( it.data() );
       *javaProcess << t;
   }

   if ( d->extraArgs != QString::null )
       *javaProcess << d->extraArgs;
   *javaProcess << d->mainClass;

   // For each class arg
   
   if ( d->classArgs != QString::null )
       *javaProcess << d->classArgs;

   warning( "Invoking JVM now..." );

   KProcess::Communication comms = ( KProcess::Communication ) (KProcess::Stdin | KProcess::Stdout);
   javaProcess->start( KProcess::NotifyOnExit, comms );
}

void KJavaProcess::killJVM()
{
   javaProcess->kill();
}

void KJavaProcess::processExited()
{
    delete javaProcess;
    d->ok = false;
}

void KJavaProcess::receivedData( KProcess *, char *buffer, int len )
{
    char *cpy = new char[ MAX_INPUT_SIZE + 1 ];
    CHECK_PTR( cpy );

    memcpy( cpy, buffer, len * sizeof( char ) );
    cpy[ len ] = 0;

    QString s;
    s = (const char *) cpy;
    delete cpy;

    emit received( s );
}
