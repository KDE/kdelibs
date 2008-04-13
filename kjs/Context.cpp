// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2006, 2007 Apple Computer, Inc.
 *  Copyright (C) 2007 Christopher E. Hyde <C.Hyde@parableuk.force9.co.uk>
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

#include "context.h"
#include "scriptfunction.h"

namespace KJS {

// ECMA 10.2
Context::Context(JSObject* glob, Interpreter* interpreter, JSObject* thisV, 
                 FunctionBodyNode* currentBody, CodeType type, Context* callingCon,
                 FunctionImp* func, const List* args)
    : m_interpreter(interpreter)
    , m_currentBody(currentBody)
    , m_function(func)
    , m_arguments(args)
{
    m_codeType = type;
    m_callingContext = callingCon;

    // ECMA 10.2
    switch(type) {
    case EvalCode:
        // 10.2.2 talks about the behavior w/o a calling context here,
        // saying it should be like global code. This can not happen
        // in actual JS code, but it may be synthesized by e.g.
        // the JS debugger calling 'eval' itself, from globalExec
        if (m_callingContext)
        {
            scope = m_callingContext->scopeChain();

            m_variable = m_callingContext->variableObject();
            m_thisVal  = m_callingContext->thisValue();
            break;
        }
        thisV = glob;
        // Fallthrough intentional.
    case GlobalCode:
        scope.clear();
        scope.push(glob);

        m_variable = glob;
        // Per 10.2.1, we should use the global object here, but 
        // Interpreter::evaluate permits it to be overridden, e.g. for LiveConnect.
        m_thisVal = thisV;
        break;
    case FunctionCode:
        scope = func->scope();
        m_variable = new ActivationImp(func, *args);// TODO: DontDelete ? (ECMA 10.2.3)
        scope.push(m_variable);
        m_thisVal  = thisV;
        break;
    }
    
    if (m_thisVal)
      assert(!(m_thisVal->getObject() && m_thisVal->getObject()->isActivation()));

    m_interpreter->setContext(this);
}

Context::~Context()
{
    m_interpreter->setContext(m_callingContext);

    // The arguments list is only needed to potentially create the  arguments object,
    // which isn't accessible from nested scopes so we can discard the list as soon
    // as the function is done running.
    // This prevents lists of Lists from building up, waiting to be garbage collected
    if (m_codeType == FunctionCode)
        activationObject()->releaseArguments();
}

void Context::mark()
{
    for (Context* context = this; context; context = context->m_callingContext)
        context->scope.mark();
}

} // namespace KJS
