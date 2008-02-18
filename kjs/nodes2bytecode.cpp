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

// A few helpers..
static void emitSyntaxError(CompileState* comp, CodeBlock& block, Node* node, const char* msgStr)
{
    OpValue me = OpValue::immNode(node);
    OpValue se = OpValue::immUInt32(SyntaxError);
    OpValue msg = OpValue::immCStr(msgStr);
    CodeGen::emitOp(comp, block, Op_ReturnErrorCompletion, 0, &me, &se, &msg);
}

OpValue Node::generateEvalCode(CompileState* state, CodeBlock& block)
{
    std::cerr << "WARNING: no generateEvalCode for:" << typeid(*this).name() << "\n";

    return OpValue::immUInt32(42);
}

CompileReference* LocationNode::generateRefStoreBegin (CompileState*, CodeBlock& block)
{
    std::cerr << "WARNING: no generateRefStoreBegin for:" << typeid(*this).name() << "\n";
    return 0;
}

void LocationNode::generateRefStoreFinish(CompileState*, CodeBlock& block,
                                          CompileReference* ref, OpValue& valToStore)
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

    OpValue out;
    CodeGen::emitOp(comp, block, Op_OwnedString, &out, &inStr);
    return out;
}

// TODO: RegExpNode::generateEvalCode(CompileState* state, CodeBlock& block)

OpValue ThisNode::generateEvalCode(CompileState* comp, CodeBlock&)
{
    return *comp->thisValue();
}

// ------------------------------ VarAccessNode ----------------------------------------

size_t VarAccessNode::localID(CompileState* comp, bool& dynamicLocal)
{
    // Are we inside a with or catch? In that case, it's all dynamic. Boo.
    if (comp->inNestedScope()) {
        dynamicLocal = true;
        return missingSymbolMarker();
    }

    size_t index = comp->functionBody()->lookupSymbolID(ident);
    if (index == missingSymbolMarker()) {
        if (comp->codeType() == GlobalCode || ident == CommonIdentifiers::shared()->arguments)
            dynamicLocal = true;
        else
            dynamicLocal = false; // Can skip the local scope..
    }
    return index;
}

OpValue VarAccessNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    // ### TODO: we want to emit Op_GetVariableObject for global code here/in refs.
    // or just use the globalGet/globalPut; but I am concerned about LiveConnect eval..

    OpValue out;
    if (index == missingSymbolMarker()) {
        // Emit a full variable read, perhaps skipping one scope
        OpValue varName = OpValue::immIdent(&ident);
        CodeGen::emitOp(comp, block, dynamicLocal ? Op_LocalVarGet : Op_NonLocalVarGet, &out, &varName);
    } else {
        // Register read. Easy.
        out.immediate = false;
        out.type      = OpType_value;
        out.value.narrow.regVal = index;
    }

    return out;
}

CompileReference* VarAccessNode::generateRefStoreBegin (CompileState* comp, CodeBlock& block)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    // If this is not a register local, we need to look up the scope first..
    if (index == missingSymbolMarker()) {
        CompileReference* ref = new CompileReference;
        ref->mayFail = true;
        OpValue foundReg;
        comp->requestTemporary(OpType_bool, ref->foundResult, foundReg);

        OpValue varName = OpValue::immIdent(&ident);
        CodeGen::emitOp(comp, block, dynamicLocal ? Op_ScopeLookup : Op_NonLocalScopeLookup,
                        &ref->val1, &foundReg, &varName);
        return ref;
    }

    return 0;
}

void VarAccessNode::generateRefStoreFinish(CompileState* comp, CodeBlock& block,
                                           CompileReference* ref, OpValue& valToStore)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    if (index == missingSymbolMarker()) {
        // Symbolic write to the appropriate scope..
        OpValue varName = OpValue::immIdent(&ident);
        CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0,
                        &ref->val1, &varName, &valToStore);
        delete ref;
    } else {
        // Straight register put..
        OpValue destReg = OpValue::immRegNum(index);
        CodeGen::emitOp(comp, block, Op_RegPut, 0, &destReg, &valToStore);
    }
}

// ------------------------------ GroupNode----------------------------------------

OpValue GroupNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return group->generateEvalCode(comp, block);
}

// ------------------------------ Object + Array literals --------------------------
// TODO: ElementNode, ArrayNode, PropertyNameNode, PropertyNode, PropertyListNode, ObjectLiteralNode

// ------------------------------ BracketAccessorNode --------------------------------


// ------------------------------ DotAccessorNode --------------------------------

// ECMA 11.2.1b
OpValue DotAccessorNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue ret;
    OpValue base    = expr->generateEvalCode(comp, block);
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymGet, &ret, &base, &varName);
}

CompileReference* DotAccessorNode::generateRefStoreBegin (CompileState* comp, CodeBlock& block)
{
    CompileReference* ref = new CompileReference;
    ref->mayFail = false;

    // base
    ref->var1 = expr->generateEvalCode(comp, block);
    return ref;
}

void DotAccessorNode::generateRefStoreFinish(CompileState* comp, CodeBlock& block,
                                             CompileReference* ref, OpValue& valToStore)
{
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymPut, 0, &base, &varName, &valToStore);
    delete ref;
}

