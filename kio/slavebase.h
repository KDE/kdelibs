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

#ifndef __slavebase_h
#define __slavebase_h

#include <kurl.h>
#include "kio/global.h"

namespace KIO {

class Connection;
class SlaveBasePrivate;

/**
 * There are two classes that specifies the protocol between application (job)
 * and kioslave. SlaveInterface is the class to use on the application end,
 * SlaveBase is the one to use on the slave end.
 *
 * Slave implementations should simply inherit SlaveBase
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class SlaveBase
{
public:
    SlaveBase( const QCString &protocol, const QCString &pool_socket, const QCString &app_socket);
    virtual ~SlaveBase();

    /**
     * @internal
     */
    void dispatchLoop();

    /**
     * @internal
     */
    void setConnection( Connection* connection ) { m_pConnection = connection; }
    /**
     * @internal
     */
    Connection *connection() const { return m_pConnection; }


    ///////////
    // Message Signals to send to the job
    ///////////

    /**
     * Call to send data in the slave (i.e. in get).
     * @param data the data read by the slave
     * Send an empty QByteArray() to signal end of data.
     */
    void data( const QByteArray &data );

    /**
     * Call to ask for data (in put)
     * @see readData
     */
    void dataReq( );

    /**
     * Call to signal an error.
     * This also finishes the job, no need to call finished.
     */
    void error( int _errid, const QString &_text );

    /**
     * Call in openConnection, if you reimplement it, when you're done.
     * Currently unused.
     */
    void connected();

    /**
     * Call to signal successful completion of any command
     * (besides openConnection and closeConnection)
     */
    void finished();

    /**
     * Call to signal that data from the sub-URL is needed
     */
    void needSubURLData();

    /**
     * Used to report the status of the slave.
     * @param host the slave is currently connected to. (Should be
     *        empty if not connected)
     * @param connected Whether an actual network connection exists.
     **/
    void slaveStatus(const QString &host, bool connected);

    /**
     * Call this once in stat()
     */
    void statEntry( const UDSEntry& _entry );

    /**
     * Call this in listDir, each time you have a bunch of entries
     * to report.
     */
    void listEntries( const UDSEntryList& _entry );

    /**
     * ???? Is this still necessary?
     */
    void canResume( bool _resume );

    ///////////
    // Info Signals to send to the job
    ///////////

    /**
     * Call this in @ref get and @ref copy, to give the total size
     * of the file
     * Call in @ref listDir too, when you know the total number of items.
     */
    void totalSize( unsigned long _bytes );
    /**
     * Call this during @ref get and @ref copy, once in a while,
     * to give some info about the current state.
     * Don't emit it in @ref listDir, @ref listEntries speaks for itself.
     */
    void processedSize( unsigned long _bytes );
    /**
     * Call this in @ref get and @ref copy, to give the current transfer
     * speed. Usually worked out as processed_size / ( t - t_start )
     */
    void speed( unsigned long _bytes_per_second );

    /**
     * Call this to signal a redirection
     * The job will take care of going to that url.
     */
    void redirection( const KURL &_url );

    /**
     * "Tell that we will only get an error page here." ?  ## FIXME
     */
    void errorPage();

    /**
     * Call this in @ref mimetype, when you know the mimetype.
     * See @ref mimetype about other ways to implement it.
     */
    void mimeType( const QString &_type );

    void gettingFile( const QString &_file ); // probably obsolete ?

    /**
     * Call to signal a warning, to be displayed in a dialog box.
     */
    void warning( const QString &msg );

    /**
     * Call to signal a message, to be displayed if the application wants to,
     * for instance in a status bar. Usual examples are "connecting to host xyz", etc.
     */
    void infoMessage( const QString &msg );

    /**
     * Call this when requesting for a login and password.
     *
     * @param head and i18n'ed message to explain the dialog box
     * @param user user name, in and out
     * @param pass password, in and out
     * @param key the string to be used to cache the password.
     * This can be for example the host, so that any authorization
     * can kept around for a whole session, or realm etc.
     *
     * @return true on ok, false on cancel
     */
    bool openPassDlg( const QString& /*head*/, QString& /*user*/, QString& /*pass*/, const QString& /*key*/ = QString::null );

