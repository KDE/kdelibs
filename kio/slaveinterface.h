/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kio_slaveinterface_h
#define __kio_slaveinterface_h

#include <qobject.h>
#include <kurl.h>
#include <kio/global.h>

namespace KIO {

class Connection;

/**
 * Commands that can be invoked by a job.
 */
 enum Command {
   CMD_CONNECT = '0', //
   CMD_DISCONNECT,
   CMD_NONE = 'A',
   CMD_TESTDIR = 'B',
   CMD_GET = 'C',
   CMD_PUT = 'D',
   CMD_STAT = 'E',
   CMD_MIMETYPE = 'F',
   CMD_LISTDIR = 'G',
   CMD_MKDIR = 'H',
   CMD_RENAME = 'I',
   CMD_COPY = 'J',
   CMD_DEL = 'K',
   CMD_CHMOD = 'L',
   CMD_SPECIAL = 'M',
   CMD_USERPASS = 'N'
   // Add new ones here once a release is done, to avoid breaking binary compatibility.
   // Note that protocol-specific commands shouldn't be added here, but should use special.
 };

 /**
  * Identifiers for KIO informational messages.
  */
 enum Info {
   INF_TOTAL_SIZE = 10,
   INF_TOTAL_COUNT_OF_FILES,
   INF_TOTAL_COUNT_OF_DIRS,
   INF_PROCESSED_SIZE,
   INF_PROCESSED_COUNT_OF_FILES,
   INF_PROCESSED_COUNT_OF_DIRS,
   INF_SCANNING_DIR,
   INF_SPEED,
   INF_COPYING_FILE,
   INF_MAKING_DIR,
   INF_REDIRECTION = 20,
   INF_MIME_TYPE,
   INF_ERROR_PAGE,
   INF_WARNING,
   INF_GETTING_FILE,
   INF_DELETING_FILE,
   INF_NEED_PASSWD
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

 /**
  * Identifiers for KIO data messages.
  */
 enum Message {
   MSG_DATA = 100,
   MSG_DATA_REQ,
   MSG_ERROR,
   MSG_READY,
   MSG_CONNECTED,
   MSG_FINISHED,
   MSG_STAT_ENTRY,
   MSG_LIST_ENTRIES,
   MSG_RENAMED,
   MSG_RESUME
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

/**
 * There are two classes that specifies the protocol between application (job)
 * and kioslave. SlaveInterface is the class to use on the application end,
 * SlaveBase is the one to use on the slave end.
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class SlaveInterface : public QObject
{
    Q_OBJECT

public:
    SlaveInterface( Connection *connection );
    virtual ~SlaveInterface() { }

    // wrong IMHO (David)
    void setConnection( Connection* connection ) { m_pConnection = connection; }
    Connection *connection() const { return m_pConnection; }

 signals:
    ///////////
    // Messages sent by the slave
    ///////////

    void data( const QByteArray & );
    void dataReq( );
    void error( int , const QString & );
    void ready();
    void connected();
    void finished();
    void listEntries( const KIO::UDSEntryList& );
    void statEntry( const KIO::UDSEntry& );

    //void isDirectory() ;
    //void isFile() ;
    // probably obsolete ?
    void renamed( const QString & ) ;
    void canResume( bool ) ;

    ///////////
    // Info sent by the slave
    //////////

    // TODO : some cleanup (processed* at least)
    void totalSize( unsigned long ) ;
    void totalFiles( unsigned long ) ;
    void totalDirs( unsigned long ) ;
    void processedSize( unsigned long ) ;
    void processedFiles( unsigned long ) ;
    void processedDirs( unsigned long ) ;
    void scanningDir( const QString & ) ;
    void speed( unsigned long ) ;
    void makingDir( const QString & ) ;
    void redirection( const KURL& ) ;
    void errorPage() ;
    void mimeType( const QString & ) ;
    void warning( const QString & ) ;
    void gettingFile( const QString & ) ;
    void deletingFile( const QString & ) ;
    void connectFinished();

    /////////////////
    // Dispatching
    ////////////////

 protected:
    virtual bool dispatch();
    virtual void dispatch( int _cmd, const QByteArray &data );
    void openPassDlg( const QString& head, const QString& user, const QString& pass );

protected:
    Connection * m_pConnection;
};

};

inline QDataStream& operator>>(QDataStream& s, unsigned long& n)
{
    Q_INT32 hi, lo;
    s >> hi >> lo;
    n = hi;
    n = (n << 16) + lo;
    return s;
}

inline QDataStream& operator<<(QDataStream& s, signed long n)
{
    register unsigned long un = n;
    return (s << Q_INT32((un & 0xffff0000) >> 16) << Q_INT32(un & 0x0000ffff));
}

inline QDataStream& operator>>(QDataStream& s, signed long& n)
{
    Q_INT32 hi, lo;
    s >> hi >> lo;

    unsigned long un = hi;
    n = static_cast<signed long>((un << 16) + lo);
    return s;
}

inline QDataStream &operator >>(QDataStream &s, KIO::UDSAtom &a )
{
    s >> a.m_uds;

    if ( a.m_uds & KIO::UDS_LONG ) {
        s >> a.m_long;
        a.m_str = QString::null;
    } else if ( a.m_uds & KIO::UDS_STRING ) {
        s >> a.m_str;
        a.m_long = 0;
    } else {} // DIE!
    //    assert( 0 );

    return s;
}

inline QDataStream &operator <<(QDataStream &s, const KIO::UDSAtom &a )
{
    s << a.m_uds;

    if ( a.m_uds & KIO::UDS_LONG )
        s << a.m_long;
    else if ( a.m_uds & KIO::UDS_STRING )
        s << a.m_str;
    else {} // DIE!
    //    assert( 0 );

    return s;
}

#endif
