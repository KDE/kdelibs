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

#include "kjavaprocess.h"

#include <kdebug.h>
#include <kio/kprotocolmanager.h>

#include <qtextstream.h>
#include <qmap.h>
#include <unistd.h>
#include <qptrlist.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

class KJavaProcessPrivate
{
friend class KJavaProcess;
private:
    QString jvmPath;
    QString classPath;
    QString mainClass;
    QString extraArgs;
    QString classArgs;
    QPtrList<QByteArray> BufferList;
    QMap<QString, QString> systemProps;
    bool processKilled;
    int sync_count;
};

KJavaProcess::KJavaProcess() : KProcess()
{
    d = new KJavaProcessPrivate;
    d->BufferList.setAutoDelete( true );
    d->processKilled = false;
    d->sync_count = 0;
    
    javaProcess = this; //new KProcess();

    connect( javaProcess, SIGNAL( wroteStdin( KProcess * ) ),
             this, SLOT( slotWroteData() ) );
    connect( javaProcess, SIGNAL( receivedStdout( int, int& ) ),
             this, SLOT( slotReceivedData(int, int&) ) );
    connect( javaProcess, SIGNAL( processExited (KProcess *) ),
             this, SLOT( slotExited (KProcess *) ) );

    d->jvmPath = "java";
    d->mainClass = "-help";
}

KJavaProcess::~KJavaProcess()
{
    if ( isRunning() )
    {
        kdDebug(6100) << "stopping java process" << endl;
        stopJava();
    }

    //delete javaProcess;
    delete d;
}

bool KJavaProcess::isRunning()
{
   return javaProcess->isRunning();
}

bool KJavaProcess::startJava()
{
   return invokeJVM();
}

void KJavaProcess::stopJava()
{
   killJVM();
}

void KJavaProcess::setJVMPath( const QString& path )
{
   d->jvmPath = path;
}

void KJavaProcess::setClasspath( const QString& classpath )
{
    d->classPath = classpath;
}

void KJavaProcess::setSystemProperty( const QString& name,
                                      const QString& value )
{
   d->systemProps.insert( name, value );
}

void KJavaProcess::setMainClass( const QString& className )
{
   d->mainClass = className;
}

void KJavaProcess::setExtraArgs( const QString& args )
{
   d->extraArgs = args;
}

void KJavaProcess::setClassArgs( const QString& args )
{
   d->classArgs = args;
}

//Private Utility Functions used by the two send() methods
QByteArray* KJavaProcess::addArgs( char cmd_code, const QStringList& args )
{
    //the buffer to store stuff, etc.
    QByteArray* buff = new QByteArray();
    QTextOStream output( *buff );
    char sep = 0;

    //make space for the command size: 8 characters...
    QCString space( "        " );
    output << space;

    //write command code
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

    return buff;
}

void KJavaProcess::storeSize( QByteArray* buff )
{
    int size = buff->size() - 8;  //subtract out the length of the size_str
    QString size_str = QString("%1").arg( size, 8 );
    kdDebug(6100) << "KJavaProcess::storeSize, size = " << size_str << endl;

    const char* size_ptr = size_str.latin1();
    for( int i = 0; i < 8; i++ )
        buff->at(i) = size_ptr[i];
}

void KJavaProcess::sendBuffer( QByteArray* buff )
{
    d->BufferList.append( buff );
    if( d->BufferList.count() == 1 )
    {
        popBuffer();
    }
}

