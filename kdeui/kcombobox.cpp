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

#include <qclipboard.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include <kcompletionbox.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kpixmapprovider.h>
#include <kstdaccel.h>
#include <kurl.h>
#include <kurldrag.h>

#include <kdebug.h>

#include "kcombobox.h"

#include <stdlib.h> // getenv

class KComboBox::KComboBoxPrivate
{
public:
    KComboBoxPrivate() : klineEdit(0L)
    {
    }
    ~KComboBoxPrivate()
    {
    }

    KLineEdit *klineEdit;
};

KComboBox::KComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name ), d(new KComboBoxPrivate)
{
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
    : QComboBox( rw, parent, name ), d(new KComboBoxPrivate)
{
    init();

    if ( rw )
    {
      KLineEdit *edit = new KLineEdit( this, "combo lineedit" );
      setLineEdit( edit );
    }
}

KComboBox::~KComboBox()
{
    delete d;
}

void KComboBox::init()
{
    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );
}


bool KComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i )
    {
        if ( text(i) == _text )
            return true;
    }
    return false;
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    if ( d->klineEdit )
    {
        if ( autocomplete )
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
            setCompletionMode( KGlobalSettings::CompletionAuto );
        }
        else
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::completionMode() );
            setCompletionMode( KGlobalSettings::completionMode() );
        }
    }
}

void KComboBox::setContextMenuEnabled( bool showMenu )
{
    if( d->klineEdit )
        d->klineEdit->setContextMenuEnabled( showMenu );
}


void KComboBox::setURLDropsEnabled( bool enable )
{
    if ( d->klineEdit )
        d->klineEdit->setURLDropsEnabled( enable );
}

bool KComboBox::isURLDropsEnabled() const
{
    return d->klineEdit && d->klineEdit->isURLDropsEnabled();
}


void KComboBox::setCompletedText( const QString& text, bool marked )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text, marked );
}

void KComboBox::setCompletedText( const QString& text )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text );
}

void KComboBox::makeCompletion( const QString& text )
{
    if( d->klineEdit )
        d->klineEdit->makeCompletion( text );

    else // read-only combo completion
    {
        if( text.isNull() || !listBox() )
            return;

        const int index = listBox()->index( listBox()->findItem( text ) );
        if( index >= 0 )
            setCurrentItem( index );
    }
}

void KComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if ( d->klineEdit )
        d->klineEdit->rotateText( type );
}

// not needed anymore
bool KComboBox::eventFilter( QObject* o, QEvent* ev )
{
    return QComboBox::eventFilter( o, ev );
}

void KComboBox::setTrapReturnKey( bool grab )
{
    if ( d->klineEdit )
        d->klineEdit->setTrapReturnKey( grab );
    else
        qWarning("KComboBox::setTrapReturnKey not supported with a non-KLineEdit.");
}

bool KComboBox::trapReturnKey() const
{
    return d->klineEdit && d->klineEdit->trapReturnKey();
}


void KComboBox::setEditURL( const KURL& url )
{
    QComboBox::setEditText( url.prettyURL() );
}

void KComboBox::insertURL( const KURL& url, int index )
{
    QComboBox::insertItem( url.prettyURL(), index );
}

void KComboBox::insertURL( const QPixmap& pixmap, const KURL& url, int index )
{
    QComboBox::insertItem( pixmap, url.prettyURL(), index );
}

void KComboBox::changeURL( const KURL& url, int index )
{
    QComboBox::changeItem( url.prettyURL(), index );
}

void KComboBox::changeURL( const QPixmap& pixmap, const KURL& url, int index )
{
    QComboBox::changeItem( pixmap, url.prettyURL(), index );
}

void KComboBox::setCompletedItems( const QStringList& items )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedItems( items );
}

KCompletionBox * KComboBox::completionBox( bool create )
{
    if ( d->klineEdit )
        return d->klineEdit->completionBox( create );
    return 0;
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KComboBox::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QComboBox::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( lineEdit(), true, true );
}

void KComboBox::wheelEvent( QWheelEvent *ev )
{
    // Not necessary anymore
    QComboBox::wheelEvent( ev );
}

void KComboBox::setLineEdit( QLineEdit *edit )
{
    if ( !editable() && edit &&
         !qstrcmp( edit->className(), "QLineEdit" ) )
    {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable( true ), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        edit = new KLineEdit( this, "combo edit" );
    }

    QComboBox::setLineEdit( edit );
    d->klineEdit = dynamic_cast<KLineEdit*>( edit );
    setDelegate( d->klineEdit );

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit)
        connect( edit, SIGNAL( returnPressed() ), SIGNAL( returnPressed() ));

    if ( d->klineEdit )
    {
        // someone calling KComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect( edit, SIGNAL( destroyed() ), SLOT( lineEditDeleted() ));

        connect( d->klineEdit, SIGNAL( returnPressed( const QString& )),
                 SIGNAL( returnPressed( const QString& ) ));

        connect( d->klineEdit, SIGNAL( completion( const QString& )),
                 SIGNAL( completion( const QString& )) );

        connect( d->klineEdit, SIGNAL( substringCompletion( const QString& )),
                 SIGNAL( substringCompletion( const QString& )) );

        connect( d->klineEdit,
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )),
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )) );

        connect( d->klineEdit,
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion )),
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion)));

        connect( d->klineEdit,
                 SIGNAL( aboutToShowContextMenu( QPopupMenu * )),
                 SIGNAL( aboutToShowContextMenu( QPopupMenu * )) );

        connect( d->klineEdit,
                 SIGNAL( completionBoxActivated( const QString& )),
                 SIGNAL( activated( const QString& )) );
    }
}

