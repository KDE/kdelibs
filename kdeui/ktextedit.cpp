/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
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

#include "ktextedit.h"

#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>

KTextEdit::KTextEdit( const QString& text, const QString& context,
                      QWidget *parent, const char *name )
    : QTextEdit ( text, context, parent, name )
{
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::KTextEdit( QWidget *parent, const char *name )
    : QTextEdit ( parent, name )
{
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::~KTextEdit()
{
}

void KTextEdit::keyPressEvent( QKeyEvent *e )
{
    KKey key( e );

    if ( KStdAccel::copy().contains( key ) ) {
        copy();
        e->accept();
	return;
    }
    else if ( KStdAccel::paste().contains( key ) ) {
        paste();
        e->accept();
	return;
    }
    else if ( KStdAccel::cut().contains( key ) ) {
        cut();
        e->accept();
	return;
    }
    else if ( KStdAccel::undo().contains( key ) ) {
        undo();
        e->accept();
	return;
    }
    else if ( KStdAccel::redo().contains( key ) ) {
        redo();
        e->accept();
	return;
    }
    else if ( KStdAccel::deleteWordBack().contains( key ) )
    {
        deleteWordBack();
        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordForward().contains( key ) )
    {
        deleteWordForward();
        e->accept();
        return;
    }

    QTextEdit::keyPressEvent( e );
}

void KTextEdit::deleteWordBack()
{
    removeSelection();
    moveCursor( MoveWordBackward, true );
    removeSelectedText();
}

void KTextEdit::deleteWordForward()
{
    removeSelection();
    moveCursor( MoveWordForward, true );
    removeSelectedText();
}

void KTextEdit::contentsWheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        QTextEdit::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        QScrollView::contentsWheelEvent( e );
}

void KTextEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextedit.moc"
