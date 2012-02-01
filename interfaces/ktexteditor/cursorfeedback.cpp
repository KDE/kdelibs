/* This file is part of the KDE project
   Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "cursorfeedback.h"

using namespace KTextEditor;

SmartCursorWatcher::SmartCursorWatcher( )
  : m_wantDirectChanges(true)
{
}

SmartCursorWatcher::~ SmartCursorWatcher( )
{
}

void SmartCursorWatcher::positionChanged( SmartCursor * )
{
}

void SmartCursorWatcher::positionDeleted( SmartCursor * )
{
}

void SmartCursorWatcher::characterDeleted( SmartCursor * , bool )
{
}

void SmartCursorWatcher::characterInserted( SmartCursor * , bool )
{
}

bool SmartCursorWatcher::wantsDirectChanges( ) const
{
  return m_wantDirectChanges;
}

void SmartCursorWatcher::setWantsDirectChanges( bool wantsDirectChanges )
{
  m_wantDirectChanges = wantsDirectChanges;
}

SmartCursorNotifier::SmartCursorNotifier( )
  : m_wantDirectChanges(true)
{
}

bool SmartCursorNotifier::wantsDirectChanges( ) const
{
  return m_wantDirectChanges;
}

void SmartCursorNotifier::setWantsDirectChanges( bool wantsDirectChanges )
{
  m_wantDirectChanges = wantsDirectChanges;
}

void SmartCursorWatcher::deleted( SmartCursor * )
{
}

#include "moc_smartcursornotifier.cpp"

// kate: space-indent on; indent-width 2; replace-tabs on;
