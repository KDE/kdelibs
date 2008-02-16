/*
 *  Opcode argument data structures KJS/Frostbyte
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Maksim Orlovich (maksim@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef OP_ARGS_H
#define OP_ARGS_H

#include "opcodes.h"

namespace KJS {

class TempDescriptor;

// The NarrowArg and WideArg unions correspond to the encoding of
// 4- and 8-byte arguments in the bytecode.
// Note that the union field names should correspond to codes.def types ---
// type foo will be accessed as fooVal

union NarrowArg
{
    bool     boolVal;
    unsigned uint32Val;
    Addr     addrVal;
    Register regVal;
    unsigned char asBytes[4];
};

union WideArg
{
    JSValue*    valueVal;
    Identifier* identVal;
    UString*    stringVal;
    double      numberVal;
    unsigned char asBytes[8];
};

// This describes where result of evaluating an expression, or an argument
// to a function, is stored. If immediate is true, it is included directly inside
// the structure. Otherwise, it's passed by the register number specified in
// regVal.
// Note: there is a difference between values stored in registers, and register names.
// The former have immediate = false, and some type such as OpType_Bool. The latter have
// immediate = true, and type = OpType_reg.
struct OpValue
{
    bool   immediate;
    OpType type;
    RefPtr<TempDescriptor> ownedTemp; // Temporary the lifetime of which we reserve
    union {
        NarrowArg  narrow;
        WideArg    wide;
    } value;
};

}

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
