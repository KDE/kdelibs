/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#include <stdlib.h>

#include <qclipboard.h>
#include <qlistbox.h>
#include <qpopupmenu.h>

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

class KComboBox::KComboBoxPrivate
{
public:
    KComboBoxPrivate()
    {
        klineEdit = 0L;
    }
    ~KComboBoxPrivate()
    {
    }

    KLineEdit *klineEdit;
};

KComboBox::KComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent, const char *name )
    : QComboBox( rw, parent, name )
{
    init();

    if ( rw ) {
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
    d = new KComboBoxPrivate;

    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Initialize enable popup menu to false.
    // Below it will be enabled if the widget
    // is editable.
    m_bEnableMenu = false;

    m_trapReturnKey = false;

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );

    // for wheelscrolling
    installEventFilter( this );
    if ( lineEdit() )
        lineEdit()->installEventFilter( this );
}


bool KComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    for (int i = 0; i < count(); i++ ) {
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
    {
        d->klineEdit->setContextMenuEnabled( showMenu );
        m_bEnableMenu = showMenu;
    }
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

        int index = listBox()->index( listBox()->findItem( text ) );
        if( index >= 0 ) {
            setCurrentItem( index );
        }
    }
}

void KComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if ( d->klineEdit )
        d->klineEdit->rotateText( type );
}

bool KComboBox::eventFilter( QObject* o, QEvent* ev )
{
    QLineEdit *edit = lineEdit();

    int type = ev->type();

    if ( o == edit )
    {
        KCursor::autoHideEventFilter( edit, ev );

        if ( type == QEvent::KeyPress )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );

            if ( e->key() == Key_Return || e->key() == Key_Enter)
            {
                // On Return pressed event, emit both
                // returnPressed(const QString&) and returnPressed() signals
                emit returnPressed();
                emit returnPressed( currentText() );
                if ( d->klineEdit && d->klineEdit->completionBox(false) &&
                     d->klineEdit->completionBox()->isVisible() )
                    d->klineEdit->completionBox()->hide();

                return m_trapReturnKey;
            }
        }
    }


    // wheel-scrolling changes the current item
    if ( type == QEvent::Wheel ) {
        if ( !listBox() || listBox()->isHidden() ) {
            QWheelEvent *e = static_cast<QWheelEvent*>( ev );
            static const int WHEEL_DELTA = 120;
            int skipItems = e->delta() / WHEEL_DELTA;
            if ( e->state() & ControlButton ) // fast skipping
                skipItems *= 10;

            int newItem = currentItem() - skipItems;

            if ( newItem < 0 )
                newItem = 0;
            else if ( newItem >= count() )
                newItem = count() -1;

            setCurrentItem( newItem );
            if ( !text( newItem ).isNull() )
                emit activated( text( newItem ) );
            emit activated( newItem );
            e->accept();
            return true;
        }
    }

    return QComboBox::eventFilter( o, ev );
}

void KComboBox::setTrapReturnKey( bool grab )
{
    m_trapReturnKey = grab;
}