void KComboBox::setCurrentItem( const QString& item, bool insert, int index )
{
    int sel = -1;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i)
    {
        if (text(i) == item)
        {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert)
    {
        insertItem(item, index);
        if (index >= 0)
            sel = index;
        else
            sel = count() - 1;
    }
    setCurrentItem(sel);
}

void KComboBox::lineEditDeleted()
{
    // yes, we need those ugly casts due to the multiple inheritance
    // sender() is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const KCompletionBase *base = static_cast<const KCompletionBase*>( static_cast<const KLineEdit*>( sender() ));

    // is it our delegate, that is destroyed?
    if ( base == delegate() )
        setDelegate( 0L );
}


// *********************************************************************
// *********************************************************************


// we are always read-write
KHistoryCombo::KHistoryCombo( QWidget *parent, const char *name )
    : KComboBox( true, parent, name ), d(0)
{
    init( true ); // using completion
}

// we are always read-write
KHistoryCombo::KHistoryCombo( bool useCompletion,
                              QWidget *parent, const char *name )
    : KComboBox( true, parent, name ), d(0)
{
    init( useCompletion );
}

void KHistoryCombo::init( bool useCompletion )
{
    // Set a default history size to something reasonable, Qt sets it to INT_MAX by default
    setMaxCount( 50 );

    if ( useCompletion )
        completionObject()->setOrder( KCompletion::Weighted );

    setInsertionPolicy( NoInsertion );
    myIterateIndex = -1;
    myRotated = false;
    myPixProvider = 0L;

    // obey HISTCONTROL setting
    QCString histControl = getenv("HISTCONTROL");
    if ( histControl == "ignoredups" || histControl == "ignoreboth" )
        setDuplicatesEnabled( false );

    connect( this, SIGNAL(aboutToShowContextMenu(QPopupMenu*)),
             SLOT(addContextMenuItems(QPopupMenu*)) );
    connect( this, SIGNAL( activated(int) ), SLOT( slotReset() ));
    connect( this, SIGNAL( returnPressed(const QString&) ), SLOT(slotReset()));
}

KHistoryCombo::~KHistoryCombo()
{
    delete myPixProvider;
}

void KHistoryCombo::setHistoryItems( QStringList items,
                                     bool setCompletionList )
{
    KComboBox::clear();

    // limit to maxCount()
    const int itemCount = items.count(); 
    const int toRemove = itemCount - maxCount();

    if (toRemove >= itemCount) {
        items.clear();
    } else {
        for (int i = 0; i < toRemove; ++i) 
            items.pop_front();
    }

    insertItems( items );

    if ( setCompletionList && useCompletion() ) {
        // we don't have any weighting information here ;(
        KCompletion *comp = completionObject();
        comp->setOrder( KCompletion::Insertion );
        comp->setItems( items );
        comp->setOrder( KCompletion::Weighted );
    }

    clearEdit();
}

QStringList KHistoryCombo::historyItems() const
{
    QStringList list;
    const int itemCount = count();
    for ( int i = 0; i < itemCount; ++i )
        list.append( text( i ) );

    return list;
}

void KHistoryCombo::clearHistory()
{
    const QString temp = currentText();
    KComboBox::clear();
    if ( useCompletion() )
        completionObject()->clear();
    setEditText( temp );
}

void KHistoryCombo::addContextMenuItems( QPopupMenu* menu )
{
    if ( menu )
    {
        menu->insertSeparator();
        int id = menu->insertItem( SmallIconSet("history_clear"), i18n("Clear &History"), this, SLOT( slotClear()));
        if (!count())
           menu->setItemEnabled(id, false);
    }
}

