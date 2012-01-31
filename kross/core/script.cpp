/***************************************************************************
 * script.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "script.h"
#include "interpreter.h"
#include "action.h"
#include "krossconfig.h"

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class Script::Private
    {
        public:
            /// The \a Interpreter used to create this Script instance.
            Interpreter* interpreter;
            /// The \a Action associated with this Script.
            Action* action;
    };

}

Script::Script(Interpreter* interpreter, Action* action)
    : QObject()
    , ErrorInterface()
    , d( new Private() )
{
    //setObjectName( action->objectName() );
    d->interpreter = interpreter;
    d->action = action;
}

Script::~Script()
{
    delete d;
}

Interpreter* Script::interpreter() const
{
    return d->interpreter;
}

Action* Script::action() const
{
    return d->action;
}

