/* This file is part of the KDE libraries
   Copyright (C) 2010 Téo Mrnjavac <teo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kaboutapplicationpersonlistview_p.h"

namespace KDEPrivate
{

KAboutApplicationPersonListView::KAboutApplicationPersonListView( QWidget *parent )
    : QListView( parent )
{
    setFrameShape( QFrame::NoFrame );

    QPalette p = palette();
    QColor c = p.color( QPalette::Base );
    c.setAlpha( 0 );
    p.setColor( QPalette::Base, c );
    setBackgroundRole( QPalette::Base );
    setPalette( p );
    setSelectionMode( NoSelection );
    setEditTriggers( NoEditTriggers );
    setVerticalScrollMode( ScrollPerPixel );
}

} //namespace KDEPrivate