void KHistoryCombo::addToHistory( const QString& item )
{
    if ( item.isEmpty() || (count() > 0 && item == text(0) )) {
        return;
    }

    bool wasCurrent = false;
    // remove all existing items before adding
    if ( !duplicatesEnabled() ) {
        int i = 0;
        int itemCount = count();
        while ( i < itemCount ) {
            if ( text( i ) == item ) {
                if ( !wasCurrent )
                  wasCurrent = ( i == currentItem() );
                removeItem( i );
                --itemCount;
            } else {
                ++i;
            }
        }
    }

    // now add the item
    if ( myPixProvider )
        insertItem( myPixProvider->pixmapFor(item, KIcon::SizeSmall), item, 0);
    else
        insertItem( item, 0 );

    if ( wasCurrent )
        setCurrentItem( 0 );

    const bool useComp = useCompletion();

    const int last = count() - 1; // last valid index
    const int mc = maxCount();
    const int stopAt = QMAX(mc, 0);

    for (int rmIndex = last; rmIndex >= stopAt; --rmIndex) {
        // remove the last item, as long as we are longer than maxCount()
        // remove the removed item from the completionObject if it isn't
        // anymore available at all in the combobox.
        const QString rmItem = text( rmIndex );
        removeItem( rmIndex );
        if ( useComp && !contains( rmItem ) )
            completionObject()->removeItem( rmItem );
    }

    if ( useComp )
        completionObject()->addItem( item );
}

bool KHistoryCombo::removeFromHistory( const QString& item )
{
    if ( item.isEmpty() )
        return false;

    bool removed = false;
    const QString temp = currentText();
    int i = 0;
    int itemCount = count();
    while ( i < itemCount ) {
        if ( item == text( i ) ) {
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

void KHistoryCombo::rotateUp()
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
        myText = currentText();

    ++myIterateIndex;

    // skip duplicates/empty items
    const int last = count() - 1; // last valid index
    const QString currText = currentText();

    while ( myIterateIndex < last &&
            (currText == text( myIterateIndex ) ||
             text( myIterateIndex ).isEmpty()) )
        ++myIterateIndex;

    if ( myIterateIndex >= count() ) {
        myRotated = true;
        myIterateIndex = -1;

        // if the typed text is the same as the first item, skip the first
        if ( count() > 0 && myText == text(0) )
            myIterateIndex = 0;

        setEditText( myText );
    }
    else
        setEditText( text( myIterateIndex ));
}

void KHistoryCombo::rotateDown()
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
        myText = currentText();

    --myIterateIndex;

    const QString currText = currentText();
    // skip duplicates/empty items
    while ( myIterateIndex >= 0 &&
            (currText == text( myIterateIndex ) ||
             text( myIterateIndex ).isEmpty()) )
        --myIterateIndex;


    if ( myIterateIndex < 0 ) {
        if ( myRotated && myIterateIndex == -2 ) {
            myRotated = false;
            myIterateIndex = count() - 1;
            setEditText( text(myIterateIndex) );
        }
        else { // bottom of history
            if ( myIterateIndex == -2 ) {
                KNotifyClient::event( (int)winId(), KNotifyClient::notification,
                                      i18n("No further item in the history."));
            }

            myIterateIndex = -1;
            if ( currentText() != myText )
                setEditText( myText );
        }
    }
    else
        setEditText( text( myIterateIndex ));

}

void KHistoryCombo::keyPressEvent( QKeyEvent *e )
{
    KKey event_key( e );

    // going up in the history, rotating when reaching QListBox::count()
    if ( KStdAccel::rotateUp().contains(event_key) )
        rotateUp();

    // going down in the history, no rotation possible. Last item will be
    // the text that was in the lineedit before Up was called.
    else if ( KStdAccel::rotateDown().contains(event_key) )
        rotateDown();
    else
        KComboBox::keyPressEvent( e );
}

void KHistoryCombo::wheelEvent( QWheelEvent *ev )
{
    // Pass to poppable listbox if it's up
    QListBox* const lb = listBox();
    if ( lb && lb->isVisible() )
    {
        QApplication::sendEvent( lb, ev );
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

void KHistoryCombo::slotReset()
{
    myIterateIndex = -1;
    myRotated = false;
}


void KHistoryCombo::setPixmapProvider( KPixmapProvider *prov )
{
    if ( myPixProvider == prov )
        return;

    delete myPixProvider;
    myPixProvider = prov;

    // re-insert all the items with/without pixmap
    // I would prefer to use changeItem(), but that doesn't honor the pixmap
    // when using an editable combobox (what we do)
    if ( count() > 0 ) {
        QStringList items( historyItems() );
        clear();
        insertItems( items );
    }
}

void KHistoryCombo::insertItems( const QStringList& items )
{
    QStringList::ConstIterator it = items.constBegin();
    const QStringList::ConstIterator itEnd = items.constEnd();

    while ( it != itEnd ) {
        const QString item = *it;
        if ( !item.isEmpty() ) { // only insert non-empty items
            if ( myPixProvider )
                insertItem( myPixProvider->pixmapFor(item, KIcon::SizeSmall),
                            item );
            else
                insertItem( item );
        }
        ++it;
    }
}

void KHistoryCombo::slotClear()
{
    clearHistory();
    emit cleared();
}

void KComboBox::virtual_hook( int id, void* data )
{ KCompletionBase::virtual_hook( id, data ); }

void KHistoryCombo::virtual_hook( int id, void* data )
{ KComboBox::virtual_hook( id, data ); }

#include "kcombobox.moc"
