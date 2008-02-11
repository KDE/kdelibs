/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2008 Maksim Orlovich (maksim@kde.org)
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

 #include <wtf/Assertions.h>
    // be nice to our fruity overlords and their silly assert-unfriendly environment

namespace KJS {

enum ConversionOp {
    NoConversionPossible,
    Conv_jsImFromUInt, // Use JSImmediate::from
    Conv_imUIntToBool, // Just an integer -> bool swizzle of the immediate value
};

// This computes the conversion op of the value to the given type (perhaps NoConversionPossible!)
static ConversionOp computeCast(const OpValue* in, OpType outType, bool outImmediate) {
    // ### if this is sufficiently hot, it can be mostly coded up as a table, which can be autogen'd.
    // could be a good idea for efficiency, too, and can just go into codes.def. Might also make sense
    // if I somehow generalize the notion of types for more general tiling. Stuff like the
    // JSImmediate::from bit may be externalized via a bit. For a table of
    // 6 types one would need 3 bits for type * 2 + 1 bit for immediate/register ->
    // 128 entries.

    bool   inImmediate = in->immediate;
    OpType inType      = in->type;

    // Can't make a register result into an an immediate value,
    // and there is no reason ever to convert immediate to register, since
    // we will always have a specialization
    if (outImmediate != inImmediate)
        return NoConversionPossible;

    // Now we always have an I/I or R/R pairing..
    if (inType == outType)
        return 0; // Nothing to do!

    if ((inType | outType) & OpType_NonConvertibleBase) {
        // This should never happen -- we're either passing void around, or somehow trying to store
        // an address in a register, etc.
        CRASH();
        return NoConversionPossible;
    }

    // At thus point, only bool, uint32, and value exist.
    if (inImmediate) {
        // We got an immediate value from an another instruction --- must be int or bool
        if (inType == OpType_UInt32) {
            // immediate UInt32 -> ???
            if (outType == OpType_Value) {
                //immediate UInt32 => immediate JSValue
                if (JSImmediate::from(in->value.uintVal))
                    return Conv_jsImFromUInt;
                else
                    return NoConversionPossible;
            } else {
                ASSERT (outType == OpType_Boolean);
                // immediate UInt32 => immediate bool
                return Conv_imUIntToBool;
            }
        } else {
            ASSERT (inType == OpType_Bool);
            return Conv_JsBool;
        }
    } else {
        // Register <-> register conversion.
    }
}

// As above, but aggregates all the arguments, and checks the alignment/padding constraint
static int castCost(const Op& goal, const Op* cand)
{
    ASSERT(goal.numParams == cand->numParams);


    // Now collect cost of casts..
    int totalCost = 0;
    for (int p = 0; p < goal.numParams; ++p)
        totalCost += conversionCost(
}

OpValue selectAndEmitInstr(const Op&, OpValue* args)
{
    OpInstanceList& candidates = opSpecializations[op.baseInstr];

    // Compute alignment requirements based on PC and instruction arguments,
    // and set the pad flag as appropriate.
    // (For now?) We always enforce 8-alignment of pointers, even on 32-bit platforms,
    // to reduce difference in behavior between 2 teams, especially since all the devels seem
    // to use 32...

}

}
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
