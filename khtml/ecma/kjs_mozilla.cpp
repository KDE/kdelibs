// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 George Staikos (staikos@kde.org)
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <klocale.h>
#include <kdebug.h>

#include "kjs_mozilla.h"
#include "kjs/lookup.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "kjs_mozilla.lut.h"

using namespace KJS;

namespace KJS {

const ClassInfo MozillaSidebarExtension::info = { "sidebar", 0, &MozillaSidebarExtensionTable, 0 };
/*
@begin MozillaSidebarExtensionTable 1
  addPanel	MozillaSidebarExtension::addPanel	DontDelete|Function 0
@end
*/
}
IMPLEMENT_PROTOFUNC_DOM(MozillaSidebarExtensionFunc)

MozillaSidebarExtension::MozillaSidebarExtension(ExecState *exec, KHTMLPart *p)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()), m_part(p) { }

Value MozillaSidebarExtension::get(ExecState *exec, const Identifier &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MozillaSidebarExtension::get " << propertyName.ascii() << endl;
#endif
  return lookupGet<MozillaSidebarExtensionFunc,MozillaSidebarExtension,ObjectImp>(exec,propertyName,&MozillaSidebarExtensionTable,this);
}

Value MozillaSidebarExtension::getValueProperty(ExecState *exec, int token) const
{
  Q_UNUSED(exec);
  switch (token) {
  default:
    kdWarning() << "Unhandled token in DOMEvent::getValueProperty : " << token << endl;
    return Value();
  }
}

Value MozillaSidebarExtensionFunc::tryCall(ExecState *exec, Object &thisObj, const List &)
{
  Q_UNUSED(exec);
  KJS_CHECK_THIS( KJS::MozillaSidebarExtension, thisObj );
  // addPanel()
  return Boolean(true);
} 


