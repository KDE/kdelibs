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
#include "html_objectimpl.h"
#include "dom_nodeimpl.h"
#include "dom_string.h"
#include "htmlhashes.h"
#include "htmlhashes.h"
#include "khtmlview.h"
#include <qstring.h>
#include <qmap.h>
#include <kdebug.h>

#include "xml/dom_docimpl.h"
#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include "rendering/render_applet.h"
#include "rendering/render_frames.h"

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------

HTMLAppletElementImpl::HTMLAppletElementImpl(DocumentImpl *doc)
  : HTMLElementImpl(doc)
{
    codeBase = 0;
    code = 0;
    name = 0;
    archive = 0;
    width = height = 0;
}

HTMLAppletElementImpl::~HTMLAppletElementImpl()
{
    if(codeBase) codeBase->deref();
    if(code) code->deref();
    if(name) name->deref();
    if(archive) archive->deref();
}

const DOMString HTMLAppletElementImpl::nodeName() const
{
    return "APPLET";
}

ushort HTMLAppletElementImpl::id() const
{
    return ID_APPLET;
}

void HTMLAppletElementImpl::parseAttribute(AttrImpl *attr)
{
    switch( attr->attrId )
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
	height = attr->val()->toInt();
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

void HTMLAppletElementImpl::attach(KHTMLView *_view)
{
  if(!code)
      return;

  khtml::RenderObject *r = _parent->renderer();
  if(!r)
      return;

  m_style = document->styleSelector()->styleForElement(this);
  view = _view;
  RenderWidget *f;

  if( view->part()->javaEnabled() )
  {
      QMap<QString, QString> args;

      args.insert( "code", QString(code->s, code->l));
      if(codeBase)
          args.insert( "codeBase", QString(codeBase->s, codeBase->l) );
      if(name)
          args.insert( "name", QString(name->s, name->l) );
      if(archive)
          args.insert( "archive", QString(archive->s, archive->l) );

      args.insert( "width", QString::number(width) );
      args.insert( "height", QString::number(height) );
      args.insert( "baseURL", view->part()->url().url() );

      f = new RenderApplet(view, args, this);
  }
  else
      f = new RenderEmptyApplet(view, QSize(width, height));

  if(f)
  {
      m_render = f;
      m_render->setStyle(m_style);
      m_render->ref();
      r->addChild(m_render, _next ? _next->renderer() : 0);
  }
  NodeBaseImpl::attach(_view);
}

void HTMLAppletElementImpl::detach()
{
    view = 0;
    HTMLElementImpl::detach();
}

// -------------------------------------------------------------------------

HTMLEmbedElementImpl::HTMLEmbedElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLEmbedElementImpl::~HTMLEmbedElementImpl()
{
}

const DOMString HTMLEmbedElementImpl::nodeName() const
{
    return "EMBED";
}

ushort HTMLEmbedElementImpl::id() const
{
    return ID_EMBED;
}

void HTMLEmbedElementImpl::parseAttribute(AttrImpl *attr)
{
  DOM::DOMStringImpl *stringImpl = attr->val();
  QString val = QConstString( stringImpl->s, stringImpl->l ).string();
  
  // export parameter
  param.append( attr->name().string() + "=\"" + val + "\"" );
  
  switch ( attr->attrId )
  {
     case ATTR_TYPE:
	serviceType = val;
	break;
     case ATTR_SRC:
	url = val;
	break;
     case ATTR_WIDTH:
	addCSSLength( CSS_PROP_WIDTH, attr->value(), false );
	break;
     case ATTR_HEIGHT:
	addCSSLength( CSS_PROP_HEIGHT, attr->value(), false );
	break;
     case ATTR_BORDER:
	addCSSLength(CSS_PROP_BORDER_WIDTH, attr->value(), false);
	break;
     case ATTR_VSPACE:
	addCSSLength(CSS_PROP_MARGIN_TOP, attr->value(), false);
	addCSSLength(CSS_PROP_MARGIN_BOTTOM, attr->value(), false);
	break;
     case ATTR_HSPACE:
	addCSSLength(CSS_PROP_MARGIN_LEFT, attr->value(), false);
	addCSSLength(CSS_PROP_MARGIN_RIGHT, attr->value(), false);
	break;
     case ATTR_ALIGN:
	// vertical alignment with respect to the current baseline of the text
	// right or left means floating images
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	{
	   addCSSProperty(CSS_PROP_FLOAT, attr->value(), false);
	   addCSSProperty(CSS_PROP_VERTICAL_ALIGN, "top", false);
	}
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	{
	   addCSSProperty(CSS_PROP_FLOAT, attr->value(), false);
	   addCSSProperty(CSS_PROP_VERTICAL_ALIGN, "top", false);
	}
	else
	   addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
	break;
     case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
	break;
     case ATTR_PLUGINPAGE:
	pluginPage = val;
	break;
     case ATTR_HIDDEN:
	if (val.lower()=="yes" || val.lower()=="true")
	   hidden = true;
	else
	   hidden = false;
	break;
     default:
	HTMLElementImpl::parseAttribute( attr );
  }
}

void HTMLEmbedElementImpl::attach(KHTMLView *w)
{
   khtml::RenderObject *r = _parent->renderer();
   if ( !r )
      return;

   m_style = document->styleSelector()->styleForElement( this );
   if ( _parent->id()!=ID_OBJECT )
   {           
      RenderPartObject *p = new RenderPartObject( w, this );
      m_render = p;
      m_render->setStyle(m_style);
      m_render->ref();
      r->addChild( m_render, _next ? _next->renderer() : 0 );
   } else
   {
      r->setStyle(m_style);
   }

  NodeBaseImpl::attach( w );
}

void HTMLEmbedElementImpl::detach()
{
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

void HTMLObjectElementImpl::parseAttribute(AttrImpl *attr)
{
  DOM::DOMStringImpl *stringImpl = attr->val();
  QString val = QConstString( stringImpl->s, stringImpl->l ).string();
  switch ( attr->attrId )
  {
    case ATTR_TYPE:
      serviceType = val;
      break;
    case ATTR_DATA:
      url = val;
      break;
    case ATTR_WIDTH:
      addCSSLength( CSS_PROP_WIDTH, attr->value(), false );
      break;
    case ATTR_HEIGHT:
      addCSSLength( CSS_PROP_HEIGHT, attr->value(), false );
      break;
    case ATTR_CLASSID:
      classId = val;
    default:
      HTMLElementImpl::parseAttribute( attr );
  }
}

void HTMLObjectElementImpl::attach(KHTMLView *w)
{
  khtml::RenderObject *r = _parent->renderer();
  if ( !r )
    return;

  m_style = document->styleSelector()->styleForElement( this );

  RenderPartObject *p = new RenderPartObject( w, this );
  m_render = p;
  m_render->setStyle(m_style);
  m_render->ref();
  r->addChild( m_render, _next ? _next->renderer() : 0 );

  NodeBaseImpl::attach( w );
}

// -------------------------------------------------------------------------

HTMLParamElementImpl::HTMLParamElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    m_name = 0;
    m_value = 0;
}

HTMLParamElementImpl::~HTMLParamElementImpl()
{
    if(m_name) m_name->deref();
    if(m_value) m_value->deref();
}

const DOMString HTMLParamElementImpl::nodeName() const
{
    return "PARAM";
}

ushort HTMLParamElementImpl::id() const
{
    return ID_PARAM;
}

void HTMLParamElementImpl::parseAttribute(AttrImpl *attr)
{
    switch( attr->attrId )
    {
    case ATTR_NAME:
        m_name = attr->val();
	m_name->ref();
	break;
    case ATTR_VALUE:
	m_value = attr->val();
	m_value->ref();
	break;
    }
}
