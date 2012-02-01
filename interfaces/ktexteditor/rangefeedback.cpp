/* This file is part of the KDE libraries
 *  Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.*/


#include "rangefeedback.h"

using namespace KTextEditor;

SmartRangeWatcher::~ SmartRangeWatcher( )
{
}

SmartRangeNotifier::SmartRangeNotifier( )
  : m_wantDirectChanges(true)
{
}

bool SmartRangeNotifier::wantsDirectChanges( ) const
{
  return m_wantDirectChanges;
}

void SmartRangeNotifier::setWantsDirectChanges( bool wantsDirectChanges )
{
  m_wantDirectChanges = wantsDirectChanges;
}

SmartRangeWatcher::SmartRangeWatcher( )
  : m_wantDirectChanges(true)
{
}

bool SmartRangeWatcher::wantsDirectChanges( ) const
{
  return m_wantDirectChanges;
}

void SmartRangeWatcher::setWantsDirectChanges( bool wantsDirectChanges )
{
  m_wantDirectChanges = wantsDirectChanges;
}

void SmartRangeWatcher::rangePositionChanged( SmartRange* )
{
}

void SmartRangeWatcher::rangeContentsChanged( SmartRange* )
{
}

void SmartRangeWatcher::rangeContentsChanged( SmartRange*, SmartRange* )
{
}

void SmartRangeWatcher::mouseEnteredRange( SmartRange*, View* )
{
}

void SmartRangeWatcher::mouseExitedRange( SmartRange*, View* )
{
}

void SmartRangeWatcher::caretEnteredRange( SmartRange*, View* )
{
}

void SmartRangeWatcher::caretExitedRange( SmartRange*, View* )
{
}

void SmartRangeWatcher::rangeEliminated( SmartRange* )
{
}

void SmartRangeWatcher::rangeDeleted( SmartRange* )
{
}

void SmartRangeWatcher::childRangeInserted( SmartRange*, SmartRange* )
{
}

void SmartRangeWatcher::childRangeRemoved( SmartRange*, SmartRange* )
{
}

void SmartRangeWatcher::rangeAttributeChanged( SmartRange*, Attribute::Ptr, Attribute::Ptr )
{
}

void SmartRangeWatcher::parentRangeChanged( SmartRange *, SmartRange *, SmartRange * )
{
}

#include "moc_smartrangenotifier.cpp"

// kate: space-indent on; indent-width 2; replace-tabs on;
