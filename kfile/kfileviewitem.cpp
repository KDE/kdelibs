// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
		  1999 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kfileviewitem.h"

#include <sys/types.h>
#include <dirent.h>
#include <grp.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#include <qfileinfo.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <kapp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include "config-kfile.h"

template class QList<KFileViewItem>;


class KFileViewItem::KFileViewItemPrivate
{
public:
    KFileViewItemPrivate() {}
    ~KFileViewItemPrivate() {}

    QString access;
    QString date;
    int pixmapSize;
    bool isReadable;

};

KFileViewItem::KFileViewItem(const QString& baseURL, const KIO::UDSEntry &e)
    : KFileItem( e, baseURL, true, true )
{
    d = new KFileViewItemPrivate();
    d->isReadable = true;

    init();
}

KFileViewItem::KFileViewItem(mode_t _mode, mode_t _permissions, const KURL& _url, bool _determineMimeTypeOnDemand, bool _urlIsDirectory )
    : KFileItem( _mode, _permissions, _url, _determineMimeTypeOnDemand,
		 _urlIsDirectory )
{
    d = new KFileViewItemPrivate();
    d->isReadable = true;

    init();
}

#if 0
void KFileViewItem::stat(bool alreadyindir)
{
    // #warning adopt in KFileItem?

    struct stat buf;
    myIsSymLink = false;
    QCString local8;
    if (alreadyindir)
	local8 = myName.local8Bit();
    else {
	if (url().isLocalFile())
	    local8 = QFile::encodeName( url().path(-1) );
	else
	    local8 = QFile::encodeName(urlString());
    }

    if (lstat(local8, &buf) == 0) {
	myIsDir = S_ISDIR(buf.st_mode) != 0;
        // check if this is a symlink to a directory
	if (S_ISLNK(buf.st_mode)) {
	  myIsSymLink = true;
	  struct stat st;
	  if (::stat(local8, &st) == 0) {
	      myIsDir = S_ISDIR(st.st_mode) != 0;
	  }
	  else {
	      myName = QString::null; // indicate, that the link is broken
	  }
	} else
	    myIsSymLink = false;

	myDate_t = buf.st_mtime;
	mySize = buf.st_size;
	myIsFile = !myIsDir;
	d->isReadable = testReadable( local8, buf );
	
	myPermissions = buf.st_mode;
	myOwner_t = buf.st_uid;
	myGroup_t = buf.st_gid;

    } else {
	// default
	myName.insert(0, '?');
	mySize = 0;
	myIsFile = false;
	myIsDir = false;
	d->isReadable = false;
	myPermissions = 0;
	parsePermissions(permissions());
    }
}
#endif

KFileViewItem::~KFileViewItem()
{
    delete d;
}

void KFileViewItem::init()
{
    myNext = 0;
    d->pixmapSize = KIcon::SizeSmall;
}


QString KFileViewItem::parsePermissions(mode_t perm) const
{
    char p[] = "----------";

    if (isDir())
	p[0]='d';
    else if (isLink())
	p[0]='l';

    if (perm & QFileInfo::ReadUser)
	p[1]='r';
    if (perm & QFileInfo::WriteUser)
	p[2]='w';
    if (perm & QFileInfo::ExeUser)
	p[3]='x';

    if (perm & QFileInfo::ReadGroup)
	p[4]='r';
    if (perm & QFileInfo::WriteGroup)
	p[5]='w';
    if (perm & QFileInfo::ExeGroup)
	p[6]='x';

    if (perm & QFileInfo::ReadOther)
	p[7]='r';
    if (perm & QFileInfo::WriteOther)
	p[8]='w';
    if (perm & QFileInfo::ExeOther)
	p[9]='x';

    return QString::fromLatin1(p);
}

QString KFileViewItem::date() const {
    if (d->date.isNull())
	d->date = dateTime(time( KIO::UDS_MODIFICATION_TIME) );
    return d->date;
}

QString KFileViewItem::dateTime(time_t _time) {
    QDateTime t;
    t.setTime_t(_time);
    return KGlobal::locale()->formatDateTime( t );
}


QString KFileViewItem::urlString() const
{
    if (myURLString.isNull())
	if (isDir())
	    myURLString = KFileItem::url().url(+1);
	else
	    myURLString = KFileItem::url().url();
    return myURLString;
}

void KFileViewItem::setDeleted()
{
    d->isReadable = false;
    d->access = "**********";
}


// FIXME: what should happen with those items in copy constructor and
// assignment operator???
void KFileViewItem::setViewItem( const KFileView *view, const void *item )
{
    viewItems[ view ] = item;
}

const void *KFileViewItem::viewItem( const KFileView *view ) const
{
    if ( !viewItems.contains( view ) )
	return 0;
    return viewItems[ view ];
}

QString KFileViewItem::access() const {
    if (d->access.isNull())
      d->access = parsePermissions(permissions());

    return d->access;
}

#if 0
// Tests if a file is readable. We don't just call ::access(), because we
// already have a stat-structure and know about the groups.
bool KFileViewItem::testReadable( const QCString& file, struct stat& buf )
{
    if (file.isEmpty())
	return false;

    bool isdir = isDir();
  // check the "others'" permissions first
  if ( (buf.st_mode & (S_IROTH | (isdir ? S_IXOTH : 0))) != 0 )
    return true;

  // check if user can read [execute dirs]
  if ( (buf.st_mode & (S_IRUSR | (isdir ? S_IXUSR : 0))) != 0 ) {
    if ( buf.st_uid == getuid() )
      return true;
  }

  // check the group permissions, if we have no user permissions
  if ( (buf.st_mode & (S_IRGRP | (isdir ? S_IXGRP : 0))) != 0 ) {
    if ( buf.st_gid == getgid() ||
	 myGroupMemberships->find( buf.st_gid ) != myGroupMemberships->end())
      return true;
  }

  return false;
}
#endif


/////////////

void KFileViewItemList::append( const KFileViewItem *item )
{
    if ( !item )
	return;

    KFileViewBaseList::append( item );
    dictdirty = true;
}


void KFileViewItemList::clear()
{
    KFileViewBaseList::clear();
    myDict.clear();
    dictdirty = false;
}

const KFileViewItem * KFileViewItemList::findByName( const QString& url ) const
{
    if (dictdirty) {
	KFileViewItemList *that = const_cast<KFileViewItemList*>(this);
	that->myDict.clear();

	QListIterator<KFileViewItem> it(*this);
	for ( ; it.current(); ++it)
	    that->myDict.insert( it.current()->name(), it.current() );
	that->dictdirty = false;
    }

    return myDict.find( url );
}

QPixmap KFileViewItem::pixmap() const
{
    return pixmap( d->pixmapSize );
}

bool KFileViewItem::isReadable() const
{
    return d->isReadable;
}
