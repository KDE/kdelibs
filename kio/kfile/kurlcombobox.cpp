/* This file is part of the KDE libraries
    Copyright (C) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

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

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>

#include <kurlcombobox.h>

class KURLComboBox::KURLComboBoxPrivate
{
public:
    KURLComboBoxPrivate() {
	dirpix = SmallIcon(QString::fromLatin1("folder"));
    }

    QPixmap dirpix;
};


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
    delete d;
}


void KURLComboBox::init( Mode mode )
{
    d = new KURLComboBoxPrivate();

    myMode    = mode;
    urlAdded  = false;
    myMaximum = 10; // default
    itemList.setAutoDelete( true );
    defaultList.setAutoDelete( true );
    setInsertionPolicy( NoInsertion );
    setTrapReturnKey( true );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));

    opendirPix = SmallIcon(QString::fromLatin1("folder_open"));

    connect( this, SIGNAL( activated( int )), SLOT( slotActivated( int )));
}


QStringList KURLComboBox::urls() const
{
    kdDebug(250) << "::urls()" << endl;
    //static const QString &fileProt = KGlobal::staticQString("file:");
    QStringList list;
    QString url;
    for ( int i = defaultList.count(); i < count(); i++ ) {
        url = text( i );
        if ( !url.isEmpty() ) {
            //if ( url.at(0) == '/' )
            //    list.append( url.prepend( fileProt ) );
            //else
                list.append( url );
        }
    }

    return list;
}


void KURLComboBox::addDefaultURL( const KURL& url, const QString& text )
{
    addDefaultURL( url, getPixmap( url ), text );
}


void KURLComboBox::addDefaultURL( const KURL& url, const QPixmap& pix,
                                  const QString& text )
{
    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = pix;
    if ( text.isEmpty() )
        if ( url.isLocalFile() )
          item->text = url.path( myMode );
        else
          item->text = url.prettyURL( myMode );
    else
        item->text = text;

    defaultList.append( item );
}


void KURLComboBox::setDefaults()
{
    clear();
    itemMapper.clear();

    KURLComboItem *item;
    for ( unsigned int id = 0; id < defaultList.count(); id++ ) {
        item = defaultList.at( id );
        insertURLItem( item );
    }
}

void KURLComboBox::setURLs( QStringList urls )
{
    setURLs( urls, RemoveBottom );
}

void KURLComboBox::setURLs( QStringList urls, OverLoadResolving remove )
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
    /* Note: overload is an (old) C++ keyword, some compilers (KCC) choke
       on that, so call it Overload (capital 'O').  (matz) */
    int Overload = urls.count() - myMaximum + defaultList.count();
    while ( Overload > 0 ) {
        urls.remove((remove == RemoveBottom) ? urls.fromLast() : urls.begin());
        Overload--;
    }

    it = urls.begin();

    KURLComboItem *item = 0L;
    KURL u;

    while ( it != urls.end() ) {
        if ( (*it).isEmpty() ) {
            ++it;
            continue;
        }
        u = KURL::fromPathOrURL( *it );

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

    blockSignals( true );

    // check for duplicates
    QMap<int,const KURLComboItem*>::ConstIterator mit = itemMapper.begin();
    QString urlToInsert = url.url(-1);
    while ( mit != itemMapper.end() ) {
        if ( urlToInsert == mit.data()->url.url(-1) ) {
            setCurrentItem( mit.key() );

            if ( myMode == Directories )
                updateItem( mit.data(), mit.key(), opendirPix );

            blockSignals( false );
            return;
        }
        ++mit;
    }

    // not in the combo yet -> create a new item and insert it

    // first remove the old item
    if ( urlAdded ) {
        itemList.removeLast();
        urlAdded = false;
    }

    setDefaults();

    QPtrListIterator<KURLComboItem> it( itemList );
    for( ; it.current(); ++it )
        insertURLItem( it.current() );

    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = getPixmap( url );
    if ( url.isLocalFile() )
        item->text = url.path( myMode );
    else
        item->text = url.prettyURL( myMode );
     kdDebug(250) << "setURL: text=" << item->text << endl;

    int id = count();
    QString text = /*isEditable() ? item->url.prettyURL( myMode ) : */ item->text;

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
        setURL( item->url );
        emit urlActivated( item->url );
    }
}


void KURLComboBox::insertURLItem( const KURLComboItem *item )
{
// kdDebug(250) << "insertURLItem " << item->text << endl;
    int id = count();
    KComboBox::insertItem( item->pixmap, item->text, id );
    itemMapper.insert( id, item );
}


void KURLComboBox::setMaxItems( int max )
{
    myMaximum = max;

    if ( count() > myMaximum ) {
        int oldCurrent = currentItem();

        setDefaults();

        QPtrListIterator<KURLComboItem> it( itemList );
        int Overload = itemList.count() - myMaximum + defaultList.count();
        for ( int i = 0; i <= Overload; i++ )
            ++it;

        for( ; it.current(); ++it )
            insertURLItem( it.current() );

        if ( count() > 0 ) { // restore the previous currentItem
            if ( oldCurrent >= count() )
                oldCurrent = count() -1;
            setCurrentItem( oldCurrent );
        }
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
    QPtrListIterator<KURLComboItem> it( itemList );
    while ( it.current() ) {
        insertURLItem( *it );
        ++it;
    }
    blockSignals( false );
}


QPixmap KURLComboBox::getPixmap( const KURL& url ) const
{
    if ( myMode == Directories )
        return d->dirpix;
    else
        return KMimeType::pixmapForURL( url, 0, KIcon::Small );
}


// updates "item" with pixmap "pixmap" and sets the URL instead of text
// works around a Qt bug.
void KURLComboBox::updateItem( const KURLComboItem *item,
                               int index, const QPixmap& pixmap )
{
    // QComboBox::changeItem() doesn't honour the pixmap when
    // using an editable combobox, so we just remove and insert
    if ( editable() ) {
	removeItem( index );
	insertItem( pixmap,
		    item->url.isLocalFile() ? item->url.path( myMode ) :
		                              item->url.prettyURL( myMode ),
		    index );
    }
    else
        changeItem( pixmap, item->text, index );
}


#include "kurlcombobox.moc"