    enum { QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3, WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6 };
    /**
     * Call this to show a message box from the slave (it will in fact be handled
     * by kio_uiserver, so that the progress info dialog for the slave is hidden
     * while this message box is shown)
     * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
     * @param text Message string. May contain newlines.
     * @param caption Message box title.
     * @param buttonYes The text for the first button.
     *                  The default is i18n("&Yes").
     * @param buttonNo  The text for the second button.
     *                  The default is i18n("&No").
     * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
     *       and for Information, none is used.
     * @return a button code, as defined in KMessageBox, or 0 on communication error.
     */
    int messageBox( int type, const QString &text,
                    const QString &caption = QString::null,
                    const QString &buttonYes = QString::null,
                    const QString &buttonNo = QString::null );

    /**
     * Sets meta-data to be send to the application before the first
     * data() or finished() signal.
     */
    void setMetaData(const QString &key, const QString &value);

    /**
     * Queries for meta-data send by the application to the slave.
     */
    QString metaData(const QString &key);

    ///////////
    // Commands sent by the job, the slave has to override what it wants to implement
    ///////////

    /**
     * Set the host
     * @param host
     * @param port
     * @param user
     * @param pass
     * Called directly by createSlave, this is why there is no equivalent in
     * SlaveInterface, unlike the other methods.
     *
     * This method is called whenever a change in host, port or user occurs.
     */
    virtual void setHost(const QString& host, int port, const QString& user, const QString& pass);

    /**
     * Prepare slave for streaming operation
     */
    virtual void setSubURL(const KURL&url);

    /**
     * Opens the connection (forced)
     * Currently unused.
     */
    virtual void openConnection();

    /**
     * Closes the connection (forced)
     * Currently unused.
     */
    virtual void closeConnection();

    /**
     * get, aka read.
     * @param url the full url for this request. Host, port and user of the URL
     *        can be assumed to be the same as in the last setHost() call.
     * The slave emits the data through @ref data
     */
    virtual void get( const KURL& url );

    /**
     * put, aka write.
     * @param path where to write the file (decoded)
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     * @param resume
     */
    virtual void put( const KURL& url, int permissions, bool overwrite, bool resume);

    /**
     * Finds all details for one file or directory.
     * The information returned is the same as what @ref listDir returns,
     * but only for one file or directory.
     */
    virtual void stat( const KURL& url );

    /**
     * Finds mimetype for one file or directory.
     *
     * This method should either emit 'mimeType' or it
     * should send a block of data big enough to be able
     * to determine the mimetype.
     *
     * If the slave doesn't reimplement it, a @ref get will
     * be issued, i.e. the whole file will be downloaded before
     * determining the mimetype on it - this is obviously not a
     * good thing in most cases.
     */
    virtual void mimetype( const KURL& url );

    /**
     * Lists the contents of @p path.
     * The slave should emit ERR_CANNOT_ENTER_DIRECTORY if it doesn't exist,
     * if we don't have enough permissions, or if it is a file
     * It should also emit @ref totalFiles as soon as it knows how many
     * files it will list.
     */
    virtual void listDir( const KURL& url );

    /**
     * Create a directory
     * @param path path to the directory to create
     * @param permissions the permissions to set after creating the directory
     * (-1 if no permissions to be set)
     * The slave emits ERR_COULD_NOT_MKDIR if failure.
     */
    virtual void mkdir( const KURL&url, int permissions );

    /**
     * Rename @p oldname into @p newname.
     * If the slave returns an error ERR_UNSUPPORTED_ACTION, the job will
     * ask for copy + del instead.
     * @param src where to move the file from
     * @param dest where to move the file to
     * @param overwrite if true, any existing file will be overwritten
     */
    virtual void rename( const KURL& src, const KURL& dest, bool overwrite );

    /**
     * Creates a symbolic link named @p dest, pointing to @p target, which
     * may be a relative or an absolute path.
     * @param target The string that will become the "target" of the link (can be relative)
     * @param dest The symlink to create.
     * @param overwrite whether to automatically overwrite if the dest exists
     */
    virtual void symlink( const QString& target, const KURL& dest, bool overwrite );

    /**
     * Change permissions on @p path
     * The slave emits ERR_DOES_NOT_EXIST or ERR_CANNOT_CHMOD
     */
    virtual void chmod( const KURL& url, int permissions );

    /**
     * Copy @p src into @p dest.
     * If the slave returns an error ERR_UNSUPPORTED_ACTION, the job will
     * ask for get + put instead.
     * @param src where to copy the file from (decoded)
     * @param dest where to copy the file to (decoded)
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     *
     */
    virtual void copy( const KURL &src, const KURL &dest, int permissions, bool overwrite );

