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
 #include "CompileState.h"

 #include <typeinfo>
 #include <iostream>

namespace KJS {

OpValue Node::generateEvalCode(CompileState* state, CodeBlock& block)
{
    std::cerr << "WARNING: no generateEvalCode for:" << typeid(*this).name() << "\n";

    return OpValue::immUInt32(42);
}

void LocationNode::generateRefStoreBegin (CodeBlock& block, CompileState*)
{
    std::cerr << "WARNING: no generateRefStoreBegin for:" << typeid(*this).name() << "\n";
}

void LocationNode::generateRefStoreFinish(CodeBlock& block, CompileState*, const OpValue& result)
{
    std::cerr << "WARNING: no generateRefStoreFinish for:" << typeid(*this).name() << "\n";
}

void StatementNode::generateExecCode(CompileState*, CodeBlock& block)
{
    std::cerr << "WARNING: no generateExecCode for:" << typeid(*this).name() << "\n";
}

// ------------------------------ Basic literals -----------------------------------------

OpValue NullNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return OpValue::immValue(jsNull());
}

OpValue BooleanNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return OpValue::immBool(value());
}

OpValue NumberNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
#if 0
    if (typeHint == OpType_Value) {
        // Try to fit into a JSValue if at all possible..
        JSValue* im = JSImmediate::from(value());
        if (im) {
            OpValue res = mkImmediateVal(OpType_value);
            return res;
        }
    }
#endif

    // Numeric-like..
    return OpValue::immNumber(value());
}


OpValue StringNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // For now, just generate a register value pointer.
    // We may want to permit string pointers as well, to help overload resolution,
    // but it's not clear whether that's useful, since we can't MM them. Perhaps
    // a special StringInstance type may be of use eventually.
    OpValue inStr = OpValue::immString(&val);

    OpValue out, regNum;
    comp->requestTemporary(OpType_value, out, regNum);
    CodeGen::emitOp(comp, block, Op_OwnedString, &regNum, &inStr);
    return out;
}

// ------------------------------ Code structure -------------------------------

void FuncDeclNode::generateExecCode(CompileState*, CodeBlock&)
{
    // No executable content...
}

void SourceElementsNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    node->generateExecCode(comp, block);

    // ### FIXME: how do we do proper completion?
    for (SourceElementsNode *n = next.get(); n; n = n->next.get()) {
        n->node->generateExecCode(comp, block);
    }
}

OpValue AssignExprNode::generateEvalCode(CompileState* state, CodeBlock& block)
{
    return expr->generateEvalCode(state, block);
}

void VarStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    for (VarDeclListNode *n = next.get(); n; n = n->next.get()) {
        // We only care about things which have an initializer ---
        // everything else is a no-op at execution time,
        // and only makes a difference at processVarDecl time
        if (n->var->init) {
            OpValue val = n->var->init->generateEvalCode(comp, block);

            size_t localID = comp->functionBody()->lookupSymbolID(n->var->ident);
            if (localID == missingSymbolMarker()) {
                // Generate a symbolic assignment, always to local scope
                // ### may want to tile this?

                OpValue ident = OpValue::immIdent(&n->var->ident);
                CodeGen::emitOp(comp, block, Op_SymPut, comp->localScope(), &ident, &val);
            } else {
                // Store to the local..
                OpValue dest = OpValue::immRegNum(localID);
                CodeGen::emitOp(comp, block, Op_RegPut, &dest, &val);
            }
        } // if initializer..
    } // for each decl..
}

void BlockNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    source->generateExecCode(comp, block);
}

void EmptyStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{}

void ExprStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    expr->generateEvalCode(comp, block);
}

void ReturnNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    OpValue arg;

    // Return is invalid in non-function..
    if (comp->codeType() != FunctionCode) {
        OpValue me = OpValue::immNode(this);
        OpValue se = OpValue::immUInt32(SyntaxError);
        OpValue msg = OpValue::immCStr("Invalid return statement.");
        CodeGen::emitOp(comp, block, Op_ReturnErrorCompletion, &me, &se, &msg);
        return;
    }

    if (!value)
        arg = OpValue::immValue(jsUndefined());
    else
        arg = value->generateEvalCode(comp, block);

    CodeGen::emitOp(comp, block, Op_Return, &arg);
}

void FunctionBodyNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    // Load 'scope' and 'this' pointer
    OpValue scopeVal, scopeReg;
    comp->requestTemporary(OpType_value, scopeVal, scopeReg);
    CodeGen::emitOp(comp, block, Op_GetVariableObject, &scopeReg);

    OpValue thisVal, thisReg;
    comp->requestTemporary(OpType_value, thisVal, thisReg);
    CodeGen::emitOp(comp, block, Op_This, &thisReg);

    comp->setPreloadRegs(&scopeVal, &thisVal);

    // Generate body...
    BlockNode::generateExecCode(comp, block);

    // Make sure we exit!
    CodeGen::emitOp(comp, block, Op_Exit);
}


}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on; hl c++;
