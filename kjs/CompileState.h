// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006 Apple Computer, Inc.
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
#ifndef COMPILE_STATE_H
#define COMPILE_STATE_H

#include "ExecState.h" // For codetype... Kinda odd.

#include "opcodes.h"
#include "bytecode/opargs.h"

#include <wtf/Assertions.h>
#include <wtf/Vector.h>

namespace KJS {

class TempDescriptor;
class FunctionBodyNode;

class CompileState
{
public:
    CompileState(CodeType ctype, FunctionBodyNode* fbody, Register initialMaxTemp):
        localScopeVal(0), thisVal(0), ctype(ctype),
        initialMaxTemp(initialMaxTemp), maxTemp(initialMaxTemp), fbody(fbody)
    {}

    FunctionBodyNode* functionBody() {
        return fbody;
    }

    CodeType codeType() {
        return ctype;
    }

    ~CompileState();

    // We distinguish two kinds of temporaries --- markable and not. They'll get
    // corresponding bits set in localStore when that's initialized.
    void requestTemporary(OpType type, OpValue& value, OpValue& reference);

    // This sets the registers containing the local scope and
    // 'this' values... It should be the rvalue, not the regnums
    void setPreloadRegs(OpValue* localReg, OpValue* thisReg) {
        localScopeVal = localReg;
        thisVal       = thisReg;
    }

    OpValue* localScope() {
        return localScopeVal;
    }

    OpValue* thisValue() {
        return thisVal;
    }

private:
    OpValue* localScopeVal;
    OpValue* thisVal;

    CodeType ctype;

    friend class TempDescriptor;
    WTF::Vector<TempDescriptor*> freeMarkTemps;
    WTF::Vector<TempDescriptor*> freeNonMarkTemps;
    WTF::Vector<bool>            shouldMark; // position 0 - initialMaxTemp
    Register initialMaxTemp;
    Register maxTemp;

    FunctionBodyNode* fbody;

    void reuse(TempDescriptor* desc, bool markable) {
        if (markable)
            freeMarkTemps.append(desc);
        else
            freeNonMarkTemps.append(desc);
    }
};

// Temporary descriptors are reference-counted by OpValue in order to automatically
// manage the lifetime of temporaries.
class TempDescriptor
{
public:
    TempDescriptor(CompileState* owner, Register temp, bool markable):
        owner(owner), temp(temp), markable(markable), refCount(0)
    {}

    Register reg() const {
        return temp;
    }

    void ref() {
        ++refCount;
    }

    void deref() {
        --refCount;
        if (refCount == 0)
            owner->reuse(this, markable);
    }
private:
    CompileState* owner;
    Register      temp;
    bool markable;
    int  refCount;
};

inline OpValue OpValue::immUInt32(uint32_t in) {
    OpValue res;
    initImm(res, OpType_uint32);
    res.value.narrow.uint32Val = in;
    return res;
}

inline OpValue OpValue::immNumber(double in) {
    OpValue res;
    initImm(res, OpType_number);
    res.value.wide.numberVal = in;
    return res;
}

inline OpValue OpValue::immValue(JSValue* in) {
    OpValue res;
    initImm(res, OpType_value);
    res.value.wide.valueVal = in;
    return res;
}

inline OpValue OpValue::immBool(bool in) {
    OpValue res;
    initImm(res, OpType_bool);
    res.value.narrow.boolVal = in;
    return res;
}

inline OpValue OpValue::immString(UString* in) {
    OpValue res;
    initImm(res, OpType_string);
    res.value.wide.stringVal = in;
    return res;
}

inline OpValue OpValue::immIdent(Identifier* in) {
    OpValue res;
    initImm(res, OpType_ident);
    res.value.wide.identVal = in;
    return res;
}

inline OpValue OpValue::immRegNum(Register in) {
    OpValue res;
    initImm(res, OpType_reg);
    res.value.narrow.regVal = in;
    return res;
}

inline OpValue OpValue::immNode(KJS::Node* in) {
    OpValue res;
    initImm(res, OpType_node);
    res.value.wide.nodeVal = in;
    return res;
}

inline OpValue OpValue::immCStr(const char* in) {
    OpValue res;
    initImm(res, OpType_cstr);
    res.value.wide.cstrVal = in;
    return res;
}

inline OpValue OpValue::immAddr(Addr in) {
    OpValue res;
    initImm(res, OpType_addr);
    res.value.narrow.addrVal = in;
    return res;
}

}

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
