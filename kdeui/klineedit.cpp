/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000, 2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
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
#include <qtimer.h>

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
    possibleTripleClick = false;
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

void KLineEdit::setReadOnly(bool readOnly)
{
    QPalette p = palette();
    if (readOnly)
    {
        QColor color = p.color(QPalette::Disabled, QColorGroup::Background);
        p.setColor(QColorGroup::Base, color);
        p.setColor(QColorGroup::Background, color);
    }
    else
    {
        QColor color = p.color(QPalette::Normal, QColorGroup::Base);
        p.setColor(QColorGroup::Base, color);
        p.setColor(QColorGroup::Background, color);
    }
    setPalette(p);

    QLineEdit::setReadOnly (readOnly);
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Filter key-events if EchoMode is normal &
    // completion mode is not set to CompletionNone
    KKey key( e );
        
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
            KShortcut cut;
            if ( keys[TextCompletion].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::TextCompletion);
            else
                cut = keys[TextCompletion];

            if ( cut.contains( key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                QString txt = text();
                int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
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
            KShortcut cut;
            if ( keys[PrevCompletionMatch].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::PrevCompletion);
            else
                cut = keys[PrevCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }

            // Handles next match
            if ( keys[NextCompletionMatch].isNull() )
                cut = KStdAccel::key(KStdAccel::NextCompletion);
            else
                cut = keys[NextCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::NextCompletionMatch );
                return;
            }
        }

        // substring completion
        if ( compObj() )
        {
            KShortcut cut;
            if ( keys[SubstringCompletion].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::SubstringCompletion);
            else
                cut = keys[SubstringCompletion];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit substringCompletion( text() );
                if ( handleSignals() )
                {
                    setCompletedItems( compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }    

    if ( KStdAccel::copy().contains( key ) )
    {
        copy();
        return;
    }
    else if ( KStdAccel::paste().contains( key ) )
    {
        paste();
        return;
    }
    else if ( KStdAccel::cut().contains( key ) )
    {
        cut();
        return;
    }
    else if ( KStdAccel::undo().contains( key ) )
    {
        undo();
        return;
    }
    else if ( KStdAccel::redo().contains( key ) )
    {
        redo();
        return;
    }
    else if ( KStdAccel::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(TRUE);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordForward().contains( key ) )
    {
        // Workaround for QT bug where
        cursorWordForward(TRUE);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );
}

void KLineEdit::mouseDoubleClickEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton  )
    {
        possibleTripleClick=true;
        QTimer::singleShot( QApplication::doubleClickInterval(),this,
                            SLOT(tripleClickTimeout()) );
    }
    QLineEdit::mouseDoubleClickEvent( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if ( possibleTripleClick && e->button() == Qt::LeftButton )
    {
        selectAll();
        return;
    }
    QLineEdit::mousePressEvent( e );
}

void KLineEdit::tripleClickTimeout()
{
    possibleTripleClick=false;
}

QPopupMenu *KLineEdit::createPopupMenu()
{
    // Return if popup menu is not enabled !!
    if ( !m_bEnableMenu )
        return 0;

    QPopupMenu *popup = QLineEdit::createPopupMenu();

    // If a completion object is present and the input
    // widget is not read-only, show the Text Completion
    // menu item.
    if ( compObj() && !isReadOnly() )
    {
        QPopupMenu *subMenu = new QPopupMenu( popup );
        connect( subMenu, SIGNAL( activated( int ) ),
                 this, SLOT( completionMenuActivated( int ) ) );

        popup->insertSeparator();
        popup->insertItem( SmallIconSet("completion"), i18n("Text Completion"),
                           subMenu );

        subMenu->insertItem( i18n("None"), NoCompletion );
        subMenu->insertItem( i18n("Manual"), ShellCompletion );
        subMenu->insertItem( i18n("Automatic"), AutoCompletion );
        subMenu->insertItem( i18n("Dropdown List"), PopupCompletion );
        subMenu->insertItem( i18n("Short Automatic"), SemiAutoCompletion );

        subMenu->setAccel( KStdAccel::completion(), ShellCompletion );

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
    
    // ### do we really need this?  Yes, Please do not remove!  This
    // allows applications to extend the popup menu without having to
    // inherit from this class! (DA)
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
            if (overrideAccel (e))
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
        connect( d->completionBox, SIGNAL( activated( const QString& )),
                 SIGNAL(completionBoxActivated( const QString& )) );
    }
}

bool KLineEdit::overrideAccel (const QKeyEvent* e)
{
    KShortcut scKey;

    KKey key( e );
    KeyBindingMap keys = getKeyBindings();

    if (keys[TextCompletion].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::TextCompletion);
    else
        scKey = keys[TextCompletion];

    if (scKey.contains( key ))
        return true;

    if (keys[NextCompletionMatch].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::NextCompletion);
    else
        scKey = keys[NextCompletionMatch];

    if (scKey.contains( key ))
        return true;

    if (keys[PrevCompletionMatch].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::PrevCompletion);
    else
        scKey = keys[PrevCompletionMatch];

    if (scKey.contains( key ))
        return true;

    // Override all the text manupilation accelerators...    
    if ( KStdAccel::copy().contains( key ) ) 
        return true;
    else if ( KStdAccel::paste().contains( key ) ) 
        return true;
    else if ( KStdAccel::cut().contains( key ) ) 
        return true;
    else if ( KStdAccel::undo().contains( key ) ) 
        return true;
    else if ( KStdAccel::redo().contains( key ) ) 
        return true;
    else if (KStdAccel::deleteWordBack().contains( key ))
        return true;    
    else if (KStdAccel::deleteWordForward().contains( key ))
        return true;

    if (d->completionBox && d->completionBox->isVisible ())
      if (e->key () == Key_Backtab)
        return true;

    return false;
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
        d->completionBox->setItems( items );
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

void KLineEdit::clear()
{
    setText( QString::null );
}

void KLineEdit::virtual_hook( int id, void* data )
{ KCompletionBase::virtual_hook( id, data ); }

