/* -*- C++ -*-

This file implements the state handling in ThreadWeaver.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
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

$Id: State.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "State.h"

#include <QtCore/QString>

using namespace ThreadWeaver;

const QString StateNames[NoOfStates] = {
    "InConstruction",
    "WorkingHard",
    "Suspending",
    "Suspended",
    "ShuttingDown",
    "Destructed"
};

class State::Private
{
public:
    Private ( WeaverInterface* theWeaver )
        : weaver( theWeaver )
    {
        Q_ASSERT_X( sizeof StateNames / sizeof StateNames[0] == NoOfStates, "State::Private ctor",
                    "Make sure to keep StateId and StateNames in sync!" );
    }

    /** The Weaver we relate to. */
    WeaverInterface *weaver;
};


State::State ( WeaverInterface *weaver )
    : d  ( new Private ( weaver ) )
{
}

State::~State()
{
    delete d;
}

QString State::stateName () const
{
    return StateNames[ stateId() ];
}

void State::activated()
{
}

WeaverInterface* State::weaver()
{
    return d->weaver;
}
