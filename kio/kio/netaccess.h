/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999-2004 David Faure (faure@kde.org)

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

#ifndef __kio_netaccess_h
#define __kio_netaccess_h

#include <qobject.h>
#include <qstring.h>
#include <kio/global.h>
#include <kurl.h>

class QStringList;
class QWidget;

template<typename T, typename K> class QMap;

namespace KIO {

  class Job;

  /**
   * Net Transparency.
   *
   * NetAccess allows you to do simple file operation (load, save,
   * copy, delete...) without working with KIO::Job directly.
   * Whereas a KIO::Job is asynchronous, meaning that the
   * developer has to connect slots for it, KIO::NetAccess provides
   * synchronous downloads and uploads, as well as temporary file
   * creation and removal. The functions appear to be blocking,
   * but the Qt event loop continues running while the operations
   * are handled. This means that the GUI will not freeze.
   *
   * This class isn't meant to be used as a class but only as a simple
   * namespace for static functions, though an instance of the class
   * is built for internal purposes.
   *
   * Port to kio done by David Faure, faure@kde.org
   *
   * @short Provides an easy, synchronous interface to KIO file operations.
   */
class KIO_EXPORT NetAccess : public QObject
{
  Q_OBJECT

public:
    /**
     * Downloads a file from an arbitrary URL (@p src) to a
     * temporary file on the local filesystem (@p target).
     *
     * If the argument
     * for @p target is an empty string, download will generate a
     * unique temporary filename in /tmp. Since @p target is a reference
     * to QString you can access this filename easily. Download will
     * return true if the download was successful, otherwise false.
     *
     * Special case:
     * If the URL is of kind file:, then no downloading is
     * processed but the full filename is returned in @p target.
     * That means you @em have to take care about the @p target argument.
     * (This is very easy to do, please see the example below.)
     *
     * Download is synchronous. That means you can use it like
     * this, (assuming @p u is a string which represents a URL and your
     * application has a loadFile() function):
     *
     * \code
     * QString tmpFile;
     * if( KIO::NetAccess::download( u, tmpFile, window ) )
     * {
     *   loadFile( tmpFile );
     *   KIO::NetAccess::removeTempFile( tmpFile );
     * } else {
     *   KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
     * }
     * \endcode
     *
     * Of course, your user interface will still process exposure/repaint
     * events during the download.
     *
     * If the download fails, lastError() and lastErrorString() will be set.
     *
     * @param src URL Reference to the file to download.
     * @param target String containing the final local location of the
     *               file.  If you insert an empty string, it will
     *               return a location in a temporary spot. <B>Note:</B>
     *               you are responsible for the removal of this file when
     *               you are finished reading it using removeTempFile.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure.  Use lastErrorString() to
     *         get the reason it failed.
     *
     * @see lastErrorString()
     */
    static bool download(const KUrl& src, QString & target, QWidget* window);

    /**
     * Removes the specified file if and only if it was created
     * by KIO::NetAccess as a temporary file for a former download.
     *
     * Note: This means that if you created your temporary with KTempFile,
     * use KTempFile::unlink() or KTempFile::setAutoDelete() to have
     * it removed.
     *
     * @param name Path to temporary file to remove.  May not be
     *             empty.
     */
    static void removeTempFile(const QString& name);

    /**
     * Uploads file @p src to URL @p target.
     *
     * Both must be specified, unlike download.
     * Note that this is assumed to be used for saving a file over
     * the network, so overwriting is set to true. This is not the
     * case with copy.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     */
    static bool upload(const QString& src, const KUrl& target, QWidget* window);

    /**
     * Alternative to upload for copying over the network.
     * Overwrite is false, so this will fail if @p target exists.
     *
     * This one takes two URLs and is a direct equivalent
     * of KIO::file_copy (not KIO::copy!).
     * It will be renamed file_copy in KDE4, so better use file_copy.
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     *
     * @return true if successful, false for failure
     */
    static bool copy( const KUrl& src, const KUrl& target, QWidget* window = 0L );
    // KDE4: rename to file_copy

    /**
     * Full-fledged equivalent of KIO::file_copy
     */
    static bool file_copy( const KUrl& src, const KUrl& dest, int permissions=-1,
                            bool overwrite=false, bool resume=false, QWidget* window = 0L );

