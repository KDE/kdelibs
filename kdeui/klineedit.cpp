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
#include <kurldrag.h>
#include <kiconloader.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    bool grabReturnKeyEvents;
    bool handleURLDrops;
    KCompletionBox *completionBox;
    QString origText ;
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
    d->handleURLDrops = true;
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

void KLineEdit::setCompletedText( const QString& t, bool marked )
{
    if ( marked )
	validateAndSet( t, text().length(), text().length(), t.length() );
    else
	if ( t != text() ) // no need to flicker
	    setText( t );
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

    KGlobalSettings::Completion mode = completionMode();
    bool compPopup = (mode == KGlobalSettings::CompletionPopup);

    if ( compPopup && !d->completionBox )
        makeCompletionBox();

    QString match = comp->makeCompletion( text );

    if ( compPopup ) {
	if ( match.isNull() ) {
	    d->completionBox->hide();
	    d->completionBox->clear();
	}
	else
	    setCompletedItems( comp->allMatches() );
    }

    else { // all other completion modes
	// If no match or the same match, simply return without completing.
	if ( match.isNull() || match == text )
	    return;

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
	KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();

	if ( mode == KGlobalSettings::CompletionAuto ||
	     mode == KGlobalSettings::CompletionMan )
        {
            QString keycode = e->text();
            if ( !keycode.isNull() && keycode.unicode()->isPrint() )
            {
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                int len = txt.length();
		if ( !hasMarkedText() && len && cursorPosition() == len )
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
        else if ( mode == KGlobalSettings::CompletionPopup )
        {
            QLineEdit::keyPressEvent ( e );
            if ( edited() )
            {
                QString txt = text();
                if ( emitSignals() )
                    emit completion( txt ); // emit when requested...
                if ( handleSignals() )
                    makeCompletion( txt );  // handle when requested...
                setEdited( false );
                e->accept();
            }
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
                if ( mode == KGlobalSettings::CompletionShell &&
		     cursorPosition() == len && len != 0 )
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
	if ( mode != KGlobalSettings::CompletionNone ) {
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
        popup->insertItem( SmallIconSet("editclear"), i18n( "Clear" ), Clear );
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


void KLineEdit::dropEvent(QDropEvent *e)
{
    KURL::List urlList;
    if(d->handleURLDrops && KURLDrag::decode( e, urlList ))
    {
        QString dropText;
        KURL::List::ConstIterator it;
        for( it = urlList.begin() ; it != urlList.end() ; ++it)
        {
            if(!dropText.isEmpty())
                dropText+=' ';

            dropText += (*it).prettyURL();
        }

        if(!text().isEmpty())
            dropText=' '+dropText;

        end(false);
        insert(dropText);

        e->accept();
    }
    else
    {
        QLineEdit::dropEvent(e);
    }
}


bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{
    if ( o == this )
    {
	KCursor::autoHideEventFilter( this, ev );

	QKeyEvent *e = dynamic_cast<QKeyEvent *>( ev );
	if ( e )
	{
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
    connect( d->completionBox, SIGNAL( highlighted( const QString& )),
             SLOT( setText( const QString& )));
/*
    connect( d->completionBox, SIGNAL( aboutToShow() ),
             SLOT( slotAboutToShow() ) );
    connect( d->completionBox, SIGNAL( cancelled() ),
             SLOT( slotCancelled() ) );
*/
}

void KLineEdit::slotAboutToShow()
{
    d->origText = text();
}

void KLineEdit::slotCancelled()
{
    setText( d->origText );
    d->origText = QString::null;
}

// FIXME: make pure virtual in KCompletionBase!
void KLineEdit::setCompletedItems( const QStringList& items )
{
    if ( completionMode() == KGlobalSettings::CompletionPopup ||
	 completionMode() == KGlobalSettings::CompletionShell )
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

void KLineEdit::setCompletionObject( KCompletion* comp, bool hsig )
{
    KCompletion *oldComp = completionObject( false, false ); // don't create!
    if ( oldComp && handleSignals() )
	disconnect( oldComp, SIGNAL( matches( const QStringList& )),
		    this, SLOT( setCompletedItems( const QStringList& )));
    
    if ( comp && hsig )
	connect( comp, SIGNAL( matches( const QStringList& )),
		 this, SLOT( setCompletedItems( const QStringList& )));
	
    KCompletionBase::setCompletionObject( comp, hsig );
}
