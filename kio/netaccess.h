/*  $Id$

    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#ifndef __kio_netaccess_h
#define __kio_netaccess_h

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>
#include <kio/global.h>

namespace KIO {

  class Job;

/** This class isn't meant to be used as a class but only as a simple
 *  namespace for static functions, though an instance of the class
 *  is built for internal purposes.
 *  KIO::NetAccess implements the Net Transparency, which was formerly
 *  provided by kfmlib, but now using KIO::Jobs.
 *  Whereas a kiojob is asynchronous, meaning that the developer has
 *  to connect slots for it, KIO::NetAccess provides synchronous downloads
 *  and uploads, as well as temporary file creation and removal.
 *
 *  Port to kio done by David Faure, faure@kde.org
 *
 * @short Provides a synchronous interface to io jobs.
 */
class NetAccess : public QObject
{
  Q_OBJECT

public:
    /**
     * This function downloads a file from an arbitrary URL (source) to a
     * temporary file on the local filesystem (target). If the argument
     * for target is an empty string, download will generate a
     * unique temporary filename in /tmp. Since target is a reference
     * to QString you can access this filename easily. Download will
     * return true if the download was successful, otherwise false.
     *
     * Special case:
     * If the url is of kind "file:" then no downloading is
     * processed but the full filename returned in target.
     * That means: you _have_ to take care about the target argument.
     * (This is very easy to do, please see the example below.)
     *
     * Download is synchronous. That means you can use it like
     * this, (assuming u is a string which represents a URL and your
     * application has a loadFile function):
     *
     * <pre>
     * QString tmpFile;
     * if( KIO::NetAccess::download( u, tmpFile ) )
     * {
     *   loadFile( tmpFile );
     *   KIO::NetAccess::removeTempFile( tmpFile );
     *   }
     * </pre>
     *
     * Of course, your user interface will still process exposure/repaint
     * events during the download.
     *
     * @param src URL referencing the file to download
     *
     * @param target String containing the final local location of the
     *               file.  If you insert an empty string, it will
     *               return a location in a temporary spot
     */
    static bool download(const KURL& src, QString & target);

    /**
     * Remove the specified file if and only if it was created
     * by KIO::NetAccess as temporary file for a former download.
     *
     * @param name Path to temporary file to remove.  May not be
     *             empty.
     */
    static void removeTempFile(const QString& name);

    /**
     * Upload file src to url target.  Both must be specified,
     * unlike @ref download.
     *
     * @param src URL referencing the file to upload
     *
     * @param target URL containing the final location of the
     *               file.
     */
    static bool upload(const QString& src, const KURL& target);

    /**
     * Alternative method for copying over the network
     * This one takes two URLs and is a direct equivalent
     * of KIO::file_copy (not KIO::copy!).
     */
    static bool copy( const KURL& src, const KURL& target );

    /**
     * Test whether a url exists
     * This is a convenience function for KIO::stat
     * (it saves creating a slot and testing for the job result)
     *
     * @param url the url we are testing
     */
    static bool exists(const KURL& url);

    /**
     * Test whether a url exists and returns information on it
     * This is a convenience function for KIO::stat
     * (it saves creating a slot and testing for the job result)
     *
     * @param url the url we are testing
     * @param entry the result of the stat. Iterate over the list
     * of atoms to get hold of name, type, size, etc., or use KFileItem.
     */
    static bool stat(const KURL& url, KIO::UDSEntry & entry);

    /**
     * Delete a file or a directory in an synchronous way
     * This is a convenience function for KIO::del
     * (it saves creating a slot and testing for the job result)
     * @param url the file or directory to delete
     * @return true on success, false on failure
     */
    static bool del( const KURL & url );

    /**
     * Create a directory in an synchronous way
     * This is a convenience function for KIO::mkdir
     * (it saves creating a slot and testing for the job result)
     * @param url the directory to create
     * @return true on success, false on failure
     */
    static bool mkdir( const KURL & url, int permissions = -1 );


    /**
     * Determines the mimetype of a given URL.
     * This is a convenience function for KIO::mimetype.
     * You should call this only when really necessary.
     * KMimeType::findByURL can determine extension a lot faster, but less reliably
     * for remote files. Only when findByURL returns unknown (application/octet-stream)
     * then this one should be used.
     *
     * @param url the URL whose mimetype we are interested in
     * @return the mimetype name
     */
    static QString mimetype( const KURL & url );

private:
    /** Private constructor */
    NetAccess() {}
    /** Private destructor */
    ~NetAccess() {}
    /** Internal methods */
    bool copyInternal(const KURL& src, const KURL& target, bool overwrite);
    bool statInternal(const KURL & url);
    UDSEntry m_entry;
    bool delInternal(const KURL & url);
    bool mkdirInternal(const KURL & url, int permissions);
    QString mimetypeInternal(const KURL & url);
    QString m_mimetype;
    /** List of temporary files */
    static QStringList* tmpfiles;
    /** Whether the download succeeded or not */
    bool bJobOK;

private slots:
    void slotResult( KIO::Job * job );
    void slotMimetype( KIO::Job * job, const QString & type );
};

};

#endif
