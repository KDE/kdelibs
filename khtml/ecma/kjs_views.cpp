// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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

#include "kjs_views.h"
#include "kjs_views.lut.h"
#include "kjs_css.h"
#include <qptrdict.h>
#include <dom2_events.h>

using namespace KJS;

QPtrDict<DOMAbstractView> abstractViews;

// -------------------------------------------------------------------------

const ClassInfo DOMAbstractView::info = { "AbstractView", 0, &DOMAbstractViewTable, 0 };
/*
@begin DOMAbstractViewTable 2
  document		DOMAbstractView::Document		DontDelete|ReadOnly
  getComputedStyle	DOMAbstractView::GetComputedStyle	DontDelete|Function 2
@end
*/
IMPLEMENT_PROTOFUNC(DOMAbstractViewFunc)

DOMAbstractView::~DOMAbstractView()
{
  abstractViews.remove(abstractView.handle());
}

Value DOMAbstractView::tryGet(ExecState *exec, const UString &p) const
{
  if ( p == "document" )
    return getDOMNode(exec,abstractView.document());
  else if ( p == "getComputedStyle" )
    return lookupOrCreateFunction<DOMAbstractViewFunc>(exec,p,this,DOMAbstractView::GetComputedStyle,2,DontDelete|Function);
  else
    return DOMObject::tryGet(exec,p);
}

Value DOMAbstractViewFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::AbstractView abstractView = static_cast<DOMAbstractView *>(thisObj.imp())->toAbstractView();
  switch (id) {
    case DOMAbstractView::GetComputedStyle: {
        DOM::Node arg0 = toNode(args[0]);
        if (arg0.nodeType() != DOM::Node::ELEMENT_NODE)
          return Undefined(); // throw exception?
        else
          return getDOMCSSStyleDeclaration(exec,abstractView.getComputedStyle(static_cast<DOM::Element>(arg0),
                                                                              args[1].toString(exec).string()));
      }
  }
  return Undefined();
}

Value KJS::getDOMAbstractView(ExecState *exec, DOM::AbstractView av)
{
  DOMAbstractView *ret;
  if (av.isNull())
    return Null();
  else if ((ret = abstractViews[av.handle()]))
    return ret;
  else {
    ret = new DOMAbstractView(exec,av);
    abstractViews.insert(av.handle(),ret);
    return ret;
  }
}

DOM::AbstractView KJS::toAbstractView (const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull() || !obj.inherits(&DOMAbstractView::info))
    return DOM::AbstractView ();

  const DOMAbstractView  *dobj = static_cast<const DOMAbstractView *>(obj.imp());
  return dobj->toAbstractView ();
}
