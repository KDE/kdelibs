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

#include <stdio.h>

#include <kjs/operations.h>
#include <dom_string.h>

#include "kjs_dom.h"
#include "kjs_text.h"

using namespace KJS;

const TypeInfo DOMCharacterData::info = { "CharacterImp", HostType,
					  &DOMNode::info, 0, 0 };

KJSO DOMCharacterData::get(const UString &p) const
{
  if (p == "data")
    return String(data.data());
  else if (p == "length")
    return Number(data.length());
  else {
    KJSO tmp(new DOMNode(data));
    return tmp.get(p);
  }
}

void DOMCharacterData::put(const UString &p, const KJSO& v)
{
  if (p == "data") {
    String s = v.toString();
    data.setData(s.value().string());
  } else {
    KJSO tmp(new DOMNode(data));
    tmp.put(p, v);
  }
}

const TypeInfo DOMText::info = { "Text", HostType,
				 &DOMCharacterData::info, 0, 0 };

KJSO DOMText::get(const UString &p) const
{
  if (p == "")
    return Undefined(); // TODO
  else {
    DOM::Node n = text;
    KJSO tmp(new DOMCharacterData(n));
    return tmp.get(p);
  }
}

const TypeInfo DOMComment::info = { "Comment", HostType,
				 &DOMCharacterData::info, 0, 0 };

KJSO DOMComment::get(const UString &p) const
{
  if (p == "")
    return Undefined(); // TODO
  else {
    DOM::Node n = comment;
    KJSO tmp(new DOMCharacterData(n));
    return tmp.get(p);
  }
}
