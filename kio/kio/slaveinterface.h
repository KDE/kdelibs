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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kio_slaveinterface_h
#define __kio_slaveinterface_h

#include <unistd.h>
#include <sys/types.h>

#include <QtCore/QObject>
#include <QtNetwork/QHostInfo>

#include <kio/global.h>
#include <kio/udsentry.h>
#include <kio/authinfo.h>

class KUrl;

namespace KIO {

class Connection;
// better there is one ...
class SlaveInterfacePrivate;

  // Definition of enum Command has been moved to global.h

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
   INF_UNUSED = 25, // now unused
   INF_INFOMESSAGE,
   INF_META_DATA,
   INF_NETWORK_STATUS,
   INF_MESSAGEBOX,
   INF_POSITION
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
   MSG_STAT_ENTRY, // 105
   MSG_LIST_ENTRIES,
   MSG_RENAMED, // unused
   MSG_RESUME,
   MSG_SLAVE_STATUS,
   MSG_SLAVE_ACK, // 110
   MSG_NET_REQUEST,
   MSG_NET_DROP,
   MSG_NEED_SUBURL_DATA,
   MSG_CANRESUME,
   MSG_AUTH_KEY, // 115 // deprecated.
   MSG_DEL_AUTH_KEY, // deprecated.
   MSG_OPENED,
   MSG_WRITTEN,
   MSG_HOST_INFO_REQ
   // add new ones here once a release is done, to avoid breaking binary compatibility
 };

/**
 * There are two classes that specifies the protocol between application
 * ( KIO::Job) and kioslave. SlaveInterface is the class to use on the application
 * end, SlaveBase is the one to use on the slave end.
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class KIO_EXPORT SlaveInterface : public QObject
{
    Q_OBJECT

protected:
    SlaveInterface(SlaveInterfacePrivate &dd, QObject *parent = 0);
public:
    virtual ~SlaveInterface();

    void setConnection( Connection* connection );
    Connection *connection() const;

    // Send our answer to the MSG_RESUME (canResume) request
    // (to tell the "put" job whether to resume or not)
    void sendResumeAnswer( bool resume );

    void setOffset( KIO::filesize_t offset );
    KIO::filesize_t offset() const;

    /**
     * Returns the top level window used as parent when displaying
     * dialogs.
     *
     * @see setWindow
     * @since 4.8.2
     */
    QWidget* window() const;

    /**
     * Sets the top level window used as a parent when displaying
     * dialogs.
     * @see window
     * @since 4.8.2
     */
    void setWindow(QWidget* window);

Q_SIGNALS:
    ///////////
    // Messages sent by the slave
    ///////////

    void data( const QByteArray & );
    void dataReq( );
    void error( int , const QString & );
    void connected();
    void finished();
    void slaveStatus(pid_t, const QByteArray&, const QString &, bool);
    void listEntries( const KIO::UDSEntryList& );
    void statEntry( const KIO::UDSEntry& );
    void needSubUrlData();

    void canResume( KIO::filesize_t );

    void open();
    void written( KIO::filesize_t );

    ///////////
    // Info sent by the slave
    //////////
    void metaData( const KIO::MetaData & );
    void totalSize( KIO::filesize_t );
    void processedSize( KIO::filesize_t );
    void redirection( const KUrl& );
    void position( KIO::filesize_t );

    void speed( unsigned long );
    void errorPage();
    void mimeType( const QString & );
    void warning( const QString & );
    void infoMessage( const QString & );
    //void connectFinished(); //it does not get emitted anywhere

protected:
    /////////////////
    // Dispatching
    ////////////////

    virtual bool dispatch();
    virtual bool dispatch( int _cmd, const QByteArray &data );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo );

    void messageBox( int type, const QString &text, const QString &caption,
                     const QString &buttonYes, const QString &buttonNo,
                     const QString &dontAskAgainName );

    // I need to identify the slaves
    void requestNetwork( const QString &, const QString &);
    void dropNetwork( const QString &, const QString &);

protected Q_SLOTS:
    void calcSpeed();
protected:
    SlaveInterfacePrivate* const d_ptr;
    Q_DECLARE_PRIVATE(SlaveInterface)
private:
    Q_PRIVATE_SLOT(d_func(), void slotHostInfo(QHostInfo))
};

}

// moved to udesentry.cpp!!!
// KIO_EXPORT QDataStream &operator <<(QDataStream &s, const KIO::UDSEntry &e );
// KIO_EXPORT QDataStream &operator >>(QDataStream &s, KIO::UDSEntry &e );

#endif
