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
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kurl.h>
#include "config-kfile.h"
#include <kdebug.h>

template class QIntDict<char>;
template class QList<KFileViewItem>;

KFileViewItem::IntCache   * KFileViewItem::passwdCache = 0L;
KFileViewItem::IntCache   * KFileViewItem::groupCache  = 0L;
KFileViewItem::GroupCache * KFileViewItem::myGroupMemberships = 0L;

KFileViewItem::KFileViewItem(const KUDSEntry &e)
{
    myIsDir = false;
    myIsFile = true;
    myIsSymLink = false;
    myPermissions = 0755;
    mySize = 0;
    myPixmap = 0L;

    debugC("KFileViewItem::KFileViewItem");

    KURL url;
    KUDSEntry::ConstIterator it = e.begin();
    for( ; it != e.end(); it++ ) {
	switch (( *it ).m_uds) {
	case KIO::UDS_NAME:
	    myName = ( *it ).m_str;
	    break;
	case KIO::UDS_SIZE:
	    mySize = ( *it ).m_long;
	    break;
	case KIO::UDS_MODIFICATION_TIME:
	    myDate = dateTime(( *it ).m_long);
	    break;
	case KIO::UDS_USER:
	    myOwner = ( *it ).m_str;
	    break;
	case KIO::UDS_GROUP:
	    myGroup = ( *it ).m_str;
	    break;
	case KIO::UDS_ACCESS:
	    myPermissions = ( *it ).m_long;
	    parsePermissions(myPermissions);
	    break;
	case KIO::UDS_FILE_TYPE:
	    myIsDir = (( *it ).m_long & S_IFDIR) != 0;
	    myIsFile = !myIsDir;
	    break;
	case KIO::UDS_LINK_DEST:
	    myIsSymLink = (( *it ).m_str.length());
	    break;
	case KIO::UDS_URL:
	    url = ( *it ).m_str;
	    myBaseURL = url.path( 1 ); // we want a trailing "/"
	    break;
	case KIO::UDS_MIME_TYPE:
	  debugC("Do something with KIO::UDS_MIME_TYPE?");
	  break;
	default:
	    debugC("got %ld", static_cast<long int>(( *it ).m_uds));
	};
    }

    debugC("COMPLETE %s %d %d", debugString(myName), myIsDir, mySize);
    myName.replace( QRegExp(QString::fromLatin1("/$")), QString::fromLatin1("") );
    myIsReadable = true;

    init();
}

KFileViewItem::KFileViewItem(const QString& dir, const QString& name, bool delaystat)
{
    myPixmap = 0L;
    myName = name;
    myBaseURL = dir;

    myIsDir = false; // assumptions to get a nice default pixmap
    myIsFile = true;
    myIsSymLink = false;
    myIsReadable = true;
    ASSERT( myBaseURL.at(myBaseURL.length() - 1) == '/' || name.at(0) == '/' );

    if (!passwdCache) {
	passwdCache = new IntCache( 317 );
	groupCache = new IntCache( 317 );
	myGroupMemberships = new GroupCache;
	
	passwdCache->setAutoDelete( true );
	groupCache->setAutoDelete( true );
	
	readUserInfo();
	qAddPostRoutine( cleanup );
    }

    init();
    if (!delaystat)
	stat(false);
}

void KFileViewItem::stat(bool alreadyindir)
{
    struct stat buf;
    myIsSymLink = false;
    QCString local8;
    if (alreadyindir)
	local8 = myName.local8Bit();
    else {
	QString fullname = url();
	if (fullname.left(5) == QString::fromLatin1("file:"))
	    fullname = fullname.mid(5);
	local8 = fullname.local8Bit();
    }

    if (lstat(local8, &buf) == 0) {
	myIsDir = (buf.st_mode & S_IFDIR) != 0;
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
	myIsReadable = testReadable( local8, buf );
	
	myPermissions = buf.st_mode;
	myOwner_t = buf.st_uid;
	myGroup_t = buf.st_gid;

	// guess we should update, now that we have the correct information
	if ( !myMimeType ) {
	    delete myPixmap;
	    myPixmap = 0L;
	}

    } else {
	// default
	debugC("the file does not exist %s%s", debugString(myBaseURL), debugString(myName));
	myName.insert(0, '?');
	mySize = 0;
	myIsFile = false;
	myIsDir = false;
	myIsReadable = false;
	myPermissions = 0;
	parsePermissions(myPermissions);
    }
}

KFileViewItem::KFileViewItem(const KFileViewItem &i)
{
    *this = i;
}

KFileViewItem::~KFileViewItem()
{
    delete myPixmap;
}

