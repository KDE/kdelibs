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

#ifndef _DOM_TEXT_H_
#define _DOM_TEXT_H_

#include <dom_text.h>

#include <kjs/object.h>
#include <kjs/function.h>

namespace KJS {

  class DOMCharacterData : public HostObject {
  public:
    DOMCharacterData(DOM::CharacterData d) : data(d) { }
    virtual KJSO *get(const UString &p);
    virtual void put(const UString &p, KJSO *v);
  private:
    DOM::CharacterData data;
  };

  class DOMText : public HostObject {
  public:
    DOMText(DOM::Text t) : text(t) { }
    virtual KJSO *get(const UString &p);
  private:
    DOM::Text text;
  };

}; // namespace

#endif
