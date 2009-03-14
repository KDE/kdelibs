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

#include "kurlcombobox.h"

#include <QtCore/QDir>
#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>

#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kiconloader.h>

class KUrlComboBox::KUrlComboBoxPrivate
{
public:
    KUrlComboBoxPrivate(KUrlComboBox *parent)
        : m_parent(parent),
          dirIcon(QLatin1String("folder"))
    {}

    ~KUrlComboBoxPrivate()
    {
      qDeleteAll( itemList );
      qDeleteAll( defaultList );
    }

    typedef struct {
        QString text;
        KUrl url;
        QIcon icon;
    } KUrlComboItem;

    void init( Mode mode );
    void insertUrlItem( const KUrlComboItem * );
    QIcon getIcon( const KUrl& url ) const;
    void updateItem( const KUrlComboItem *item, int index, const QIcon& icon );

    void _k_slotActivated( int );

    KUrlComboBox *m_parent;
    KIcon dirIcon;
    bool urlAdded;
    int myMaximum;
    Mode myMode; // can be used as parameter to KUR::path( int ) or url( int )
                 // to specify if we want a trailing slash or not
    QPoint m_dragPoint;

    QList<const KUrlComboItem*> itemList;
    QList<const KUrlComboItem*> defaultList;
    QMap<int,const KUrlComboItem*> itemMapper;

    QIcon opendirIcon;
};


KUrlComboBox::KUrlComboBox( Mode mode, QWidget *parent)
    : KComboBox( parent),d(new KUrlComboBoxPrivate(this))
{
    d->init( mode );
}


KUrlComboBox::KUrlComboBox( Mode mode, bool rw, QWidget *parent)
    : KComboBox( rw, parent),d(new KUrlComboBoxPrivate(this))
{
    d->init( mode );
}


KUrlComboBox::~KUrlComboBox()
{
    delete d;
}


void KUrlComboBox::KUrlComboBoxPrivate::init( Mode mode )
{
    myMode = mode;
    urlAdded = false;
    myMaximum = 10; // default
    m_parent->setInsertPolicy( NoInsert );
    m_parent->setTrapReturnKey( true );
    m_parent->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
    m_parent->setLayoutDirection( Qt::LeftToRight );
    if ( m_parent->completionObject() ) {
        m_parent->completionObject()->setOrder( KCompletion::Sorted );
    }

    opendirIcon = KIcon(QLatin1String("folder-open"));

    m_parent->connect( m_parent, SIGNAL( activated( int )), SLOT( _k_slotActivated( int )));
}


QStringList KUrlComboBox::urls() const
{
    kDebug(250) << "::urls()";
    //static const QString &fileProt = KGlobal::staticQString("file:");
    QStringList list;
    QString url;
    for ( int i = d->defaultList.count(); i < count(); i++ ) {
        url = itemText( i );
        if ( !url.isEmpty() ) {
            //if ( url.at(0) == '/' )
            //    list.append( url.prepend( fileProt ) );
            //else
                list.append( url );
        }
    }

    return list;
}


void KUrlComboBox::addDefaultUrl( const KUrl& url, const QString& text )
{
    addDefaultUrl( url, d->getIcon( url ), text );
}


void KUrlComboBox::addDefaultUrl( const KUrl& url, const QIcon& icon,
                                  const QString& text )
{
    KUrlComboBoxPrivate::KUrlComboItem *item = new KUrlComboBoxPrivate::KUrlComboItem;
    item->url = url;
    item->icon = icon;
    if ( text.isEmpty() )
        item->text = url.pathOrUrl(d->myMode == Directories
                                   ? KUrl::AddTrailingSlash
                                   : KUrl::RemoveTrailingSlash);
    else
        item->text = text;

    d->defaultList.append( item );
}


void KUrlComboBox::setDefaults()
{
    clear();
    d->itemMapper.clear();

    const KUrlComboBoxPrivate::KUrlComboItem *item;
    for ( int id = 0; id < d->defaultList.count(); id++ ) {
        item = d->defaultList.at( id );
        d->insertUrlItem( item );
    }
}

void KUrlComboBox::setUrls( const QStringList &urls )
{
    setUrls( urls, RemoveBottom );
}

