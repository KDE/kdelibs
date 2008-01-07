/***************************************************************************
 * interpreter.cpp
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

#include "interpreter.h"
#include "script.h"
#include "action.h"
#include "manager.h"

extern "C"
{
    typedef void* (*def_interpreter_func)(int version, Kross::InterpreterInfo*);
}

using namespace Kross;

/*************************************************************************
 * InterpreterInfo
 */

namespace Kross {

    /// \internal d-pointer class.
    class InterpreterInfo::Private
    {
        public:
            /// The name the interpreter has. Could be something like "python" or "kjs".
            QString interpretername;
            /// The function-pointer to the module factory function.
            void* funcPtr;
            /// The file wildcard used to determinate extensions.
            QString wildcard;
            /// List of mimetypes this interpreter supports.
            QStringList mimetypes;
            /// A map with options.
            Option::Map options;
            /// The \a Interpreter instance.
            Interpreter* interpreter;
    };

}

InterpreterInfo::InterpreterInfo(const QString& interpretername, void* funcPtr, const QString& wildcard, const QStringList& mimetypes, const Option::Map& options)
    : d( new Private() )
{
    d->interpretername = interpretername;
    d->funcPtr = funcPtr;
    d->wildcard = wildcard;
    d->mimetypes = mimetypes;
    d->options = options;
    d->interpreter = 0;
}

InterpreterInfo::~InterpreterInfo()
{
    delete d->interpreter;
    d->interpreter = 0;
    delete d;
}

const QString InterpreterInfo::interpreterName() const
{
    return d->interpretername;
}

const QString InterpreterInfo::wildcard() const
{
    return d->wildcard;
}

const QStringList InterpreterInfo::mimeTypes() const
{
    return d->mimetypes;
}

bool InterpreterInfo::hasOption(const QString& name) const
{
    return d->options.contains(name);
}

InterpreterInfo::Option* InterpreterInfo::option(const QString& name) const
{
    return d->options.contains(name) ? d->options[name] : 0;
}

InterpreterInfo::Option::Map& InterpreterInfo::options()
{
    return d->options;
}

const QVariant InterpreterInfo::optionValue(const QString& name, const QVariant& defaultvalue) const
{
    return d->options.contains(name) ? d->options[name]->value : defaultvalue;
}

Interpreter* InterpreterInfo::interpreter()
{
    if(d->interpreter) // buffered
        return d->interpreter;

    //#ifdef KROSS_INTERPRETER_DEBUG
        krossdebug( QString("Loading the interpreter library for %1").arg(d->interpretername) );
    //#endif

    // Get the extern "C" krosspython_instance function.
    def_interpreter_func interpreter_func = (def_interpreter_func) d->funcPtr;

    // and execute the extern krosspython_instance function.
    d->interpreter = interpreter_func
        ? (Interpreter*) (interpreter_func)(KROSS_VERSION, this)
        : 0;

    if(! d->interpreter) {
        //#ifdef KROSS_INTERPRETER_DEBUG
            krosswarning("Incompatible interpreter library.");
        //#endif
    }
    else {
        // Job done. The library is loaded and our Interpreter* points
        // to the external Kross::Python::Interpreter* instance.
        //#ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Successfully loaded Interpreter instance from library.");
        //#endif
    }

    return d->interpreter;
}

/*************************************************************************
 * Interpreter
 */

namespace Kross {

    /// \internal d-pointer class.
    class Interpreter::Private
    {
        public:
            InterpreterInfo* interpreterinfo;
            Private(InterpreterInfo* info) : interpreterinfo(info) {}
    };

}

Interpreter::Interpreter(InterpreterInfo* info)
    : QObject()
    , ErrorInterface()
    , d( new Private(info) )
{
}

Interpreter::~Interpreter()
{
    delete d;
}

InterpreterInfo* Interpreter::interpreterInfo() const
{
    return d->interpreterinfo;
}

#include "interpreter.moc"
