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
#include "CompileState.h"

#include <wtf/Assertions.h>
#include <wtf/Vector.h>

namespace KJS {

void CompileState::requestTemporary(OpType type, OpValue& value, OpValue& reference)
{
    ASSERT(type == OpType_value || type == OpType_bool || type == OpType_uint32);

    value.type      = type;
    value.immediate = false;

    reference.type      = OpType_reg;
    reference.immediate = true;

    TempDescriptor* temp = 0;

    bool markable = (type == OpType_value);

    if (markable && !freeMarkTemps.isEmpty()) {
        temp = freeMarkTemps.last();
        freeMarkTemps.removeLast();
    } else if (!markable && !freeNonMarkTemps.isEmpty()) {
        temp = freeNonMarkTemps.last();
        freeNonMarkTemps.removeLast();
    }

    if (!temp) {
        Register id = maxTemp;
        shouldMark.append(markable);
        temp = new TempDescriptor(this, id, markable);
        ++maxTemp;
    }

    value.ownedTemp = temp;
    value.value.narrow.regVal = temp->reg();

    reference.ownedTemp = temp;
    reference.value.narrow.regVal = temp->reg();
}

} //namespace KJS

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