void KUrlComboBox::setUrls( const QStringList &_urls, OverLoadResolving remove )
{
    setDefaults();
    qDeleteAll( d->itemList );
    d->itemList.clear();
    d->urlAdded = false;

    if ( _urls.isEmpty() )
        return;

    QStringList urls;
    QStringList::ConstIterator it = _urls.constBegin();

    // kill duplicates
    while ( it != _urls.constEnd() ) {
        if ( !urls.contains( *it ) )
            urls += *it;
        ++it;
    }

    // limit to myMaximum items
    /* Note: overload is an (old) C++ keyword, some compilers (KCC) choke
       on that, so call it Overload (capital 'O').  (matz) */
    int Overload = urls.count() - d->myMaximum + d->defaultList.count();
    while ( Overload > 0) {
        if (remove == RemoveBottom) {
            if (!urls.isEmpty())
                urls.removeLast();
        }
        else {
            if (!urls.isEmpty())
                urls.removeFirst();
        }
        Overload--;
    }

    it = urls.constBegin();

    KUrlComboBoxPrivate::KUrlComboItem *item = 0L;

    while ( it != urls.constEnd() ) {
        if ( (*it).isEmpty() ) {
            ++it;
            continue;
        }
        KUrl u = *it;

        // Don't restore if file doesn't exist anymore
        if (u.isLocalFile() && !QFile::exists(u.toLocalFile())) {
            ++it;
            continue;
        }

        item = new KUrlComboBoxPrivate::KUrlComboItem;
        item->url = u;
        item->icon = d->getIcon( u );
        item->text = u.pathOrUrl(d->myMode == Directories
                                 ? KUrl::AddTrailingSlash
                                 : KUrl::RemoveTrailingSlash);

        d->insertUrlItem( item );
        d->itemList.append( item );
        ++it;
    }
}


void KUrlComboBox::setUrl( const KUrl& url )
{
    if ( url.isEmpty() )
        return;

    bool blocked = blockSignals( true );

    // check for duplicates
    QMap<int,const KUrlComboBoxPrivate::KUrlComboItem*>::ConstIterator mit = d->itemMapper.constBegin();
    QString urlToInsert = url.url(KUrl::RemoveTrailingSlash);
    while ( mit != d->itemMapper.constEnd() ) {
      Q_ASSERT( mit.value() );

      if ( urlToInsert == mit.value()->url.url(KUrl::RemoveTrailingSlash) ) {
            setCurrentIndex( mit.key() );

            if (d->myMode == Directories)
                d->updateItem( mit.value(), mit.key(), d->opendirIcon );

            blockSignals( blocked );
            return;
        }
        ++mit;
    }

    // not in the combo yet -> create a new item and insert it

    // first remove the old item
    if (d->urlAdded) {
        Q_ASSERT(!d->itemList.isEmpty());
        d->itemList.removeLast();
        d->urlAdded = false;
    }

    setDefaults();

    QListIterator<const KUrlComboBoxPrivate::KUrlComboItem*> it( d->itemList );
    while ( it.hasNext() )
        d->insertUrlItem( it.next() );

    KUrlComboBoxPrivate::KUrlComboItem *item = new KUrlComboBoxPrivate::KUrlComboItem;
    item->url = url;
    item->icon = d->getIcon( url );
    item->text = url.pathOrUrl(d->myMode == Directories
                               ? KUrl::AddTrailingSlash
                               : KUrl::RemoveTrailingSlash);
     kDebug(250) << "setURL: text=" << item->text;

    int id = count();
    QString text = /*isEditable() ? item->url.prettyUrl( (KUrl::AdjustPathOption)myMode ) : */ item->text;

    if (d->myMode == Directories)
        KComboBox::insertItem( id, d->opendirIcon, text);
    else
        KComboBox::insertItem( id,item->icon, text);

    d->itemMapper.insert( id, item );
    d->itemList.append( item );

    setCurrentIndex( id );
    Q_ASSERT(!d->itemList.isEmpty());
    d->urlAdded = true;
    blockSignals( blocked );
}


void KUrlComboBox::KUrlComboBoxPrivate::_k_slotActivated( int index )
{
    const KUrlComboItem *item = itemMapper.value(index);

    if ( item ) {
        m_parent->setUrl( item->url );
        emit m_parent->urlActivated( item->url );
    }
}


void KUrlComboBox::KUrlComboBoxPrivate::insertUrlItem( const KUrlComboBoxPrivate::KUrlComboItem *item )
{
    Q_ASSERT( item );

// kDebug(250) << "insertURLItem " << item->text;
    int id = m_parent->count();
    m_parent->KComboBox::insertItem(id, item->icon, item->text);
    itemMapper.insert( id, item );
}