    /**
     * Delete a file or directory.
     * @param path file/directory to delete
     * @param isfile if true, a file should be deleted.
     *               if false, a directory should be deleted.
     */
    virtual void del( const KURL &url, bool isfile);

    /**
     * Used for any command that is specific to this slave (protocol)
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param data packed data; the meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     * Document your slave's commands, at least in its header file.
     */
    virtual void special( const QByteArray & );

    /**
     * Called to get the status of the slave. Slave should respond
     * by calling slaveStatus(...)
     */
    virtual void slave_status();

    /**
     * Called by the scheduler to tell the slave that the configuration
     * changed (i.e. proxy settings) .
     */
    virtual void reparseConfiguration();

    /**
     * @internal
     */
    static void sigsegv_handler(int);
    /**
     * @internal
     */
    static void sigpipe_handler(int);

    /////////////////
    // Dispatching (internal)
    ////////////////

    /**
     * @internal
     */
    virtual bool dispatch();
    /**
     * @internal
     */
    virtual void dispatch( int command, const QByteArray &data );

    /**
     * Read data send by the job, after a @ref dataReq
     *
     * @param buffer buffer where data is stored
     * @return 0 on end of data,
     *         > 0 bytes read
     *         < 0 error
     **/
    int readData( QByteArray &buffer );

protected:
    /**
     * internal function to be called by the slave.
     * It collects entries and emits them via listEntries
     * when enough of them are there or a certain time
     * frame exceeded (to make sure the app gets some
     * items in time but not too many items one by one
     * as this will cause a drastic performance penalty)
     * @param ready set to true after emitting all items. _entry is not
     *        used in this case
     */
    void listEntry( const UDSEntry& _entry, bool ready);

    /**
     * internal function to connect a slave to/ disconnect from
     * either the slave pool or the application
     */
    void connectSlave(const QString& path);
    void disconnectSlave();

    /**
     * internal function that slaves use to see if there is any cached
     * authentication entries
     */
    bool checkCachedAuthentication(QString& user, QString& passwd, int& auth_type, QString& valid_path, const QString& realm);

    /**
     * internal function to store authentication entries
     */
    void cacheAuthentication(const KURL& url, const QString& user,
                             const QString& password, int auth_type);

    /**
     * Used by the slave to check if it can connect
     * to a given host. This should be called where the slave is ready
     * to do a ::connect() on a socket. For each call to @ref
     * requestNetwork must exist a matching call to
     * @ref dropNetwork, or the system will stay online until
     * KNetMgr gets closed (or the SlaveBase gets destructed)!
     *
     * If KNetMgr is not running, then this is a no-op and returns true
     *
     * @param host tells the netmgr the host the slave wants to connect
     *             to. As this could also be a proxy, we can't just take
     *             the host currenctly connected to (but that's the default
     *             value)
     *
     * @return true in theorie, the host is reachable
     *         false the system is offline and the host is in a remote network.
     */
    bool requestNetwork(const QString& host = QString::null);

    /**
     * Used by the slave to withdraw a connection requested by
     * @ref requestNetwork. This function cancels the last call to
     * @ref requestNetwork. If a client uses more than one internet
     * connection, it must use dropNetwork(host) to
     * stop each request.
     *
     * If KNetMgr is not running, then this is a no-op.
     *
     * @param host the host passed to requestNetwork
     *
     * A slave should call this function every time it disconnect from a host.
     * */
    void dropNetwork(const QString& host = QString::null);

    /**
     * Wait for an answer to our request, until we get @p expected1 or @p expected2
     * @return the cmd, and the data in @p data
     */
    int waitForAnswer( int expected1, int expected2, QByteArray & data );

protected:
    /**
     * Name of the protocol supported by this slave
     */
    QCString mProtocol;

    Connection * m_pConnection;

    /**
     * Internal function to transmit meta data to the application.
     */
    void sendMetaData();

private:
    UDSEntryList pendingListEntries;
    uint listEntryCurrentSize;
    long listEntry_sec, listEntry_usec;
    Connection *appconn;
    QString mPoolSocket;
    QString mAppSocket;
    MetaData mOutgoingMetaData;
    MetaData mIncomingMetaData;
    bool mConnectedToApp;
    SlaveBasePrivate *d;
};

// by kahl for netmgr
class SlaveBasePrivate {
public:
    QString slaveid;
};

};

#endif
