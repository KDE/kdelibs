/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Harri Porten (porten@kde.org)
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

#include "kjsinterpreter.h"
#include "kjsprivate.h"
#include "kjs/interpreter.h"
#include "kjs/completion.h"
#include <qstring.h>
#include <stdio.h>

using namespace KJS;

KJSInterpreter::KJSInterpreter()
    : globCtx(0)
{
    Interpreter* ip = new Interpreter();
    ip->ref();
    hnd = reinterpret_cast<KJSInterpreterHandle*>(ip);
}

KJSInterpreter::~KJSInterpreter()
{
    Interpreter* ip = INTERPRETER(this);
    ip->deref();
    ip = 0;
}

KJSContext* KJSInterpreter::globalContext()
{
    Interpreter* ip = INTERPRETER(this);

    globCtx.hnd = EXECSTATE_HANDLE(ip->globalExec());
    return &globCtx;
}

KJSObject KJSInterpreter::globalObject()
{
    Interpreter* ip = INTERPRETER(this);

    return KJSObject(JSVALUE_HANDLE(ip->globalObject()));
}

KJSObject KJSInterpreter::evaluate(const QString& sourceURL,
                                   int startingLineNumber,
                                   const QString& code,
                                   KJSObject* thisValue)
{
    Interpreter* ip = INTERPRETER(this);
    
    JSValue* tv = thisValue ? JSVALUE(thisValue) : 0;
    KJS::Completion c = ip->evaluate(toUString(sourceURL), startingLineNumber,
                                     toUString(code), tv);

    if (c.complType() == Throw) {
#if 0
        CString msg = res.value()->toString(exec).UTF8String();
        JSObject* resObj = res.value()->toObject(exec);
        CString message = resObj->toString(exec).UTF8String();
        int line = resObj->toObject(exec)->get(exec, "line")->toUInt32(exec);

        if (fileName)
            fprintf(stderr, "%s (line %d): ", fileName, line);
        fprintf(stderr, "%s\n", msg.c_str());
#endif
        fprintf(stderr, "evaluate() threw an exception\n");
        return KJSObject();
    } else {
        if (c.isValueCompletion())
            return KJSObject(JSVALUE_HANDLE(c.value()));
        else
            return KJSUndefined();
    }
}

KJSObject KJSInterpreter::evaluate(const QString& code,
                                   KJSObject* thisValue)
{
    return evaluate("<string>", 0, code, thisValue);
}
