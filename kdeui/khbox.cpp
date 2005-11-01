/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "khbox.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>

KHBox::KHBox( QWidget* parentWidget )
    : QFrame( parentWidget )
{
    setLayout( new QHBoxLayout( this ) );
}


KHBox::KHBox( bool /*vertical*/, QWidget* parentWidget )
    : QFrame( parentWidget )
{
    setLayout( new QVBoxLayout( this ) );
}

bool KHBox::event( QEvent* ev )
{
    switch ( ev->type() ) {
    case QEvent::ChildAdded:
    {
        QChildEvent* childEv = static_cast<QChildEvent *>( ev );
        if ( childEv->child()->isWidgetType() ) {
            QWidget* w = static_cast<QWidget *>( childEv->child() );
            static_cast<QBoxLayout *>( layout() )->addWidget( w );
        }
        return QFrame::event( ev );
    }
    case QEvent::ChildRemoved:
    {
        QChildEvent* childEv = static_cast<QChildEvent *>( ev );
        if ( childEv->child()->isWidgetType() ) {
            QWidget* w = static_cast<QWidget *>( childEv->child() );
            static_cast<QBoxLayout *>( layout() )->removeWidget( w );
        }
        return QFrame::event( ev );
    }
    default:
        return QFrame::event( ev );
    }
}

QSize KHBox::sizeHint() const
{
    KHBox* that = const_cast<KHBox *>( this );
    QApplication::sendPostedEvents( that, QEvent::ChildAdded );
    return QFrame::sizeHint();
}

void KHBox::setSpacing( int sp )
{
    layout()->setSpacing( sp );
}

void KHBox::setStretchFactor( QWidget* w, int stretch )
{
    static_cast<QBoxLayout *>( layout() )->setStretchFactor( w, stretch );
}

void KHBox::setMargin(int margin)
{
	layout()->setMargin(margin);
}

#include "khbox.moc"
