// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ecma/kjs_views.h"
#include "ecma/kjs_css.h"
#include "kjs_views.lut.h"

using namespace KJS;

// -------------------------------------------------------------------------

const ClassInfo DOMAbstractView::info = { "AbstractView", 0, &DOMAbstractViewTable, 0 };

/*
@begin DOMAbstractViewTable 2
  document		DOMAbstractView::Document		DontDelete|ReadOnly
@end
@begin DOMAbstractViewProtoTable 1
  getComputedStyle	DOMAbstractView::GetComputedStyle	DontDelete|Function 2
@end
*/

DEFINE_PROTOTYPE("DOMAbstractView",DOMAbstractViewProto)
IMPLEMENT_PROTOFUNC(DOMAbstractViewProtoFunc)
IMPLEMENT_PROTOTYPE(DOMAbstractViewProto,DOMAbstractViewProtoFunc)

DOMAbstractView::DOMAbstractView(ExecState *exec, DOM::AbstractViewImpl* av)
  : m_impl(av)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMAbstractView::~DOMAbstractView()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp *DOMAbstractView::getValueProperty(ExecState *exec, int token)
{
    assert(token == Document);
    return getDOMNode(exec, impl()->document());
}

bool DOMAbstractView::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<DOMAbstractView, DOMObject>(exec, &DOMAbstractViewTable, this, propertyName, slot);
}


ValueImp *DOMAbstractViewProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMAbstractView, thisObj );
  DOM::AbstractViewImpl &abstractView = *static_cast<DOMAbstractView *>(thisObj)->impl();
  switch (id) {
    case DOMAbstractView::GetComputedStyle: {
        DOM::ElementImpl *arg0 = toElement(args[0]);
        if (!arg0)
          return Undefined(); // throw exception?
        else
          return getDOMCSSStyleDeclaration(exec, abstractView.getComputedStyle(arg0, args[1]->toString(exec).domString().implementation()));
      }
  }
  return Undefined();
}

ValueImp *KJS::getDOMAbstractView(ExecState *exec, DOM::AbstractViewImpl* av)
{
  return cacheDOMObject<DOM::AbstractViewImpl, DOMAbstractView>(exec, av);
}

DOM::AbstractViewImpl *KJS::toAbstractView (ValueImp *val)
{
  ObjectImp *obj = val->getObject();
  if (!obj || !obj->inherits(&DOMAbstractView::info))
    return 0;

  const DOMAbstractView  *dobj = static_cast<const DOMAbstractView *>(obj);
  return dobj->impl();
}
