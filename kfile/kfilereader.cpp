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

#include "kfileviewitem.h"
#include <qdir.h>
#include <qfileinfo.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#ifdef HAVE_FSTAB_H
#include <fstab.h>
#endif

#include <qapplication.h>
#include <qtimer.h>
#include <qstringlist.h>
#include <qpixmap.h>

#include "kfilereader.h"
#include <kio/job.h>
#include <kdebug.h>
#include <config-kfile.h>

template class QList<QRegExp>;

/*
** KFileReader - URL aware directory operator
**
*/

static bool probably_slow_mounted(const char *filename);

KFileReader::KFileReader()
    : QObject(0, "KFileReader")
{
    init();
    setURL(QDir::currentDirPath());
}

KFileReader::KFileReader(const KURL& url, const QString& nameFilter)
    : QObject(0, "KFileReader")
{
    init();
    if (!nameFilter.isNull())
	setNameFilter(nameFilter);
    setURL(url);
}

void KFileReader::init()
{
    myUpdateDir = 0L;
    readable  = false;

    myAutoUpdate = false;
    myDirtyFlag  = true;
    showHidden   = false;
    myJob = 0L;
    myDirWatch = 0L;
    myUpdateTimer = 0L;
    myEntries.setAutoDelete(true);
    myNewEntries.setAutoDelete(false);
    myUpdateList.setAutoDelete(false);
    filters.setAutoDelete(true);
}

KFileReader::~KFileReader()
{
    delete myDirWatch;
}

KFileReader &KFileReader::operator= (const QString& url)
{
    setURL(url);
    return *this;
}

void KFileReader::setURL(const KURL& url)
{
    KURL oldurl = *this;
    KURL::operator=(url);

    if (isLocalFile()) { // we can check, if the file is there
	struct stat buf;
	QString ts = path(+1);
	int ret = ::stat( ts.local8Bit(), &buf);
	readable = (ret == 0);
	if (readable) { // further checks
	    DIR *test;
	    test = opendir( ts.local8Bit() ); // we do it just to test here
	    readable = (test != 0);
	    if (test) {
		closedir(test);
		if ( myAutoUpdate ) {
                    if (oldurl.isLocalFile()) {
                        myDirWatch->removeDir( oldurl.path() );
                    }
		    myDirWatch->addDir( path() );
		}
	    }
	}
    } else {
	readable = true; // what else can we say?

        if ( myAutoUpdate && oldurl.isLocalFile()) {
	    myDirWatch->removeDir( oldurl.path() );
	}
    }

    root = (path() == QChar('/'));

    if (!readable)
	return;  // nothing more we can do here

    myDirtyFlag= true;
    currentSize = 50;
}


void KFileReader::setNameFilter(const QString& nameFilter)
{
    filters.clear();
    // Split on white space
    char *s = qstrdup(nameFilter.latin1());
    char *g = strtok(s, " ");
    while (g) {
	filters.append(new QRegExp(QString::fromLatin1(g), false, true ));
	g = strtok(0, " ");
    }
    delete [] s;

    updateFiltered();
}


void KFileReader::setAutoUpdate( bool b )
{
    if ( b == myAutoUpdate )
	return;

    myAutoUpdate = b;

    if ( myAutoUpdate ) {
	if ( !myDirWatch ) {
	    myDirWatch = new KDirWatch;
	    myUpdateTimer = new QTimer( this );
	}
	connect( myDirWatch, SIGNAL( deleted(const QString&) ),
		 SLOT(slotDirDeleted(const QString&) ));
	connect( myDirWatch, SIGNAL( dirty(const QString&)),
		 this, SLOT( slotDirDirty(const QString&) ));
	connect( myUpdateTimer, SIGNAL( timeout() ),
		 this, SLOT( slotDirUpdate() ));

	if (isLocalFile())
           myDirWatch->addDir( path() );
	myDirWatch->startScan();
    }
    else {
	disconnect( myDirWatch, SIGNAL( dirty(const QString&)),
		    this, SLOT( slotDirDirty(const QString&)) );
        if (isLocalFile())
           myDirWatch->removeDir( path() );
    }
}

