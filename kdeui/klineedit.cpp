/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000, 2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qclipboard.h>

#include <kcursor.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    KLineEditPrivate()
    {
        grabReturnKeyEvents = false;
        handleURLDrops = true;
        completionBox = 0L;
    }
    ~KLineEditPrivate()
    {
        delete completionBox;
    }

    bool grabReturnKeyEvents;
    bool handleURLDrops;
    KCompletionBox *completionBox;
};


KLineEdit::KLineEdit( const QString &string, QWidget *parent, const char *name )
    : QLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    delete d;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    // Enable the context menu by default.
    setContextMenuEnabled( true );
    KCursor::setAutoHideCursor( this, true, true );
    installEventFilter( this );
}

void KLineEdit::setCompletionMode( KGlobalSettings::Completion mode )
{
    KGlobalSettings::Completion oldMode = completionMode();
    if ( oldMode != mode && oldMode == KGlobalSettings::CompletionPopup &&
         d->completionBox && d->completionBox->isVisible() )
        d->completionBox->hide();

    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobalSettings::CompletionNone; // Override the request.

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::setCompletedText( const QString& t, bool marked )
{
    QString txt = text();
    if ( t != txt )
    {
        int curpos = marked ? txt.length() : t.length();
        validateAndSet( t, curpos, curpos, t.length() );
    }
}

void KLineEdit::setCompletedText( const QString& text )
{
    KGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan ||
                    mode == KGlobalSettings::CompletionPopup );
    setCompletedText( text, marked );
}

void KLineEdit::rotateText( KCompletionBase::KeyBindingType type )
{
    KCompletion* comp = compObj();
    if ( comp &&
       (type == KCompletionBase::PrevCompletionMatch ||
        type == KCompletionBase::NextCompletionMatch ) )
    {
       QString input = (type == KCompletionBase::PrevCompletionMatch) ? comp->previousMatch() : comp->nextMatch();
       // Skip rotation if previous/next match is null or the same text
       if ( input.isNull() || input == displayText() )
            return;
       setCompletedText( input, hasSelectedText() );
    }
}

