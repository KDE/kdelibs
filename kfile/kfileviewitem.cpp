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

#include <qfileinfo.h>
#include <qpixmap.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include "config-kfile.h"


class KFileViewItem::KFileViewItemPrivate
{
public:
    KFileViewItemPrivate() {}
    ~KFileViewItemPrivate() {}

    QString access;
    QString date;
    int pixmapSize;
    // if you add more, be sure to update KFileViewItem::operator= !
};

KFileViewItem::KFileViewItem(const KURL& baseURL, const KIO::UDSEntry &e)
    : KFileItem( e, baseURL, true, true )
{
    d = new KFileViewItemPrivate();

    init();
}

KFileViewItem::KFileViewItem(mode_t _mode, mode_t _permissions,
			     const KURL& _url,
			     bool _determineMimeTypeOnDemand )
    : KFileItem( _mode, _permissions, _url, _determineMimeTypeOnDemand )
{
    d = new KFileViewItemPrivate();

    init();
}

KFileViewItem::KFileViewItem( const KFileViewItem& item )
    : KFileItem( item )
{
    d = new KFileViewItemPrivate( *item.d );
    myURLString = item.myURLString;
    myNext = 0L; // we can't guarantee that this one is being copied as well
    // we don't copy the viewMap, as the view doesn't have a reference to us
    // and can get destroyed anytime without us noticing.
}

KFileViewItem&  KFileViewItem::operator=( const KFileViewItem& item )
{
    d->access = item.d->access;
    d->date = item.d->date;
    d->pixmapSize = item.d->pixmapSize;

    myURLString = item.myURLString;
    myNext = 0L; // we can't guarantee that this one is being copied as well
    // we don't copy the viewMap, as the view doesn't have a reference to us
    // and can get destroyed anytime without us noticing.

    return *this;
}


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


QPixmap KFileViewItem::pixmap() const
{
    return pixmap( d->pixmapSize );
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

