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
    OpValue se = OpValue::immInt32(SyntaxError);
    OpValue msg = OpValue::immCStr(msgStr);
    CodeGen::emitOp(comp, block, Op_RaiseError, 0, &me, &se, &msg);
}

OpValue Node::generateEvalCode(CompileState* state, CodeBlock& block)
{
    std::cerr << "WARNING: no generateEvalCode for:" << typeid(*this).name() << "\n";

    return OpValue::immInt32(42);
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

    OpValue out;
    if (index == missingSymbolMarker()) {
        // Emit a symbolic variable
        OpValue varName = OpValue::immIdent(&ident);

        OpName op = Op_VarGet; // in general, have to search the whole chain..
        if (comp->codeType() == GlobalCode && !comp->inNestedScope())
                // unless we're in GlobalCode, w/o extra stuff on top, so there is nothing to search
            op = Op_SymGetVarObject;
        else if (!dynamicLocal) // can skip one scope..
            op = Op_NonLocalVarGet;

        CodeGen::emitOp(comp, block, op, &out, &varName);
    } else {
        // Register read. Easy.
        out = OpValue::reg(OpType_value, index);
    }

    return out;
}

CompileReference* VarAccessNode::generateRefBegin (CompileState* comp, CodeBlock& block, bool errorOnFail)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    // If this is not a register local, we need to look up the scope first..
    if (index == missingSymbolMarker()) {
        CompileReference* ref = new CompileReference;

        // We can take a shortcut if we're in global and don't need to detect lookup failure,
        // since the scope will always be the variable object
        if (comp->codeType() == GlobalCode && !errorOnFail) {
            ref->val1 = *comp->localScope();
            return ref;
        }

        OpValue varName = OpValue::immIdent(&ident);
        OpValue errFail = OpValue::immBool(errorOnFail);
        CodeGen::emitOp(comp, block, dynamicLocal ? Op_ScopeLookup : Op_NonLocalScopeLookup,
                        &ref->val1, &varName, &errFail);
        return ref;
    }

    return 0;
}

OpValue VarAccessNode::generateRefRead(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    OpValue out;
    if (index == missingSymbolMarker()) {
        // Symbolic read from the appropriate scope, which is in base..
        OpValue varName = OpValue::immIdent(&ident);
        CodeGen::emitOp(comp, block, Op_SymGetKnownObject, &out, &ref->val1, &varName);
    } else {
        // Straight register get..
        out = OpValue::reg(OpType_value, index);
    }
    return out;
}

OpValue VarAccessNode::generateRefBase(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    return *comp->globalScope();
}

void VarAccessNode::generateRefWrite(CompileState* comp, CodeBlock& block,
                                           CompileReference* ref, OpValue& valToStore)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    if (index == missingSymbolMarker()) {
        // Symbolic write to the appropriate scope..
        OpValue varName = OpValue::immIdent(&ident);
        CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0,
                        &ref->val1, &varName, &valToStore);
    } else {
        // Straight register put..
        OpValue destReg = OpValue::immRegNum(index);
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &destReg, &valToStore);
    }
}

// ------------------------------ GroupNode----------------------------------------

OpValue GroupNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return group->generateEvalCode(comp, block);
}

// ------------------------------ Object + Array literals --------------------------
// TODO: ElementNode, ArrayNode, PropertyNameNode, PropertyNode, PropertyListNode, ObjectLiteralNode

OpValue ObjectLiteralNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue obj;
    CodeGen::emitOp(comp, block, Op_NewObject, &obj);

    for (PropertyListNode* entry = list.get(); entry; entry = entry->next.get()) {
        PropertyNode* prop = entry->node.get();
        OpValue name = OpValue::immIdent(&prop->name->str);
        OpValue val  = prop->assign->generateEvalCode(comp, block);

        switch (prop->type) {
        case PropertyNode::Getter:
            CodeGen::emitOp(comp, block, Op_DefineGetter, 0, &obj, &name, &val);
            break;
        case PropertyNode::Setter:
            CodeGen::emitOp(comp, block, Op_DefineSetter, 0, &obj, &name, &val);
            break;
        case PropertyNode::Constant:
            CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0, &obj, &name, &val);
            break;
        }
    }

    return obj;
}

