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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdir.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <qstringlist.h>

#include <kio/global.h>
#include <kio/job.h>
#include <kdebug.h>

#include "config-kfile.h"
#include "kfilereader.h"
#include "kfileviewitem.h"

/*
** KFileReader - URL aware directory operator
**
*/

class KFileReader::KFileReaderPrivate
{
public:
    KFileReaderPrivate() {}
    ~KFileReaderPrivate() {}
};


KFileReader::KFileReader()
    : KDirLister(true)
{
    init();
    setURL( QDir::currentDirPath() );
    // openURL(QDir::currentDirPath(), showingDotFiles());
}

KFileReader::KFileReader(const KURL& url, const QString& nameFilter)
    : KDirLister(true)
{
    init();
    if (!nameFilter.isNull())
	setNameFilter(nameFilter);
    //    openURL(url, showingDotFiles());
    setURL( url );
}

void KFileReader::init()
{
    setShowingDotFiles( false );
}

KFileReader::~KFileReader()
{
}

KFileItem * KFileReader::createFileItem( const KIO::UDSEntry& entry,
					 const KURL& url, bool )
{
    return new KFileViewItem( url, entry );
}

bool KFileReader::isReadable() const
{
    if ( !url().isLocalFile() )
	return true; // what else can we say?

    struct stat buf;
    QString ts = url().path(+1);
    bool readable = ( ::stat( QFile::encodeName( ts ), &buf) == 0 );
    if (readable) { // further checks
	DIR *test;
	test = opendir( QFile::encodeName( ts )); // we do it just to test here
	readable = (test != 0);
	if (test)
	    closedir(test);
    }
    return readable;
}


#include "kfilereader.moc"