KFileViewItem &KFileViewItem::operator=(const KFileViewItem &i)
{
    myName= i.myName;
    myBaseURL= i.myBaseURL;
    myAccess = i.myAccess;
    myDate= i.myDate;
    myDate_t = i.myDate_t;
    myOwner= i.myOwner;
    myGroup= i.myGroup;
    myIsDir = i.myIsDir;
    myIsFile = i.myIsFile;
    myIsSymLink = i.myIsSymLink;
    myPermissions= i.myPermissions;
    mySize= i.mySize;
    myIsReadable = i.myIsReadable;
    myFilePath = i.myFilePath;
    myPixmap = i.myPixmap;
    myMimeType = i.myMimeType;
    myPixmapDirty = i.myPixmapDirty;

    return *this;
}


void KFileViewItem::init()
{
    myNext = 0;
    myMimeType = 0L;
    myHidden = false;
    myPixmapDirty = false;
    myPixmapSize = KIconLoader::Small;
}


void KFileViewItem::parsePermissions(const char *perms)
{
    myPermissions = 0;
    char p[11] = {0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0,
		  0};

    strncpy(p, perms, sizeof(p));

    myIsDir = (bool)(p[0] == 'd');
    myIsSymLink = (bool)(p[0] == 'l');
    myIsFile = !myIsDir;

    if(p[1] == 'r')
      myPermissions |= QFileInfo::ReadUser;

    if(p[2] == 'w')
      myPermissions |= QFileInfo::WriteUser;

    if(p[3] == 'x')
      myPermissions |= QFileInfo::ExeUser;

    if(p[4] == 'r')
      myPermissions |= QFileInfo::ReadGroup;

    if(p[5] == 'w')
      myPermissions |= QFileInfo::WriteGroup;

    if(p[6] == 'x')
      myPermissions |= QFileInfo::ExeGroup;

    if(p[7] == 'r')
      myPermissions |= QFileInfo::ReadOther;

    if(p[8] == 'w')
      myPermissions |= QFileInfo::WriteOther;

    if(p[9] == 'x')
      myPermissions |= QFileInfo::ExeOther;
}

QString KFileViewItem::parsePermissions(uint perm) const
{
    char p[] = "----------";

    if (myIsDir)
	p[0]='d';
    else if (myIsSymLink)
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
    if (myDate.isNull())
	myDate = dateTime(myDate_t);
    return myDate;
}

QString KFileViewItem::dateTime(time_t _time) {
    QDateTime t;
    t.setTime_t(_time);
    return KGlobal::locale()->formatDateTime( t );
}

