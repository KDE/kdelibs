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
    virtual ~SlaveBase() { }

    void dispatchLoop();

    void setConnection( Connection* connection ) { m_pConnection = connection; }
    Connection *connection() const { return m_pConnection; }

    ///////////
    // Message Signals to send to the job
    ///////////

    /**
     * Emit to send data in the slave (i.e. in get).
     * @param data the data read by the slave
     * Send an empty QByteArray() to signal end of data.
     */
    void data( const QByteArray &data );

    /**
     * Emit to ask for data (in put)
     */
    void dataReq( );

    /**
     * Emit to signal an error. 
     * This also finishes the job, no need to call finished.
     */
    void error( int _errid, const QString &_text );

    /**
     * Emit in openConnection, if you reimplement it, when you're done.
     */
    void connected();

    /**
     * Emit to signal successful completion of any command
     * (besides openConnection and closeConnection)
     */
    void finished();
    
    /**
     * Used to report the status of the slave. 
     * @param host the slave is currently connected to. (Should be
     *        empty if not connected)
     * @param connected Whether an actual network connection exists.
     **/
    void slaveStatus(const QString &host, bool connected);

    /**
     * Emit this once in stat()
     */
    void statEntry( const UDSEntry& _entry );

    /**
     * Emit this in listDir, each time you have a bunch of entries
     * to report.
     */
    void listEntries( const UDSEntryList& _entry );

    /**
     * ???? Is this still necessary?
     */
    void renamed( const QString &_new );

    /**
     * ???? Is this still necessary?
     */
    void canResume( bool _resume );

    ///////////
    // Info Signals to send to the job (most are OLD)
    ///////////

    void totalSize( unsigned long _bytes );
    void totalFiles( unsigned int _files );
    void totalDirs( unsigned int _dirs );
    void processedSize( unsigned long _bytes );
    void scanningDir( const KURL& _dir );
    void speed( unsigned long _bytes_per_second );
    // bool copyingFile( const KURL&_from, const KURL&_to );
    // bool makingDir( const KURL&_dir );
    void redirection( const KURL &_url );
    void errorPage();
    void mimeType( const QString &_type );
    void gettingFile( const QString &_type );
    void warning( const QString &msg );
    bool openPassDlg( const QString& head, QString& user, QString& pass );

    ///////////
    // Commands sent by the job, the slave has to override what it wants to implement
    ///////////

    /**
     * Set the host
     * @param host
     * @param user
     * @param pass
     * Called directly by createSlave, this is why there is no equivalent in
     * SlaveInterface, unlike the other methods.
     */
    virtual void setHost(const QString& host, int port, const QString& user, const QString& pass);

    /**
     * Opens the connection (forced)
     */
    virtual void openConnection();

    /**
     * Closes the connection (forced)
     */
    virtual void closeConnection();

    /**
     * get, aka read.
     * @param path the file to retrieve (decoded)
     * @param query an optionnal query (the part after the '?' in the URL)
     * @param reload if true, make sure to get an up to date version
     * The slave emits the data through @ref data
     */
    virtual void get( const QString& path, const QString& query, bool reload );

    /**
     * put, aka write.
     * @param path where to write the file (decoded)
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten
     * @param resume
     */
    virtual void put( const QString& path, int permissions, bool overwrite, bool resume);

    /**
     * Finds all details for one file or directory.
     * The information returned is the same as what @ref listDir returns,
     * but only for one file or directory.
     */
    virtual void stat( const QString & );

    /**
     * Finds mimetype for one file or directory.
     *
     * This method should either emit 'mimetype' or it
     * should send a block of data big enough to be able
     * to determine the mimetype.
     */
    virtual void mimetype( const QString& path );

    /**
     * Lists the contents of @p path.
     * The slave should emit ERR_CANNOT_ENTER_DIRECTORY if it doesn't exist,
     * if we don't have enough permissions, or if it is a file
     */
    virtual void listDir( const QString& path );

    /**
     * Create a directory
     * @param path path to the directory to create
     * @param permissions the permissions to set after creating the directory
     * (-1 if no permissions to be set)
     * The slave emits ERR_COULD_NOT_MKDIR if failure.
     */
    virtual void mkdir( const QString& path, int permissions );

    /**
     * Rename @p oldname into @p newname.
     * If the slave returns an error ERR_UNSUPPORTED_ACTION, the job will
     * ask for copy + del instead.
     * @param src where to move the file from (decoded)
     * @param dest where to move the file to (decoded)
     * @param overwrite if true, any existing file will be overwritten
     */
    virtual void rename( const QString& src, const QString& dest, bool overwrite );

    /**
     * Change permissions on @p path
     * The slave emits ERR_DOES_NOT_EXIST or ERR_CANNOT_CHMOD
     */
    virtual void chmod( const QString& path, int permissions );

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
    virtual void copy( const QString &src, const QString &dest, int permissions, bool overwrite );

    /**
     * Delete a file or directory.
     * @param path file/directory to delete
     * @param isfile if true, a file should be deleted.
     *               if false, a directory should be deleted.
     */
    virtual void del( const QString &path, bool isfile);

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
     * @internal
     */
    static void sigsegv_handler(int);

    /////////////////
    // Dispatching (internal)
    ////////////////

    virtual bool dispatch();
    virtual void dispatch( int command, const QByteArray &data );

    /**
     * Read data send by the job
     *
     * @param buffer buffer where data is stored
     * @return 0 on end of data,
     *         > 0 bytes read
     *         < 0 error
     **/
    int readData( QByteArray &buffer);

protected:
    /**
     * internal function to be called by the slave.
     * It collects entries and emits them via listEntries
     * when enough of them are there or a certain time
     * frame exceeded (to make sure the app gets some
     * items in time but not too many items one by one
     * as this will cause a drastic performance penalty
     */
    void listEntry( const UDSEntry& _entry, bool ready);

    /**
     * internal function to connect a slave to/ disconnect from
     * either the slave pool or the application
     */
    void connectSlave(const QString& path);
    void disconnectSlave();

    /**
     * Name of the protocol supported by this slave
     */
    QCString mProtocol;

    Connection * m_pConnection;
private:
    UDSEntryList pendingListEntries;
    uint listEntryCurrentSize;
    long listEntry_sec, listEntry_usec;
    Connection *appconn;
    QString mPoolSocket;
    QString mAppSocket;
    bool mConnectedToApp;
};

};

#endif
