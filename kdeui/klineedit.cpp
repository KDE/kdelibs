/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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
#include <kiconloader.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    bool grabReturnKeyEvents;
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
    delete d->completionBox;
    delete d;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    d->grabReturnKeyEvents = false;
    d->completionBox = 0L;

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

void KLineEdit::setCompletedText( const QString& text, bool marked )
{
    int pos = cursorPosition();
    if ( text != QLineEdit::text() ) // no need to flicker if not necessary
        setText( text );
    if ( marked )
    {
        setSelection( pos, text.length() );
        setCursorPosition( pos );
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
       // Skip rotation if previous/next match is null or the same tex
       if ( input.isNull() || input == displayText() )
            return;
       setCompletedText( input, hasMarkedText() );
    }
}

void KLineEdit::makeCompletion( const QString& text )
{

    KCompletion *comp = compObj();
    if ( !comp )
       return;  // No completion object...

    bool compPopup = (completionMode() == KGlobalSettings::CompletionPopup);

    if ( compPopup && !d->completionBox )
        makeCompletionBox();

    QString match = comp->makeCompletion( text );

    // If no match or the same match, simply return
    // without completing.
    KGlobalSettings::Completion mode = completionMode();
    if ( match.isNull() || match == text )
    {
        if ( compPopup && match.isNull() )
        {
            d->completionBox->hide();
            d->completionBox->clear();
        }

        // Put the cursor at the end when in semi-automatic
        // mode and completion is invoked with the same text.
        if ( mode == KGlobalSettings::CompletionMan )
            end( false );
        return;
    }

    if ( compPopup )
        setCompletedItems( comp->allMatches() );

    else
    {
        bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                        mode == KGlobalSettings::CompletionMan );
        setCompletedText( match, marked );
    }
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    // Filter key-events if EchoMode is normal &
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        KGlobalSettings::Completion mode = completionMode();
        if ( mode == KGlobalSettings::CompletionAuto )
        {
            QString keycode = e->text();
            if( !keycode.isNull() && keycode.unicode()->isPrint() )
            {
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                if ( !hasMarkedText() && txt.length() &&
                    cursorPosition() == (int)txt.length() )
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

        if ( mode == KGlobalSettings::CompletionPopup )
        {
            QString prev_txt = text();
            QLineEdit::keyPressEvent( e );
            QString curr_txt = text();
            if ( prev_txt != curr_txt )
            {
                if ( emitSignals() )
                    emit completion( curr_txt );
                if ( handleSignals() )
                    makeCompletion( curr_txt );
                e->accept();
            }
	   return;
        }

        // Handles completion.
        KeyBindingMap keys = getKeyBindings();
        int key = ( keys[TextCompletion] == 0 ) ? KStdAccel::key(KStdAccel::TextCompletion) : keys[TextCompletion];
        if ( KStdAccel::isEqual( e, key ) )
        {
            // Emit completion if the completion mode is NOT
            // CompletionAuto and if the mode is CompletionShell,
            // the cursor is at the end of the string.
            QString txt = text();
            int len = txt.length();
            if ( (mode == KGlobalSettings::CompletionMan ||
                  mode == KGlobalSettings::CompletionShell) &&
                  cursorPosition() == len && len != 0 )
            {
                if ( emitSignals() )
                    emit completion( txt );
                if ( handleSignals() )
                    makeCompletion( txt );
                return;
            }
        }
        // Handles previous match
        key = ( keys[PrevCompletionMatch] == 0 ) ? KStdAccel::key(KStdAccel::PrevCompletion) : keys[PrevCompletionMatch];
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
    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::RightButton )
    {
        // Return if popup menu is not enabled !!
        if ( !m_bEnableMenu )
            return;

        KPopupMenu *popup = new KPopupMenu( this );
        popup->insertItem( SmallIconSet("editcut"), i18n( "Cut" ), Cut );
        popup->insertItem( SmallIconSet("editcopy"), i18n( "Copy" ), Copy );
        popup->insertItem( SmallIconSet("editpaste"), i18n( "Paste" ), Paste );
        popup->insertItem( i18n( "Clear" ), Clear );
        // Create and insert the completion sub-menu iff
        // a completion object is present.
        if( compObj() )
        {
            KPopupMenu* subMenu = new KPopupMenu( popup );
            KGlobalSettings::Completion mode = completionMode();
            subMenu->insertItem( i18n("None"), NoCompletion );
            subMenu->setItemChecked( NoCompletion, mode == KGlobalSettings::CompletionNone );
            subMenu->insertItem( i18n("Manual"), ShellCompletion );
            subMenu->setItemChecked( ShellCompletion, mode == KGlobalSettings::CompletionShell );
            subMenu->insertItem( i18n("Popup"), PopupCompletion );
            subMenu->setItemChecked( PopupCompletion, mode == KGlobalSettings::CompletionPopup );
            subMenu->insertItem( i18n("Automatic"), AutoCompletion );
            subMenu->setItemChecked( AutoCompletion, mode == KGlobalSettings::CompletionAuto );
            subMenu->insertItem( i18n("Semi-Automatic"), SemiAutoCompletion );
            subMenu->setItemChecked( SemiAutoCompletion, mode == KGlobalSettings::CompletionMan );
            if( mode != KGlobalSettings::completionMode() )
            {
                subMenu->insertSeparator();
                subMenu->insertItem( i18n("Default"), Default );
            }
            popup->insertSeparator();
            popup->insertItem( SmallIconSet("completion"), i18n("Completion"), subMenu );
        }
        popup->insertSeparator();
        popup->insertItem( i18n( "Unselect" ), Unselect );
        popup->insertItem( i18n( "Select All" ), SelectAll );

        bool flag = ( echoMode()==QLineEdit::Normal && !isReadOnly() );
        bool allMarked = ( markedText().length() == text().length() );
        popup->setItemEnabled( Cut, flag && hasMarkedText() );
        popup->setItemEnabled( Copy, flag && hasMarkedText() );
        popup->setItemEnabled( Paste, flag && (bool)QApplication::clipboard()->text().length() );
        popup->setItemEnabled( Clear, flag && ( text().length() > 0) );
        popup->setItemEnabled( Unselect, hasMarkedText() );
        popup->setItemEnabled( SelectAll, flag && !allMarked );

        KGlobalSettings::Completion oldMode = completionMode();
        int result = popup->exec( e->globalPos() );
        delete popup;

        if ( result == Cut )
            cut();
        else if ( result == Copy )
            copy();
        else if ( result == Paste )
            paste();
        else if ( result == Clear )
            clear();
        else if ( result == Unselect )
            deselect();
        else if ( result == SelectAll )
            selectAll();
        else if ( result == Default )
            setCompletionMode( KGlobalSettings::completionMode() );
        else if ( result == NoCompletion )
            setCompletionMode( KGlobalSettings::CompletionNone );
        else if ( result == AutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionAuto );
        else if ( result == SemiAutoCompletion )
            setCompletionMode( KGlobalSettings::CompletionMan );
        else if ( result == ShellCompletion )
            setCompletionMode( KGlobalSettings::CompletionShell );
        else if ( result == PopupCompletion )
            setCompletionMode( KGlobalSettings::CompletionPopup );


        if ( oldMode != completionMode() )
        {
            if ( oldMode == KGlobalSettings::CompletionPopup &&
                 d->completionBox && d->completionBox->isVisible() )
                d->completionBox->hide();
            emit completionModeChanged( completionMode() );
        }
        return;
    }
    QLineEdit::mousePressEvent( e );
}

bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{

    if ( o == this )
    {
        KCursor::autoHideEventFilter( this, ev );
        QKeyEvent* e = static_cast<QKeyEvent*>(ev);
        if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        {
            emit returnPressed( displayText() );
            if ( d->grabReturnKeyEvents )
            {
                e->accept();
                return true;
            }
        }
    }
    return QLineEdit::eventFilter( o, ev );
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
    connect( d->completionBox, SIGNAL( highlighted( const QString& )),
             SLOT( setText( const QString& )));
}

// FIXME: make pure virtual in KCompletionBase!
void KLineEdit::setCompletedItems( const QStringList& items )
{
    if ( completionMode() == KGlobalSettings::CompletionPopup )
    {
        if ( !d->completionBox )
            makeCompletionBox();
        d->completionBox->clear();

        if ( !items.isEmpty() )
        {
            d->completionBox->insertStringList( items );
            d->completionBox->popup();
            // setFocus(); // No need for this now since the popup neve gains focus
        }
        else
            d->completionBox->hide();
    }

    else {
        if ( !items.isEmpty() ) // fallback
            setCompletedText( items.first() );
    }
}

KCompletionBox * KLineEdit::completionBox()
{
    makeCompletionBox();
    return d->completionBox;
}
