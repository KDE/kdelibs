/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "khistorycombobox.h"

#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QWheelEvent>

#include <klocale.h>
#include <knotification.h>
#include <kpixmapprovider.h>
#include <kstandardshortcut.h>
#include <kicontheme.h>
#include <kicon.h>

#include <kdebug.h>

class KHistoryComboBox::Private
{
public:
    Private(KHistoryComboBox *q): q(q), myPixProvider(0) {}

    KHistoryComboBox *q;

    /**
    * The current position (index) in the combobox, used for Up and Down
     */
    int myIterateIndex;

    /**
    * The text typed before Up or Down was pressed.
     */
    QString myText;

    /**
    * Indicates that the user at least once rotated Up through the entire list
     * Needed to allow going back after rotation.
     */
    bool myRotated;
    KPixmapProvider *myPixProvider;
};

// we are always read-write
KHistoryComboBox::KHistoryComboBox( QWidget *parent )
     : KComboBox( true, parent ), d(new Private(this))
{
    init( true ); // using completion
}

// we are always read-write
KHistoryComboBox::KHistoryComboBox( bool useCompletion,
                              QWidget *parent )
    : KComboBox( true, parent ), d(new Private(this))
{
    init( useCompletion );
}

void KHistoryComboBox::init( bool useCompletion )
{
    // Set a default history size to something reasonable, Qt sets it to INT_MAX by default
    setMaxCount( 50 );

    if ( useCompletion )
        completionObject()->setOrder( KCompletion::Weighted );

    setInsertPolicy( NoInsert );
    d->myIterateIndex = -1;
    d->myRotated = false;
    d->myPixProvider = 0L;

    // obey HISTCONTROL setting
    QByteArray histControl = qgetenv("HISTCONTROL");
    if ( histControl == "ignoredups" || histControl == "ignoreboth" )
        setDuplicatesEnabled( false );

    connect(this, SIGNAL(aboutToShowContextMenu(QMenu*)), SLOT(addContextMenuItems(QMenu*)));
    connect(this, SIGNAL(activated(int)), SLOT(slotReset()));
    connect(this, SIGNAL(returnPressed(QString)), SLOT(slotReset()));
    // We want slotSimulateActivated to be called _after_ QComboBoxPrivate::_q_returnPressed
    // otherwise there's a risk of emitting activated twice (slotSimulateActivated will find
    // the item, after some app's slotActivated inserted the item into the combo).
    connect(this, SIGNAL(returnPressed(QString)), SLOT(slotSimulateActivated(QString)), Qt::QueuedConnection);
}

KHistoryComboBox::~KHistoryComboBox()
{
    delete d->myPixProvider;
    delete d;
}

void KHistoryComboBox::setHistoryItems( const QStringList &items )
{
    setHistoryItems(items, false);
}

void KHistoryComboBox::setHistoryItems( const QStringList &items,
                                     bool setCompletionList )
{
    QStringList insertingItems = items;
    KComboBox::clear();

    // limit to maxCount()
    const int itemCount = insertingItems.count();
    const int toRemove = itemCount - maxCount();

    if (toRemove >= itemCount) {
        insertingItems.clear();
    } else {
        for (int i = 0; i < toRemove; ++i)
            insertingItems.pop_front();
    }

    insertItems( insertingItems );

    if ( setCompletionList && useCompletion() ) {
        // we don't have any weighting information here ;(
        KCompletion *comp = completionObject();
        comp->setOrder( KCompletion::Insertion );
        comp->setItems( insertingItems );
        comp->setOrder( KCompletion::Weighted );
    }

    clearEditText();
}

QStringList KHistoryComboBox::historyItems() const
{
    QStringList list;
    const int itemCount = count();
    for ( int i = 0; i < itemCount; ++i )
        list.append( itemText( i ) );

    return list;
}

bool KHistoryComboBox::useCompletion() const
{
    return compObj();
}

void KHistoryComboBox::clearHistory()
{
    const QString temp = currentText();
    KComboBox::clear();
    if ( useCompletion() )
        completionObject()->clear();
    setEditText( temp );
}

void KHistoryComboBox::addContextMenuItems( QMenu* menu )
{
    if ( menu )
    {
        menu->addSeparator();
        QAction* clearHistory = menu->addAction( KIcon("edit-clear-history"), i18n("Clear &History"), this, SLOT( slotClear()));
        if (!count())
           clearHistory->setEnabled(false);
    }
}

void KHistoryComboBox::addToHistory( const QString& item )
{
    if ( item.isEmpty() || (count() > 0 && item == itemText(0) )) {
        return;
    }

    bool wasCurrent = false;
    // remove all existing items before adding
    if ( !duplicatesEnabled() ) {
        int i = 0;
        int itemCount = count();
        while ( i < itemCount ) {
            if ( itemText( i ) == item ) {
                if ( !wasCurrent )
                  wasCurrent = ( i == currentIndex() );
                removeItem( i );
                --itemCount;
            } else {
                ++i;
            }
        }
    }

    // now add the item
    if ( d->myPixProvider )
        insertItem( 0, d->myPixProvider->pixmapFor(item, KIconLoader::SizeSmall), item);
    else
        insertItem( 0, item );

    if ( wasCurrent )
        setCurrentIndex( 0 );

    const bool useComp = useCompletion();

    const int last = count() - 1; // last valid index
    const int mc = maxCount();
    const int stopAt = qMax(mc, 0);

    for (int rmIndex = last; rmIndex >= stopAt; --rmIndex) {
        // remove the last item, as long as we are longer than maxCount()
        // remove the removed item from the completionObject if it isn't
        // anymore available at all in the combobox.
        const QString rmItem = itemText( rmIndex );
        removeItem( rmIndex );
        if ( useComp && !contains( rmItem ) )
            completionObject()->removeItem( rmItem );
    }

    if ( useComp )
        completionObject()->addItem( item );
}

