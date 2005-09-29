// -*- c-basic-offset: 2 -*-
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

#ifndef KJS_CONTEXT_H
#define KJS_CONTEXT_H

#include "function.h"

namespace KJS  {

  /**
   * @short Execution context.
   */
  class ContextImp {
    friend class Context;
    friend class StatementNode;
  public:
    // TODO: remove glob parameter. deducable from exec.
    ContextImp(Object &glob, InterpreterImp *interpreter, Object &thisV, int _sourceId, CodeType type = GlobalCode,
               ContextImp *callingContext = 0L, FunctionImp *func = 0L, const List *args = 0);
    virtual ~ContextImp();

    const ScopeChain &scopeChain() const { return scope; }
    CodeType codeType() const { return m_codeType; }
    Object variableObject() const { return variable; }
    void setVariableObject(const Object &v) { variable = v; }
    Object thisValue() const { return thisVal; }
    ContextImp *callingContext() { return _callingContext; }
    ObjectImp *activationObject() { return activation.imp(); }
    FunctionImp *function() const { return _function; }
    const List *arguments() const { return _arguments; }

    void pushScope(const Object &s) { scope.push(s.imp()); }
    void popScope() { scope.pop(); }
    LabelStack *seenLabels() { return &ls; }
    
    void mark();

    void pushTryCatch() { tryCatch++; };
    void popTryCatch() { tryCatch--; };
    bool inTryCatch() const;

    void setLines(int l0, int l1) { line0 = l0; line1 = l1; }

  private:
    InterpreterImp *_interpreter;
    ContextImp *_callingContext;
    FunctionImp *_function;
    const List *_arguments;
    Object activation;
    
    ScopeChain scope;
    Object variable;
    Object thisVal;

    LabelStack ls;
    CodeType m_codeType;

    int tryCatch;
    int sourceId;
    int line0;
    int line1;
    Identifier functionName;
    List args;
  };

} // namespace KJS

#endif
