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

#include "kjs_css.h"

#include <xml/dom_nodeimpl.h>
#include <dom/html_element.h>
#include <rendering/render_style.h>
#include <kjs/types.h>
#include <css/cssproperties.h>
#include <qregexp.h>
#include <dom/dom_string.h>
#include <kjs_binding.h>

using namespace KJS;
#include <kdebug.h>

static QString jsNameToProp( const UString &p )
{
    QString prop = p.qstring();
    int i = prop.length();
    while( --i ) {
	char c = prop[i].latin1();
	if ( c < 'A' || c > 'Z' )
	    continue;
	prop.insert( i, '-' );
    }

    return prop.lower();
}

KJSO Style::get(const UString &p) const
{
    DOM::HTMLElement el = node;
    KJSO result;
    if ( el.isNull() )
      result = Undefined();
    else {
	DOM::DOMString val = el.getCSSProperty( jsNameToProp( p ) );
	result = String( val );
    }
    return result;
}

void Style::put(const UString &p, const KJSO& v)
{
    DOM::HTMLElement el = node;
    if ( el.isNull() )
      return;

    QString prop = jsNameToProp( p );
    QString propvalue = v.toString().value().string().string();

    if(prop.left(6) == "pixel-")
    {
        prop = prop.mid(6); // cut it away
        propvalue += "px";
    }
    el.removeCSSProperty( prop );
    el.addCSSProperty( prop, DOM::DOMString(propvalue) );
}