bool KComboBox::trapReturnKey() const
{
    return m_trapReturnKey;
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

void KComboBox::setLineEdit( QLineEdit *edit )
{
    QComboBox::setLineEdit( edit );
    d->klineEdit = dynamic_cast<KLineEdit*>( edit );
    setDelegate( d->klineEdit );

    // forward some signals. We only emit returnPressed() ourselves.
    if ( d->klineEdit )
    {
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

// Temporary functions until QT3 appears. - Seth Chaiklin 20 may 2001
void KComboBox::deleteWordForward()
{
    lineEdit()->cursorWordForward(TRUE);
    if ( lineEdit()->hasSelectedText() )
        lineEdit()->del();
}

void KComboBox::deleteWordBack()
{
    lineEdit()->cursorWordBackward(TRUE);
    if ( lineEdit()->hasSelectedText() )
        lineEdit()->del();
}

void KComboBox::setCurrentItem( const QString& item, bool insert, int index )
{
    int sel = -1;
    for (int i = 0; i < count(); ++i)
        if (text(i) == item)
        {
            sel = i;
            break;
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

// *********************************************************************
// *********************************************************************


// we are always read-write
KHistoryCombo::KHistoryCombo( QWidget *parent, const char *name )
    : KComboBox( true, parent, name )
{
    init( true ); // using completion
}

// we are always read-write
KHistoryCombo::KHistoryCombo( bool useCompletion,
                              QWidget *parent, const char *name )
    : KComboBox( true, parent, name )
{
    init( useCompletion );
}

void KHistoryCombo::init( bool useCompletion )
{
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
    while ( (int) items.count() > maxCount() && !items.isEmpty() )
        items.remove( items.begin() );

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
    for ( int i = 0; i < count(); i++ )
        list.append( text( i ) );

    return list;
}

void KHistoryCombo::clearHistory()
{
    KComboBox::clear();
    if ( useCompletion() )
        completionObject()->clear();
}

void KHistoryCombo::addContextMenuItems( QPopupMenu* menu )
{
    if ( menu &&!lineEdit()->text().isEmpty())
    {
        menu->insertSeparator();
        menu->insertItem( i18n("Empty Contents"), this, SLOT( slotClear()));
    }
}

void KHistoryCombo::addToHistory( const QString& item )
{
    if ( item.isEmpty() || (count() > 0 && item == text(0) ))
        return;

    // remove all existing items before adding
    if ( !duplicatesEnabled() ) {
        for ( int i = 0; i < count(); i++ ) {
            if ( text( i ) == item )
                removeItem( i );
        }
    }

    // now add the item
    if ( myPixProvider )
        insertItem( myPixProvider->pixmapFor(item, KIcon::SizeSmall), item, 0);
    else
        insertItem( item, 0 );

    int last;
    QString rmItem;

    bool useComp = useCompletion();
    while ( count() > maxCount() && count() > 0 ) {
        // remove the last item, as long as we are longer than maxCount()
        // remove the removed item from the completionObject if it isn't
        // anymore available at all in the combobox.
        last = count() - 1;
        rmItem = text( last );
        removeItem( last );
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
    QString temp = currentText();
    for ( int i = 0; i < count(); i++ ) {
        while ( item == text( i ) ) {
            removed = true;
            removeItem( i );
        }
    }

    if ( removed && useCompletion() )
        completionObject()->removeItem( item );

    setEditText( temp );
    return removed;
}

void KHistoryCombo::keyPressEvent( QKeyEvent *e )
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
        myText = currentText();

    // going up in the history, rotating when reaching QListBox::count()
    if ( KStdAccel::isEqual( e, KStdAccel::rotateUp() ) ) {
        myIterateIndex++;

        // skip duplicates/empty items
        while ( myIterateIndex < count()-1 &&
                (currentText() == text( myIterateIndex ) ||
                text( myIterateIndex ).isEmpty()) )
            myIterateIndex++;

        if ( myIterateIndex >= count() ) {
            myRotated = true;
            myIterateIndex = -1;

            // if the typed text is the same as the first item, skip the first
            if ( myText == text(0) )
                myIterateIndex = 0;

            setEditText( myText );
        }
        else
            setEditText( text( myIterateIndex ));
    }


    // going down in the history, no rotation possible. Last item will be
    // the text that was in the lineedit before Up was called.
    else if ( KStdAccel::isEqual( e, KStdAccel::rotateDown() ) ) {
        myIterateIndex--;

        // skip duplicates/empty items
        while ( myIterateIndex >= 0 &&
                (currentText() == text( myIterateIndex ) ||
                text( myIterateIndex ).isEmpty()) )
            myIterateIndex--;


        if ( myIterateIndex < 0 ) {
            if ( myRotated && myIterateIndex == -2 ) {
                myRotated = false;
                myIterateIndex = count() - 1;
                setEditText( text(myIterateIndex) );
            }
            else { // bottom of history
                if ( myIterateIndex == -2 ) {
                    KNotifyClient::event( KNotifyClient::notification,
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

    else
        KComboBox::keyPressEvent( e );
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
    // I would prefer to use changeItem(), but that doesn't honour the pixmap
    // when using an editable combobox (what we do)
    if ( count() > 0 ) {
        QStringList items( historyItems() );
        clear();
        insertItems( items );
    }
}

void KHistoryCombo::insertItems( const QStringList& items )
{
    QStringList::ConstIterator it = items.begin();
    QString item;
    while ( it != items.end() ) {
        item = *it;
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