uint KFileReader::count() const
{
    return myEntries.count();
}

uint KFileReader::dirCount() const
{
    return myEntries.count() + myPendingEntries.count();
}

void KFileReader::updateFiltered()
{
    bool changed = false;

    for (KFileViewItem *i= myEntries.first(); i; i=myEntries.next()) {
	register bool test = i->isHidden();
	if (test == filterEntry(i)) {
	    i->setHidden(!test);
	    changed = true;
	}
    }

    if (changed)
	emit filterChanged();
}

void KFileReader::getEntries()
{
    if (myDirtyFlag) {
	myEntries.clear();
	myDirtyFlag = false;
    }

    myDirtyFlag = true;
    startLoading();

}

void KFileReader::listContents()
{
    if (myDirtyFlag)
	getEntries();
    else
        emit contents( myEntries, true );
}

bool KFileReader::match(const QString& name) const
{
    bool matched = false;
    for (QListIterator<QRegExp> it(filters);
	 it.current(); ++it)
	if ( it.current()->match( name ) != -1 ) {
	    matched = true;
	    break;
	}

    return matched;
}

bool KFileReader::filterEntry(KFileViewItem *i)
{
    static QString up = QString::fromLatin1("..");
    if (i->name() == up)
	return !root;

    if (!showHidden && i->name()[0] == '.')
	return false;

    static QChar dot('.');
    if (i->name() == dot)
	return false;

    if (i->isDir())
	return true;

    if (filters.isEmpty() || match(i->name()))
	return true;

    return false;
}

