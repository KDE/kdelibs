// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_TEXT_H_
#define _KJS_TEXT_H_

#include <dom_text.h>

#include "kjs_binding.h"
#include "kjs_dom.h"

namespace KJS {

  class DOMCharacterData : public DOMNode {
  public:
    // Build a DOMCharacterData
    DOMCharacterData(ExecState *exec, DOM::CharacterData d);
    // Constructor for inherited classes
    DOMCharacterData(Object proto, DOM::CharacterData d);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValue(ExecState *, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::CharacterData toData() const { return static_cast<DOM::CharacterData>(node); }
    enum { Data, Length,
           SubstringData, AppendData, InsertData, DeleteData, ReplaceData };
  };

  class DOMText : public DOMCharacterData {
  public:
    DOMText(ExecState *exec, DOM::Text t);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValue(ExecState *, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::Text toText() const { return static_cast<DOM::Text>(node); }
    enum { SplitText };
  };

}; // namespace

#endif