void KLineEdit::makeCompletion( const QString& text )
{
    KCompletion *comp = compObj();
    if ( !comp )
        return;  // No completion object...

    QString match = comp->makeCompletion( text );
    KGlobalSettings::Completion mode = completionMode();
    if ( mode == KGlobalSettings::CompletionPopup )
    {
        if ( match.isNull() )
        {
            if ( d->completionBox ) {
                d->completionBox->hide();
                d->completionBox->clear();
            }
        }
        else
            setCompletedItems( comp->allMatches() );
    }
    else
    {
        // all other completion modes
        // If no match or the same match, simply return without completing.
        if ( match.isNull() || match == text )
            return;

        setCompletedText( match );
    }
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( KStdAccel::isEqual( e, KStdAccel::deleteWordBack()) ) {
        deleteWordBack();  // to be replaced with QT3 function
        e->accept();
        return;
    }
    else if ( KStdAccel::isEqual( e, KStdAccel::deleteWordForward()) ) {
        deleteWordForward(); // to be replaced with QT3 function
        e->accept();
        return;
    }

    // Filter key-events if EchoMode is normal &
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();
        bool noModifier = (e->state() == NoButton || e->state()== ShiftButton);

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            QString keycode = e->text();
            if ( !keycode.isNull() && keycode.unicode()->isPrint() )
            {
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                int len = txt.length();
                if ( !hasSelectedText() && len && cursorPosition() == len )
                {
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    e->accept();
                }
                return;
            }
        }

        else if ( mode == KGlobalSettings::CompletionPopup && noModifier )
        {
            QString old_txt = text();
            QLineEdit::keyPressEvent ( e );
            QString txt = text();
            int len = txt.length();
            QString keycode = e->text();

            
            if ( txt != old_txt && len && cursorPosition() == len &&
                 ( (!keycode.isNull() && keycode.unicode()->isPrint()) ||
                   e->key() == Key_Backspace ) )
            {
                if ( emitSignals() )
                    emit completion( txt ); // emit when requested...
                if ( handleSignals() )
                    makeCompletion( txt );  // handle when requested...
                e->accept();
            }
            else if (!len && d->completionBox && d->completionBox->isVisible())
                d->completionBox->hide();

            return;
        }

        else if ( mode == KGlobalSettings::CompletionShell )
        {
            // Handles completion.
            int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
            if ( KStdAccel::isEqual( e, key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                QString txt = text();
                int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
                    kdDebug(293) << "Shell Completion" << endl;
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    return;
                }
            }
            else if ( d->completionBox )
                d->completionBox->hide();
        }

        // handle rotation
        if ( mode != KGlobalSettings::CompletionNone )
        {
            // Handles previous match
            int key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
            if ( KStdAccel::isEqual( e, key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }
            // Handles next match
            key = ( keys[NextCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::NextCompletion) : keys[NextCompletionMatch];
            if ( KStdAccel::isEqual( e, key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::NextCompletionMatch );
                return;
            }
        }

        // substring completion
        if ( compObj() ) {
            int key = ( keys[SubstringCompletion] == 0 ) ?
                      KStdAccel::key(KStdAccel::SubstringCompletion) :
                      keys[SubstringCompletion];
            if ( KStdAccel::isEqual( e, key ) ) {
                if ( emitSignals() )
                    emit substringCompletion( text() );
                if ( handleSignals() ) {
                    setCompletedItems( compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );
}

QPopupMenu *KLineEdit::createPopupMenu()
{
    // Return if popup menu is not enabled !!
    if ( !m_bEnableMenu )
        return 0;

    QPopupMenu *popup = QLineEdit::createPopupMenu();

    // completion object is present.
    if ( compObj() )
    {
        QPopupMenu *subMenu = new QPopupMenu( popup );
	connect( subMenu, SIGNAL( activated( int ) ),
		this, SLOT( completionMenuActivated( int ) ) );

        popup->insertSeparator();
        popup->insertItem( SmallIconSet("completion"), i18n("Completion"), subMenu );

        subMenu->insertItem( i18n("None"), NoCompletion );
        subMenu->insertItem( i18n("Manual"), ShellCompletion );
        subMenu->insertItem( i18n("Popup"), PopupCompletion );
        subMenu->insertItem( i18n("Automatic"), AutoCompletion );
        subMenu->insertItem( i18n("Short Automatic"), SemiAutoCompletion );

        KGlobalSettings::Completion mode = completionMode();
        subMenu->setItemChecked( NoCompletion,
                                 mode == KGlobalSettings::CompletionNone );
        subMenu->setItemChecked( ShellCompletion,
                                 mode == KGlobalSettings::CompletionShell );
        subMenu->setItemChecked( PopupCompletion,
                                 mode == KGlobalSettings::CompletionPopup );
        subMenu->setItemChecked( AutoCompletion,
                                 mode == KGlobalSettings::CompletionAuto );
        subMenu->setItemChecked( SemiAutoCompletion,
                                 mode == KGlobalSettings::CompletionMan );
        if ( mode != KGlobalSettings::completionMode() )
        {
            subMenu->insertSeparator();
            subMenu->insertItem( i18n("Default"), Default );
        }
    }
    // ### do we really need this?
    emit aboutToShowContextMenu( popup );

    return popup;
}

void KLineEdit::completionMenuActivated( int id )
{
    KGlobalSettings::Completion oldMode = completionMode();

    switch ( id )
    {
        case Default:
           setCompletionMode( KGlobalSettings::completionMode() ); break;
        case NoCompletion:
           setCompletionMode( KGlobalSettings::CompletionNone ); break;
        case AutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionAuto ); break;
        case SemiAutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionMan ); break;
        case ShellCompletion:
            setCompletionMode( KGlobalSettings::CompletionShell ); break;
        case PopupCompletion:
            setCompletionMode( KGlobalSettings::CompletionPopup ); break;
        default: return;
    }

    if ( oldMode != completionMode() )
    {
	if ( oldMode == KGlobalSettings::CompletionPopup &&
		d->completionBox && d->completionBox->isVisible() )
	    d->completionBox->hide();
	emit completionModeChanged( completionMode() );
    }
}

void KLineEdit::dropEvent(QDropEvent *e)
{
    KURL::List urlList;
    if( d->handleURLDrops && KURLDrag::decode( e, urlList ) )
    {
        QString dropText = text();
        KURL::List::ConstIterator it;
        for( it = urlList.begin() ; it != urlList.end() ; ++it )
        {
            if(!dropText.isEmpty())
                dropText+=' ';

            dropText += (*it).prettyURL();
        }

        validateAndSet( dropText, dropText.length(), 0, 0);

        e->accept();
    }
    else
        QLineEdit::dropEvent(e);
}

bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{
    if( o == this )
    {
        KCursor::autoHideEventFilter( this, ev );
        if ( ev->type() == QEvent::AccelOverride )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );
            KeyBindingMap keys = getKeyBindings();
            int tc_key = ( keys[TextCompletion] == 0 ) ?
                           KStdAccel::key(KStdAccel::TextCompletion) :
                           keys[TextCompletion];
            int nc_key = ( keys[NextCompletionMatch] == 0 ) ?
                           KStdAccel::key(KStdAccel::NextCompletion) :
                           keys[NextCompletionMatch];
            int pc_key = ( keys[PrevCompletionMatch] == 0 ) ?
                           KStdAccel::key(KStdAccel::PrevCompletion) :
                           keys[PrevCompletionMatch];

            if ( KStdAccel::isEqual( e, KStdAccel::deleteWordBack() ) ||
                 KStdAccel::isEqual( e, KStdAccel::deleteWordForward() ) ||
                 KStdAccel::isEqual( e, tc_key ) ||
                 KStdAccel::isEqual( e, nc_key ) ||
                 KStdAccel::isEqual( e, pc_key ) )
            {
                e->accept();
                return true;
            }
        }
        else if( ev->type() == QEvent::KeyPress )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );

            if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
            {
                bool trap = d->completionBox && d->completionBox->isVisible();

                // Qt will emit returnPressed() itself if we return false
                if ( d->grabReturnKeyEvents || trap )
                    emit QLineEdit::returnPressed();

                emit returnPressed( displayText() );

                if ( trap )
                    d->completionBox->hide();

                // Eat the event if the user asked for it, or if a completionbox was visible
                return d->grabReturnKeyEvents || trap;
            }
        }
    }
    return QLineEdit::eventFilter( o, ev );
}


