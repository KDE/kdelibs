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

 #ifndef KJS_BYTECODE_TYPES_H
 #define KJS_BYTECODE_TYPES_H

namespace KJS {
    class JSValue;
    class Identifier;

    enum OpType {
        OpType_Bool,
        OpType_UInt32,
        OpType_Value, // This can be immediate in input, but not output;
                      // immediate on input will be produced from other types, if possible
        OpType_NonConvertibleBase = 64, // All arguments with this bit set should never be converted
        OpType_Void  = OpType_NonConvertibleBase,
        OpType_Ident = OpType_NonConvertibleBase + 1, // Immediate only
        OpType_Addr  = OpType_NonConvertibleBase + 2, // Immediate only
    };

    // Represents a value, either as a result of evaluation of an expression, or
    // just a plain argument
    struct OpValue {
        bool   immediate;
        OpType type;
        union {
            bool        boolVal;
            unsigned    uintVal;
            JSValue*    jsVal;
            Identifier* identVal;
            unsigned    addrVal;
        } value;
    };
}

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
