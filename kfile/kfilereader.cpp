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

class KFileReader::KFileReaderPrivate
{
public:
    KFileReaderPrivate() {}
    ~KFileReaderPrivate() {}

    KDirWatch *dirWatch;
    bool autoUpdate;
};


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
    d = new KFileReaderPrivate();
    readable  = false;

    d->autoUpdate = false;
    myDirtyFlag  = true;
    showHidden   = false;
    d->dirWatch = new KDirWatch();
    myJob = 0L;
    myEntries.setAutoDelete(true);
    myNewEntries.setAutoDelete(false);
    filters.setAutoDelete(true);
}

KFileReader::~KFileReader()
{
    delete d;
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
		if ( d->autoUpdate ) {
                    if (oldurl.isLocalFile()) {
			#warning fixme (KDirWatch)
			// d->dirWatch->removeDir( oldurl.path() );
                    }
		    // d->dirWatch->addDir( path() );
		}
	    }
	}
    } else {
	readable = true; // what else can we say?

        if ( d->autoUpdate && oldurl.isLocalFile()) {
	    // d->dirWatch->removeDir( oldurl.path() );
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
    myDirtyFlag = true;
}


void KFileReader::setAutoUpdate( bool b )
{
    if ( b == d->autoUpdate )
	return;

    d->autoUpdate = b;
}

bool KFileReader::autoUpdate() const
{
    return d->autoUpdate;
}

uint KFileReader::count() const
{
    return myEntries.count();
}

uint KFileReader::dirCount() const
{
    return myEntries.count() + myPendingEntries.count();
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

      if (!filterEntry(i)) {
	  delete i;
	  continue;
      }

      myEntries.append(i);
      myNewEntries.append(i);
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

void KFileReader::setShowHiddenFiles(bool b)
{
    myDirtyFlag = (showHidden != b);
    showHidden = b;
}

#include "kfilereader.moc"

