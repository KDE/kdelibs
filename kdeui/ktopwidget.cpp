/* This file is part of the KDE libraries
    Copyright  (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
               (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
               (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)

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

 // $Id$
 // $Log$
 // Revision 1.48  1999/10/08 23:23:23  bero
 // ktopwidget.h removals
 //
 // Revision 1.47  1999/09/20 13:13:26  ettrich
 // clean ups
 //


#include <ktmainwindow.h>

KTopLevelWidget::KTopLevelWidget( const char *name )
  : KTMainWindow(name, 0)
{}

KTopLevelWidget::~KTopLevelWidget()
{}

#include "ktopwidget.moc"

