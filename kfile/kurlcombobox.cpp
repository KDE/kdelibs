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

#include <kurlcombobox.h>

KURLComboBox::KURLComboBox( Mode mode, QWidget *parent, const char *name )
    : KComboBox( parent, name )
{
    init( mode );
}


KURLComboBox::KURLComboBox( Mode mode, bool rw, QWidget *parent,
			    const char *name )
    : KComboBox( rw, parent, name )
{
    init( mode );
}


KURLComboBox::~KURLComboBox()
{
}


void KURLComboBox::init( Mode mode )
{
    myMode    = mode;
    urlAdded  = false;
    myMaximum = 10; // default
    firstItemIndex = 0;
    itemList.setAutoDelete( true );
    defaultList.setAutoDelete( true );
    setInsertionPolicy( NoInsertion );
    setTrapReturnKey( true );

    opendirPix = SmallIcon(QString::fromLatin1("folder_open"));

    connect( this, SIGNAL( activated( int )), SLOT( slotActivated( int )));
}


QStringList KURLComboBox::urls() const
{
    static const QString &fileProt = KGlobal::staticQString("file:");
    QStringList list;
    QString url;
    for ( int i = firstItemIndex; i < count(); i++ ) {
	url = text( i );
	if ( !url.isEmpty() ) {
	    if ( url.at(0) == '/' )
		list.append( url.prepend( fileProt ) );
	    else
		list.append( url );
	}
    }

    return list;
}


void KURLComboBox::addDefaultURL( const KURL& url, const QString& text )
{
    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = getPixmap( url );
    if ( text.isEmpty() )
	item->text = url.url( myMode );
    else
	item->text = text;

    firstItemIndex++;
    defaultList.append( item );
}


void KURLComboBox::addDefaultURL( const KURL& url, const QPixmap& pix,
				  const QString& text )
{
    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = pix;
    if ( text.isEmpty() )
	item->text = url.url( myMode );
    else
	item->text = text;

    firstItemIndex++;
    defaultList.append( item );
}
				

void KURLComboBox::setDefaults()
{
    clear();
    itemMapper.clear();

    KURLComboItem *item;
    for ( int id = 0; id < firstItemIndex; id++ ) {
	item = defaultList.at( id );
	insertURLItem( item );
    }
}


void KURLComboBox::setURLs( QStringList urls )
{
    setDefaults();
    itemList.clear();

    if ( urls.isEmpty() )
	return;

    QStringList::Iterator it = urls.begin();

    // kill duplicates
    QString text;
    while ( it != urls.end() ) {
	while ( urls.contains( *it ) > 1 ) {
	    it = urls.remove( it );
	    continue;
	}
	++it;
    }

    // limit to myMaximum items
    int overload = urls.count() - myMaximum + firstItemIndex;
    if ( overload < 0 )
	overload = 0;
    it = urls.at( overload );


    KURLComboItem *item = 0L;
    KURL u;

    while ( it != urls.end() ) {
	if ( (*it).isEmpty() ) {
	    ++it;
	    continue;
	}
	u = *it;

	item = new KURLComboItem;
	item->url = u;
	item->pixmap = getPixmap( u );

	if ( u.isLocalFile() )
	    item->text = u.path( myMode ); // don't show file:/
	else
	    item->text = *it;

	insertURLItem( item );
	itemList.append( item );
	++it;
    }
}


void KURLComboBox::setURL( const KURL& url )
{
    if ( url.isEmpty() )
        return;

    if ( urlAdded ) {
	itemList.removeLast();
        urlAdded = false;
    }

    blockSignals( true );
    setDefaults();

    QListIterator<KURLComboItem> it( itemList );
    for( ; it.current(); ++it )
        insertURLItem( it.current() );

    // check for duplicates
    QMap<int,const KURLComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
        if ( url.url(-1) == mit.data()->url.url(-1) ) {
            setCurrentItem( mit.key() );

	    if ( myMode == Directories ) {
		
		// QComboBox::changeItem() doesn't honour the pixmap when
		// using an editable combobox, so we just remove and insert
// hmm, this seems to be fixed?
//		if ( isEditable() ) {
//		    removeItem( mit.key() );
//		    insertItem( opendirPix, mit.data()->url.url( myMode ),
//				mit.key() );
//		}
//		else
		    changeItem( opendirPix, mit.data()->text, mit.key() );
	    }
            blockSignals( false );
            return;
        }
        ++mit;
    }

    // not in the combo yet -> create a new item and insert it
    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = getPixmap( url );
    if ( url.isLocalFile() )
        item->text = url.path( myMode );
    else
        item->text = url.url( myMode );

    int id = count();
    QString text = isEditable() ? item->url.url( myMode ) : item->text;

    if ( myMode == Directories )
	KComboBox::insertItem( opendirPix, text, id );
    else
	KComboBox::insertItem( item->pixmap, text, id );
    itemMapper.insert( id, item );
    itemList.append( item );

    setCurrentItem( id );
    urlAdded = true;
    blockSignals( false );
}


void KURLComboBox::slotActivated( int index )
{
    const KURLComboItem *item = itemMapper[ index ];

    if ( item ) {
	blockSignals( true ); // don't produce a textChanged() signal
	setEditText( item->url.url( myMode ) );
	blockSignals( false );
	emit urlActivated( item->url );
    }
}


void KURLComboBox::insertURLItem( const KURLComboItem *item )
{
    int id = count();
    KComboBox::insertItem( item->pixmap, item->text, id );
    itemMapper.insert( id, item );
}


void KURLComboBox::setMaxItems( int max )
{
    myMaximum = max;

    if ( count() > myMaximum ) {
	setDefaults();

	QListIterator<KURLComboItem> it( itemList );
	int overload = itemList.count() - myMaximum + firstItemIndex;
	for ( int i = 0; i <= overload; i++ )
	    ++it;
	
	for( ; it.current(); ++it )
	    insertURLItem( it.current() );
    }
}


void KURLComboBox::removeURL( const KURL& url, bool checkDefaultURLs )
{
    QMap<int,const KURLComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
        if ( url.url(-1) == mit.data()->url.url(-1) ) {
	    if ( !itemList.remove( mit.data() ) && checkDefaultURLs )
		defaultList.remove( mit.data() );
	}
	++mit;
    }

    blockSignals( true );
    setDefaults();
    QListIterator<KURLComboItem> it( itemList );
    while ( it.current() ) {
	insertURLItem( *it );
	++it;
    }
    blockSignals( false );
}


QPixmap KURLComboBox::getPixmap( const KURL& url ) const
{
    static QPixmap dirpix = SmallIcon(QString::fromLatin1("folder"));

    if ( myMode == Directories )
	return dirpix;
    else
	return KMimeType::pixmapForURL( url, 0, KIcon::Small );
}


#include "kurlcombobox.moc"
