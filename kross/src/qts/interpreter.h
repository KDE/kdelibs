/***************************************************************************
 * interpreter.h
 * This file is part of the KDE project
 * copyright (C)2007-2008 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_QTS_INTERPRETER_H
#define KROSS_QTS_INTERPRETER_H

#include <kross/core/krossconfig.h>
#include <kross/core/interpreter.h>
#include <kross/core/script.h>
#include <kross/core/action.h>

namespace Kross {

    class EcmaScript;

    /**
    * The EcmaInterpreter class implements a \a Kross::Interpreter to provide
    * a factory for \a EcmaScript instances. The interpreter is a singleton
    * managed by Kross to share information between different script instances.
    */
    class EcmaInterpreter : public Kross::Interpreter
    {
            friend class EcmaScript;
        public:

            /**
            * Constructor.
            *
            * \param info The \a Kross::InterpreterInfo instance that
            * describes this interpreter.
            */
            explicit EcmaInterpreter(Kross::InterpreterInfo* info);

            /**
            * Destructor.
            */
            virtual ~EcmaInterpreter();

            /**
            * Factory method to create a new \a EcmaScript instance.
            *
            * \param action The \a Kross::Action instance that decorates
            * the script and contains details like the scripting code.
            * \return a new \a EcmaScript instance.
            */
            virtual Kross::Script* createScript(Kross::Action* action);

        private:
            class Private;
            Private* const d;
    };

}

#endif