    /**
     * Full-fledged equivalent of KIO::file_move.
     * Moves or renames *one file*.
     * @since 3.2
     */
    static bool file_move( const KUrl& src, const KUrl& target, int permissions=-1,
                           bool overwrite=false, bool resume=false, QWidget* window = 0L );


    /**
     * Alternative method for copying over the network.
     * Overwrite is false, so this will fail if @p target exists.
     *
     * This one takes two URLs and is a direct equivalent
     * of KIO::copy!.
     * This means that it can copy files and directories alike
     * (it should have been named copy()).
     *
     * @param src URL Referencing the file to upload.
     * @param target URL containing the final location of the
     *               file.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be cached
     *               only for a short duration after which the user will again be
     *               prompted for passwords as needed.
     * @return true if successful, false for failure
     */
    static bool dircopy( const KUrl& src, const KUrl& target, QWidget* window );

    /**
     * Overloaded method, which takes a list of source URLs
     */
    static bool dircopy( const KUrl::List& src, const KUrl& target, QWidget* window = 0L );

    /**
     * Full-fledged equivalent of KIO::move.
     * Moves or renames one file or directory.
     * @since 3.2
     */
    static bool move( const KUrl& src, const KUrl& target, QWidget* window = 0L );

    /**
     * Full-fledged equivalent of KIO::move.
     * Moves or renames a list of files or directories.
     * @since 3.2
     */
    static bool move( const KUrl::List& src, const KUrl& target, QWidget* window = 0L );

    /**
     * Tests whether a URL exists.
     *
     * @param url the URL we are testing
     * @param source if true, we want to read from that URL.
     *               If false, we want to write to it.
     * IMPORTANT: see documentation for KIO::stat for more details about this.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if the URL exists and we can do the operation specified by
     *              @p source, false otherwise
     * @since 3.2
     */
    static bool exists(const KUrl& url, bool source, QWidget* window);

    /**
     * Tests whether a URL exists and return information on it.
     *
     * This is a convenience function for KIO::stat
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The URL we are testing.
     * @param entry The result of the stat. Iterate over the list
     * of atoms to get hold of name, type, size, etc., or use KFileItem.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true if successful, false for failure
     */
    static bool stat(const KUrl& url, KIO::UDSEntry & entry, QWidget* window);


    /**
     * Tries to map a local URL for the given URL.
     *
     * This is a convenience function for KIO::stat + parsing the
     * resulting UDSEntry.
     *
     * @param url The URL we are testing.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return a local URL corresponding to the same ressource than the
     *         original URL, or the original URL if no local URL can be mapped
     * @since 3.5
     */
    static KUrl mostLocalURL(const KUrl& url, QWidget* window);

    /**
     * Deletes a file or a directory in a synchronous way.
     *
     * This is a convenience function for KIO::del
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The file or directory to delete.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return true on success, false on failure.
     */
    static bool del( const KUrl & url, QWidget* window );

    /**
     * Creates a directory in a synchronous way.
     *
     * This is a convenience function for @p KIO::mkdir
     * (it saves creating a slot and testing for the job result).
     *
     * @param url The directory to create.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @param permissions directory permissions.
     * @return true on success, false on failure.
     */
    static bool mkdir( const KUrl & url, QWidget* window, int permissions = -1 );

    /**
     * Executes a remote process via the fish ioslave in a synchronous way.
     *
     * @param url The remote machine where the command should be executed.
     *            e.g. fish://someuser\@somehost:sshport/
     *            some special cases exist.
     *            fish://someuser\@localhost/
     *            will use su instead of ssh to connect and execute the command.
     *            fish://someuser\@localhost:port/
     *            will use ssh to connect and execute the command.
     * @param command The command to be executed.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The resulting output of the @p command that is executed.
     */
    static QString fish_execute( const KUrl & url, const QString command, QWidget* window );

