#include "kjavaprocess.moc"

KJavaProcess::KJavaProcess()
{
   javaProcess = new KProcess();
   CHECK_PTR( javaProcess );

   connect( javaProcess, SIGNAL( wroteStdin( KProcess * ) ),
	    this, SLOT( wroteData() ) );

   jvmPath = "java";
   mainClass = "-help"; // This should always get overwritten
}

KJavaProcess::~KJavaProcess()
{
   delete javaProcess;
}

bool KJavaProcess::isOK()
{
   return ok;
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
   jvmPath = path;
}

void KJavaProcess::setJVMVersion( int major, int minor, int patch )
{
   versionMajor = major;
   versionMinor = minor;
   versionPatch = patch;
}

void KJavaProcess::setHTTPProxy( const QString host, int port )
{
   httpProxyHost = host;
   httpProxyPort = port;
}

void KJavaProcess::setFTPProxy( const QString host, int port )
{
   ftpProxyHost = host;
   ftpProxyPort = port;
}

void KJavaProcess::setSystemProperty( const QString name, const QString value )
{
}

void KJavaProcess::setMainClass( const QString clazzName )
{
   mainClass = clazzName;
}

void KJavaProcess::setExtraArgs( const QString args )
{
   extraArgs = args;
}

void KJavaProcess::setClassArgs( const QString args )
{
   classArgs = args;
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
   *javaProcess << jvmPath;

   // For each system property
   // arg = "-D" + name + "=" + value
   // *javaProcess << arg

   // For each extra arg
   // *javaProcess << arg

   *javaProcess << mainClass;

   // For each class arg
   // *javaProcess << arg

   warning( "Invoking JVM now..." );

   javaProcess->start( KProcess::NotifyOnExit, KProcess::Stdin );
}

void KJavaProcess::killJVM()
{
   javaProcess->kill();
}


