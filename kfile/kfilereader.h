// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998,1999,2000 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
		  1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KDIR_H
#define KDIR_H

#include <sys/types.h>
#include <dirent.h>

#include <qdict.h>
#include <qdir.h>
#include <qobject.h>
#include <qstring.h>
#include <qregexp.h>
#include <qlist.h>
#include <qtimer.h>
#include <kdirwatch.h>
#include <kurl.h>
#include <kio/global.h>

#include "kfileviewitem.h"

namespace KIO {
class Job;
}
class QStrList;
class KDirWatch;
class KURL;

class KFileReader : public QObject, public KURL
{
    Q_OBJECT

public:
    KFileReader();
    KFileReader(const KURL& url,
		const QString& nameFilter= QString::null);
    ~KFileReader();

    KFileReader &operator= (const QString& url);

    /**
     * Set the current url.
     * An url without a protocol (file:/, ftp://) will be treated as a
     * local directory and file:/ will be prepended.
     */
    virtual void setURL(const KURL&);

    /**
     * Set the current name filter.
     */
    void setNameFilter(const QString&);

    /**
     * Returns the number of directory entries expected.
     */
    uint dirCount() const;

    /**
     * Returns the number of directory entries read.
     */
    uint count() const;

    /**
     * Returns true if KFM has finished the operation.
     */
  //    bool isFinished() const { return (myJob == 0); }

    /**
     * Initiates reading the current directory or url. Returns the contents
     * as a list of KFileViewItem objects matching the specified filter by
     * emitting the signal @ref contents. If we are working on a local
     * directory this will be a blocking readdir(). If we are working on a
     * remote url, we use KIO to list the contents.
     * The contents() signal may be emitted more than once to supply you with
     * KFileViewItems, up until the signal @ref finished is emitted
     * (and isFinished() returns true).
     * If the contents are already cached, you will get the entire contents
     * with just one contents() signal.
     */
    void listContents();

    /**
     * This gives you access to the current list of items. It is not
     * guaranteed that this list is complete (e.g. during asynchronous reading
     * of remote directories. Use only if really necessary. Normally, you
     * should just use @ref listContents and get the list via the signal
     * contents. Never returns 0L.
     */
    const KFileViewItemList & currentContents() const { return myEntries; }

    /**
     * Returns true if the name matches the filter.
     */
    bool match(const QString& name) const;

    /**
     * Indicates if the path is empty
     **/
    bool isRoot() const { return root; }

    /**
     * Indicates if the path is readable. That means, if there are
     * entries to expect
     **/
    bool isReadable() const { return readable; }

    void setShowHiddenFiles(bool b);

    bool showHiddenFiles() const { return showHidden; }

    /**
     * Enables/disables automatic updates, when a directory is changed
     * (works only for local files, using @ref KDirWatch). New files will be
     * emitted via @ref newFilesArrived, deleted files with @ref filesDeleted
     * Default is enabled.
     */
    virtual void setAutoUpdate( bool b );
    bool autoUpdate() const { return myAutoUpdate; }

 signals:
    /**
     * Emitted when the url is changed. It is NOT emitted, when you manually
     * call setURL().
     */
    void urlChanged(const QString&);

    /**
     * Emitted when a directory entry that matches the filter is received.
     */
    void dirEntry(KFileViewItem *);

    /**
     * Emitted when the directory has been completely loaded.
     */
  //    void finished();

    /**
     * Emitted if a network transfer goes wrong.
     */
    void error(int, const QString&);

    /**
     * Emitted, if new files are queued
     **/
    void contents(const KFileViewItemList &, bool ready);

    void itemsDeleted(const KFileViewItemList &);

    void dirDeleted();

    void filterChanged();

protected slots:

    /**
     * Called when a directory entry is received.
     */
    void slotEntries(KIO::Job*, const KIO::UDSEntryList&);

    /**
     * Called when Job has finished the current network operation.
     */
    void slotIOFinished( KIO::Job * );

    void slotDirDirty(const QString& dir);
    void slotDirDeleted(const QString& dir);
    void slotDirUpdate();

protected:
    void updateFiltered();
    bool filterEntry(KFileViewItem *);
    void init();

    virtual void getEntries();

    /*
     * Start listing the directory in the background (returns immeditately).
     */
    virtual void startLoading();

    KIO::Job *myJob;
    KDirWatch *myDirWatch;

private:
    KFileReader(const KFileReader&);
    KFileReader& operator=(const KFileReader&);

    QList<QRegExp> filters;
    // Raw entries (we cache these for performance)
    KFileViewItemList myEntries, myNewEntries, myUpdateList;
    KFileViewItemList myPendingEntries;
    QTimer *myUpdateTimer;

    bool myDirtyFlag;
    bool myAutoUpdate;
    bool root;

    bool readable;
    bool showHidden;

    // for KDirWatch update (async via QTimer)
    DIR *myUpdateDir;

    long int currentSize;

private:
    class KFileReaderPrivate;
    KFileReaderPrivate *d;

};


#endif // KDIR_H
