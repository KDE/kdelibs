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

#include "java/kjavaappletwidget.h"
#include "html_objectimpl.h"
#include "dom_nodeimpl.h"
#include "dom_string.h"
#include "htmlhashes.h"
#include "htmlhashes.h"
#include "khtml.h"
#include <qstring.h>

#include <stdio.h>

using namespace DOM;

// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentImpl *doc)
  : HTMLPositionedElementImpl(doc)
{
    applet = 0;
    codeBase = 0;
    code = 0;
    name = 0;
    archive = 0;
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
    if(codeBase) codeBase->deref();
    if(code) code->deref();
    if(name) name->deref();
    if(archive) archive->deref();    
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
    	codeBase = attr->val();
	codeBase->ref();
	break;	
    case ATTR_ARCHIVE:
        archive = attr->val();
	archive->ref();
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
	width = attr->val()->toInt();
	break;
    case ATTR_HEIGHT:
	ascent = attr->val()->toInt();
	break;
    default:
	HTMLPositionedElementImpl::parseAttribute(attr);
    }
}

void HTMLAppletElementImpl::layout( bool /*deep*/ )
{

    if(!applet || layouted()) return;

    applet->create();

    printf("initializing java widget II\n");
    NodeImpl *child = firstChild();
    while(child)
    {
	if(child->id() == ID_PARAM)
	{
	    HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>(child);
	    QString n = QConstString(p->name->s, p->name->l).string();
	    QString v = QConstString(p->value->s, p->value->l).string();
	    applet->setParameter(n, v);
	}
	child = child->nextSibling();
    }

    applet->show();
    setLayouted();
}

void HTMLAppletElementImpl::attach(KHTMLWidget *_view)
{
    descent = 0;
    if(!code) return;

    printf("initializing java widget I\n");
    view = _view;
    if(!view->javaEnabled()) return;
    applet = new KJavaAppletWidget(view->viewport());

    // Set applet parameters
    applet->resize(width, getHeight());
    applet->setBaseURL(view->url());
    if(codeBase)
      applet->setCodeBase(QString(codeBase->s, codeBase->l));
    if(code)
      applet->setAppletClass(QString(code->s, code->l));
    if(archive)
      applet->setJARFile(QString(archive->s, archive->l));
    
    QString tmp;
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

void HTMLAppletElementImpl::setXPos( int xPos )
{
    printf("setXPos I\n");
    x = xPos;
    int absX, absY;
    getAbsolutePosition(absX, absY);
    absY -= ascent;
    if(view && applet)
    {
	view->addChild(applet, absX, absY);
    }
}

void HTMLAppletElementImpl::setYPos( int yPos )
{
    printf("setYPos\n");
    y = yPos;
    int absX, absY;
    getAbsolutePosition(absX, absY);
    absY -= ascent;
    if(view && applet)
    {
             view->addChild(applet, absX, absY);
    }
}

bool HTMLAppletElementImpl::childrenRendered()
{
    if(!view) return false;
    return !view->javaEnabled();
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
