// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

// $Id$

#ifndef KDIR_H
#define KDIR_H

#include "kfileinfo.h"

#include <sys/types.h> // To be safe
#include <dirent.h>

#include <qdir.h>
#include <qobject.h>
#include <qstrlist.h>

#include <kurl.h>

class KIOJob;

/*
** KDir - URL aware directory interface
**
*/

/**
 * @short A URL aware directory object.
 */
class KDir : public QObject
{
    Q_OBJECT

public:
    KDir();
    KDir (const KDir &);
    
    KDir(const QString& path,
	 const QString& nameFilter= QString::null,
	 QDir::SortSpec sortSpec = 
	 static_cast<QDir::SortSpec>(QDir::Name | QDir::IgnoreCase),
	 QDir::FilterSpec filterSpec= QDir::All);
    KDir(const QDir &);
    virtual ~KDir();
    
    KDir &operator= (const KDir &);
    KDir &operator= (const QDir &);
    
    /**
      * @param path or url.
      */
    KDir &operator= (const QString&);
    
    /**
      * Set the current path (accepts normal paths or urls)
      */
    virtual void setPath(const QString&);
    
    /**
      * Change to the parent directory
      */
    void cdUp();
    
    /**
      * Returns the current host.
      */
    QString host() const { return myLocation.host(); }
    
    /**
      * Returns the current protocol.
      */
    QString protocol() const { return myLocation.protocol(); }

    /**
      * Returns the current path (or the path part of the url).
      */
    QString path() const {  return myLocation.path(); }
    
    /**
      * Set the current url (accepts normal paths or urls).
      */
    void setURL(const QString&);
    
    /**
      * Returns the current url.
      */
    QString url() const {  return myLocation.url(); }
    
    /**
      * Returns the current name filter.
      */
    const QString& nameFilter() const { return myNameFilter; }
    
    /**
      * Set the current name filter.
      */
    void setNameFilter(const QString&);
    
    /**
      * Returns the current filter.
      */
    QDir::FilterSpec filter() const {  return myFilterSpec; }
    
    /**
      * Set the current filter.
      */
    void setFilter(int);
    
    /**
      * Returns the current sort order.
      */
    QDir::SortSpec sorting() const { return mySortSpec; }
    
    /**
      * Set the current sort order.
      */
    void setSorting(int);
    
    /**
      * Returns the number of directory entries read.
   */
    uint count();
    
    /**
      * Returns true if KFM has finished the operation.
      */
    bool isFinished() const { return (myJob == 0); }
    
    /**
      * Return a list of KFileInfo objects matching the specified filter.
      * This call returns immediately, but may provide only partial
      * information. If isFinished() returns true then you will receive
      * complete information. If this is the contents of the directory are
      * not cached then this method will call startLoading().
      *
      * In blocking I/O mode, it will only return the first maxReturn 
      * (default 100) entries. The rest will be send through signals
      */
    const KFileInfoList *entryInfoList(int filterSpec = QDir::DefaultFilter,
				       int sortSpec = QDir::DefaultSort);
    
    /**
      * Returns true if the name matches the filter. Unlike the match() method
      * provided by QDir, KDir allows you to specify more than one glob in the
      * pattern. For example the pattern '*.cpp *.h' is valid.
      */
    static bool match(const QString& filter, const QString& name);
    
    /**
      * sets, if the class should get the data via blocking I/O.
      * This is just useful, if you want to override the internal logic
      */
    void setBlocking(bool _block);

    /**
      * returns, if the class gets its data via blocking I/O
      */
    bool blocking() { return isBlocking; }

    static void setMaxReturns(uint max);

    /**      
      * indicates, if the path is empty
      **/
    bool isRoot() { return root; }
    
    /**
      * indicates, if the path is readable. That means, if there are
      * entries to expect
      **/
    bool isReadable() { return readable; }

signals:
    /**
      * Emitted when the url is changed
      */
    void urlChanged(const QString&);
    
    /**
      * Emitted when a directory entry that matches the filter is received.
      */
    void dirEntry(KFileInfo *);
    
    /**
      * Emitted when the directory has been completely loaded.
      */
    void finished();
    
    /**
      * Emitted if a network transfer goes wrong.
      */
    void error(int, const QString&);

    /**
      * Emitted, if new files are queued
      **/
    void newFilesArrived(const KFileInfoList *);

protected slots:
 
    /**
      * Called when a directory entry is received.
      */
    void slotListEntry(int id, const UDSEntry&);
    
    /**
      * Called when kfm has finished the current network operation.
      */
    void slotKfmFinished();
    
    void slotKfmError(int, int _errid, const char *_txt );

    void timerEvent();

    bool matchAllDirs() const { return myMatchDirs; }

    void setMatchAllDirs( bool f ) { myMatchDirs = f; }

protected:
    void updateFiltered();
    bool filterEntry(KFileInfo *);
    void initLists();
    virtual void getEntries();
    /**
      * Start listing the directory in the background (returns immeditately).
      * @return true if KFM was started successfully.
      */
    virtual bool startLoading();

private:
    KURL myLocation;
    QString myNameFilter;
    QDir::SortSpec mySortSpec;
    QDir::FilterSpec myFilterSpec;
    KIOJob *myJob;
    // Raw entries (we cache these for performance)
    KFileInfoList myEntries;
    bool myDirtyFlag;
    // Filtered entries (what you actually get)
    KFileInfoList myFilteredEntries;
    // contains up to maxReturn entries (queueing)
    KFileInfoList myTmpEntries;
    QStrList myFilteredNames;
    bool myFilteredDirtyFlag;
    bool isBlocking;
    bool root;
    static uint maxReturns;
    // if the directory is to big, we must divide it
    DIR *myOpendir; 
    uint readFiles;
    bool readable;
    bool myMatchDirs;
};

#endif // KDIR_H
