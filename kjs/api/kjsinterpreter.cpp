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
#include "kjs/object.h"
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

KJSInterpreter::KJSInterpreter(const KJSObject& global)
    : globCtx(0)
{
    JSValue* gv = JSVALUE(&global);
    assert(gv->isObject());
    JSObject* go = static_cast<JSObject*>(gv);
    Interpreter* ip = new Interpreter(go);
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
        ExecState* exec = ip->globalExec();
        CString msg = c.value()->toString(exec).UTF8String();
        JSObject* resObj = c.value()->toObject(exec);
        CString message = resObj->toString(exec).UTF8String();
        int line = resObj->toObject(exec)->get(exec, "line")->toUInt32(exec);

        if (!sourceURL.isEmpty())
            fprintf(stderr, "%s (line %d): ", qPrintable(sourceURL), line);
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

bool KJSInterpreter::normalizeCode(const QString& code, QString* normalized,
                                   int* errLine, QString* errMsg)
{
    assert(normalized);

    UString codeOut, msg;
    bool success = Interpreter::normalizeCode(toUString(code), &codeOut,
                                              errLine, &msg);

    *normalized = toQString(codeOut);
    *errMsg = toQString(msg);

    return success;
}