bool KHistoryComboBox::removeFromHistory( const QString& item )
{
    if ( item.isEmpty() )
        return false;

    bool removed = false;
    const QString temp = currentText();
    int i = 0;
    int itemCount = count();
    while ( i < itemCount ) {
        if ( item == itemText( i ) ) {
            removed = true;
            removeItem( i );
            --itemCount;
        } else {
            ++i;
        }
    }

    if ( removed && useCompletion() )
        completionObject()->removeItem( item );

    setEditText( temp );
    return removed;
}

void KHistoryComboBox::rotateUp()
{
    // save the current text in the lineedit
    if ( d->myIterateIndex == -1 )
        d->myText = currentText();

    ++d->myIterateIndex;

    // skip duplicates/empty items
    const int last = count() - 1; // last valid index
    const QString currText = currentText();

    while ( d->myIterateIndex < last &&
            (currText == itemText( d->myIterateIndex ) ||
             itemText( d->myIterateIndex ).isEmpty()) )
        ++d->myIterateIndex;

    if ( d->myIterateIndex >= count() ) {
        d->myRotated = true;
        d->myIterateIndex = -1;

        // if the typed text is the same as the first item, skip the first
        if ( count() > 0 && d->myText == itemText(0) )
            d->myIterateIndex = 0;

        setEditText( d->myText );
    }
    else
        setEditText( itemText( d->myIterateIndex ));
}

void KHistoryComboBox::rotateDown()
{
    // save the current text in the lineedit
    if ( d->myIterateIndex == -1 )
        d->myText = currentText();

    --d->myIterateIndex;

    const QString currText = currentText();
    // skip duplicates/empty items
    while ( d->myIterateIndex >= 0 &&
            (currText == itemText( d->myIterateIndex ) ||
             itemText( d->myIterateIndex ).isEmpty()) )
        --d->myIterateIndex;


    if ( d->myIterateIndex < 0 ) {
        if ( d->myRotated && d->myIterateIndex == -2 ) {
            d->myRotated = false;
            d->myIterateIndex = count() - 1;
            setEditText( itemText(d->myIterateIndex) );
        }
        else { // bottom of history
            if ( d->myIterateIndex == -2 ) {
                KNotification::event( "Textcompletion: No Match" ,
                                      i18n("No further items in the history."),
                                       QPixmap() , this, KNotification::DefaultEvent);
            }

            d->myIterateIndex = -1;
            if ( currentText() != d->myText )
                setEditText( d->myText );
        }
    }
    else
        setEditText( itemText( d->myIterateIndex ));

}

void KHistoryComboBox::keyPressEvent( QKeyEvent *e )
{
    int event_key = e->key() | e->modifiers();

    // going up in the history, rotating when reaching QListBox::count()
    if ( KStandardShortcut::rotateUp().contains(event_key) )
        rotateUp();

    // going down in the history, no rotation possible. Last item will be
    // the text that was in the lineedit before Up was called.
    else if ( KStandardShortcut::rotateDown().contains(event_key) )
        rotateDown();
    else
        KComboBox::keyPressEvent( e );
}

void KHistoryComboBox::wheelEvent( QWheelEvent *ev )
{
    // Pass to poppable listbox if it's up
    QAbstractItemView* const iv = view();
    if ( iv && iv->isVisible() )
    {
        QApplication::sendEvent( iv, ev );
        return;
    }
    // Otherwise make it change the text without emitting activated
    if ( ev->delta() > 0 ) {
        rotateUp();
    } else {
        rotateDown();
    }
    ev->accept();
}

void KHistoryComboBox::slotReset()
{
    d->myIterateIndex = -1;
    d->myRotated = false;
}


void KHistoryComboBox::setPixmapProvider( KPixmapProvider *prov )
{
    if ( d->myPixProvider == prov )
        return;

    delete d->myPixProvider;
    d->myPixProvider = prov;

    // re-insert all the items with/without pixmap
    // I would prefer to use changeItem(), but that doesn't honor the pixmap
    // when using an editable combobox (what we do)
    if ( count() > 0 ) {
        QStringList items( historyItems() );
        clear();
        insertItems( items );
    }
}

void KHistoryComboBox::insertItems( const QStringList& items )
{
    QStringList::ConstIterator it = items.constBegin();
    const QStringList::ConstIterator itEnd = items.constEnd();

    while ( it != itEnd ) {
        const QString item = *it;
        if ( !item.isEmpty() ) { // only insert non-empty items
            if ( d->myPixProvider )
                addItem( d->myPixProvider->pixmapFor(item, KIconLoader::SizeSmall),
                            item );
            else
                addItem( item );
        }
        ++it;
    }
}

void KHistoryComboBox::slotClear()
{
    clearHistory();
    emit cleared();
}

void KHistoryComboBox::slotSimulateActivated( const QString& text )
{
    /* With the insertion policy NoInsert, which we use by default,
       Qt doesn't emit activated on typed text if the item is not already there,
       which is perhaps reasonable. Generate the signal ourselves if that's the case.
    */
    if ((insertPolicy() == NoInsert && findText(text, Qt::MatchFixedString|Qt::MatchCaseSensitive) == -1)) {
        emit activated(text);
    }

    /*
       Qt also doesn't emit it if the box is full, and policy is not
       InsertAtCurrent
    */
    else if (insertPolicy() != InsertAtCurrent && count() >= maxCount()) {
        emit activated(text);
    }
}

KPixmapProvider *KHistoryComboBox::pixmapProvider() const
{
  return d->myPixProvider;
}

void KHistoryComboBox::reset()
{
  slotReset();
}

#include "khistorycombobox.moc"
