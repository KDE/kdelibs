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
#include <qptrdict.h>
#include <dom2_events.h>

using namespace KJS;

QPtrDict<DOMAbstractView> abstractViews;

// -------------------------------------------------------------------------

const TypeInfo DOMAbstractView::info = { "AbstractView", HostType, 0, 0, 0 };


DOMAbstractView::~DOMAbstractView()
{
  abstractViews.remove(abstractView.handle());
}

KJSO DOMAbstractView::tryGet(const UString &p) const
{
  if (p == "document")
    return getDOMNode(abstractView.document());
  else
    return DOMObject::tryGet(p);
}


KJSO KJS::getDOMAbstractView(DOM::AbstractView av)
{
  DOMAbstractView *ret;
  if (av.isNull())
    return Null();
  else if ((ret = abstractViews[av.handle()]))
    return ret;
  else {
    ret = new DOMAbstractView(av);
    abstractViews.insert(av.handle(),ret);
    return ret;
  }
}


DOM::AbstractView KJS::toAbstractView (const KJSO& obj)
{
  if (!obj.derivedFrom("AbstractView"))
    return DOM::AbstractView ();

  const DOMAbstractView  *dobj = static_cast<const DOMAbstractView *>(obj.imp());
  return dobj->toAbstractView ();
}
