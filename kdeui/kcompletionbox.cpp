/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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


#include "kcompletionbox.h"
#include <qapplication.h>

#include <X11/Xlib.h>
#undef KeyPress
#undef FocusIn

KCompletionBox::KCompletionBox( QWidget *parent, const char *name )
    : KListBox( 0L, name, WType_Popup )
{
    m_parent = parent;

    setColumnMode( 1 );
    setLineWidth( 1 );
    setFrameStyle( QFrame::Box | QFrame::Plain );

    setVScrollBarMode( Auto );
    setHScrollBarMode( AlwaysOff );

    connect( this, SIGNAL( selected( QListBoxItem * )),
	     SLOT( slotActivated( QListBoxItem * )) );
    connect( this, SIGNAL( clicked( QListBoxItem * )),
	     SLOT( slotActivated( QListBoxItem * )));

    // highlight on mouseover
    connect( this, SIGNAL( onItem( QListBoxItem * )),
 	     SLOT( slotSetCurrentItem( QListBoxItem * )));

    installEventFilter( this );
}

KCompletionBox::~KCompletionBox()
{
}

QStringList KCompletionBox::items() const
{
    QStringList list;
    for ( uint i = 0; i < count(); i++ ) {
	list.append( text( i ) );
    }
    return list;
}

void KCompletionBox::slotActivated( QListBoxItem *item )
{
    if ( !item )
	return;

    hide();
    emit activated( item->text() );
}

bool KCompletionBox::eventFilter( QObject *o, QEvent *e )
{
    int type = e->type();
    
    switch( type ) {
    case QEvent::Show:
 	move( m_parent->mapToGlobal( QPoint(0, m_parent->height())) );
 	resize( sizeHint() );
 	break;
    case QEvent::Hide:
	revertFocus();
	break;
    case QEvent::KeyPress: {
 	QKeyEvent *ev = static_cast<QKeyEvent *>( e );
 	if ( ev->key() == Key_Escape ) {
 	    hide();
	    return true;
	}
	else if ( ev->key() == Key_Up && currentItem() == 0 ) {
	    revertFocus();
	    return true;
	}

	break;
    }
    case QEvent::FocusIn: // workaround for "first item not highlighted"
	if ( currentItem() == 0 )
	    setSelected( currentItem(), true );
    default:
	break;
    }

    return KListBox::eventFilter( o, e );
}


void KCompletionBox::show()
{
    KListBox::show();
    XUngrabKeyboard( x11Display(), CurrentTime );
}


void KCompletionBox::popup()
{
    if ( count() == 0 ) {
	hide();
	return;
    }

    ensureCurrentVisible();
    show();
}

void KCompletionBox::revertFocus()
{
    if ( !m_parent->isActiveWindow() )
	m_parent->setActiveWindow();
    m_parent->setFocus();
    setSelected( 0, false );
}

QSize KCompletionBox::sizeHint() const
{
    int ih = itemHeight();
    int h = QMIN( 10 * ih, (int) count() * ih ) +1;
    h = QMAX( h, KListBox::minimumSizeHint().height() );

    int w = QMAX( KListBox::sizeHint().width(), m_parent->width() );
    return QSize( w, h );
}

#include "kcompletionbox.moc"
