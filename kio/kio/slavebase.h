/*
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
#include <kconfigbase.h>
#include <kio/global.h>
#include <kio/authinfo.h>

class DCOPClient;

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
     * Terminate the slave by calling the destructor and then ::exit()
     */
    void exit();

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
     * Sends data in the slave to the job (i.e. in get).
     *
     * To signal end of data, simply send an empty
     * QByteArray().
     *
     * @param data the data read by the slave
     */
    void data( const QByteArray &data );

    /**
     * Asks for data from the job.
     * @see readData
     */
    void dataReq( );

    /**
     * Call to signal an error.
     * This also finishes the job, no need to call finished.
     *
     * If the Error code is KIO::ERR_SLAVE_DEFINED then the
     * _text should contain the complete translated text of
     * of the error message.  This message will be displayed
     * in an KTextBrowser which allows rich text complete
     * with hyper links.  Email links will call the default
     * mailer, "exec:/command arg1 arg2" will be forked and
     * all other links will call the default browser.
     *
     * @see KIO::Error
     * @see KTextBrowser
     * @param _errid the error code from KIO::Error
     * @param _text the rich text error message
     */
    void error( int _errid, const QString &_text );

    /**
     * Call in openConnection, if you reimplement it, when you're done.
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
     * Call this from stat() to express details about an object, the
     * UDSEntry customarily contains the atoms describing file name, size,
     * mimetype, etc.
     * @param _entry The UDSEntry containing all of the object attributes.
     */
    void statEntry( const UDSEntry& _entry );

    /**
     * Call this in listDir, each time you have a bunch of entries
     * to report.
     */
    void listEntries( const UDSEntryList& _entry );

    /**
     * Call this at the beginning of put(), to give the size of the existing
     * partial file, if there is one. The @p offset argument notifies the
     * other job (the one that gets the data) about the offset to use.
     * In this case, the boolean returns whether we can indeed resume or not
     * (we can't if the protocol doing the get() doesn't support setting an offset)
     */
    bool canResume( KIO::filesize_t offset );

    /*
     * Call this at the beginning of get(), if the "resume" metadata was set
     * and resuming is implemented by this protocol.
     */
    void canResume();

    ///////////
    // Info Signals to send to the job
    ///////////

    /**
     * Call this in @ref get and @ref copy, to give the total size
     * of the file
     * Call in @ref listDir too, when you know the total number of items.
     */
    void totalSize( KIO::filesize_t _bytes );
    /**
     * Call this during @ref get and @ref copy, once in a while,
     * to give some info about the current state.
     * Don't emit it in @ref listDir, @ref listEntries speaks for itself.
     */
    void processedSize( KIO::filesize_t _bytes );

    /**
     * Only use this if you can't know in advance the size of the
     * copied data. For example, if you're doing variable bitrate
     * compression of the source.
     *
     * STUB ! Currently unimplemented. Here now for binary compatibility.
     *
     * Call this during @ref get and @ref copy, once in a while,
     * to give some info about the current state.
     * Don't emit it in @ref listDir, @ref listEntries speaks for itself.
     */
    void processedPercent( float percent );

    /**
     * Call this in @ref get and @ref copy, to give the current transfer
     * speed, but only if it can't be calculated out of the size you 
     * passed to processedSize (in most cases you don't want to call it)
     */
    void speed( unsigned long _bytes_per_second );
  
    /**
     * Call this to signal a redirection
     * The job will take care of going to that url.
     */
    void redirection( const KURL &_url );

    /**
     * Tell that we will only get an error page here.
     * This means: the data you'll get isn't the data you requested,
     * but an error page (usually HTML) that describes an error.
     */
    void errorPage();

    /**
     * Call this in @ref mimetype, when you know the mimetype.
     * See @ref mimetype about other ways to implement it.
     */
    void mimeType( const QString &_type );

    /**
     * Call to signal a warning, to be displayed in a dialog box.
     */
    void warning( const QString &msg );

    /**
     * Call to signal a message, to be displayed if the application wants to,
     * for instance in a status bar. Usual examples are "connecting to host xyz", etc.
     */
    void infoMessage( const QString &msg );

    enum MessageBoxType { QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3, WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6 };

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
    int messageBox( MessageBoxType type, const QString &text,
                    const QString &caption = QString::null,
                    const QString &buttonYes = QString::null,
                    const QString &buttonNo = QString::null );

    /**
     * Sets meta-data to be send to the application before the first
     * data() or finished() signal.
     */
    void setMetaData(const QString &key, const QString &value);

    /**
     * Queries for the existance of a certain config/meta-data entry
     * send by the application to the slave.
     */
    bool hasMetaData(const QString &key);

    /**
     * Queries for config/meta-data send by the application to the slave.
     */
    QString metaData(const QString &key);

    /**
     * Returns a configuration object to query config/meta-data information
     * from.
     *
     * The application provides the slave with all configuration information
     * relevant for the current protocol and host.
     */
    KConfigBase* config();

    ///////////
    // Commands sent by the job, the slave has to
    // override what it wants to implement
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
     * When this function gets called the slave is operating in
     * connection-oriented mode.
     * When a connection gets lost while the slave operates in
     * connection oriented mode, the slave should report
     * ERR_CONNECTION_BROKEN instead of reconnecting.
     */
    virtual void openConnection();

    /**
     * Closes the connection (forced)
     * Called when the application disconnects the slave to close
     * any open network connections.
     *
     * When the slave was operating in connection-oriented mode,
     * it should reset itself to connectionless (default) mode.
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
     * @param overwrite if true, any existing file will be overwritten.
     * If the file indeed already exists, the slave should NOT apply the
     * permissions change to it.
     * @param resume
     */
    virtual void put( const KURL& url, int permissions, bool overwrite, bool resume );

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
     * Used for multiple get. Currently only used foir HTTP pielining 
     * support.
     *
     * @param data packed data; Contains number of URLs to fetch, and for
     * each URL the URL itself and its associated MetaData.
     */
    virtual void multiGet( const QByteArray & );

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
     * @return timeout value for connecting to remote host.
     */
    int connectTimeout();

    /**
     * @return timeout value for connecting to proxy in secs.
     */
    int proxyConnectTimeout();

    /**
     * @return timeout value for read from first data from
     * remote host in seconds.
     */
    int responseTimeout();

    /**
     * @return timeout value for read from subsequent data from
     * remote host in secs.
     */
    int readTimeout();

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
     * Checks whether the password daemon kdesud is
     * running or if it can be started if it is not.
     *
     * @return true if password daemon is/can be started successfully.
     */
    bool pingCacheDaemon() const;

    /**
     * Prompt the user for Authorization info (login & password).
     *
     * Use this function to request authorization info from the
     * the end user. You can also pass an errorMsg which explains
     * why a previous authorisation attempt failed.
     * For example to open an empty password dialog
     * using default values:
     *
     * <pre>
     * KIO::AuthInfo authInfo;
     * if ( openPassDlg( authInfo ) )
     * {
     *    printf( "Username: %s", authInfo.username.latin1() );
     *    printf( "Password: %s", authInfo.password.latin1() );
     * }
     * </pre>
     *
     * You can also pre-set some values like the username before hand
     * if it is known as well as the comment and caption to be displayed:
     *
     * <pre>
     * KIO::AuthInfo authInfo;
     * authInfo.caption= "Acme Password Dialog";
     * authInfo.username= "Wile E. Coyote";
     * QString errorMsg = "You entered an incorrect password.";
     * if ( openPassDlg( authInfo, errorMsg ) )
     * {
     *    printf( "Username: %s", authInfo.username.latin1() );
     *    printf( "Password: %s", authInfo.password.latin1() );
     * }
     * </pre>
     *
     * NOTE: A call to this function can also fail and result
     * in a return value of @p false, if the UIServer could not
     * be started for whatever reason.
     *
     * @param info  See @ref AuthInfo.
     * @param errorMsg Error message to show
     * @return      @p TRUE if user clicks on "OK", @p FALSE otherwsie.
     */
    bool openPassDlg( KIO::AuthInfo& info, const QString &errorMsg );

    bool openPassDlg( KIO::AuthInfo& info );

    /**
     * Checks for cached authentication based on parameters
     * given by @p info.
     *
     * Use this function to check if any cached password exists
     * for the URL given by @p info.  If @p AuthInfo::realmValue
     * is present and/or the @p AuthInfo::verifyPath flag is set,
     * then they will also be factored in determining the presence
     * of a cached password.  Note that @p Auth::url is a required
     * parameter when attempting to check for cached authorization
     * info. Here is a simple example:
     *
     * <pre>
     * AuthInfo info;
     * info.url = KURL("http://www.foobar.org/foo/bar");
     * info.username = "somename";
     * info.verifyPath = true;
     * if ( !checkCachedAuthentication( info ) )
     * {
     *    if ( !openPassDlg(info) )
     *     ....
     * }
     * </pre>
     *
     * If the protocol allows multiple resources within the same
     * location to be protected by different passwords, then to
     * determine the correct password and send pre-emtively, i.e.
     * before the other end requires it, you can use one or both
     * of the following methods: set the unique identifier using
     * @p AuthInfo::realmValue or require that a path match be
     * performed using @p AuthInfo::verifyPath.
     *
     * <pre>
     * info.url = KURL("http://www.foobar.org/foo/bar");
     * info.verifyPath = true;
     * info.realmValue = "unique_identifier";
     * </pre>
     *
     * NOTE: A call to this function will fail and return false,
     * whenever the "kdesud" could not be started for whatever reason
     * or an invalid URL is supplied.
     *
     * @param       See @ref AuthInfo.
     * @return      @p TRUE if cached Authorization is found, false otherwise.
     */
    bool checkCachedAuthentication( AuthInfo& info );

    /**
     * Initializes the cookiejar if it is not running already
     * and returns true if successful, i.e. the cookiejar started
     * okay or is currently running.
     *
     * The cookiejar is used for HTTP cookies and password storage.
     */
    bool initCookieJar();

    /**
     * @obsolete
     *
     * Cache authentication information is now stored automatically
     * by openPassDlg.
     */
    bool cacheAuthentication( const AuthInfo& info );

    /**
     * Creates a basic key to be used to cache the password.
     *
     * @param url   the url from which the key is supposed to be generated
     */
    QString createAuthCacheKey( const KURL& url );

    /**
     * @obsolete
     *
     * Cache authentication information is now stored automatically
     * by openPassDlg.
     */
    void sendAuthenticationKey( const QCString& gKey, const QCString& key, bool keep );

    /**
     * @obsolete
     *
     * Cache authentication information is now stored automatically
     * by openPassDlg.
     */
    void delCachedAuthentication( const QString& key );

    /**
     * Setup support for multiple auth-info caching
     * to a single server.
     *
     * Calling this function with the argument set to @p true
     * will allow a user to work on multiple resources located
     * under different accounts but on the same server without
     * being re-prompted for authorization each time. Simply put
     * if you have a "foo" and a "bar" account on a given machine
     * at "foobar.com" and you log into this machine using both of
     * the accounts, then the authorization information you supplied
     * for both accounts will be cached. This is also true if you
     * have N number of accounts and you logged into all of them.
     * Otherwise, the default behavior is for the latest login will
     * simply overwrite the previous one.
     *
     * @param enable if true allow multiple auth-info caching.
     */
    void setMultipleAuthCaching( bool enable );

    /**
     * @return true if multiple auth-info caching is enabled.
     */
    bool multipleAuthCaching() const;

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
     * Return the dcop client used by this slave.
     */
    DCOPClient *dcopClient();

    /**
     * Wait for an answer to our request, until we get @p expected1 or @p expected2
     * @return the result from readData, as well as the cmd in *pCmd if set, and the data in @p data
     */
    int waitForAnswer( int expected1, int expected2, QByteArray & data, int * pCmd = 0 );

    /**
     * Internal function to transmit meta data to the application.
     */
    void sendMetaData();

    /**
     * Name of the protocol supported by this slave
     */
    QCString mProtocol;

    Connection * m_pConnection;

    MetaData mOutgoingMetaData;
    MetaData mIncomingMetaData;

    /** If your ioslave was killed by a signal, wasKilled() returns true.
     Check it regularly in lengthy functions (e.g. in get();) and return
     as fast as possible from this function if wasKilled() returns true.
     This will ensure that your slave destructor will be called correctly.
     */
    bool wasKilled() const;
    /** Internally used.
     */
    void setKillFlag();
private:
    bool storeAuthInfo( const QCString&, const QCString&, const AuthInfo& );

    UDSEntryList pendingListEntries;
    uint listEntryCurrentSize;
    long listEntry_sec, listEntry_usec;
    Connection *appconn;
    QString mPoolSocket;
    QString mAppSocket;
    bool mConnectedToApp;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    SlaveBasePrivate *d;
};

};

#endif