// ------------------------------ BracketAccessorNode --------------------------------
OpValue BracketAccessorNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue ret;
    OpValue base  = expr1->generateEvalCode(comp, block);
    OpValue index = expr2->generateEvalCode(comp, block);

    // ### optimize foo["bar"] ?
    if (index.type == OpType_int32)
        CodeGen::emitOp(comp, block, Op_IndexGet, &ret, &base, &index);
    else
        CodeGen::emitOp(comp, block, Op_BracketGet, &ret, &base, &index);
    return ret;
}

CompileReference* BracketAccessorNode::generateRefBegin (CompileState* comp, CodeBlock& block, bool)
{
    CompileReference* ref = new CompileReference;
    // base
    ref->val1 = expr1->generateEvalCode(comp, block);
    return ref;
}

OpValue BracketAccessorNode::generateRefRead(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    OpValue ret;
    OpValue index = expr2->generateEvalCode(comp, block);

    // ### in cases like ++/-- we may check redundantly.
    if (index.type == OpType_int32)
        CodeGen::emitOp(comp, block, Op_IndexGet, &ret, &ref->val1, &index);
    else
        CodeGen::emitOp(comp, block, Op_BracketGet, &ret, &ref->val1, &index);

    return ret;
}

OpValue BracketAccessorNode::generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref)
{
    return ref->val1;
}

void BracketAccessorNode::generateRefWrite(CompileState* comp, CodeBlock& block,
                                             CompileReference* ref, OpValue& valToStore)
{
    OpValue index = expr2->generateEvalCode(comp, block);
    if (index.type == OpType_int32)
        CodeGen::emitOp(comp, block, Op_IndexPut, 0, &ref->val1, &index, &valToStore);
    else
        CodeGen::emitOp(comp, block, Op_BracketPut, 0, &ref->val1, &index, &valToStore);
}

// ------------------------------ DotAccessorNode --------------------------------

// ECMA 11.2.1b
OpValue DotAccessorNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue ret;
    OpValue base    = expr->generateEvalCode(comp, block);
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymGet, &ret, &base, &varName);
    return ret;
}

CompileReference* DotAccessorNode::generateRefBegin (CompileState* comp, CodeBlock& block, bool)
{
    CompileReference* ref = new CompileReference;
    // base
    ref->val1 = expr->generateEvalCode(comp, block);
    return ref;
}

OpValue DotAccessorNode::generateRefRead(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    OpValue out;
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymGet, &out, &ref->val1, &varName);
    return out;
}

OpValue DotAccessorNode::generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref)
{
    return ref->val1;
}

void DotAccessorNode::generateRefWrite(CompileState* comp, CodeBlock& block,
                                             CompileReference* ref, OpValue& valToStore)
{
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymPut, 0, &ref->val1, &varName, &valToStore);
}

// ------------------ ........

void ArgumentsNode::generateEvalArguments(CompileState* comp, CodeBlock& block)
{
    WTF::Vector<OpValue> args;

    // We need evaluate arguments and push them in separate steps as there may be
    // function/ctor calls inside.
    for (ArgumentListNode* arg = list.get(); arg; arg = arg->next.get()) {
        args.append(arg->expr->generateEvalCode(comp, block));
    }

    CodeGen::emitOp(comp, block, Op_ClearArgs, 0);

    size_t c = 0;
    while (c < args.size()) {
        if (c + 3 <= args.size()) {
            CodeGen::emitOp(comp, block, Op_Add3Arg, 0, &args[c], &args[c+1], &args[c+2]);
            c += 3;
        } else if (c + 2 <= args.size()) {
            CodeGen::emitOp(comp, block, Op_Add2Arg, 0, &args[c], &args[c+1]);
            c += 2;
        } else {
            CodeGen::emitOp(comp, block, Op_AddArg, 0, &args[c]);
            c += 1;
        }
    }
}

OpValue NewExprNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = expr->generateEvalCode(comp, block);

    if (args)
        args->generateEvalArguments(comp, block);
    else
        CodeGen::emitOp(comp, block, Op_ClearArgs, 0);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_CtorCall, &out, &v);
    return out;
}

