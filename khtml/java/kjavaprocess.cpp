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

void KJavaProcess::setJVMPath( QString path )
{
   jvmPath = path;
}

void KJavaProcess::setJVMVersion( int major, int minor = 0, int patch = 0 )
{
   versionMajor = major;
   versionMinor = minor;
   versionPatch = patch;
}

void KJavaProcess::setHTTPProxy( QString host, int port )
{
   httpProxyHost = host;
   httpProxyPort = port;
}

void KJavaProcess::setFTPProxy( QString host, int port )
{
   ftpProxyHost = host;
   ftpProxyPort = port;
}

void KJavaProcess::setSystemProperty( QString name, QString value )
{
}

void KJavaProcess::setMainClass( QString clazzName )
{
   mainClass = clazzName;
}

void KJavaProcess::setExtraArgs( QString args )
{
   extraArgs = args;
}

void KJavaProcess::setClassArgs( QString args )
{
   classArgs = args;
}

void KJavaProcess::send( QString command )
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


