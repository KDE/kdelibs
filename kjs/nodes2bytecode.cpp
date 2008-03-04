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
 #include <wtf/Assertions.h>

 #include <typeinfo>
 #include <iostream>

namespace KJS {

// A few helpers..
static void emitError(CompileState* comp, CodeBlock& block, Node* node, ErrorType type, const char* msgStr)
{
    OpValue me = OpValue::immNode(node);
    OpValue se = OpValue::immInt32(type);
    OpValue msg = OpValue::immCStr(msgStr);
    CodeGen::emitOp(comp, block, Op_RaiseError, 0, &me, &se, &msg);
}

static void emitSyntaxError(CompileState* comp, CodeBlock& block, Node* node, const char* msgStr)
{
    emitError(comp, block, node, SyntaxError, msgStr);
}

static void emitReferenceError(CompileState* comp, CodeBlock& block, Node* node, const char* msgStr)
{
    emitError(comp, block, node, ReferenceError, msgStr);
}


OpValue Node::generateEvalCode(CompileState* state, CodeBlock& block)
{
    std::cerr << "WARNING: no generateEvalCode for:" << typeid(*this).name() << "\n";
    ASSERT(0);

    return OpValue::immInt32(42);
}

void StatementNode::generateExecCode(CompileState*, CodeBlock& block)
{
    std::cerr << "WARNING: no generateExecCode for:" << typeid(*this).name() << "\n";
    ASSERT(0);
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
    double d = value();
    int32_t i32 = JSValue::toInt32(d);
    if (double(i32) == d)
        return OpValue::immInt32(i32);
    else
        return OpValue::immNumber(d);
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

OpValue RegExpNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // ### TODO: cache the engine object?
    OpValue out;
    OpValue patternV = OpValue::immString(&pattern);
    OpValue flagsV   = OpValue::immString(&flags);
    CodeGen::emitOp(comp, block, Op_NewRegExp, &out, &patternV, &flagsV);
    return out;
}

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
        if (comp->codeType() != FunctionCode || ident == CommonIdentifiers::shared()->arguments)
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
            op = Op_LocalVarGet;
        else if (!dynamicLocal) // can skip one scope..
            op = Op_NonLocalVarGet;

        CodeGen::emitOp(comp, block, op, &out, &varName);
    } else {
        // Register read. Easy.
        out = OpValue::reg(OpType_value, index);
    }

    return out;
}

CompileReference* VarAccessNode::generateRefBegin (CompileState* comp, CodeBlock& block, int flags)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    bool errorOnFail = (flags & ErrorOnFail);
    bool fetchNow    = (flags & ImmediateRead);

    // If this is not a register local, we need to look up the scope first..
    if (index == missingSymbolMarker()) {
        CompileReference* ref = new CompileReference;

        // We can take a shortcut if we're in global and don't need to detect lookup failure,
        // since the scope will always be the variable object
        if (comp->codeType() == GlobalCode && !errorOnFail) {
            ref->baseObj = *comp->localScope();
            return ref;
        }

        OpValue varName = OpValue::immIdent(&ident);
        if (fetchNow) {
            ref->preloaded = true;
            OpName op;
            if (errorOnFail) {
                if (dynamicLocal)
                    op = Op_ScopeLookupAndGetChecked;
                else
                    op = Op_NonLocalScopeLookupAndGetChecked;
            } else {
                if (dynamicLocal)
                    op = Op_ScopeLookupAndGet;
                else
                    op = Op_NonLocalScopeLookupAndGet;
            }

            OpValue readReg;
            comp->requestTemporary(OpType_value, ref->preloadVal, readReg);
            // ### I should probably go to 4 args, for better error messages
            CodeGen::emitOp(comp, block, op, &ref->baseObj, &readReg, &varName);
        } else {
            OpValue errFail = OpValue::immNode(errorOnFail ? this : 0);
            CodeGen::emitOp(comp, block, dynamicLocal ? Op_ScopeLookup : Op_NonLocalScopeLookup,
                        &ref->baseObj, &varName, &errFail);
        }
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
        if (ref->preloaded) {
            return ref->preloadVal;
        } else {
            // Symbolic read from the appropriate scope, which is in base..
            OpValue varName = OpValue::immIdent(&ident);
            CodeGen::emitOp(comp, block, Op_SymGetKnownObject, &out, &ref->baseObj, &varName);
        }
    } else {
        // Straight register get..
        out = OpValue::reg(OpType_value, index);
    }
    return out;
}

