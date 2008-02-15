/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 *  Copyright (C) 2007, 2008 Maksim Orlovich (maksim@kde.org)
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
 #include "nodes2bytecode.h"

 #include <typeinfo>
 #include <iostream>

namespace KJS {

static inline OpValue mkImmediateResult(OpType type)
{
    OpValue ret;
    ret.immediate = true;
    ret.type      = type;
}

OpValue Node::generateEvalCode(CodeBlock& block, CompileState*)
{
    std::cerr << "WARNING: no generateEvalCode for:" << typeid(*this).name() << "\n";

    OpValue dummy = mkImmediateResult(OpType_uint32);
    dummy.value.narrow.uint32Val = 42;
    return dummy;
}

void LocationNode::generateRefStoreBegin (CodeBlock& block, CompileState*)
{
    std::cerr << "WARNING: no generateRefStoreBegin for:" << typeid(*this).name() << "\n";
}

void LocationNode::generateRefStoreFinish(CodeBlock& block, CompileState*, const OpValue& result)
{
    std::cerr << "WARNING: no generateRefStoreFinish for:" << typeid(*this).name() << "\n";
}

void StatementNode::generateExecCode(CodeBlock& block, CompileState*)
{
    std::cerr << "WARNING: no generateExecCode for:" << typeid(*this).name() << "\n";
}

// ------------------------------ Basic literals -----------------------------------------

void NullNode::generateEvalCode(CodeBlock& block, ExecState *)
{
    OpValue res = mkImmediateResult(OpType_uint32);
    res.value.wide.valueVal = jsNull();
    return res;
}

void BooleanNode::generateEvalCode(CodeBlock& block, ExecState *)
{
    OpValue res = mkImmediateResult(OpType_bool);
    res.value.narrow.boolVal = value();
    return res;
}

void NumberNode::generateEvalCode(CodeBlock& block, ExecState *)
{
    // ### do we want a number type in wider use? Check if we can do an immediate value...
    JSValue* im = JSImmediate::from(value());
    if (im) {
    } else {
        // Allocate a register..

    }
}

// Uff. How does register release work !?

JSValue *StringNode::generateEvalCode(CodeBlock& ExecState *)
{
    return jsOwnedString(value());
}



}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on; hl c++;