OpValue FunctionCallReferenceNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    CompileReference* ref =  expr->generateRefBegin(comp, block, true /* issue error if not there*/);
    OpValue funVal = expr->generateRefRead(comp, block, ref);

    args->generateEvalArguments(comp, block);

    // Can do this safely before checking function, since it's been computed before
    OpValue newThis = expr->generateRefBase(comp, block, ref);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_FunctionCall, &out, &funVal, &newThis);
    delete ref;
    return out;
}

OpValue PostfixNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // ### we want to fold this in if the kid is a local -- any elegant way?

    CompileReference* ref = m_loc->generateRefBegin(comp, block, true /* issue error if not there*/);

    //read current value
    OpValue curV = m_loc->generateRefRead(comp, block, ref);

    // We need it to be a number..
    if (curV.type != OpType_number) {
        OpValue numVal;
        CodeGen::emitConvertTo(comp, block, &curV, OpType_number, &numVal);
        curV = numVal;
    }

    // Compute new one
    OpValue newV;
    CodeGen::emitOp(comp, block, (m_oper == OpPlusPlus) ? Op_Add1 : Op_Sub1,
                    &newV, &curV);

    m_loc->generateRefWrite(comp, block, ref, newV);
    delete ref;
    return curV;
}

OpValue PrefixNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // ### we want to fold this in if the kid is a local -- any elegant way?
    CompileReference* ref = m_loc->generateRefBegin(comp, block, true /* issue error if not there*/);

    //read current value
    OpValue curV = m_loc->generateRefRead(comp, block, ref);

    OpValue newV;
    CodeGen::emitOp(comp, block, (m_oper == OpPlusPlus) ? Op_Add1 : Op_Sub1,
                    &newV, &curV);

    // Write out + return new value.
    m_loc->generateRefWrite(comp, block, ref, newV);
    delete ref;
    return newV;
}

OpValue UnaryPlusNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // This is basically just a number cast
    OpValue curV = expr->generateEvalCode(comp, block);

    if (curV.type != OpType_number) {
        OpValue numVal;
        CodeGen::emitConvertTo(comp, block, &curV, OpType_number, &numVal);
        curV = numVal;
    }

    return curV;
}

OpValue NegateNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = expr->generateEvalCode(comp, block);
    OpValue negV;
    CodeGen::emitOp(comp, block, Op_Neg, &negV, &v);
    return negV;
}

OpValue BitwiseNotNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = expr->generateEvalCode(comp, block);
    OpValue out;
    //CodeGen::emitOp(comp, block, Op_BitNot, &out, &v);
    CodeGen::emitOp(comp, block, Op_PrintInfo, 0, &v);
    out = OpValue::immInt32(0);
#warning "Debug only" // I need to remove this, so may as well use a gcc-ism's :-)
    return out;
}

OpValue LogicalNotNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = expr->generateEvalCode(comp, block);
    OpValue out;
    CodeGen::emitOp(comp, block, Op_LogicalNot, &out, &v);
    return out;
}

