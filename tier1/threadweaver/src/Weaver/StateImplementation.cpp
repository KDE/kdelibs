/* -*- C++ -*-

   This file implements the StateImplementation class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $

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

   $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "StateImplementation.h"

#include "WeaverImpl.h"

using namespace ThreadWeaver;

WeaverImpl* StateImplementation::weaver()
{
    Q_ASSERT ( dynamic_cast<WeaverImpl*> ( State::weaver() ) );
    return static_cast<WeaverImpl*> ( State::weaver() );
}