void KLineEdit::setURLDropsEnabled(bool enable)
{
    d->handleURLDrops=enable;
}

bool KLineEdit::isURLDropsEnabled() const
{
    return d->handleURLDrops;
}

void KLineEdit::setTrapReturnKey( bool grab )
{
    d->grabReturnKeyEvents = grab;
}

bool KLineEdit::trapReturnKey() const
{
    return d->grabReturnKeyEvents;
}

void KLineEdit::setURL( const KURL& url )
{
    QLineEdit::setText( url.prettyURL() );
}

void KLineEdit::makeCompletionBox()
{
    if ( d->completionBox )
        return;

    d->completionBox = new KCompletionBox( this, "completion box" );
    if ( handleSignals() )
    {
        connect( d->completionBox, SIGNAL(highlighted( const QString& )),
                 SLOT(setText( const QString& )) );
        connect( d->completionBox, SIGNAL(userCancelled( const QString& )),
                 SLOT(setText( const QString& )) );
    }
}

void KLineEdit::setCompletedItems( const QStringList& items )
{
    QString txt = text();
    if ( !items.isEmpty() &&
         !(items.count() == 1 && txt == items.first()) )
    {
        if ( !d->completionBox )
            makeCompletionBox();

        if ( !txt.isEmpty() )
            d->completionBox->setCancelledText( txt );
        d->completionBox->clear();
        d->completionBox->insertItems( items );
        d->completionBox->popup();
    }
    else
    {
        if ( d->completionBox && d->completionBox->isVisible() )
            d->completionBox->hide();
    }
}

KCompletionBox * KLineEdit::completionBox( bool create )
{
    if ( create )
        makeCompletionBox();

    return d->completionBox;
}

void KLineEdit::setCompletionObject( KCompletion* comp, bool hsig )
{
    KCompletion *oldComp = compObj();
    if ( oldComp && handleSignals() )
        disconnect( oldComp, SIGNAL( matches( const QStringList& )),
                    this, SLOT( setCompletedItems( const QStringList& )));

    if ( comp && hsig )
      connect( comp, SIGNAL( matches( const QStringList& )),
               this, SLOT( setCompletedItems( const QStringList& )));

    KCompletionBase::setCompletionObject( comp, hsig );
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KLineEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QLineEdit::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( this, true, true );
}

// Temporary functions until QT3 appears. - Seth Chaiklin 20 may 2001
void KLineEdit::deleteWordForward()
{
    cursorWordForward(TRUE);
    if ( hasSelectedText() )
        del();
}

void KLineEdit::deleteWordBack()
{
    cursorWordBackward(TRUE);
    if ( hasSelectedText() )
        del();
}
