// -*- c-basic-offset: 2 -*-
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

#include "ExecState.h"
#include "function.h"
#include "scriptfunction.h"
#include "internal.h"
#include "nodes.h"

namespace KJS {

Interpreter* ExecState::lexicalInterpreter() const
{
    Interpreter* result = Interpreter::interpreterWithGlobalObject(scopeChain().bottom());

    if (!result)
        return dynamicInterpreter();

    return result;
}

void ExecState::mark()
{
    if (m_codeType != FunctionCode && m_localStore) {
        //### some code dupe here with JSVariableObject::mark. Not sure how to best
        // restructure. Perhaps this should always mark and not JSVariableObject?
        size_t size = m_localStore->size();
        for (size_t i = 0; i < size; ++i) {
            JSValue* value = (*m_localStore)[i].val.valueVal;
            if ((*m_markDescriptor)[i] && !value->marked())
                value->mark();
        }
    }

    for (ExecState* exec = this; exec; exec = exec->m_callingExec)
        exec->scope.mark();
}

ExecState::ExecState(Interpreter* intp) :
  m_interpreter(intp),
  m_exception(0),
  m_propertyNames(CommonIdentifiers::shared()),
  m_callingExec(0),
  m_currentBody(0),
  m_function(0),
  m_arguments(0),
  m_localStore(0),
  m_markDescriptor(0)
{
    m_interpreter->setExecState(this);
}

ExecState::~ExecState()
{
    m_interpreter->setExecState(m_callingExec);
}

GlobalExecState::GlobalExecState(Interpreter* intp, JSObject* glob): ExecState(intp)
{
    scope.clear();
    scope.push(glob);
    m_codeType  = GlobalCode;
    m_variable = glob;
    m_thisVal  = glob;
}

InterpreterExecState::InterpreterExecState(Interpreter* intp, JSObject* glob,
                                           JSObject* thisObject, ProgramNode* body):
  ExecState(intp)
{
    m_currentBody = body;
    scope.clear();
    scope.push(glob);
    m_codeType = GlobalCode;
    m_variable = glob;
    // Per 10.2.1, we should use the global object here, but
    // Interpreter::evaluate permits it to be overriden, e.g. for LiveConnect.
    m_thisVal  = thisObject;
}

EvalExecState::EvalExecState(Interpreter* intp, JSObject* glob,
                             ProgramNode* body, ExecState* callingExecState):
  ExecState(intp)
{
    m_currentBody = body;
    m_codeType    = EvalCode;
    m_callingExec = callingExecState;
    if (m_callingExec) {
        scope = m_callingExec->scopeChain();
        m_variable = m_callingExec->variableObject();
        m_thisVal  = m_callingExec->thisValue();
        return;
    }

    // 10.2.2 talks about the behavior w/o a calling context here,
    // saying it should be like global code. This can not happen
    // in actual JS code, but it may be synthesized by e.g.
    // the JS debugger calling 'eval' itself, from globalExec
    m_thisVal  = glob;
    m_variable = glob;
    scope.clear();
    scope.push(glob);
}

FunctionExecState::FunctionExecState(Interpreter* intp, JSObject* thisObject,
                                     FunctionBodyNode* body, ExecState* callingExecState,
                                     FunctionImp* function, const List* args): ExecState(intp)
{
    m_function    = function;
    m_currentBody = body;
    m_arguments   = args;

    m_codeType    = FunctionCode;
    m_callingExec = callingExecState;
    scope = function->scope();
    m_variable = new ActivationImp(function, *args);// TODO: DontDelete ? (ECMA 10.2.3)
    scope.push(m_variable);
    m_thisVal  = thisObject;
}

FunctionExecState::~FunctionExecState()
{
    // The arguments list is only needed to potentially create the  arguments object,
    // which isn't accessible from nested scopes so we can discard the list as soon
    // as the function is done running.
    // This prevents lists of Lists from building up, waiting to be garbage collected
    static_cast<ActivationImp*>(activationObject())->releaseArguments();
}

} // namespace KJS

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
