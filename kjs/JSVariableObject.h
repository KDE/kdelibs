/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSVariableObject_h
#define JSVariableObject_h

#include "LocalStorage.h"
#include "SymbolTable.h"
#include "object.h"

#include <wtf/Vector.h>

namespace KJS {

    class JSVariableObject : public JSObject {
    public:
        virtual bool deleteProperty(ExecState*, const Identifier&);
        virtual void getPropertyNames(ExecState*, PropertyNameArray&);

        virtual void mark();

        enum {
          LengthSlot,
          OnStackSlot, // whether the storage is on stack
          NumVarObjectSlots = 2
        };

        int32_t& lengthSlot() { return localStorage[LengthSlot].val.int32Val; }
        const int32_t& lengthSlot() const { return localStorage[LengthSlot].val.int32Val; }

        bool& onStackSlot() { return localStorage[OnStackSlot].val.boolVal; }
    protected:
        JSVariableObject(): localStorage(0), symbolTable(0) { }
        ~JSVariableObject() { if (localStorage && !onStackSlot()) delete[] localStorage; }

        bool symbolTableGet(const Identifier&, PropertySlot&);
        bool symbolTablePut(const Identifier&, JSValue*, bool checkReadOnly);

    public:
        LocalStorageEntry* localStorage; // Storage for variables in the symbol table.
        SymbolTable*       symbolTable; // Maps name -> index in localStorage.
    };

    inline bool JSVariableObject::symbolTableGet(const Identifier& propertyName, PropertySlot& slot)
    {
        size_t index = symbolTable->get(propertyName.ustring().rep());
        if (index != missingSymbolMarker()) {
            slot.setValueSlot(this, &localStorage[index].val.valueVal);
            return true;
        }
        return false;
    }

    inline bool JSVariableObject::symbolTablePut(const Identifier& propertyName, JSValue* value, bool checkReadOnly)
    {
        size_t index = symbolTable->get(propertyName.ustring().rep());
        if (index == missingSymbolMarker())
            return false;
        LocalStorageEntry& entry = localStorage[index];
        if (checkReadOnly && (entry.attributes & ReadOnly))
            return true;
        entry.val.valueVal = value;
        return true;
    }

} // namespace KJS

#endif // JSVariableObject_h