void KFileReader::startLoading()
{
    if (myJob != 0) // sorry, get out of my way
	myJob->kill();
    // debug("KFileReader::startLoading( %s )", this->url().latin1());
    myJob = KIO::listDir(*this, false /* no progress info */);
    CHECK_PTR(myJob);

    connect(myJob, SIGNAL(result(KIO::Job*)),
	    SLOT(slotIOFinished(KIO::Job*)));
    connect(myJob, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
	    SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
}

void KFileReader::slotEntries(KIO::Job*, const KIO::UDSEntryList& entries)
{
    KIO::UDSEntryListIterator it(entries);
    myNewEntries.clear();
    QString baseurl = url();
    for (; it.current(); ++it) {
      KFileViewItem *i= new KFileViewItem(baseurl, *(it.current()));
      CHECK_PTR(i);

      myEntries.append(i);
      i->setHidden(!filterEntry(i));
      if (!i->isHidden()) {
	  emit dirEntry(i);
	  myNewEntries.append(i);
      }
    }

    if ( myNewEntries.count() > 0 )
      emit contents( myNewEntries, false);
}

void KFileReader::slotIOFinished( KIO::Job * job )
{
    kdDebug(kfile_area) << "slotIOFinished" << endl;
    myJob= 0;
    myNewEntries.clear();
    myDirtyFlag = false;
    if (job->error())
	emit error( job->error(), job->errorText() );
    else
	emit contents(myNewEntries, true);
}

// called when KDirWatch tells us that our directory contents have changed
// rereading it is delayed for half a second to avoid reading it too often on
// heavy changes.
void KFileReader::slotDirDirty( const QString& dir )
{
    if ( dir == path(-1) ) // no a slash at the end
	myUpdateTimer->start( 500, true );
}

// this is similar to getEntries(), but it's a bit different, because
// it will read the dir in chunks, delayed thru a QTimer
void KFileReader::slotDirUpdate()
{
    if (myJob) { // still reading, or someone called setURL() during update
        if (myUpdateDir) {
	    closedir(myUpdateDir);
	    myUpdateDir = 0L;
	    myUpdateList.clear();
	    myNewEntries.clear();
	}
        return;
    }

    if (!myUpdateDir) { // ok, let's try reading and init some stuff
        myUpdateDir = opendir(path().local8Bit());
	if (myUpdateDir) {
	    (void) readdir(myUpdateDir); // take out the "."
	    myUpdateList = myEntries;    // separate list for comparing
	    myNewEntries.clear();
	}
	else {
	    slotDirDeleted( path() ); //maybe not deleted, but we can't read it
	    return;
	}
    }

    QString _path = path(+1);
    struct dirent *dp = readdir(myUpdateDir);
    if ( dp ) {
        KFileViewItem *i;
        QString name = QString::fromLocal8Bit(dp->d_name);
        const KFileViewItem *item = myEntries.findByName( name );

	if ( item ) {
	    myUpdateList.remove( item );
	}
	else { // new item
	    i = new KFileViewItem( _path, name );
	    CHECK_PTR(i);
	    if (i->name().isNull())
	        delete i;
	    else {
	        // call mimeType() as well?
	        i->setHidden(!filterEntry(i));
		myEntries.append(i);
	        myNewEntries.append(i);
	    }
	}
	// on to the next dir entry (sort of nonblocking)
	QTimer::singleShot( 0, this, SLOT(slotDirUpdate()) );
	return;
    }

    else { // finished reading dir
        closedir(myUpdateDir);
	myUpdateDir = 0L;

	// remove items, that were not in the directory anymore
	KFileViewItem *item = 0L;
	for ( item = myUpdateList.first(); item; item = myUpdateList.next() )
	    item->setDeleted(); // mark them as removed

	if ( myUpdateList.count() > 0 )
	    emit itemsDeleted( myUpdateList );
	
	// new signal because of no sorting?
	if ( myNewEntries.count() > 0 )
	    emit contents(myNewEntries, true);
    }
}

void KFileReader::slotDirDeleted( const QString&  )
{
}

void KFileReader::setShowHiddenFiles(bool b)
{
    showHidden = b;
    updateFiltered();
}

#include "kfilereader.moc"

#ifndef MNTTAB
#ifdef MTAB_FILE
#define MNTTAB MTAB_FILE
#else
#define MNTTAB "/etc/mnttab"
#endif
#endif

// hopefully there are only two kind of APIs. If not we need a configure test
#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != NULL)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define FSNAME(var) var->mnt_fsname
#elif defined(BSD)
#define SETMNTENT(x, y) setfsent()
#define ENDMNTENT(x) endfsent()
#define STRUCT_MNTENT struct fstab *
#define STRUCT_SETMNTENT int
#define GETMNTENT(file, var) ((var=getfsent()) != NULL)
#define MOUNTPOINT(var) var->fs_file
#define MOUNTTYPE(var) var->fs_type
#define FSNAME(var) var->fs_spec
#else /* no setmntent and no BSD */
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define FSNAME(var) var.mnt_special
#endif

/**
 * Idea and code by Olaf Kirch <okir@caldera.de>
 **/
static bool probably_slow_mounted(const char *filename)
{
    STRUCT_SETMNTENT	mtab;
    char		realname[MAXPATHLEN];
    int		        length, max;

    memset(realname, 0, MAXPATHLEN);

    /* If the path contains symlinks, get the real name */
    if (realpath(filename, realname) == NULL) {
	if (strlen(filename) >= sizeof(realname))
	    return false;
	strcpy(realname, filename);
    }

    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == NULL) {
	perror("setmntent");
	return false;
    }

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

    max = 0;
    STRUCT_MNTENT me;

    enum { Unseen, Right, Wrong } isauto = Unseen, isslow = Unseen;

    while (true) {
      if (!GETMNTENT(mtab, me))
	break;

      length = strlen(MOUNTPOINT(me));

      if (!strncmp(MOUNTPOINT(me), realname, length)
	  && length > max) {
	  max = length;
	  if (length == 1 || realname[length] == '/' || realname[length] == '\0') {

	      bool nfs = !strcmp(MOUNTTYPE(me), "nfs");
	      bool autofs = !strcmp(MOUNTTYPE(me), "autofs");
	      bool pid = (strstr(FSNAME(me), ":(pid") != 0);

	      if (nfs && !pid)
		  isslow = Right;
	      else if (isslow == Right)
		  isslow = Wrong;

	      /* Does this look like automounted? */
	      if (autofs || (nfs && pid)) {
		  isauto = Right;
		  isslow = Right;
	      }
	  }
      }
    }

    if (isauto == Right && isslow == Unseen)
	isslow = Right;

    ENDMNTENT(mtab);
    return (isslow == Right);
}