    /**
     * This function executes a job in a synchronous way.
     * If a job fetches some data, pass a QByteArray pointer as data parameter to this function
     * and after the function returns it will contain all the data fetched by this job.
     *
     * <code>
     * KIO::Job *job = KIO::get( url, false, false );
     * QMap<QString, QString> metaData;
     * metaData.insert( "PropagateHttpHeader", "true" );
     * if ( NetAccess::synchronousRun( job, 0, &data, &url, &metaData ) ) {
     *   QString responseHeaders = metaData[ "HTTP-Headers" ];
     *   kdDebug()<<"Response header = "<< responseHeaders << endl;
     * }
     * </code>
     *
     * @param job job which the function will run. Note that after this function
     *            finishes running, job is deleted and you can't access it anymore!
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @param data if passed and relevant to this job then it will contain the data
     *               that was fetched by the job
     * @param finalURL if passed will contain the final url of this job (it might differ
     *                 from the one it was created with if there was a redirection)
     * @param metaData you can pass a pointer to the map with meta data you wish to
     *                 set on the job. After the job finishes this map will hold all the
     *                 meta data from the job.
     *
     * @return true on success, false on failure.
     *
     * @since 3.4
     */
    static bool synchronousRun( Job* job, QWidget* window, QByteArray* data=0,
                                KUrl* finalURL=0, QMap<QString,QString>* metaData=0 );

    /**
     * @internal
     * This function is not implemented!?
     * (only mimetypeInternal)
     *
     * Determines the mimetype of a given URL.
     *
     * This is a convenience function for KIO::mimetype.  You
     * should call this only when really necessary.
     * KMimeType::findByURL can determine extension a lot faster, but
     * less reliably for remote files. Only when findByURL() returns
     * unknown (application/octet-stream) then this one should be
     * used.
     *
     * @param url The URL whose mimetype we are interested in.
     * @param window main window associated with this job. This is used to
     *               automatically cache and discard authentication information
     *               as needed. If NULL, authentication information will be
     *               cached only for a short duration after which the user will
     *               again be prompted for passwords as needed.
     * @return The mimetype name.
     */
    static QString mimetype( const KUrl & url, QWidget* window );


    /**
     * Returns the error string for the last job, in case it failed.
     * Note that this is already translated.
     * @return the last error string, or QString()
     */
    static QString lastErrorString() { return lastErrorMsg ? *lastErrorMsg : QString(); }

    /**
     * Returns the error code for the last job, in case it failed.
     * @return the last error code
     * @since 3.3
     */
    static int lastError() { return lastErrorCode; }

Q_SIGNALS:
    void leaveModality();
private:
    /**
     * Private constructor
     */
    NetAccess() : m_metaData(0), d(0) {}

    /**
     * Private destructor
     */
    ~NetAccess() {}

    /**
     * Internal methods
     */
    bool filecopyInternal(const KUrl& src, const KUrl& target, int permissions,
                          bool overwrite, bool resume, QWidget* window, bool move);
    bool dircopyInternal(const KUrl::List& src, const KUrl& target,
                         QWidget* window, bool move);
    bool statInternal(const KUrl & url, int details, bool source, QWidget* window = 0);

    bool delInternal(const KUrl & url, QWidget* window = 0);
    bool mkdirInternal(const KUrl & url, int permissions, QWidget* window = 0);
    QString fish_executeInternal(const KUrl & url, const QString command, QWidget* window = 0);
    bool synchronousRunInternal( Job* job, QWidget* window, QByteArray* data,
                                 KUrl* finalURL, QMap<QString,QString>* metaData );

    QString mimetypeInternal(const KUrl & url, QWidget* window = 0);
    void enter_loop();

    /**
     * List of temporary files
     */
    static QStringList* tmpfiles;

    static QString* lastErrorMsg;
    static int lastErrorCode;

    friend class I_like_this_class;

private Q_SLOTS:
    void slotResult( KIO::Job * job );
    void slotMimetype( KIO::Job * job, const QString & type );
    void slotData( KIO::Job*, const QByteArray& );
    void slotRedirection( KIO::Job*, const KUrl& );

private:
    UDSEntry m_entry;
    QString m_mimetype;
    QByteArray m_data;
    KUrl m_url;
    QMap<QString, QString> *m_metaData;

    /**
     * Whether the download succeeded or not
     */
    bool bJobOK;

private:
    class NetAccessPrivate* const d;//notreallyneeded,thectorisprivatealready.
};

}

#endif
