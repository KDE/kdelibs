/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef ExecState_H
#define ExecState_H

#include "value.h"
#include "types.h"
#include "CommonIdentifiers.h"

namespace KJS {
    class Context;
    class Interpreter;
    class FunctionImp;
    class GlobalFuncImp;
    
  /**
   * Represents the current state of script execution. This object allows you
   * obtain a handle the interpreter that is currently executing the script,
   * and also the current execution context.
   */
  class KJS_EXPORT ExecState {
    friend class Interpreter;
    friend class FunctionImp;
    friend class GlobalFuncImp;
  public:
    /**
     * Returns the interpreter associated with this execution state
     *
     * @return The interpreter executing the script
     */
    Interpreter* dynamicInterpreter() const { return m_interpreter; }

    /**
     * Returns the interpreter associated with the current scope's
     * global object
     *
     * @return The interpreter currently in scope
     */
    Interpreter* lexicalInterpreter() const;

    /**
     * Returns the execution context associated with this execution state
     *
     * @return The current execution state context
     */
    Context* context() const { return m_context; }


    /**
     * Set the exception associated with this execution state
     * @param e The JSValue of the exception being set
     */
    void setException(JSValue* e) { m_exception = e; }

    /**
     * Clears the exception set on this execution state.
     */
    void clearException() { m_exception = 0; }

    /**
     * Returns the exception associated with this execution state.
     * @return The current execution state exception
     */
    JSValue* exception() const { return m_exception; }

    /**
     * Returns the slot for the exception associated with this
     * execution state.
     */
    JSValue** exceptionSlot() { return &m_exception; }

    /**
     * Use this to check if an exception was thrown in the current
     * execution state.
     *
     * @return Whether an exception was thrown
     */
    bool hadException() const { return !!m_exception; }

    // This is a workaround to avoid accessing the global variables for these identifiers in
    // important property lookup functions, to avoid taking PIC branches in Mach-O binaries
    const CommonIdentifiers& propertyNames() const { return *m_propertyNames; }

  private:
    ExecState(Interpreter* interp, Context* con)
        : m_interpreter(interp)
        , m_context(con)
        , m_exception(0)
	, m_propertyNames(CommonIdentifiers::shared())
    { 
    }
    Interpreter* m_interpreter;
    Context* m_context;
    JSValue* m_exception;
    CommonIdentifiers* m_propertyNames;
  };

} // namespace KJS

#endif // ExecState_H
