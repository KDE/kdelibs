/***************************************************************************
 * interpreter.h
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_KJSINTERPRETER_H
#define KROSS_KJSINTERPRETER_H

#include <kross/core/krossconfig.h>
#include <kross/core/interpreter.h>
#include <kross/core/manager.h>
#include <kross/core/script.h>
#include <kross/core/action.h>

namespace Kross {

    /**
    * The EcmaScript class implements a \a Kross::Script to handle
    * a single script. Each script and script file will have it's
    * own \a EcmaScript instance as container for a Ecma QtScript
    * that is managed by the \a Kross::Action class.
    */
    class EcmaScript : public Kross::Script
    {
            Q_OBJECT
        public:

            /**
            * Constructor.
            *
            * \param interpreter The \a EcmaInterpreter instance this
            * script belongs to.
            * \param action The \a Kross::Action instance that contains
            * details about the script and that decorates this script.
            */
            EcmaScript(Kross::Interpreter* interpreter, Kross::Action* action);

            /**
            * Destructor.
            */
            virtual ~EcmaScript();

        public Q_SLOTS:

            /**
            * Executes the script.
            */
            virtual void execute();

            /**
            * \return a list of function-names.
            */
            virtual QStringList functionNames();

            /**
            * Execute a function.
            *
            * \param name The name of the function that should be called.
            * \param args The optional arguments for the function.
            * \return The return value of the function.
            */
            virtual QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

            /**
            * \return the internal used QScriptEngine instance.
            */
            QObject* engine() const;

        private:
            class Private;
            Private* const d;
    };

    /**
    * The EcmaInterpreter class implements a \a Kross::Interpreter to provide
    * a factory for \a EcmaScript instances. The interpreter is a singleton
    * managed by Kross to share informations between different script instances.
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
