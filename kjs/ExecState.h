/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2008 Maksim Orlovich (maksim@kde.org)
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
#include "scope_chain.h"
#include "LocalStorage.h"
#include "wtf/Vector.h"

namespace KJS {
    class ActivationImp;
    class Context;
    class Interpreter;
    class FunctionImp;
    class FunctionBodyNode;
    class ProgramNode;

    enum CodeType { GlobalCode, EvalCode, FunctionCode };

  /**
   * Represents the current state of script execution. This object allows you
   * obtain a handle the interpreter that is currently executing the script,
   * and also the current execution context.
   */
  class KJS_EXPORT ExecState : Noncopyable {
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
     *
     * @param
     * @return
     */
    JSValue** exceptionSlot() { return &m_exception; }

    /**
     * Use this to check if an exception was thrown in the current
     * execution state.
     *
     * @return Whether an exception was thrown
     */
    bool hadException() const { return !!m_exception; }

    /**
     * Returns the scope chain for this execution context. This is used for
     * variable lookup, with the list being searched from start to end until a
     * variable is found.
     *
     * @return The execution context's scope chain
     */
    const ScopeChain& scopeChain() const { return scope; }

    /**
     * Returns the variable object for the execution context. This contains a
     * property for each variable declared in the execution context.
     *
     * @return The execution context's variable object
     */
    JSObject* variableObject() const { return m_variable; }
    void setVariableObject(JSObject* v) { m_variable = v; }

    /**
     * Returns the "this" value for the execution context. This is the value
     * returned when a script references the special variable "this". It should
     * always be an Object, unless application-specific code has passed in a
     * different type.
     *
     * The object that is used as the "this" value depends on the type of
     * execution context - for global contexts, the global object is used. For
     * function objewcts, the value is given by the caller (e.g. in the case of
     * obj.func(), obj would be the "this" value). For code executed by the
     * built-in "eval" function, the this value is the same as the calling
     * context.
     *
     * @return The execution context's "this" value
     */
    JSObject* thisValue() const { return m_thisVal; }

    /**
     * Returns the context from which the current context was invoked. For
     * global code this will be a null context (i.e. one for which
     * isNull() returns true). You should check isNull() on the returned
     * value before calling any of it's methods.
     *
     * @return The calling execution context
     */
    ExecState* callingExecState() { return m_callingExec; }

    JSObject* activationObject() {
        assert(m_codeType == FunctionCode);
        return m_variable;
    }

    CodeType codeType() { return m_codeType; }
    FunctionBodyNode* currentBody() { return m_currentBody; }
    FunctionImp* function() const { return m_function; }
    const List* arguments() const { return m_arguments; }

    void pushScope(JSObject* s) { scope.push(s); }
    void popScope() { scope.pop(); }

    void mark();

    void setLocalStorage(LocalStorage* store, WTF::Vector<bool>* markDescriptor) {
        m_localStore = store; m_markDescriptor = markDescriptor;
    }

    LocalStorage* localStorage() { return m_localStore; }

    // This is a workaround to avoid accessing the global variables for these identifiers in
    // important property lookup functions, to avoid taking PIC branches in Mach-O binaries
    const CommonIdentifiers& propertyNames() const { return *m_propertyNames; }
  protected:
    ExecState(Interpreter* intp);
    ~ExecState();

    Interpreter* m_interpreter;
    JSValue* m_exception;
    CommonIdentifiers* m_propertyNames;
    ExecState* m_callingExec;

    FunctionBodyNode* m_currentBody;
    FunctionImp* m_function;
    const List* m_arguments;

    ScopeChain scope;
    JSObject* m_variable;
    JSObject* m_thisVal;

    LocalStorage*      m_localStore;
    WTF::Vector<bool>* m_markDescriptor;

    CodeType m_codeType;
  };

    class GlobalExecState : public ExecState {
    public:
        GlobalExecState(Interpreter* intp, JSObject* global);
    };

    class InterpreterExecState : public ExecState {
    public:
        InterpreterExecState(Interpreter* intp, JSObject* global, JSObject* thisObject, ProgramNode*);
    };

    class EvalExecState : public ExecState {
    public:
        EvalExecState(Interpreter* intp, JSObject* global, ProgramNode* body, ExecState* callingExecState);
    };

    class FunctionExecState : public ExecState {
    public:
        FunctionExecState(Interpreter* intp, JSObject* thisObject,
                          FunctionBodyNode*, ExecState* callingExecState, FunctionImp*, const List* args);
        ~FunctionExecState();
    };

} // namespace KJS

#endif // ExecState_H
