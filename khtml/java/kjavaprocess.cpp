#include "kjavaprocess.h"
#include "kjavaappletserver.h"

#include <kdebug.h>
#include <kprotocolmanager.h>

#include <qtextstream.h>

#include <iostream.h>
#include <unistd.h>


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
   QList<QByteArray> BufferList;
};

KJavaProcess::KJavaProcess()
    : inputBuffer(),
      systemProps()
{
    d = new KJavaProcessPrivate;
    CHECK_PTR( d );
    d->BufferList.setAutoDelete( true );

    javaProcess = new KProcess();
    CHECK_PTR( javaProcess );

    connect( javaProcess, SIGNAL( wroteStdin( KProcess * ) ),
             this, SLOT( wroteData() ) );
    connect( javaProcess, SIGNAL( processExited( KProcess * ) ),
             this, SLOT( javaHasDied() ) );
    connect( javaProcess, SIGNAL( receivedStdout( int, int& ) ),
             this, SLOT( receivedData(int, int&) ) );

    d->jvmPath = "java";
    d->mainClass = "-help";

    //check for proxy settings
    if( KProtocolManager::useProxy() )
    {
        d->httpProxyHost = KProtocolManager::proxyFor( "http" );
        setSystemProperty( "kjas.proxy", d->httpProxyHost );
    }
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

void KJavaProcess::setJVMPath( const QString& path )
{
   d->jvmPath = path;
}

void KJavaProcess::setJVMVersion( int major, int minor, int patch )
{
   d->versionMajor = major;
   d->versionMinor = minor;
   d->versionPatch = patch;
}

void KJavaProcess::setHTTPProxy( const QString& host, int port )
{
   d->httpProxyHost = host;
   d->httpProxyPort = port;
}

void KJavaProcess::setFTPProxy( const QString& host, int port )
{
   d->ftpProxyHost = host;
   d->ftpProxyPort = port;
}

void KJavaProcess::setSystemProperty( const QString& name,
                                      const QString& value )
{
   systemProps.insert( name, value );
}

void KJavaProcess::setMainClass( const QString& clazzName )
{
   d->mainClass = clazzName;
}

void KJavaProcess::setExtraArgs( const QString& args )
{
   d->extraArgs = args;
}

void KJavaProcess::setClassArgs( const QString& args )
{
   d->classArgs = args;
}

void KJavaProcess::send( const QString& /*command*/ )
{
    kdWarning() << "you called the deprecated send command- it won't work" << endl;
}

void KJavaProcess::send( char cmd_code, const QStringList& args )
{
    kdDebug() << "KJavaProcess::send( the new one)" << endl;

    //the buffer to store stuff, etc.
    QByteArray* buff = new QByteArray();
    QTextOStream output( *buff );
    char sep = 0;

    //make space for the command size: 8 characters...
    QCString space( "        " );
    output << space;

    //write command code
    kdDebug() << "cmd_code = " << (int)cmd_code << endl;
    output << cmd_code;

    //store the arguments...
    if( args.count() == 0 )
    {
        output << sep;
    }
    else
    {
        for( QStringList::ConstIterator it = args.begin();
             it != args.end(); ++it )
        {
            if( !(*it).isEmpty() )
            {
                output << (*it).latin1();
            }
            output << sep;
        }
    }

    int size = buff->size() - 8;  //subtract out the length of the size_str
    QString size_str = QString("%1").arg( size, 8 );
    kdDebug() << "size of message = " << size_str << endl;

    const char* size_ptr = size_str.latin1();
    for( int i = 0; i < 8; i++ )
        buff->at(i) = size_ptr[i];

    d->BufferList.append( buff );

    kdDebug() << "just added this buffer of size: " << buff->size() << " to the queue: " << endl;

    if( d->BufferList.count() == 1 )
    {
        popBuffer();
    }
}

void KJavaProcess::popBuffer()
{
    QByteArray* buf = d->BufferList.first();
    if( buf )
    {
        cout << "Sending buffer to java, buffer = >>";
        for( unsigned int i = 0; i < buf->size(); i++ )
        {
            if( buf->at(i) == (char)0 )
                cout << "<SEP>";
            else if( buf->at(i) > 0 && buf->at(i) < 10 )
                cout << "<CMD " << (int) buf->at(i) << ">";
            else
                cout << buf->at(i);
        }
        cout << "<<" << endl;

        //write the data
        if ( !javaProcess->writeStdin( buf->data(),
                                       buf->size() ) )
        {
            qWarning( "Could not write command" );
        }
    }
}

void KJavaProcess::wroteData( )
{
    //do this here- we can't free the data until we know it went through
    d->BufferList.removeFirst();  //this should delete it since we setAutoDelete(true)

    if ( d->BufferList.count() >= 1 )
    {
        popBuffer();
    }
}


void KJavaProcess::invokeJVM()
{
    *javaProcess << d->jvmPath;

    //set the system properties, iterate through the qmap of system properties
    for( QMap<QString,QString>::Iterator it = systemProps.begin();
         it != systemProps.end(); ++it )
    {
        QString currarg;

        if( !it.key().isEmpty() )
        {
            currarg = "-D" + it.key();
            if( !it.data().isEmpty() )
                currarg += "=" + it.data();
        }

        if( !currarg.isEmpty() )
            *javaProcess << currarg;
    }

    //load the extra user-defined arguments
    if( !d->extraArgs.isEmpty() )
    {
        // BUG HERE: if an argument contains space (-Dname="My name")
        // this parsing will fail. Need more sophisticated parsing
        QStringList args = QStringList::split( " ", d->extraArgs );
        for ( QStringList::Iterator it = args.begin(); it != args.end(); ++it )
            *javaProcess << *it;
    }

    *javaProcess << d->mainClass;

    if ( d->classArgs != QString::null )
        *javaProcess << d->classArgs;

    kdDebug() << "Invoking JVM now...with arguments = " << endl;
    QStrList* args = javaProcess->args();
    QString str_args;
    for( char* it = args->first(); it; it = args->next() )
    {
        str_args += it;
        str_args += ' ';
    }
    kdDebug() << str_args << endl;

    KProcess::Communication flags =  (KProcess::Communication)
                                     (KProcess::Stdin | KProcess::Stdout |
                                      KProcess::NoRead);
    kdDebug() << "kprocess flags = " << flags << endl;
    javaProcess->start( KProcess::NotifyOnExit, flags );
    javaProcess->resume(); //start processing stdout on the java process

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

/*  In this method, read one command and send it to the d->appletServer
 *  then return, so we don't block the event handling
 */
void KJavaProcess::receivedData( int fd, int& )
{
    kdDebug() << "KJavaProcess::receivedData" << endl;

    //read out the length of the message,
    //read the message and send it to the applet server
    char length[9] = { 0 };
    int num_bytes = ::read( fd, length, 8 );
    if( num_bytes == -1 )
    {
        kdError() << "could not read 8 characters for the message length!!!!" << endl;
        return;
    }

    QString lengthstr( length );
    bool ok;
    int num_len = lengthstr.toInt( &ok );
    if( !ok )
    {
        kdError() << "could not parse length out of: " << lengthstr << endl;
        return;
    }

    kdDebug() << "msg length = " << num_len << endl;

    //now parse out the rest of the message.
    char* msg = new char[num_len];
    num_bytes = ::read( fd, msg, num_len );
    if( num_bytes == -1 ||  num_bytes != num_len )
    {
        kdError() << "could not read the msg, num_bytes = " << num_bytes << endl;
        return;
    }

    QByteArray qb;
    QByteArray copied_data = qb.duplicate( msg, num_len );
    delete msg;

    emit received( copied_data );
}

void KJavaProcess::receivedData( KProcess*, char*, int )
{
    kdWarning() << "This method is deprecated- it doesn't do anything anymore" << endl;
}

#include "kjavaprocess.moc"
