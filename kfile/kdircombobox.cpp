/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qdir.h>
#include <qlistbox.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>

#include <kdircombobox.h>

KDirComboBox::KDirComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    urlAdded  = false;
    myMaximum = 10; // default
    itemList.setAutoDelete( true );

    rootItem = new KDirComboItem;
    homeItem = new KDirComboItem;
    desktopItem = new KDirComboItem;

    KURL u = QDir::rootDirPath();
    rootItem->url = u.url( +1 );
    rootItem->text = i18n("Root Directory: %1").arg( u.path( +1 ) );

    u = QDir::homeDirPath();
    homeItem->url = u.url( +1 );
    homeItem->text = i18n("Home Directory: %1").arg( u.path( +1 ) );

    u = (u.url( +1 ) + QString::fromLocal8Bit("Desktop"));
    desktopItem->url= u.url( +1 );
    desktopItem->text = i18n("Desktop: %1").arg( u.path( +1 ) );

    connect( this, SIGNAL( activated( int )), SLOT( slotActivated( int )));

    KIconLoader *loader = KGlobal::iconLoader();
    KIconLoader::Size size = KIconLoader::Small;
    opendirPix = loader->loadIcon(QString::fromLocal8Bit("folder_open"), size);
    dirPix = loader->loadIcon( QString::fromLocal8Bit("folder"), size );
    rootPix = KMimeType::pixmapForURL( rootItem->url, 0, size );
    homePix = KMimeType::pixmapForURL( homeItem->url, 0, size );
    desktopPix = KMimeType::pixmapForURL( desktopItem->url, 0, size );
}

KDirComboBox::~KDirComboBox()
{
    delete rootItem;
    delete homeItem;
    delete desktopItem;
}


QStringList KDirComboBox::urls() const
{
    QStringList list;
    for ( int i = FIRSTITEM; i < count(); i++ ) {
        list.append( text( i ) );
    }

    return list;
}

void KDirComboBox::insertDefaults()
{
    clear();
    itemMapper.clear();

    int id = 0;
    QComboBox::insertItem( rootPix, rootItem->text, id );
    itemMapper.insert( id++, rootItem );

    QComboBox::insertItem( homePix, homeItem->text, id );
    itemMapper.insert( id++, homeItem );

    QComboBox::insertItem( desktopPix, desktopItem->text, id );
    itemMapper.insert( id, desktopItem );
}


void KDirComboBox::setURLs( const QStringList& urls )
{
    itemList.clear();
    insertDefaults();

    QStringList::ConstIterator it = urls.begin();
    int overload = urls.count() - myMaximum + FIRSTITEM;
    if ( overload > 0 )
	it = urls.at( overload );

    KDirComboItem *item = 0L;
    QListBox *box = listBox();
    KURL u;
    while ( it != urls.end() ) {
	u = *it;
	if ( !box->findItem( u.path( +1 )) ) { // ignore duplicates
	    item = new KDirComboItem;
	    item->url = u.url( +1 );

	    if ( u.isLocalFile() )
		item->text = u.path( +1 );
	    else
		item->text = *it;

	    insertItem( item );
	    itemList.append( item );
	}
	++it;
    }
}


void KDirComboBox::setURL( const KURL& url )
{
    if ( url.isEmpty() )
        return;

    if ( urlAdded ) {
	itemList.removeLast();
        urlAdded = false;
    }

    insertDefaults();

    QListIterator<KDirComboItem> it( itemList );
    for( ; it.current(); ++it )
        insertItem( it.current() );

    QString urlstr = url.url( +1 ); // we want a / at the end

    // check for duplicates
    QMap<int,const KDirComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
        if ( urlstr == mit.data()->url ) {
            setCurrentItem( mit.key() );
	    changeItem( opendirPix, mit.data()->text, mit.key() );
            return;
        }
        ++mit;
    }

    KDirComboItem *item = new KDirComboItem;
    item->url = urlstr;
    if ( url.isLocalFile() )
        item->text = url.path( +1 );
    else
        item->text = urlstr;

    int id = count();
    QComboBox::insertItem( opendirPix, item->text, id );
    itemMapper.insert( id, item );
    itemList.append( item );

    setCurrentItem( id );
    urlAdded = true;
}

void KDirComboBox::slotActivated( int index )
{
    const KDirComboItem *item = itemMapper[ index ];

    if ( item )
        emit urlActivated( item->url );
}

void KDirComboBox::insertItem( const KDirComboItem *item )
{
    int id = count();
    QComboBox::insertItem( dirPix, item->text, id );
    itemMapper.insert( id, item );
}


void KDirComboBox::setMaxItems( int max )
{
    myMaximum = max;

    if ( count() > myMaximum ) {
	insertDefaults();

	QListIterator<KDirComboItem> it( itemList );
	int overload = itemList.count() - myMaximum + FIRSTITEM;
	for ( int i = 0; i <= overload; i++ )
	    ++it;
	
	for( ; it.current(); ++it )
	    insertItem( it.current() );
    }
}

#include "kdircombobox.moc"
