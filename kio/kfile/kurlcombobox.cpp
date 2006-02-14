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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qdir.h>
#include <q3listbox.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>

#include <kurlcombobox.h>

class KUrlComboBox::KUrlComboBoxPrivate
{
public:
    KUrlComboBoxPrivate() {
	dirIcon = SmallIconSet(QLatin1String("folder"));
    }

    QIcon dirIcon;
};


KUrlComboBox::KUrlComboBox( Mode mode, QWidget *parent)
    : KComboBox( parent),d(new KUrlComboBoxPrivate())
{
    init( mode );
}


KUrlComboBox::KUrlComboBox( Mode mode, bool rw, QWidget *parent)
    : KComboBox( rw, parent),d(new KUrlComboBoxPrivate())
{
    init( mode );
}


KUrlComboBox::~KUrlComboBox()
{
    qDeleteAll( itemList );
    qDeleteAll( defaultList );
    delete d;
}


void KUrlComboBox::init( Mode mode )
{

    myMode    = mode;
    urlAdded  = false;
    myMaximum = 10; // default
    setInsertionPolicy( NoInsertion );
    setTrapReturnKey( true );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));

    opendirIcon = SmallIconSet(QLatin1String("folder_open"));

    connect( this, SIGNAL( activated( int )), SLOT( slotActivated( int )));
}


QStringList KUrlComboBox::urls() const
{
    kDebug(250) << "::urls()" << endl;
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


void KUrlComboBox::addDefaultURL( const KUrl& url, const QString& text )
{
    addDefaultURL( url, getIcon( url ), text );
}


void KUrlComboBox::addDefaultURL( const KUrl& url, const QIcon& icon,
                                  const QString& text )
{
    KUrlComboItem *item = new KUrlComboItem;
    item->url = url;
    item->icon = icon;
    if ( text.isEmpty() )
        if ( url.isLocalFile() )
          item->text = url.path( myMode );
        else
          item->text = url.prettyURL( myMode );
    else
        item->text = text;

    defaultList.append( item );
}


void KUrlComboBox::setDefaults()
{
    clear();
    itemMapper.clear();

    const KUrlComboItem *item;
    for ( int id = 0; id < defaultList.count(); id++ ) {
        item = defaultList.at( id );
        insertURLItem( item );
    }
}

void KUrlComboBox::setURLs( QStringList urls )
{
    setURLs( urls, RemoveBottom );
}

void KUrlComboBox::setURLs( QStringList urls, OverLoadResolving remove )
{
    setDefaults();
    qDeleteAll( itemList );
    itemList.clear();

    if ( urls.isEmpty() )
        return;

    QStringList::Iterator it = urls.begin();

    // kill duplicates
    while ( it != urls.end() ) {
        while ( urls.count( *it ) > 1 ) {
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
        if (remove == RemoveBottom) urls.removeLast();
	else urls.removeFirst();
        Overload--;
    }

    it = urls.begin();

    KUrlComboItem *item = 0L;
    KUrl u;

    while ( it != urls.end() ) {
        if ( (*it).isEmpty() ) {
            ++it;
            continue;
        }
        u = KUrl::fromPathOrURL( *it );

        // Don't restore if file doesn't exist anymore
        if (u.isLocalFile() && !QFile::exists(u.path())) {
            ++it;
            continue;
        }

        item = new KUrlComboItem;
        item->url = u;
        item->icon = getIcon( u );

        if ( u.isLocalFile() )
            item->text = u.path( myMode ); // don't show file:/
        else
            item->text = *it;

        insertURLItem( item );
        itemList.append( item );
        ++it;
    }
}


void KUrlComboBox::setURL( const KUrl& url )
{
    if ( url.isEmpty() )
        return;

    blockSignals( true );

    // check for duplicates
    QMap<int,const KUrlComboItem*>::ConstIterator mit = itemMapper.begin();
    QString urlToInsert = url.url(-1);
    while ( mit != itemMapper.end() ) {
        if ( urlToInsert == mit.value()->url.url(-1) ) {
            setCurrentIndex( mit.key() );

            if ( myMode == Directories )
                updateItem( mit.value(), mit.key(), opendirIcon );

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

    QListIterator<const KUrlComboItem*> it( itemList );
    while ( it.hasNext() )
        insertURLItem( it.next() );

    KUrlComboItem *item = new KUrlComboItem;
    item->url = url;
    item->icon = getIcon( url );
    if ( url.isLocalFile() )
        item->text = url.path( myMode );
    else
        item->text = url.prettyURL( myMode );
     kDebug(250) << "setURL: text=" << item->text << endl;

    int id = count();
    QString text = /*isEditable() ? item->url.prettyURL( myMode ) : */ item->text;

    if ( myMode == Directories )
        KComboBox::insertItem( id,opendirIcon, text);
    else
        KComboBox::insertItem( id,item->icon, text);
    itemMapper.insert( id, item );
    itemList.append( item );

    setCurrentIndex( id );
    urlAdded = true;
    blockSignals( false );
}


void KUrlComboBox::slotActivated( int index )
{
    const KUrlComboItem *item = itemMapper[ index ];

    if ( item ) {
        setURL( item->url );
        emit urlActivated( item->url );
    }
}


void KUrlComboBox::insertURLItem( const KUrlComboItem *item )
{
// kDebug(250) << "insertURLItem " << item->text << endl;
    int id = count();
    KComboBox::insertItem(id, item->icon, item->text);
    itemMapper.insert( id, item );
}


void KUrlComboBox::setMaxItems( int max )
{
    myMaximum = max;

    if ( count() > myMaximum ) {
        int oldCurrent = currentItem();

        setDefaults();

        QListIterator<const KUrlComboItem*> it( itemList );
        int Overload = itemList.count() - myMaximum + defaultList.count();
        for ( int i = 0; i <= Overload; i++ )
            it.next();

        while ( it.hasNext() )
            insertURLItem( it.next() );

        if ( count() > 0 ) { // restore the previous currentItem
            if ( oldCurrent >= count() )
                oldCurrent = count() -1;
            setCurrentIndex( oldCurrent );
        }
    }
}


void KUrlComboBox::removeURL( const KUrl& url, bool checkDefaultURLs )
{
    QMap<int,const KUrlComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
        if ( url.url(-1) == mit.value()->url.url(-1) ) {
            if ( !itemList.remove( mit.value() ) && checkDefaultURLs )
                defaultList.remove( mit.value() );
        }
        ++mit;
    }

    blockSignals( true );
    setDefaults();
    QListIterator<const KUrlComboItem*> it( itemList );
    while ( it.hasNext() ) {
        insertURLItem( it.next() );
    }
    blockSignals( false );
}


QIcon KUrlComboBox::getIcon( const KUrl& url ) const
{
    if ( myMode == Directories )
        return d->dirIcon;
    else
        return SmallIconSet(KMimeType::iconNameForURL( url, 0));
}


// updates "item" with pixmap "pixmap" and sets the URL instead of text
// works around a Qt bug.
void KUrlComboBox::updateItem( const KUrlComboItem *item,
                               int index, const QIcon& icon)
{
    // QComboBox::changeItem() doesn't honor the pixmap when
    // using an editable combobox, so we just remove and insert
    if ( editable() ) {
	removeItem( index );
	insertItem( index,
                    icon,
		    item->url.isLocalFile() ? item->url.path( myMode ) :
		                              item->url.prettyURL( myMode ));
    }
    else {
        setItemIcon(index,icon);
        setItemText(index,item->text);
    }
}


#include "kurlcombobox.moc"
