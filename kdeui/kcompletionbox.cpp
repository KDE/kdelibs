/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2000 Stefan Schimanski <1Stein@gmx.de>

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
#include <qevent.h>

#include <knotifyclient.h>


KCompletionBox::KCompletionBox( QWidget *parent, const char *name )
    : KListBox( 0L, name, WStyle_Customize | WStyle_Tool )
{
    m_parent = parent;

    setFocusPolicy( NoFocus );
    setColumnMode( 1 );
    setLineWidth( 1 );
    setFrameStyle( QFrame::Box | QFrame::Plain );

    setVScrollBarMode( Auto );
    setHScrollBarMode( AlwaysOff );

    connect( this, SIGNAL( doubleClicked( QListBoxItem * )),
             SLOT( slotActivated( QListBoxItem * )) );

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

    if ( o == m_parent ) {
        if ( isVisible() ) {
            if ( type == QEvent::KeyPress ) {
                QKeyEvent *ev = static_cast<QKeyEvent *>( e );
                switch ( ev->key() ) {
                case Key_Tab:
                    if ( ev->state() & ShiftButton )
                        up();
                    else if ( !ev->state() )
                        down(); // Only on TAB!!
                    ev->accept();
                    return true;
                case Key_Down:
                    down();
                    ev->accept();
                    return true;
                case Key_Up:
                    up();
                    ev->accept();
                    return true;
                case Key_Prior:
                    pageUp();
                    ev->accept();
                    return true;
                case Key_Next:
                    pageDown();
                    ev->accept();
                    return true;
                case Key_Home: {
		    // shift/ctrl involved -> let our parent handle that!
		    bool ours = (ev->state() == 0 && currentItem() != -1);
		    if ( ours ) {
			home();
			ev->accept();
		    }
		    return ours;
		}
                case Key_End: {
		    bool ours = (ev->state() == 0 && currentItem() != -1);
		    if ( ours ) {
			end();
			ev->accept();
		    }
                    return ours;
		}
                case  Key_Escape:
                    hide();
                    ev->accept();
                    return true;
                default:
                    break;
                }
            }
	
	    // parent loses focus -> we hide
	    else if ( type == QEvent::FocusOut || type == QEvent::Resize ||
		      type == QEvent::Close || type == QEvent::Hide ||
                      type == QEvent::Move ) {
		hide();
		return false;
	    }

            else if ( type == QEvent::Move && m_parent )
                move( m_parent->mapToGlobal(QPoint(0, m_parent->height())));

            else if ( type == QEvent::Resize )
                resize( sizeHint() );
        }
    }

    else if ( o == this ) {
        switch( type ) {
        case QEvent::Show:
            if ( m_parent ) {
                move( m_parent->mapToGlobal( QPoint(0, m_parent->height())));
		qApp->installEventFilter( this );
            }
            resize( sizeHint() );
            break;
        case QEvent::Hide:
            if ( m_parent )
		qApp->removeEventFilter( this );
            break;
        default:
            break;
	}
    }

    else { // any other object received an event while we're visible
        if ( (type == QEvent::MouseButtonPress && o->parent() != this) ||
             (type == QEvent::Move && m_parent &&
              o == m_parent->topLevelWidget() ))
            hide();                    // allow presses on the scrollbar
    }

    return KListBox::eventFilter( o, e );
}


void KCompletionBox::popup()
{
    if ( count() == 0 )
        hide();
    else {
        ensureCurrentVisible();
        if ( !isVisible() )
            show();
        else if ( size().height() < sizeHint().height() )
            resize( sizeHint() );
    }
}

QSize KCompletionBox::sizeHint() const
{
    int ih = itemHeight();
    int h = QMIN( 15 * ih, (int) count() * ih ) +1;
    h = QMAX( h, KListBox::minimumSizeHint().height() );

    int w = (m_parent) ? m_parent->width() : KListBox::minimumSizeHint().width();
    w = QMAX( KListBox::minimumSizeHint().width(), w );
    return QSize( w, h );
}

void KCompletionBox::down()
{
    if ( currentItem() < (int)count() - 1 )
        setCurrentItem( currentItem() + 1 );
}

void KCompletionBox::up()
{
    if ( currentItem() > 0 )
        setCurrentItem( currentItem() - 1 );
}

void KCompletionBox::pageDown()
{
    int i = currentItem() + numItemsVisible();
    i = i > (int)count() - 1 ? (int)count() - 1 : i;
    setCurrentItem( i );
}

void KCompletionBox::pageUp()
{
    int i = currentItem() - numItemsVisible();
    i = i < 0 ? 0 : i;
    setCurrentItem( i );
}

void KCompletionBox::home()
{
    setCurrentItem( 0 );
}

void KCompletionBox::end()
{
    setCurrentItem( count() -1 );
}

#include "kcompletionbox.moc"
