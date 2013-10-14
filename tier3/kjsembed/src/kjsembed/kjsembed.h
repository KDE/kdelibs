/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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


#ifndef KJSEMBED_H
#define KJSEMBED_H

class QObject;

#include <kjsembed/kjsembed_export.h>
#include <kjs/object.h>

namespace KJS {
    class Interpreter;
    class Object;
    class Completion;
}

namespace KJSEmbed {

/**
 * The main interface for running embedded Javascript.
 *
 * \code
 * KJSEmbed::Engine *engine = new KJSEmbed::Engine();
 * KJS::Interpreter *interpreter = engine->interpreter();
 * interpreter->setShouldPrintExceptions(true);
 * KJS::ExecState *exec = interpreter->globalExec();
 * KJS::UString code("print(\"Hello World\")");
 * KJSEmbed::Engine::ExitStatus exitstatus = engine->execute(code);
 * KJS::Completion completion = engine->completion();
 * if(exitstatus != KJSEmbed::Engine::Success) {
 *     KJS::JSValue* value = completion.value();
 *     qDebug() << value->toString(exec).qstring();
 * }
 * \endcode
 */
class KJSEMBED_EXPORT Engine
{
public:
    /**
     * Status codes for script execution. Note that you can access the completion
     * object itself with completion() for more detail.
     */
    enum ExitStatus { Success = 0, Failure = 1 };

    /**
     * Constructs an embedded JS engine.
     *
     * @param enableBindings If true then the bindings will be added to the interpreter
     * created. Otherwise a plain interpreter with nothing beyond the JS built in functions
     * and objects is created. This allows users who want to run untrusted scripts to choose
     * exactly which bindings they offer.
     */
    Engine( bool enableBindings = true);
    /** Clean up. */
    virtual ~Engine();

    /** Returns true if the Engine was created with the bindings enabled. */
    bool isBindingsEnabled() const;

    /**
     * Execute the file with the specified name using the current interpreter.
     * @param file Filename to execute.
     */
    ExitStatus runFile( const KJS::UString &file );

    /**
     * Execute the file with the specified name using the specified interpreter.
     * @param interpreter Interpreter to use.
     * @param file Filename to execute.
     */
    static KJS::Completion runFile( KJS::Interpreter *interpreter, const KJS::UString &file );

    /**
     * Execute a code string using the current interpreter.
     * @param code The script code to execute.
     */
    ExitStatus execute( const KJS::UString &code );

    /**
     * Returns the Completion object for the last script executed.
     */
    KJS::Completion completion() const;

    /**
     * Returns the current interpreter.
     */
    KJS::Interpreter *interpreter() const;

    /**
    *  publishes a QObject to the global context of the javascript interpereter.
    */
    KJS::JSObject *addObject( QObject *obj, const KJS::UString &name = KJS::UString() ) const;

    /**
    * publishes a QObject to a parent object.
    */
    KJS::JSObject *addObject( QObject *obj, KJS::JSObject *parent, const KJS::UString &name = KJS::UString()) const;

    /**
    * Create a new instance of an object that the Javascript engine knows about.  If the object
    * doesn't exist a KJS::jsNull() is returned and an exception thrown.
    */
    KJS::JSObject *construct( const KJS::UString &className, const KJS::List &args = KJS::List() ) const;

    /**
    * Execute a method at the global scope of the javascript interperter.
    */
    KJS::JSValue *callMethod( const KJS::UString &methodName, const KJS::List &args = KJS::List() );

    /**
    * Execute a method on an object.
    */
    KJS::JSValue *callMethod( KJS::JSObject *parent, const KJS::UString &methodName, const KJS::List &args = KJS::List() );

private:
    class EnginePrivate *dptr;
};

}

#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
