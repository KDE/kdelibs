/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include "khtml_part.h"
#include "java/kjavaappletwidget.h"
#include "html_objectimpl.h"
#include "dom_nodeimpl.h"
#include "dom_string.h"
#include "htmlhashes.h"
#include "htmlhashes.h"
#include "khtmlview.h"
#include <qstring.h>

#include "xml/dom_docimpl.h"
#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include <stdio.h>

using namespace DOM;

// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentImpl *doc)
  : HTMLElementImpl(doc)
{
    applet = 0;
    base = 0;
    code = 0;
    name = 0;
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
    if(base) base->deref();
    if(code) code->deref();
    if(name) name->deref();
    if(applet) delete applet;
}

const DOMString HTMLAppletElementImpl::nodeName() const
{
    return "APPLET";
}

ushort HTMLAppletElementImpl::id() const
{
    return ID_APPLET;
}

void HTMLAppletElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_CODEBASE:
    	base = attr->val();
	base->ref();
	break;	
    case ATTR_ARCHIVE:
	break;
    case ATTR_CODE:
	code = attr->val();
	code->ref();
	break;	
    case ATTR_OBJECT:
	break;
    case ATTR_ALT:
	break;
    case ATTR_NAME:
	name = attr->val();
	name->ref();
	break;
    case ATTR_WIDTH:
	//###	width = attr->val()->toInt();
	break;
    case ATTR_HEIGHT:
	//###ascent = attr->val()->toInt();
	break;
    case ATTR_ALIGN:
	// vertical alignment with respect to the current baseline of the text
	// right or left means floating images
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	{
	    addCSSProperty(CSS_PROP_FLOAT, attr->value(), false);
	    valign = khtml::Top;
	}
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	{
	    addCSSProperty(CSS_PROP_FLOAT, attr->value(), false);
	    valign = khtml::Top;
	}
	else if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = khtml::Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = khtml::VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = khtml::Bottom;
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLAppletElementImpl::attach(KHTMLWidget *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    if(!code) return;

    printf("initializing java widget I\n");
    view = _view;
    if(!view->part()->javaEnabled()) return;
    applet = new KJavaAppletWidget(view->viewport());

    //    printf("resizing applet to %d/%d\n", width, getHeight());
    //applet->resize(width, getHeight());
    //applet->show();
    applet->setBaseURL(view->part()->url().url()); // ### use KURL!
    QString tmp;
    if(base)
	tmp = QString(base->s, base->l) + '/';
    tmp += QString(code->s, code->l);
    printf("setting applet to %s\n", tmp.ascii());
    applet->setAppletClass(tmp);
    if(name)
	tmp = QConstString(name->s, name->l).string();
    else
	tmp = QConstString(code->s, code->l).string();
    applet->setAppletName(tmp);
}

void HTMLAppletElementImpl::detach()
{
    if(applet) delete applet;
    applet = 0;
    view = 0;
    NodeBaseImpl::detach();
}

// -------------------------------------------------------------------------

HTMLObjectElementImpl::HTMLObjectElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLObjectElementImpl::~HTMLObjectElementImpl()
{
}

const DOMString HTMLObjectElementImpl::nodeName() const
{
    return "OBJECT";
}

ushort HTMLObjectElementImpl::id() const
{
    return ID_OBJECT;
}

HTMLFormElementImpl *HTMLObjectElementImpl::form() const
{
  return 0;
}

long HTMLObjectElementImpl::tabIndex() const
{
  return 0;
}

void HTMLObjectElementImpl::setTabIndex( long  )
{
}

// -------------------------------------------------------------------------

HTMLParamElementImpl::HTMLParamElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    name = 0;
    value = 0;
}

HTMLParamElementImpl::~HTMLParamElementImpl()
{
    if(name) delete name;
    if(value) delete value;
}

const DOMString HTMLParamElementImpl::nodeName() const
{
    return "PARAM";
}

ushort HTMLParamElementImpl::id() const
{
    return ID_PARAM;
}

void HTMLParamElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_NAME:
	name = attr->val();
	name->ref();
	break;
    case ATTR_VALUE:
	value = attr->val();
	value->ref();
	break;
    }
}