void KJavaProcess::sendSync( char cmd_code, const QStringList& args ) {
    kdDebug(6100) << ">KJavaProcess::sendSync " << d->sync_count << endl;
    if (d->sync_count++ == 0)
        javaProcess->suspend();
    QByteArray* buff = addArgs( cmd_code, args );
    storeSize( buff );
    int dummy;
    int current_sync_count;
    int size = buff->size();
    char *data = buff->data();
    fd_set fds;
    timeval tv;
    do {
        FD_ZERO(&fds);
        FD_SET(in[1], &fds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        int retval = select(in[1]+1, 0L, &fds, 0L, &tv);
        FD_CLR(in[1], &fds);
        if (retval < 0 && errno == EINTR) {
            continue;
        } else if (retval <= 0) {
            kdError(6100) << "KJavaProcess::sendSync " << retval << endl;
            goto bail_out;
        } else if (KProcess::input_data) {
            KProcess::slotSendData(dummy);
        } else {
            int nr = ::write(in[1], data, size);
            size -= nr;
            data += nr;
        }
    } while (size > 0);
    current_sync_count = d->sync_count;
    do {
        FD_ZERO(&fds);
        FD_SET(out[0], &fds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        kdDebug(6100) << "KJavaProcess::sendSync bf read" << endl;
        int retval = select(out[0]+1, &fds, 0L, 0L, &tv);
        FD_CLR(out[0], &fds);
        if (retval < 0 && errno == EINTR) {
            continue;
        } else if (retval <= 0) {
            kdError(6100) << "KJavaProcess::sendSync timeout" <<endl;
            d->sync_count--;
            break;
        } else {
            slotReceivedData(out[0], dummy);
        }
        if (d->sync_count < current_sync_count)
            break;
    } while(true);
bail_out:
    delete buff;
    if (d->sync_count == 0)
        javaProcess->resume();
    kdDebug(6100) << "<KJavaProcess::sendSync " << d->sync_count << endl;
}

void KJavaProcess::syncCommandReceived() {
    if (--d->sync_count < 0) {
        kdError(6100) << "syncCommandReceived() sync_count below zero" << endl;
        d->sync_count = 0;
    }
}

void KJavaProcess::send( char cmd_code, const QStringList& args )
{
    if( isRunning() )
    {
        QByteArray* buff = addArgs( cmd_code, args );
        storeSize( buff );
        sendBuffer( buff );
    }
}

void KJavaProcess::send( char cmd_code, const QStringList& args,
                         const QByteArray& data )
{
    if( isRunning() )
    {
        kdDebug(6100) << "KJavaProcess::send, qbytearray is size = " << data.size() << endl;

        QByteArray* buff = addArgs( cmd_code, args );
        int cur_size = buff->size();
        int data_size = data.size();
        buff->resize( cur_size + data_size );
        memcpy( buff->data() + cur_size, data.data(), data_size );

        storeSize( buff );
        sendBuffer( buff );
    }
}

void KJavaProcess::popBuffer()
{
    QByteArray* buf = d->BufferList.first();
    if( buf )
    {
//        DEBUG stuff...
//	kdDebug(6100) << "Sending buffer to java, buffer = >>";
//        for( unsigned int i = 0; i < buf->size(); i++ )
//        {
//            if( buf->at(i) == (char)0 )
//                kdDebug(6100) << "<SEP>";
//            else if( buf->at(i) > 0 && buf->at(i) < 10 )
//                kdDebug(6100) << "<CMD " << (int) buf->at(i) << ">";
//            else
//                kdDebug(6100) << buf->at(i);
//        }
//        kdDebug(6100) << "<<" << endl;

        //write the data
        if ( !javaProcess->writeStdin( buf->data(),
                                       buf->size() ) )
        {
            kdError(6100) << "Could not write command" << endl;
        }
    }
}

void KJavaProcess::slotWroteData( )
{
    //do this here- we can't free the data until we know it went through
    d->BufferList.removeFirst();  //this should delete it since we setAutoDelete(true)
    kdDebug(6100) << "slotWroteData " << d->BufferList.count() << endl;

    if ( d->BufferList.count() >= 1 )
    {
        popBuffer();
    }
}


bool KJavaProcess::invokeJVM()
{
    
    *javaProcess << d->jvmPath;

    if( !d->classPath.isEmpty() )
    {
        *javaProcess << "-classpath";
        *javaProcess << d->classPath;
    }

    //set the system properties, iterate through the qmap of system properties
    for( QMap<QString,QString>::Iterator it = d->systemProps.begin();
         it != d->systemProps.end(); ++it )
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

    kdDebug(6100) << "Invoking JVM now...with arguments = " << endl;
    QString argStr;
    QTextOStream stream( &argStr );
    QValueList<QCString> args = javaProcess->args();
    qCopy( args.begin(), args.end(), QTextOStreamIterator<QCString>( stream, " " ) );
    kdDebug(6100) << argStr << endl;

    KProcess::Communication flags =  (KProcess::Communication)
                                     (KProcess::Stdin | KProcess::Stdout |
                                      KProcess::NoRead);

    bool rval = javaProcess->start( KProcess::NotifyOnExit, flags );
    if( rval )
        javaProcess->resume(); //start processing stdout on the java process

    return rval;
}

void KJavaProcess::killJVM()
{
   d->processKilled = true;
   javaProcess->kill();
}

/*  In this method, read one command and send it to the d->appletServer
 *  then return, so we don't block the event handling
 */
void KJavaProcess::slotReceivedData( int fd, int& )
{
    //read out the length of the message,
    //read the message and send it to the applet server
    char length[9] = { 0 };
    int num_bytes = ::read( fd, length, 8 );
    if( num_bytes == -1 )
    {
        kdError(6100) << "could not read 8 characters for the message length!!!!" << endl;
        return;
    }

    QString lengthstr( length );
    bool ok;
    int num_len = lengthstr.toInt( &ok );
    if( !ok )
    {
        kdError(6100) << "could not parse length out of: " << lengthstr << endl;
        return;
    }

    //now parse out the rest of the message.
    char* msg = new char[num_len];
    num_bytes = ::read( fd, msg, num_len );
    if( num_bytes == -1 ||  num_bytes != num_len )
    {
        kdError(6100) << "could not read the msg, num_bytes = " << num_bytes << endl;
        delete[] msg;
        return;
    }

    QByteArray qb;
    emit received( qb.duplicate( msg, num_len ) );
    delete[] msg;
}

void KJavaProcess::slotExited( KProcess *process )
{
  if (process && process == javaProcess) {
    int status = -1;
    if (!d->processKilled) {
     status = javaProcess->exitStatus();
    }
    kdDebug(6100) << "jvm exited with status " << status << endl; 
    emit exited(status);
  }
}

#include "kjavaprocess.moc"