OpValue VarAccessNode::generateRefBase(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    if (ref)
        return ref->baseObj;
    else
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
                        &ref->baseObj, &varName, &valToStore);
    } else {
        // Straight register put..
        OpValue destReg = OpValue::immRegNum(index);
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &destReg, &valToStore);
    }
}

OpValue VarAccessNode::generateRefDelete(CompileState* comp, CodeBlock& block)
{
    bool dynamicLocal;
    size_t index = localID(comp, dynamicLocal);

    if (index == missingSymbolMarker()) {
        // Fetch the scope...
        OpValue base;
        OpValue varName = OpValue::immIdent(&ident);
        OpValue silent  = OpValue::immNode(0);
        CodeGen::emitOp(comp, block, dynamicLocal ? Op_ScopeLookup : Op_NonLocalScopeLookup,
                        &base, &varName, &silent);

        // Remove the property..
        OpValue out;
        CodeGen::emitOp(comp, block, Op_SymDeleteKnownObject, &out, &base, &varName);
        return out;
    } else {
        // Normal locals are DontDelete, so this always fails.
        return OpValue::immBool(false);
    }
}

// ------------------------------ GroupNode----------------------------------------

OpValue GroupNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return group->generateEvalCode(comp, block);
}

// ------------------------------ Object + Array literals --------------------------

OpValue ArrayNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue arr;
    CodeGen::emitOp(comp, block, Op_NewEmptyArray, &arr);

    OpValue und = OpValue::immValue(jsUndefined());

    int pos = 0;
    for (ElementNode* el = element.get(); el; el = el->next.get()) {
        if (!el->node) {
            // Fill elision w/undefined, unless we can just skip over to a value
            for (int i = 0; i < el->elision; i++) {
                OpValue ind = OpValue::immInt32(pos);
                CodeGen::emitOp(comp, block, Op_BracketPutKnownObject, 0, &arr, &ind, &und);
                ++pos;
            }
        } else {
            pos += el->elision;
        }

        if (el->node) {
            OpValue val = el->node->generateEvalCode(comp, block);
            OpValue ind = OpValue::immInt32(pos);
            CodeGen::emitOp(comp, block, Op_BracketPutKnownObject, 0, &arr, &ind, &val);
            ++pos;
        }
    }

    for (int i = 0; i < elision; i++) {
        OpValue ind = OpValue::immInt32(pos);
        CodeGen::emitOp(comp, block, Op_BracketPutKnownObject, 0, &arr, &ind, &und);
        ++pos;
    }

    return arr;
}

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
    CodeGen::emitOp(comp, block, Op_BracketGet, &ret, &base, &index);
    return ret;
}

CompileReference* BracketAccessorNode::generateRefBegin (CompileState* comp, CodeBlock& block, int)
{
    CompileReference* ref = new CompileReference;
    // base
    OpValue baseV = expr1->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_ToObject, &ref->baseObj, &baseV);
    return ref;
}

OpValue BracketAccessorNode::generateRefRead(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    OpValue ret;
    OpValue index = expr2->generateEvalCode(comp, block);

    // ### in cases like ++/-- we may check redundantly.
    CodeGen::emitOp(comp, block, Op_BracketGetKnownObject, &ret, &ref->baseObj, &index);

    return ret;
}

OpValue BracketAccessorNode::generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref)
{
    return ref->baseObj;
}

void BracketAccessorNode::generateRefWrite(CompileState* comp, CodeBlock& block,
                                             CompileReference* ref, OpValue& valToStore)
{
    OpValue index = expr2->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_BracketPutKnownObject, 0, &ref->baseObj, &index, &valToStore);
}

OpValue BracketAccessorNode::generateRefDelete(CompileState* comp, CodeBlock& block)
{
    OpValue base  = expr1->generateEvalCode(comp, block);
    OpValue index = expr2->generateEvalCode(comp, block);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_BracketDelete, &out, &base, &index);
    return out;
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

