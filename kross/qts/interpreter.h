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

    class EcmaScript : public Kross::Script
    {
        public:
            EcmaScript(Kross::Interpreter* interpreter, Kross::Action* action);
            virtual ~EcmaScript();
            virtual void execute();
            virtual QStringList functionNames();
            virtual QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());
        private:
            class Private;
            Private* const d;
    };

    class EcmaInterpreter : public Kross::Interpreter
    {
            friend class EcmaScript;
        public:
            explicit EcmaInterpreter(Kross::InterpreterInfo* info);
            virtual ~EcmaInterpreter();
            virtual Kross::Script* createScript(Kross::Action* Action);
        private:
            class Private;
            Private* const d;
    };

}

#endif