#if 0
/* the following will go into KLocale after Beta4!!! */
static QString *months[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

QString KFileViewItem::dateTime(time_t _time) {

    if (!months[0]) {
	months[ 0] = new QString(i18n("Jan"));
	months[ 1] = new QString(i18n("Feb"));
	months[ 2] = new QString(i18n("Mar"));
	months[ 3] = new QString(i18n("Apr"));
	months[ 4] = new QString(i18n("May"));
	months[ 5] = new QString(i18n("Jun"));
	months[ 6] = new QString(i18n("Jul"));
	months[ 7] = new QString(i18n("Aug"));
	months[ 8] = new QString(i18n("Sep"));
	months[ 9] = new QString(i18n("Oct"));
	months[10] = new QString(i18n("Nov"));
	months[11] = new QString(i18n("Dec"));
    }

    QDateTime t;
    time_t now = time(0);
    t.setTime_t(_time);

    QString sTime;
    if (_time > now || now - _time >= 365 * 24 * 60 * 60)
	sTime.sprintf(" %04d", t.date().year());
    else
	sTime.sprintf("%02d:%02d", t.time().hour(), t.time().minute());

    QString text = QString("%1 %2 %3").
      arg(months[t.date().month() - 1]->left(3), 3).
      arg(t.date().day(), 2).arg(sTime);

    /* hmm... supposed to be in KLocale after Beta4, huh? :D
       QDateTime t;
       t.setTime_t(_time);
       return KGlobal::locale()->formatDateTime( t );
    */
    return text;
}
#endif

void KFileViewItem::readUserInfo()
{
    struct passwd *pass;
    while ( (pass = getpwent()) != 0L ) {
	passwdCache->insert(pass->pw_uid, qstrdup(pass->pw_name));
    }
    delete pass;
    endpwent();

    char *myUsername = passwdCache->find( getuid() );
    struct group *gr;
    while( (gr = getgrent()) != 0L ) {
	groupCache->insert(gr->gr_gid, qstrdup(gr->gr_name));

	int i = 0;
	char *member = 0L;
	while ( (member = gr->gr_mem[i++]) != 0L ) {
	    if ( myUsername && strcmp( myUsername, member ) == 0 ) {
	        myGroupMemberships->append( gr->gr_gid );
		break;
	    }
	}
    }
    delete gr;
    endgrent();
}


void KFileViewItem::cleanup()
{
    delete passwdCache;
    passwdCache = 0L;

    delete groupCache;
    groupCache = 0L;

    delete myGroupMemberships;
    myGroupMemberships = 0L;
}

QString KFileViewItem::url() const
{
    if (myFilePath.isNull())
	if (isDir())
	    myFilePath = myBaseURL + myName + '/';
	else
	    myFilePath = myBaseURL + myName;
    return myFilePath;
}

QString KFileViewItem::mimeType()
{
    if ( !myMimeType ) {
	myMimeType = KMimeType::findByURL( url(), 0, true );
	
	if ( myIsReadable ) // keep the "locked" icon for unreadable files/dirs
	    myPixmapDirty = true;
    }

    return myMimeType->name();
}

QPixmap KFileViewItem::pixmap( KIconLoader::Size size ) const
{
    if ( !myPixmapDirty ) {
	if ( myPixmapSize != size || !myPixmap ) {
	    delete myPixmap;
	    myPixmap = new QPixmap(
				   KGlobal::iconLoader()->loadApplicationIcon(defaultIcon(), size,
									      0L, false));
	}
    }

    else  { // pixmap dirty, we shall load the pixmap according to our mimetype
	QString icon = myMimeType->icon( url(), true );
	if ( icon != defaultIcon() || (size == mySize) || !myPixmap ) {
	
	    if ( icon.isEmpty() )
		icon = defaultIcon();
	
	    QPixmap temp = KGlobal::iconLoader()->loadApplicationIcon(icon, size);
	    delete myPixmap;
	    myPixmap = new QPixmap( temp );
	
	    // we either have found the correct pixmap, or there is none,
	    // anyway, we won't ever search for one again
	    myPixmapDirty = false;
	}
    }
    return *myPixmap;
}

QString KFileViewItem::defaultIcon() const
{
    // avoid creating a QString every time this function is called
    static QString folder = QString::fromLatin1("folder");
    static QString lockedfolder = QString::fromLatin1("lockedfolder");
    static QString unknown = QString::fromLatin1("mimetypes/unknown");
    static QString locked  = QString::fromLatin1("locked");

    if ( myIsDir ) {
	if ( myIsReadable )
	    return folder;
	else
	    return lockedfolder;
    }
    else {
	if ( myIsReadable )
	    return unknown;
	else
	    return locked;
    }

    return QString::null;
}

void KFileViewItem::setDeleted()
{
    myIsReadable = false;
    myAccess = "**********";
    delete myPixmap; // next pixmap() call will load the "locked" icon
    myPixmap = 0L;
}


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
    if (myAccess.isNull())
      myAccess = parsePermissions(myPermissions);

    return myAccess;
}

QString KFileViewItem::owner() const
{
    static QString unknown = i18n("unknown");

    if (myOwner.isNull()) {
        myOwner = QString::fromLocal8Bit( passwdCache->find(myOwner_t) );

	if (myOwner.isNull())
	    myOwner = unknown;
    }
    return myOwner;
}


QString KFileViewItem::group() const
{
    static QString unknown = i18n("unknown");

    if ( myGroup.isNull() ) {
        myGroup = QString::fromLocal8Bit( groupCache->find(myGroup_t) );

	if (myGroup.isNull())
	    myGroup = unknown;
    }
    return myGroup;
}


// this is a little wrapper around ::access(). We already have a stat-buffer,
// so we can make some easy tests if the file is readable. This is a little
// faster than calling ::access()
bool KFileViewItem::testReadable( const QCString& file, struct stat& buf )
{
    if (file.isEmpty())
	return false;

  // check the "others'" permissions first
  if ( (buf.st_mode & (S_IROTH | (myIsDir ? S_IXOTH : 0))) != 0 )
    return true;

  // check if user can read [execute dirs]
  if ( (buf.st_mode & (S_IRUSR | (myIsDir ? S_IXUSR : 0))) != 0 ) {
    if ( buf.st_uid == getuid() )
      return true;
  }

  // check the group permissions, if we have no user permissions
  if ( (buf.st_mode & (S_IRGRP | (myIsDir ? S_IXGRP : 0))) != 0 ) {
    if ( buf.st_gid == getgid() ||
	 myGroupMemberships->find( buf.st_gid ) != myGroupMemberships->end())
      return true;
  }

  return false;
}


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
