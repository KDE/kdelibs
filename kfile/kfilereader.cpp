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
#include <kio_job.h>
#include <kdebug.h>
#include <config-kfile.h>
#include <kdebug.h>

template class QList<QRegExp>;

/*
** KFileReader - URL aware directory operator
**
*/

KDirWatch * KFileReader::dirWatch = 0L;
bool KFileReader::performChdir = true;

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
    myOpendir = 0L;
    myUpdateDir = 0L;
    readable  = false;

    myAutoUpdate = false;
    myDirtyFlag  = true;
    myJob= 0;
    myEntries.setAutoDelete(true);
    myNewEntries.setAutoDelete(false);
    myUpdateList.setAutoDelete(false);
    filters.setAutoDelete(true);
}

KFileReader::~KFileReader()
{
}

void qt_qstring_stats();

void KFileReader::cleanup()
{
    delete dirWatch;
    dirWatch = 0L;
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
    kDebugInfo(kfile_area, "%s is %sslowly mounted", debugString(url.url()),
	  (probably_slow_mounted(path(+1).local8Bit()) ? "" : "not "));

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
		    dirWatch->removeDir( oldurl.url() );
		    dirWatch->addDir( path() );
		}
	    }
	}
    } else {
	readable = true; // what else can we say?

        if ( myAutoUpdate ) {
	    dirWatch->removeDir( oldurl.url() );
	}
    }

    root = (path() == QChar('/'));

    if (!readable)
	return;  // nothing more we can do here

    if (myOpendir) {
	closedir(myOpendir);
	myOpendir = 0;
    }
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

    myAutoUpdate = b && isLocalFile();

    if ( myAutoUpdate ) {
	if ( !dirWatch ) {
	    dirWatch = new KDirWatch;
	    qAddPostRoutine( cleanup );
	}
	connect( dirWatch, SIGNAL( deleted(const QString&) ),
		 SLOT(slotDirDeleted(const QString&) ));
	connect( dirWatch, SIGNAL( dirty(const QString&)),
		 this, SLOT( slotDirDirty(const QString&) ));
	
	dirWatch->addDir( path() );
	dirWatch->startScan();
    }
    else {
	disconnect( dirWatch, SIGNAL( dirty(const QString&)),
		    this, SLOT( slotDirDirty(const QString&)) );
	dirWatch->removeDir( path() );
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

    // remote files are loaded asynchronously
    if ( !isLocalFile() ) {
	myDirtyFlag = startLoading();
	return;
    }

    struct dirent *dp;

    if (!myOpendir) {
	QString ts = path();
	myOpendir = opendir(ts.local8Bit());
	if (!myOpendir)
	    return;
	(void) readdir(myOpendir); // take out the "."
    }

#ifdef Q2HELPER
    qt_qstring_stats();
#endif
    kDebugInfo(kfile_area, "getEntries %ld", time(0));

    KFileViewItem *i;
    QString _url = url(+1);

    while (true) {
	dp = readdir(myOpendir);
	if (!dp)
	    break;
	
	i = new KFileViewItem(_url, QString::fromLocal8Bit(dp->d_name),
			      true);
	
	CHECK_PTR(i);
	myPendingEntries.append(i);
    }

    myPendingEntries.first();
    closedir(myOpendir);
    myOpendir = 0;

    kDebugInfo(kfile_area, "got %d entries %ld", myPendingEntries.count(), time(0));

    statLocalFiles();
}