CompileReference* DotAccessorNode::generateRefBegin (CompileState* comp, CodeBlock& block, int)
{
    CompileReference* ref = new CompileReference;
    // base
    // ### since we don't need to do toString conversion on field here,
    // we can move the object evaluation later in some cases, w/o violating
    // exception precision wrt to 11.2.1
    OpValue baseV = expr->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_ToObject, &ref->baseObj, &baseV);
    return ref;
}

OpValue DotAccessorNode::generateRefRead(CompileState* comp, CodeBlock& block, CompileReference* ref)
{
    OpValue out;
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymGetKnownObject, &out, &ref->baseObj, &varName);
    return out;
}

OpValue DotAccessorNode::generateRefBase(CompileState*, CodeBlock& block, CompileReference* ref)
{
    return ref->baseObj;
}

void DotAccessorNode::generateRefWrite(CompileState* comp, CodeBlock& block,
                                             CompileReference* ref, OpValue& valToStore)
{
    OpValue varName = OpValue::immIdent(&ident);
    CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0, &ref->baseObj, &varName, &valToStore);
}

OpValue DotAccessorNode::generateRefDelete(CompileState* comp, CodeBlock& block)
{
    OpValue base = expr->generateEvalCode(comp, block);
    OpValue varName = OpValue::immIdent(&ident);
    OpValue out;
    CodeGen::emitOp(comp, block, Op_SymDelete, &out, &base, &varName);
    return out;
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

OpValue FunctionCallValueNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = expr->generateEvalCode(comp, block);
    args->generateEvalArguments(comp, block);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_FunctionCall, &out, &v, comp->globalScope());
    return out;
}

OpValue FunctionCallReferenceNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    Node* cand = expr->nodeInsideAllParens();
    ASSERT(cand->isLocation());
    LocationNode* loc = static_cast<LocationNode*>(cand);

    CompileReference* ref =  loc->generateRefBegin(comp, block, LocationNode::ErrorOnFail
                                                                | LocationNode::ImmediateRead);
    OpValue funVal = loc->generateRefRead(comp, block, ref);

    args->generateEvalArguments(comp, block);

    // Can do this safely before checking function, since it's been computed before
    OpValue newThis = loc->generateRefBase(comp, block, ref);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_FunctionCall, &out, &funVal, &newThis);
    delete ref;
    return out;
}

OpValue PostfixNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    Node* cand = m_loc->nodeInsideAllParens();
    if (!cand->isLocation()) {
        emitReferenceError(comp, block, this,
                m_oper == OpPlusPlus ?
                    "Postfix ++ operator applied to value that is not a reference." :
                    "Postfix -- operator applied to value that is not a reference.");
        return OpValue::immValue(jsUndefined());
    }

    LocationNode* loc = static_cast<LocationNode*>(cand);

    // ### we want to fold this in if the kid is a local -- any elegant way?
    CompileReference* ref = loc->generateRefBegin(comp, block, LocationNode::ErrorOnFail
                                                               | LocationNode::ImmediateRead);

    //read current value
    OpValue curV = loc->generateRefRead(comp, block, ref);

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

    loc->generateRefWrite(comp, block, ref, newV);
    delete ref;
    return curV;
}

OpValue DeleteReferenceNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return loc->generateRefDelete(comp, block);
}

OpValue DeleteValueNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    return OpValue::immBool(true);
}

OpValue VoidNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    (void)expr->generateEvalCode(comp, block);
    return OpValue::immValue(jsUndefined());
}

OpValue TypeOfReferenceNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // false: no error if not there, gives undefined instead
    CompileReference* ref = loc->generateRefBegin(comp, block, LocationNode::ImmediateRead);

    OpValue v = loc->generateRefRead(comp, block, ref);

    OpValue out;
    CodeGen::emitOp(comp, block, Op_TypeOf, &out, &v);
    delete ref;
    return out;
}

OpValue TypeOfValueNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue v = m_expr->generateEvalCode(comp, block);
    OpValue typeOfV;
    CodeGen::emitOp(comp, block, Op_TypeOf, &typeOfV, &v);
    return typeOfV;
}

