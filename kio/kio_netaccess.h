/*  This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
 
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

#include <qstring.h>
#include <qstringlist.h>

/** This class isn't meant to be used as a class but only as a simple 
 *  namespace for static functions, though an instance of the class
 *  is built for internal purposes.
 *  KIONetAccess implements the Net Transparency, which was formerly
 *  provided by kfmlib, but now using KIOJobs.
 *  Whereas a kiojob is asynchronous, meaning that the developer has
 *  to connect slots for it, KIONetAccess provides synchronous downloads
 *  and uploads, as well as temporary file creation and removal.
 *
 *  Port to kio done by David Faure, faure@kde.org
 */
class KIONetAccess : public QObject
{
  Q_OBJECT

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
     * (This is very comfortable, please see the example below.)
     *
     * Download is synchronous. That means you can use it like
     * this, (assuming u is a string which represents a URL and your
     * application has a loadFile function):
     *
     *       QString s;
     *       if (KIONetAccess::download(u, s)){
     *         loadFile(s);
     *         KIONetAccess::removeTempFile(s);
     *       }
     *
     * Of course your user interface will still process exposure/repaint
     * events during the download.
     *
     * (Matthias)
     */
    static bool download(const QString src, QString & target);

    /**
     * Remove the specified file if and only if it was created
     * by KIONetAccess as temporary file for a former download.
     * (Matthias)
     */
    static void removeTempFile(const QString name); 

    /**
     * Upload : TODO
     */
    static bool upload(const QString src, const QString target);

protected:
    /** Private constructor */
    KIONetAccess();
    /** Private destructor */
    ~KIONetAccess();
    /** Internal method */
    bool downloadInternal(const QString src, QString & target);
    /** List of temporary files */
    static QStringList* tmpfiles;
    /** Whether the download succeeded or not */
    bool bDownloadOk;

    QWidget * modalWidgetHack;

protected slots:
    virtual void slotFinished( int id );
    virtual void slotError( int id, int errid, const char * _text );
};

#endif