void KUrlComboBox::setMaxItems( int max )
{
    d->myMaximum = max;

    if (count() > d->myMaximum) {
        int oldCurrent = currentIndex();

        setDefaults();

        QListIterator<const KUrlComboBoxPrivate::KUrlComboItem*> it( d->itemList );
        int Overload = d->itemList.count() - d->myMaximum + d->defaultList.count();
        for ( int i = 0; i <= Overload; i++ )
            it.next();

        while ( it.hasNext() )
            d->insertUrlItem( it.next() );

        if ( count() > 0 ) { // restore the previous currentItem
            if ( oldCurrent >= count() )
                oldCurrent = count() -1;
            setCurrentIndex( oldCurrent );
        }
    }
}

int KUrlComboBox::maxItems() const
{
    return d->myMaximum;
}

void KUrlComboBox::removeUrl( const KUrl& url, bool checkDefaultURLs )
{
    QMap<int,const KUrlComboBoxPrivate::KUrlComboItem*>::ConstIterator mit = d->itemMapper.constBegin();
    while ( mit != d->itemMapper.constEnd() ) {
      if ( url.url(KUrl::RemoveTrailingSlash) == mit.value()->url.url(KUrl::RemoveTrailingSlash) ) {
            if ( !d->itemList.removeAll( mit.value() ) && checkDefaultURLs )
                d->defaultList.removeAll( mit.value() );
        }
        ++mit;
    }

    bool blocked = blockSignals( true );
    setDefaults();
    QListIterator<const KUrlComboBoxPrivate::KUrlComboItem*> it( d->itemList );
    while ( it.hasNext() ) {
        d->insertUrlItem( it.next() );
    }
    blockSignals( blocked );
}

void KUrlComboBox::setCompletionObject(KCompletion* compObj, bool hsig)
{
    if ( compObj ) {
        // on a url combo box we want completion matches to be sorted. This way, if we are given
        // a suggestion, we match the "best" one. For instance, if we have "foo" and "foobar",
        // and we write "foo", the match is "foo" and never "foobar". (ereslibre)
        compObj->setOrder( KCompletion::Sorted );
    }
    KComboBox::setCompletionObject( compObj, hsig );
}

void KUrlComboBox::mousePressEvent(QMouseEvent *event)
{
    QStyleOptionComboBox comboOpt;
    comboOpt.initFrom(this);
    const int x0 = QStyle::visualRect(layoutDirection(), rect(),
                                      style()->subControlRect(QStyle::CC_ComboBox, &comboOpt, QStyle::SC_ComboBoxEditField, this)).x();
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &comboOpt, this);

    if (event->x() < (x0 + KIconLoader::SizeSmall + frameWidth)) {
        d->m_dragPoint = event->pos();
    } else {
        d->m_dragPoint = QPoint();
    }

    KComboBox::mousePressEvent(event);
}

void KUrlComboBox::mouseMoveEvent(QMouseEvent *event)
{
    const int index = currentIndex();

    if (!itemIcon(index).isNull() && !d->m_dragPoint.isNull() && event->buttons() & Qt::LeftButton &&
        (event->pos() - d->m_dragPoint).manhattanLength() > KGlobalSettings::dndEventDelay()) {
        QDrag *drag = new QDrag(this);
        QMimeData *mime = new QMimeData();
        mime->setUrls(QList<QUrl>() << KUrl(itemText(index)));
        mime->setText(itemText(index));
        drag->setPixmap(itemIcon(index).pixmap(KIconLoader::SizeMedium));
        drag->setMimeData(mime);
        drag->exec();
    }

    KComboBox::mouseMoveEvent(event);
}

QIcon KUrlComboBox::KUrlComboBoxPrivate::getIcon( const KUrl& url ) const
{
    if (myMode == Directories)
        return dirIcon;
    else
        return KIcon(KMimeType::iconNameForUrl(url, 0));
}


// updates "item" with icon "icon" and sets the URL instead of text
void KUrlComboBox::KUrlComboBoxPrivate::updateItem( const KUrlComboBoxPrivate::KUrlComboItem *item,
                                                    int index, const QIcon& icon)
{
    m_parent->setItemIcon(index,icon);

    if ( m_parent->isEditable() ) {
        m_parent->setItemText(index, item->url.pathOrUrl(myMode == Directories
                                                         ? KUrl::AddTrailingSlash
                                                         : KUrl::RemoveTrailingSlash));
    }
    else {
        m_parent->setItemText(index,item->text);
    }
}


#include "kurlcombobox.moc"