void KFileReader::statLocalFiles()
{
    myNewEntries.clear();
    char *cwd = getcwd(0, 0);

    chdir(path(+1).local8Bit());

    struct timeval tp;

    gettimeofday(&tp, 0);
    long tv_sec = tp.tv_sec;
    long tv_usec = tp.tv_usec;

    int counter = 0;

    KFileViewItem *i;

    static const int maximum_updatetime = 300;
    static const int minimum_updatetime = (maximum_updatetime * 3) / 4;

    while (!myPendingEntries.isEmpty()) {
	
	i = myPendingEntries.take(); // removes it as first item
	i->stat(performChdir);

	if (i->name().isEmpty()) {
	    delete i;
	    continue;
	}
	myEntries.append(i);
	if (filterEntry(i)) {
	    myNewEntries.append(i);
	    i->setHidden(false);
	} else
	    i->setHidden(true);

	counter++;
	if (counter > currentSize) {
	    gettimeofday(&tp, 0);
	    long diff = ((tp.tv_sec - tv_sec) * 1000000 +
			 tp.tv_usec - tv_usec) / 1000;
	
	    /*
	      debug("called gettimeofday %ld %d %ld",
	      diff, counter, currentSize);
	    */

	    if (diff > maximum_updatetime) {
		currentSize = currentSize * 3 / 4;
		break;
	    } else if (diff < minimum_updatetime) {
		currentSize = currentSize * 5 / 4;
	    } else
		break;
	}
    }

#ifdef Q2HELPER
    qt_qstring_stats();
#endif

    chdir(cwd);
    free(cwd);

    kDebugInfo(kfile_area, "emit contents %ld", time(0));
    if (!myNewEntries.isEmpty())
	emit contents( &myNewEntries, myPendingEntries.isEmpty());

    if (!myPendingEntries.isEmpty())
	QTimer::singleShot(0, this, SLOT(statLocalFiles()));
}

void KFileReader::listContents()
{
    if (myDirtyFlag)
	getEntries();
    else
        emit contents( &myEntries, true );
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
    if (i->name() == QString::fromLatin1(".."))
	return !root;

    if (!showHidden && i->name()[0] == '.')
	return false;

    if (i->name() == QString::fromLatin1("."))
	return false;

    if (i->isDir())
	return true;

    if (filters.isEmpty() || match(i->name()))
	return true;

    return false;
}

bool KFileReader::startLoading()
{
    // If KIOJob is not busy
    if (myJob == 0) {
	myJob = new KIOJob("kiojob");
	CHECK_PTR(myJob);
	myJob->setGUImode( KIOJob::SIMPLE );

	// FIXME: still necessary?
	/* Check connection was made
	 if ( !myJob->isOK() ) {
	    myJob = 0;
	    readable = false;
	    emit finished(); // what else can I say? ;)
	}
	*/
	//	else { // If all is well
	    connect(myJob, SIGNAL(sigFinished(int)), SLOT(slotIOFinished()));
	    connect(myJob, SIGNAL(sigError(int, int, const char * )),
		    SLOT(slotIOError(int, int, const char * )));
	    connect(myJob, SIGNAL(sigListEntry(int, const KUDSEntry&)),
		    SLOT(slotListEntry(int, const KUDSEntry&)));
#ifdef AFTER_KRASH_API
	    myJob->listDir(*this);
#else
	    myJob->listDir(url().ascii());
#endif
	    return true;
	    //	}
    }

    return false;
}

void KFileReader::slotListEntry(int, const KUDSEntry& entry) // SLOT
{
    kDebugInfo(kfile_area, "slotListEntry");
    KFileViewItem *i= new KFileViewItem(entry);
    CHECK_PTR(i);

    myEntries.append(i);
    myNewEntries.clear();

    i->setHidden(!filterEntry(i));
    if (!i->isHidden()) {
	emit dirEntry(i);
	myNewEntries.append(i);
    }

    // TODO when do we know we're done?
    if ( myNewEntries.count() > 0 )
      emit contents( &myNewEntries, false);
}

void KFileReader::slotIOFinished()
{
    myJob= 0;
    //    emit finished();
}

void KFileReader::slotIOError(int, int _errid, const char *_txt )
{
    myJob = 0;
    emit error(_errid, QString::fromLatin1(_txt)); // TODO change KIOJob to emit unicode
}

void KFileReader::slotDirDirty( const QString& )
{
    slotDirUpdate();
}

// this is similar to getEntries(), but it's a bit different, because
// it will read the dir in chunks, delayed thru a QTimer
void KFileReader::slotDirUpdate()
{
    if (myOpendir) { // still reading, or someone called setURL() during update
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
	    emit itemsDeleted( &myUpdateList );
	
	// new signal because of no sorting?
	if ( myNewEntries.count() > 0 )
	    emit contents(&myNewEntries, true);
    }
}

void KFileReader::slotDirDeleted( const QString& dir )
{
  kDebugInfo(kfile_area, "********** dir deleted: %s ***********", dir.ascii());
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
bool KFileReader::probably_slow_mounted(const char *filename)
{
    STRUCT_SETMNTENT	mtab;
    char		realname[MAXPATHLEN];
    int		        length, max;

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