OpValue PrefixNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    Node* cand = m_loc->nodeInsideAllParens();
    if (!cand->isLocation()) {
        emitReferenceError(comp, block, this,
                m_oper == OpPlusPlus ?
                    "Prefix ++ operator applied to value that is not a reference." :
                    "Prefix -- operator applied to value that is not a reference.");
        return OpValue::immValue(jsUndefined());
    }

    LocationNode* loc = static_cast<LocationNode*>(cand);

    // ### we want to fold this in if the kid is a local -- any elegant way?
    CompileReference* ref = loc->generateRefBegin(comp, block, LocationNode::ErrorOnFail
                                                               | LocationNode::ImmediateRead);

    //read current value
    OpValue curV = loc->generateRefRead(comp, block, ref);

    OpValue newV;
    CodeGen::emitOp(comp, block, (m_oper == OpPlusPlus) ? Op_Add1 : Op_Sub1,
                    &newV, &curV);

    // Write out + return new value.
    loc->generateRefWrite(comp, block, ref, newV);
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
    CodeGen::emitOp(comp, block, Op_BitNot, &out, &v);
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
    case OpURShift:
        // operator >>>
        codeOp = Op_URShift;
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
        codeOp = Op_In;
        break;
    case OpInstanceOf:
        codeOp = Op_InstanceOf;
        break;

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

OpValue ConditionalNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    // As above, we have some difficulty here, since we do not have a way of knowing
    // the types in advance, but since we can't reasonably speculate on them both being bool,
    // we just always produce a value.
    OpValue resVal, resReg;

    // Evaluate conditional, and jump..
    OpValue v = logical->generateEvalCode(comp, block);
    Addr jumpToElse = CodeGen::emitOp(comp, block, Op_IfNotJump, 0, &v, OpValue::dummyAddr());

    // True branch
    OpValue v1out = expr1->generateEvalCode(comp, block);

    // Is the register here sufficient? If so, just use it
    if (v1out.type == OpType_value && !v1out.immediate) {
        resVal = v1out;
        resReg = OpValue::immRegNum(v1out.value.narrow.regVal);
    } else {
        comp->requestTemporary(OpType_value, resVal, resReg);
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &resReg, &v1out);
    }

    Addr jumpToAfter = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

    // Jump to else goes here.
    CodeGen::patchJumpToNext(block, jumpToElse, 1);

    // : part..
    OpValue v2out = expr2->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &resReg, &v2out);

    // After everything
    CodeGen::patchJumpToNext(block, jumpToAfter, 0);

    return resVal;
}

OpValue FuncExprNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    OpValue out;
    OpValue nameV = OpValue::immIdent(&ident);
    OpValue bodyV = OpValue::immNode(body.get());
    CodeGen::emitOp(comp, block, Op_EvalFuncExpr, &out, &nameV, &bodyV);
    return out;
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
    Node* cand = m_loc->nodeInsideAllParens();
    if (!cand->isLocation()) {
        emitReferenceError(comp, block, this, "Left side of assignment is not a reference.");
        return OpValue::immValue(jsUndefined());
    }

    LocationNode* loc = static_cast<LocationNode*>(cand);

    int flags = (m_oper == OpEqual ? 0 : LocationNode::ErrorOnFail | LocationNode::ImmediateRead);
    // Lookup failure is OK for assignments, but not for += and such; those also
    // fetch the old value

    CompileReference* ref = loc->generateRefBegin(comp, block, flags);

    OpValue v;
    if (m_oper == OpEqual) {
        v = m_right->generateEvalCode(comp, block);
    } else {
        OpValue v1 = loc->generateRefRead(comp, block, ref);
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
        case OpURShift:
            codeOp = Op_URShift;
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
        default:
            ASSERT(0);
        }

        CodeGen::emitOp(comp, block, codeOp, &v, &v1, &v2);
    }

    loc->generateRefWrite(comp, block, ref, v);

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