OpValue BinaryOperatorNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v1 = expr1->generateEvalCode(comp, block);
    OpValue v2 = expr2->generateEvalCode(comp, block);

    OpName codeOp; // ### could perhaps skip conversion entirely,
                   // and set these in the parser?
    switch (oper) {
    case OpMult:
        // operator *
        codeOp = Op_Mult;
        break;
    case OpDiv:
        // operator /
        codeOp = Op_Div;
        break;
    case OpMod:
        // operator %
        codeOp = Op_Mod;
        break;
    case OpPlus:
        // operator +
        codeOp = Op_Add;
        break;
    case OpMinus:
        // operator -
        codeOp = Op_Sub;
        break;
    case OpLShift:
        // operator <<
        codeOp = Op_LShift;
        break;
    case OpRShift:
        // operator >>
        codeOp = Op_RShift;
        break;
    case OpLess:
        // operator <
        codeOp = Op_Less;
        break;
    case OpGreaterEq:
        // operator >=
        codeOp = Op_GreaterEq;
        break;
    case OpGreater:
        // operator >
        codeOp = Op_Greater;
        break;
    case OpLessEq:
        // operator <=
        codeOp = Op_LessEq;
        break;
    case OpEqEq:
        // operator ==
        codeOp = Op_EqEq;
        break;
    case OpNotEq:
        // operator !=
        codeOp = Op_NotEq;
        break;
    case OpStrEq:
        // operator ===
        codeOp = Op_StrEq;
        break;
    case OpStrNEq:
        // operator !==
        codeOp = Op_StrNEq;
        break;
    case OpBitAnd:
        // operator &
        codeOp = Op_BitAnd;
        break;
    case OpBitXOr:
        // operator ^
        codeOp = Op_BitXOr;
        break;
    case OpBitOr:
        // operator |
        codeOp = Op_BitOr;
        break;

    case OpIn:
    case OpInstanceOf:
    case OpURShift:
        //TODO!
#if 0
        // operator >>>
        return jsNumber(v1->toUInt32(exec) >> (v2->toUInt32(exec) & 0x1f));
#endif
    default:
        assert(!"BinaryOperatorNode: unhandled switch case");
    }

    OpValue out;
    CodeGen::emitOp(comp, block, codeOp, &out, &v1, &v2);
    return out;
}

OpValue BinaryLogicalNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // This is somewhat ugly since we can't patchup labels in already generated
    // code, and don't know the types in advance. It could also benefit from
    // a type hint, since it's easier if we only want a bool, which is quite common

    OpValue a = expr1->generateEvalCode(comp, block);

    // Make a register for storing the result, and put 'a' there, as out first guess.
    OpValue aVal, aReg;
    comp->requestTemporary(a.type, aVal, aReg);
    CodeGen::emitRegStore(comp, block, &aReg, &a);

    // Is this enough to shortcircuit?
    // if op is && and a is false, we jump out, ditto
    // for || and true.
    Addr jumpToShortCircuit = CodeGen::emitOp(comp, block, oper == OpAnd ? Op_IfNotJump : Op_IfJump,
                                              0, &a, OpValue::dummyAddr());

    // Now, generate the code for b...
    OpValue b = expr2->generateEvalCode(comp, block);

    // Hopefully, either the types match, or the result slot is already a value,
    // so we can just promote b (which will happen automatically to produce param for Op_RegPutVal)
    if (a.type == b.type || a.type == OpType_value) {
        if (a.type == OpType_value)
            CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &aReg, &b);
        else
            CodeGen::emitRegStore(comp, block, &aReg, &b);
        CodeGen::patchJumpToNext(block, jumpToShortCircuit, 1);
        return aVal;
    } else {
        // We need to promote 'a' as well, which means we need to skip over the code jumpToShortCircuit
        // went to after handling store of 'b'.

        // Get a new register for the result, put b there..
        OpValue resVal, resReg;
        comp->requestTemporary(OpType_value, resVal, resReg);
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &resReg, &b);

        // skip to after a promotion..
        Addr jumpToAfter = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

        // a's promotion goes here..
        CodeGen::patchJumpToNext(block, jumpToShortCircuit, 1);
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &resReg, &a);

        // now we're after it..
        CodeGen::patchJumpToNext(block, jumpToAfter, 0);

        return resVal;
    }
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

OpValue AssignNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    CompileReference* ref = m_loc->generateRefBegin(comp, block,
        m_oper != OpEqual /* Lookup failure is OK for assignments, but not for += and such */);
    OpValue v;

    if (m_oper == OpEqual) {
        v = m_right->generateEvalCode(comp, block);
    } else {
        OpValue v1 = m_loc->generateRefRead(comp, block, ref);
        OpValue v2 = m_right->generateEvalCode(comp, block);

        OpName codeOp;
        switch (m_oper) {
        case OpMultEq:
            codeOp = Op_Mult;
            break;
        case OpDivEq:
            codeOp = Op_Div;
            break;
        case OpModEq:
            codeOp = Op_Mod;
            break;
        case OpPlusEq:
            codeOp = Op_Add;
            break;
        case OpMinusEq:
            codeOp = Op_Sub;
            break;
        case OpLShift:
            codeOp = Op_LShift;
            break;
        case OpRShift:
            codeOp = Op_RShift;
            break;
        case OpAndEq:
            codeOp = Op_BitAnd;
            break;
        case OpXOrEq:
            codeOp = Op_BitXOr;
            break;
        case OpOrEq:
            codeOp = Op_BitOr;
            break;
        case OpURShift:
        default:
            ASSERT(0);
        }

        CodeGen::emitOp(comp, block, codeOp, &v, &v1, &v2);
    }

    m_loc->generateRefWrite(comp, block, ref, v);

    delete ref;
    return v;
}