OpValue PostfixNode::generateEvalCode(CompileState* state, CodeBlock& block)
{
#if 0
  Reference ref = m_loc->evaluateReference(exec);

  //handle error..
  if (!ref.found)
    return throwUndefinedVariableError(exec, ref.ident);

  KJS_CHECKEXCEPTIONVALUE

  //do the op
  JSValue *v = ref.read(exec);
  double n = v->toNumber(exec);
  double newValue = (m_oper == OpPlusPlus) ? n + 1 : n - 1;
  ref.write(exec, jsNumber(newValue));
  return jsNumber(n);
#endif
}

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

OpValue VarDeclListNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    for (VarDeclListNode *n = this; n; n = n->next.get()) {
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
                CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0, comp->localScope(), &ident, &val);
            } else {
                // Store to the local..
                OpValue dest = OpValue::immRegNum(localID);
                CodeGen::emitOp(comp, block, Op_RegPut, 0, &dest, &val);
            }
        } // if initializer..
    } // for each decl..

    return OpValue::immUInt32(0); // unused..
}

void VarStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    next->generateEvalCode(comp, block);
}

void BlockNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    if (source)
        source->generateExecCode(comp, block);
}

void EmptyStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{}

void ExprStatementNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    expr->generateEvalCode(comp, block);
}

void ForNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    comp->enterLoop(this);

    // Initializer, if any..
    if (expr1)
        expr1->generateEvalCode(comp, block);

    // Insert a jump to the loop test (address not yet known)
    OpValue testAddr   = OpValue::immAddr(0);
    Addr    jumpToTest = CodeGen::emitOp(comp, block, Op_Jump, 0, &testAddr);

    // Generate loop body..
    OpValue bodyAddr = OpValue::immAddr(CodeGen::nextPC(block));
    statement->generateExecCode(comp, block);

    // We're about to generate the increment... The continues should go here..
    comp->resolvePendingContinues(this, block, CodeGen::nextPC(block));

    // ### there is a CheckTimeout hook here in nodes.cpp...

    // Generate increment...
    if (expr3)
      expr3->generateEvalCode(comp, block);

    // The test goes here, so patch up the previous jump..
    testAddr = OpValue::immAddr(CodeGen::nextPC(block));
    CodeGen::patchOpArgument(block, jumpToTest, 0, testAddr);

    // Make the test itself --- if it exists..
    if (expr2) {
        OpValue cond = expr2->generateEvalCode(comp, block);
        CodeGen::emitOp(comp, block, Op_IfJump, 0, &cond, &bodyAddr);
    } else {
        // Just jump back to the body.
        CodeGen::emitOp(comp, block, Op_Jump, 0, &bodyAddr);
    }

    comp->exitLoop(this, block);
}

void ContinueNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    Node* dest = comp->resolveContinueLabel(ident);
    if (!dest) {
        if (ident.isEmpty())
            emitSyntaxError(comp, block, this, "Illegal continue without target outside a loop.");
        else
            emitSyntaxError(comp, block, this, "Invalid label in continue.");
    } else {
        // Continue can only be used for a loop
        if (dest->isIterationStatement()) {
            // Emit a jump...
            Addr    pc    = CodeGen::nextPC(block);
            OpValue dummy = OpValue::immAddr(0);
            CodeGen::emitOp(comp, block, Op_Jump, 0, &dummy);

            // Queue destination for resolution
            comp->addPendingContinue(dest, pc);
        } else {
            emitSyntaxError(comp, block, this, "Invalid continue target; must be a loop.");
        }
    }
}

void BreakNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    Node* dest = comp->resolveBreakLabel(ident);
    if (!dest) {
        if (ident.isEmpty())
            emitSyntaxError(comp, block, this, "Illegal break without target outside a loop or switch.");
        else
            emitSyntaxError(comp, block, this, "Invalid label in break.");
    } else {
        // Break can be used everywhere..
        // Hence, emit a jump...
        Addr    pc    = CodeGen::nextPC(block);
        OpValue dummy = OpValue::immAddr(0);
        CodeGen::emitOp(comp, block, Op_Jump, 0, &dummy);

        // Queue destination for resolution
        comp->addPendingBreak(dest, pc);
    }
}

void ReturnNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    OpValue arg;

    // Return is invalid in non-function..
    if (comp->codeType() != FunctionCode) {
        emitSyntaxError(comp, block, this, "Invalid return.");
        return;
    }

    if (!value)
        arg = OpValue::immValue(jsUndefined());
    else
        arg = value->generateEvalCode(comp, block);

    CodeGen::emitOp(comp, block, Op_Return, 0, &arg);
}

void LabelNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    if (!comp->pushLabel(label)) {
        // ### FIXME
        //return createErrorNode(SyntaxError, "Duplicated label %s found.", label);
        return;
    }

    if (!statement->isLabelNode()) // we're the last label..
        comp->bindLabels(statement.get());

    // Generate code for stuff inside the label...
    statement->generateExecCode(comp, block);

    // Fix up any breaks..
    if (!statement->isLabelNode())
        comp->resolvePendingBreaks(statement.get(), block, CodeGen::nextPC(block));

    comp->popLabel();
}

void FunctionBodyNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    // Load 'scope' and 'this' pointer
    OpValue scopeVal;
    CodeGen::emitOp(comp, block, Op_GetVariableObject, &scopeVal);

    OpValue thisVal;
    CodeGen::emitOp(comp, block, Op_This, &thisVal);

    comp->setPreloadRegs(&scopeVal, &thisVal);

    // Generate body...
    BlockNode::generateExecCode(comp, block);

    // Make sure we exit!
    CodeGen::emitOp(comp, block, Op_Exit);
}


}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on; hl c++;
