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
#include <kio/authinfo.h>
#include <kdatastream.h>

namespace KIO {

class Connection;
// better there is one ...
class SlaveInterfacePrivate;

/**
 * Commands that can be invoked by a job.
 */
 enum Command {
   CMD_HOST = '0',
   CMD_CONNECT = '1',
   CMD_DISCONNECT = '2',
   CMD_SLAVE_STATUS = '3',
   CMD_SLAVE_CONNECT = '4',
   CMD_SLAVE_HOLD = '5',
   CMD_NONE = 'A', // 65
   CMD_TESTDIR = 'B', // 66
   CMD_GET = 'C', // 67
   CMD_PUT = 'D', // 68
   CMD_STAT = 'E', // 69
   CMD_MIMETYPE = 'F', // 70
   CMD_LISTDIR = 'G', // 71
   CMD_MKDIR = 'H', // 72
   CMD_RENAME = 'I', // 73
   CMD_COPY = 'J', // 74
   CMD_DEL = 'K', // 75
   CMD_CHMOD = 'L', // 76
   CMD_SPECIAL = 'M', // 77
   CMD_USERPASS = 'N', // 78
   CMD_REPARSECONFIGURATION = 'O', // 79
   CMD_META_DATA = 'P', // 80
   CMD_SYMLINK = 'Q', // 81
   CMD_SUBURL = 'R', // 82  Inform the slave about the url it is streaming on.
   CMD_MESSAGEBOXANSWER = 'S', // 83
   CMD_RESUMEANSWER = 'T', // 84
   CMD_CONFIG = 'U', // 85
   CMD_MULTI_GET = 'V' // 86
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
   INF_MIME_TYPE = 21,
   INF_ERROR_PAGE = 22,
   INF_WARNING = 23,
   INF_GETTING_FILE, // Deprecated
   INF_NEED_PASSWD = 25,
   INF_INFOMESSAGE,
   INF_META_DATA,
   INF_NETWORK_STATUS,
   INF_MESSAGEBOX
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
   MSG_SLAVE_ACK,
   MSG_NET_REQUEST,
   MSG_NET_DROP,
   MSG_NEED_SUBURL_DATA,
   MSG_CANRESUME,
   MSG_AUTH_KEY, // deprecated.
   MSG_DEL_AUTH_KEY // deprecated.
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

/**
 * There are two classes that specifies the protocol between application 
 * (@ref KIO::Job) and kioslave. SlaveInterface is the class to use on the application 
 * end, SlaveBase is the one to use on the slave end.
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class SlaveInterface : public QObject
{
    Q_OBJECT

public:
    SlaveInterface( Connection *connection );
    virtual ~SlaveInterface();

    void setConnection( Connection* connection ) { m_pConnection = connection; }
    Connection *connection() const { return m_pConnection; }

    void setProgressId( int id ) { m_progressId = id; }
    int progressId() const { return m_progressId; }

    // Send our answer to the MSG_RESUME (canResume) request
    // (to tell the "put" job whether to resume or not)
    void sendResumeAnswer( bool resume );

    void setOffset( KIO::filesize_t offset );
    KIO::filesize_t offset() const;

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
    void needSubURLData();
    void needProgressId();

    void canResume( KIO::filesize_t ) ;

    ///////////
    // Info sent by the slave
    //////////
    void metaData( const KIO::MetaData & );
    void totalSize( KIO::filesize_t ) ;
    void processedSize( KIO::filesize_t ) ;
    void redirection( const KURL& ) ;

    void speed( unsigned long ) ;
    void errorPage() ;
    void mimeType( const QString & ) ;
    void warning( const QString & ) ;
    void infoMessage( const QString & ) ;
    void connectFinished();

    /**
     * @deprecated. Obselete as of 3.1. Replaced by a kpassword, a kded module.
     */     
    void authorizationKey( const QCString&, const QCString&, bool );

    /**
     * @deprecated. Obselete as of 3.1. Replaced by a kpassword, a kded module.
     */
    void delAuthorization( const QCString& grpkey );

protected:
    /////////////////
    // Dispatching
    ////////////////

    virtual bool dispatch();
    virtual bool dispatch( int _cmd, const QByteArray &data );

   /**
    * Prompt the user for authrization info (login & password).
    *
    * Use this function to request authorization info from the
    * the end user. For example to open an empty password dialog
    * using default values:
    *
    * <pre>
    * KIO::AuthInfo authInfo;
    * bool result = openPassDlg( authInfo );
    * if ( result )
    * {
    *    printf( "Username: %s", result.username.latin1() );
    *    printf( "Username: %s", result.username.latin1() );
    * }
    * </pre>
    *
    * You can also pre-set some values like the username before hand
    * if it is known as well as the comment and caption to be displayed:
    *
    * <pre>
    * authInfo.comment= "Enter username and password to access acmeone";
    * authInfo.caption= "Acme Password Dialog";
    * authInfo.username= "Wily E. kaiody";
    * bool result = openPassDlg( authInfo );
    * if ( result )
    * {
    *    printf( "Username: %s", result.username.latin1() );
    *    printf( "Username: %s", result.username.latin1() );
    * }
    * </pre>
    *
    * NOTE: A call to this function can also fail and result
    * in a return value of @p false, if the UIServer could not
    * be started for whatever reason.
     *
     * @param       info See @ref AuthInfo.
     * @return      TRUE if user clicks on "OK", FALSE otherwsie.
     */
    void openPassDlg( KIO::AuthInfo& info );

   /**
    * @deprecated. Use @ref openPassDlg( AuthInfo& ) instead.
    */
    void openPassDlg( const QString& prompt, const QString& user,
                      const QString& caption, const QString& comment,
                      const QString& label, bool readOnly );

   /**
    * @deprecated. Use @ref openPassDlg( AuthInfo& ) instead.
    */
    void openPassDlg( const QString& prompt, const QString& user, bool readOnly );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo );

    // I need to identify the slaves
    void requestNetwork( const QString &, const QString &);
    void dropNetwork( const QString &, const QString &);

    /**
     * @internal
     */
    static void sigpipe_handler(int);

protected slots:
    void calcSpeed();
 
protected:
    Connection * m_pConnection;

private:
    int m_progressId;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    SlaveInterfacePrivate *d;
};

};

inline QDataStream &operator >>(QDataStream &s, KIO::UDSAtom &a )
{
    long l;
    s >> a.m_uds;

    if ( a.m_uds & KIO::UDS_LONG ) {
        s >> l;
        a.m_long = l;
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
        s << (long) a.m_long;
    else if ( a.m_uds & KIO::UDS_STRING )
        s << a.m_str;
    else {} // DIE!
    //    assert( 0 );

    return s;
}

QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e );
QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e );

#endif