void VarDeclNode::generateCode(CompileState* comp, CodeBlock& block)
{
    // We only care about things which have an initializer ---
    // everything else is a no-op at execution time,
    // and only makes a difference at processVarDecl time
    if (init) {
        OpValue val = init->generateEvalCode(comp, block);

        size_t localID = comp->functionBody()->lookupSymbolID(ident);
        if (localID == missingSymbolMarker()) {
            // Generate a symbolic assignment, always to local scope
            // ### may want to tile this?

            OpValue identV = OpValue::immIdent(&ident);
            CodeGen::emitOp(comp, block, Op_SymPutKnownObject, 0, comp->localScope(), &identV, &val);
        } else {
            // Store to the local..
            OpValue dest = OpValue::immRegNum(localID);
            CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &dest, &val);
        }
    } // if initializer..
}

OpValue VarDeclListNode::generateEvalCode(CompileState* comp, CodeBlock& block)
{
    for (VarDeclListNode *n = this; n; n = n->next.get())
        n->var->generateCode(comp, block);

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
    OpValue val = expr->generateEvalCode(comp, block);

    // Update the result for eval..
    if (comp->codeType() == EvalCode)
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, comp->evalResultReg(), &val);
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

void ForInNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    if (varDecl)
        varDecl->generateCode(comp, block);

    OpValue val = expr->generateEvalCode(comp, block);
    OpValue obj; // version of val after toObject, returned by BeginForIn.

    // Fetch the property name array..
    CodeGen::emitOp(comp, block, Op_BeginForIn, &obj, &val);
    comp->pushUnwindHandler();
    comp->enterLoop(this); // must do this here, since continue shouldn't pop our iterator!

    // We put the test first here, since the test and the fetch are combined.
    OpValue sym;
    Addr fetchNext = CodeGen::emitOp(comp, block, Op_NextForInEntry, &sym, &obj, OpValue::dummyAddr());

    // Write to the variable
    assert (lexpr->isLocation());
    LocationNode* loc = static_cast<LocationNode*>(lexpr.get());
    CompileReference* ref = loc->generateRefBegin(comp, block);
    loc->generateRefWrite (comp, block, ref, sym);
    delete ref;

    // Run the body.
    statement->generateExecCode(comp, block);

    // Can fix the continues to go back to the test...
    comp->resolvePendingContinues(this, block, fetchNext);

    // Jump back..
    OpValue backVal = OpValue::immAddr(fetchNext);
    CodeGen::emitOp(comp, block, Op_Jump, 0, &backVal);

    // The end address is here (#2 since return val..)
    CodeGen::patchJumpToNext(block, fetchNext, 2);

    // Cleanup
    CodeGen::emitOp(comp, block, Op_EndForIn);
    comp->popUnwindHandler();

    comp->exitLoop(this, block);
}

// Helper for continue/break -- emits stack cleanup call if needed.
static void handleJumpOut(CompileState* comp, CodeBlock& block, int targetNest)
{
    int toUnwind = comp->unwindHandlerDepth() - targetNest;
    if (toUnwind) {
        OpValue unwind = OpValue::immInt32(toUnwind);
        CodeGen::emitOp(comp, block, Op_UnwindStacks, 0, &unwind);
    }
}

void ContinueNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    CompileState::LabelInfo dest = comp->resolveContinueLabel(ident);
    if (!dest.handlerNode) {
        if (ident.isEmpty())
            emitSyntaxError(comp, block, this, "Illegal continue without target outside a loop.");
        else
            emitSyntaxError(comp, block, this, "Invalid label in continue.");
    } else {
        // Continue can only be used for a loop
        if (dest.handlerNode->isIterationStatement()) {
            // Make sure we pop any nested scopes, etc.
            handleJumpOut(comp, block, dest.handlerDepth);

            // Emit a jump...
            Addr    pc    = CodeGen::nextPC(block);
            CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

            // Queue destination for resolution
            comp->addPendingContinue(dest.handlerNode, pc);
        } else {
            emitSyntaxError(comp, block, this, "Invalid continue target; must be a loop.");
        }
    }
}

void BreakNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    CompileState::LabelInfo dest = comp->resolveBreakLabel(ident);
    if (!dest.handlerNode) {
        if (ident.isEmpty())
            emitSyntaxError(comp, block, this, "Illegal break without target outside a loop or switch.");
        else
            emitSyntaxError(comp, block, this, "Invalid label in break.");
    } else {
        // Make sure we pop any nested scopes, etc.
        handleJumpOut(comp, block, dest.handlerDepth);

        // Break can be used everywhere.. Hence, emit a jump...
        Addr    pc    = CodeGen::nextPC(block);
        CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

        // Queue destination for resolution
        comp->addPendingBreak(dest.handlerNode, pc);
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

    comp->pushScope();
    CodeGen::emitOp(comp, block, Op_PushScope, 0, &scopeObj);

    statement->generateExecCode(comp, block);

    CodeGen::emitOp(comp, block, Op_PopScope, 0);
    comp->popScope();
}

void LabelNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    if (!comp->pushLabel(label)) {
        emitSyntaxError(comp, block, this, "Duplicated label found.");
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

void ThrowNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    OpValue projectile = expr->generateEvalCode(comp, block);
    CodeGen::emitOp(comp, block, Op_Throw, 0, &projectile);
}

void TryNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    // Set the catch handler, run the try clause, pop the try handler..
    Addr setCatchHandler = CodeGen::emitOp(comp, block, Op_PushExceptionHandler, 0, OpValue::dummyAddr());
    comp->pushUnwindHandler();

    tryBlock->generateExecCode(comp, block);

    CodeGen::emitOp(comp, block, Op_PopExceptionHandler);
    comp->popUnwindHandler();

    // Jump over the catch if try is OK
    Addr jumpOverCatch;
    if (catchBlock)
        jumpOverCatch = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

    // Exceptions would go here --- either in a catch or a finally.
    CodeGen::patchJumpToNext(block, setCatchHandler, 0);

    Addr catchToFinallyEH;
    if (catchBlock) {
        // If there is a finally block, that acts as an exception handler for the catch;
        // we need to set it before entering the catch scope, so the cleanup entries for that
        // are on top
        if (finallyBlock) {
            catchToFinallyEH = CodeGen::emitOp(comp, block, Op_PushExceptionHandler, 0, OpValue::dummyAddr());
            comp->pushUnwindHandler();
        }

        // Emit the catch.. Note: the unwinder has already popped the catch handler entry,
        // but the exception object is still set, since we need to make a scope for it.
        // EnterCatch would do that for us, given the name
        OpValue catchVar = OpValue::immIdent(&exceptionIdent);
        CodeGen::emitOp(comp, block, Op_EnterCatch, 0, &catchVar);
        comp->pushScope();

        catchBlock->generateExecCode(comp, block);

        // If needed, cleanup the binding to finally, and always cleans the catch scope
        CodeGen::emitOp(comp, block, Op_ExitCatch);
        comp->popScope();

        if (finallyBlock) {
            CodeGen::emitOp(comp, block, Op_PopExceptionHandler);
            comp->popUnwindHandler();
        }

        // after an OK 'try', we always go to finally, if any, which needs an op if there is a catch block
        CodeGen::patchJumpToNext(block, jumpOverCatch, 0);
    }


    if (finallyBlock) {
        if (catchBlock) // if a catch was using us an EH, patch that instruction to here
            CodeGen::patchJumpToNext(block, catchToFinallyEH, 0);

        CodeGen::emitOp(comp, block, Op_DeferException);
        comp->pushUnwindHandler();

        finallyBlock->generateExecCode(comp, block);

        CodeGen::emitOp(comp, block, Op_ReactivateException);
        comp->popUnwindHandler();
    }
}

void FunctionBodyNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    // Load 'scope', global and 'this' pointers.
    // ### probably want to do all of these direct, and skip a bunch of ops.
    OpValue scopeVal;
    CodeGen::emitOp(comp, block, Op_GetVariableObject, &scopeVal);

    OpValue globalVal;
    CodeGen::emitOp(comp, block, Op_GetGlobalObject, &globalVal);

    OpValue thisVal;
    CodeGen::emitOp(comp, block, Op_This, &thisVal);

    comp->setPreloadRegs(&scopeVal, &globalVal, &thisVal);

    OpValue evalResReg, evalResVal;
    if (comp->codeType() == EvalCode) {
        comp->requestTemporary(OpType_value, evalResVal, evalResReg);
        comp->setEvalResultRegister(&evalResReg);

        OpValue und = OpValue::immValue(jsUndefined());
        CodeGen::emitOp(comp, block, Op_RegPutValue, 0, &evalResReg, &und);
    }

    // Set unwind..
    Addr unwind = CodeGen::emitOp(comp, block, Op_PushExceptionHandler, 0, OpValue::dummyAddr());

    // Generate body...
    BlockNode::generateExecCode(comp, block);

    // Make sure we exit!
    if (comp->codeType() == EvalCode)
        CodeGen::emitOp(comp, block, Op_ExitEval, 0, &evalResVal);
    else
        CodeGen::emitOp(comp, block, Op_Exit);

    // Unwind stuff..
    CodeGen::patchJumpToNext(block, unwind, 0);
    CodeGen::emitOp(comp, block, Op_PropagateException);
}

void SwitchNode::generateExecCode(CompileState* comp, CodeBlock& block)
{
    CaseBlockNode* caseBlock = this->block.get();

    // The code we produce has 2 stages: first, we emit all the conditionals, and pick
    // the label to jump to (with the jump to the default being last),
    // then we just emit all the clauses in the row. The breaks will be
    // resolved at the end --- for that, we bind ourselves for label'less break.
    comp->pushDefaultBreak(this);

    // What we compare with
    OpValue switchOn = expr->generateEvalCode(comp, block);

    WTF::Vector<Addr> list1jumps;
    WTF::Vector<Addr> list2jumps;
    Addr defJump;

    // Jumps for list 1..
    for (ClauseListNode* iter = caseBlock->list1.get(); iter; iter = iter->next.get()) {
        OpValue ref = iter->clause->expr->generateEvalCode(comp, block);
        OpValue match;
        CodeGen::emitOp(comp, block, Op_StrEq, &match, &switchOn, &ref);

        Addr jumpToClause = CodeGen::emitOp(comp, block, Op_IfJump, 0, &match, OpValue::dummyAddr());
        list1jumps.append(jumpToClause);
    }

    // Jumps for list 2..
    for (ClauseListNode* iter = caseBlock->list2.get(); iter; iter = iter->next.get()) {
        OpValue ref = iter->clause->expr->generateEvalCode(comp, block);
        OpValue match;
        CodeGen::emitOp(comp, block, Op_StrEq, &match, &switchOn, &ref);

        Addr jumpToClause = CodeGen::emitOp(comp, block, Op_IfJump, 0, &match, OpValue::dummyAddr());
        list2jumps.append(jumpToClause);
    }

    // Jump to default (or after, if there is no default)
    defJump = CodeGen::emitOp(comp, block, Op_Jump, 0, OpValue::dummyAddr());

    // Now, we can actually emit the bodies, fixing the addresses as we go
    int p = 0;
    for (ClauseListNode* iter = caseBlock->list1.get(); iter; iter = iter->next.get()) {
        CodeGen::patchJumpToNext(block, list1jumps[p], 1);
        if (iter->clause->source)
            iter->clause->source->generateExecCode(comp, block);
        ++p;
    }

    if (caseBlock->def) {
        CodeGen::patchJumpToNext(block, defJump, 0);
        if (caseBlock->def->source)
            caseBlock->def->source->generateExecCode(comp, block);
    }

    p = 0;
    for (ClauseListNode* iter = caseBlock->list2.get(); iter; iter = iter->next.get()) {
        CodeGen::patchJumpToNext(block, list2jumps[p], 1);
        if (iter->clause->source)
            iter->clause->source->generateExecCode(comp, block);
        ++p;
    }

    // If we didn't have a default, that jump is to here..
    if (!caseBlock->def)
        CodeGen::patchJumpToNext(block, defJump, 0);

    // Breaks should go after us..
    comp->popDefaultBreak();
    comp->resolvePendingBreaks(this, block, CodeGen::nextPC(block));
}

void ImportStatement::generateExecCode(CompileState*, CodeBlock&)
{} // handled as a declaration..


}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on; hl c++;
