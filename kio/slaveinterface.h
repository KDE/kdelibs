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

#include <unistd.h>
#include <sys/types.h>

#include <qobject.h>

#include <kurl.h>

#include <kio/global.h>
#include <kdatastream.h>

namespace KIO {

class Connection;

/**
 * Commands that can be invoked by a job.
 */
 enum Command {
   CMD_HOST = '0',
   CMD_CONNECT = '1',
   CMD_DISCONNECT = '2',
   CMD_SLAVE_STATUS = '3',
   CMD_SLAVE_CONNECT = '4',
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
   CMD_USERPASS = 'N',
   CMD_REPARSECONFIGURATION = 'O'
   // Add new ones here once a release is done, to avoid breaking binary compatibility.
   // Note that protocol-specific commands shouldn't be added here, but should use special.
 };

 /**
  * Identifiers for KIO informational messages.
  */
 enum Info {
   INF_TOTAL_SIZE = 10,
   INF_PROCESSED_SIZE = 11,
   INF_SPEED,
   INF_REDIRECTION = 20,
   INF_MIME_TYPE,
   INF_ERROR_PAGE,
   INF_WARNING,
   INF_GETTING_FILE, // ?
   INF_NEED_PASSWD,
   INF_INFOMESSAGE
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

 /**
  * Identifiers for KIO data messages.
  */
 enum Message {
   MSG_DATA = 100,
   MSG_DATA_REQ,
   MSG_ERROR,
   MSG_CONNECTED,
   MSG_FINISHED,
   MSG_STAT_ENTRY,
   MSG_LIST_ENTRIES,
   MSG_RENAMED, // unused
   MSG_RESUME,
   MSG_SLAVE_STATUS,
   MSG_SLAVE_ACK
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
    void connected();
    void finished();
    void slaveStatus(pid_t, const QCString &, const QString &, bool);
    void listEntries( const KIO::UDSEntryList& );
    void statEntry( const KIO::UDSEntry& );

    void canResume( bool ) ;

    ///////////
    // Info sent by the slave
    //////////

    void totalSize( unsigned long ) ;
    void processedSize( unsigned long ) ;
    void redirection( const KURL& ) ;

    void speed( unsigned long ) ;
    void errorPage() ;
    void mimeType( const QString & ) ;
    void warning( const QString & ) ;
    void gettingFile( const QString & ) ; // ?
    void infoMessage( const QString & ) ;
    void connectFinished();

protected:
    /////////////////
    // Dispatching
    ////////////////

    virtual bool dispatch();
    virtual void dispatch( int _cmd, const QByteArray &data );

    /**
     * Call this when requesting for a login and password.
     *
     * This function is only different from the above only because
     * it takes one more extra argument, the name of the host, so
     * that any authorization can kept around for a whole session.
     *
     * @param head and i18n'ed message to explain the dialog box
     * @param user user name, in and out
     * @param pass password, in and out
     * @param key the string to be used to cache the password.
     *
     * @return true on ok, false on cancel
     */
    void openPassDlg( const QString& /*head*/, const QString& /*user*/, const QString& /*pass*/, const QString& /*key*/ = QString::null );

    /**
     * @internal
     */
    static void sigpipe_handler(int);

protected:
    Connection * m_pConnection;
};

};

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
