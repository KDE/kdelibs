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

#include <qapplication.h>
#include <qclipboard.h>

#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>

class KTextEdit::KTextEditPrivate
{
public:
    KTextEditPrivate()
        : customPalette( false )
    {}

    bool customPalette;
};

KTextEdit::KTextEdit( const QString& text, const QString& context,
                      QWidget *parent, const char *name )
    : QTextEdit ( text, context, parent, name )
{
    d = new KTextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::KTextEdit( QWidget *parent, const char *name )
    : QTextEdit ( parent, name )
{
    d = new KTextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::~KTextEdit()
{
    delete d;
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

    else if ( e->key() == Key_Insert &&
              (e->state() == (ShiftButton | ControlButton)) )
    {
        QString text = QApplication::clipboard()->text( QClipboard::Selection);
        if ( !text.isEmpty() )
            insert( text );
        e->accept();
        return;
    }
    
    // ignore Ctrl-Return so that KDialogs can close the dialog
    else if ( e->state() == ControlButton &&
              (e->key() == Key_Return || e->key() == Key_Enter) &&
              topLevelWidget()->inherits( "KDialog" ) )
    {
        e->ignore();
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

void KTextEdit::setPalette( const QPalette& palette )
{
    QTextEdit::setPalette( palette );
    // unsetPalette() is not virtual and calls setPalette() as well
    // so we can use ownPalette() to find out about unsetting
    d->customPalette = ownPalette();
}

void KTextEdit::setReadOnly(bool readOnly)
{
    if ( readOnly == isReadOnly() )
        return;

    if (readOnly)
    {
        bool custom = ownPalette();
        QPalette p = palette();
        QColor color = p.color(QPalette::Disabled, QColorGroup::Background);
        p.setColor(QColorGroup::Base, color);
        p.setColor(QColorGroup::Background, color);
        setPalette(p);
        d->customPalette = custom;
    }
    else
    {
        if ( d->customPalette )
        {
            QPalette p = palette();
        QColor color = p.color(QPalette::Normal, QColorGroup::Base);
        p.setColor(QColorGroup::Base, color);
        p.setColor(QColorGroup::Background, color);
    setPalette(p);
        }
        else
            unsetPalette();
    }

    QTextEdit::setReadOnly (readOnly);
}

void KTextEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextedit.moc"