OpValue CommaNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    expr1->generateEvalCode(comp, block);
    return expr2->generateEvalCode(comp, block);
}

OpValue AssignExprNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return expr->generateEvalCode(comp, block);
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
                CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &dest, &val);
            }
        } // if initializer..
    } // for each decl..

    return OpValue::immInt32(0); // unused..
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

void IfNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    // eval the condition
    OpValue cond = expr->generateEvalCode(comp, block);

    // If condition is not true, jump to after or else..
    Addr afterTrueJmp = CodeGen::emitOp(comp, block, Op_IfNotJump, 0, &cond, OpValue::dummyAddr());

    // Emit the body of true...
    statement1->generateExecCode(comp, block);

    // If we have an else, add in a jump to skip over it.
    Addr afterAllJmp = 0;
    if (statement2)
        afterAllJmp = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

    // This is where we go if true fails --- else, or afterwards.
    CodeGen::patchJumpToNext(block, afterTrueJmp, 1);

    if (statement2) {
        // Body of else
        statement2->generateExecCode(comp, block);

        // Fix up the jump-over code
        CodeGen::patchJumpToNext(block, afterAllJmp, 0);
    }
}

void DoWhileNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    comp->enterLoop(this);

    // Body
    OpValue beforeBody = OpValue::immAddr(CodeGen::nextPC(block));
    statement->generateExecCode(comp, block);

    // continues go to just before the test..
    comp->resolvePendingContinues(this, block, CodeGen::nextPC(block));

    // test
    OpValue cond = expr->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_IfJump, 0, &cond, &beforeBody);

    comp->exitLoop(this, block);
}

void WhileNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    comp->enterLoop(this);

    // Jump to test.
    Addr  jumpToTest = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

    // Body
    OpValue beforeBody = OpValue::immAddr(CodeGen::nextPC(block));
    statement->generateExecCode(comp, block);

    // continues go to just before the test..
    comp->resolvePendingContinues(this, block, CodeGen::nextPC(block));

    // patch up the destination of the initial jump to test
    CodeGen::patchJumpToNext(block, jumpToTest, 0);

    // test
    OpValue cond = expr->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_IfJump, 0, &cond, &beforeBody);

    comp->exitLoop(this, block);
}

void ForNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    comp->enterLoop(this);

    // Initializer, if any..
    if (expr1)
        expr1->generateEvalCode(comp, block);

    // Insert a jump to the loop test (address not yet known)
    Addr jumpToTest = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

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
    CodeGen::patchJumpToNext(block, jumpToTest, 0);

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
            CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

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
        CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

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

void WithNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    OpValue scopeObj = expr->generateEvalCode(comp, block);

    CodeGen::emitOp(comp, block, Op_PushScope, 0, &scopeObj);
    comp->pushScope();
    statement->generateExecCode(comp, block);
    comp->popScope();
    CodeGen::emitOp(comp, block, Op_PopScope, 0);
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
    // Load 'scope', global and 'this' pointers.
    // ### probably want to do direct, and skip 3 ops.
    OpValue scopeVal;
    CodeGen::emitOp(comp, block, Op_GetVariableObject, &scopeVal);

    OpValue globalVal;
    CodeGen::emitOp(comp, block, Op_GetGlobalObject, &globalVal);

    OpValue thisVal;
    CodeGen::emitOp(comp, block, Op_This, &thisVal);

    comp->setPreloadRegs(&scopeVal, &globalVal, &thisVal);

    // Generate body...
    BlockNode::generateExecCode(comp, block);

    // Make sure we exit!
    CodeGen::emitOp(comp, block, Op_Exit);
}


}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on; hl c++;
