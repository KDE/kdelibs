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
#include "html_imageimpl.h"

#include <qpixmap.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qstack.h>

#include "htmlhashes.h"

#include "html_image.h"
#include "dom_string.h"
#include "dom_nodeimpl.h"
#include "html_documentimpl.h"

#include "khtmlview.h"

#include <kdebug.h>

#include <iostream>

#include "rendering/render_image.h"
#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include <iostream.h>
#include <qstring.h>
#include <qlist.h>
#include <qpoint.h>
#include <qregion.h>

#include <stdlib.h>

using namespace DOM;
using namespace khtml;

// -------------------------------------------------------------------------



// -------------------------------------------------------------------------

HTMLImageElementImpl::HTMLImageElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    ismap = false;
}

HTMLImageElementImpl::~HTMLImageElementImpl()
{
}


// DOM related

const DOMString HTMLImageElementImpl::nodeName() const
{
    return "IMG";
}

ushort HTMLImageElementImpl::id() const
{
    return ID_IMG;
}

bool HTMLImageElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
                                       int _tx, int _ty, DOMString &url,
                                       NodeImpl *&innerNode, long &offset)
{
    //kdDebug( 6030 ) << "_x=" << _x << " _tx=" << _tx << " _y=" << _y << ", _ty=" << _ty << endl;
    if (usemap.length()>0)
    {
	if(m_render->parent()->isAnonymousBox())
	{
	    //kdDebug( 6030 ) << "parent is anonymous!" << endl;
	    // we need to add the offset of the anonymous box
	    _tx += m_render->parent()->xPos();
	    _ty += m_render->parent()->yPos();
	}

        //cout << "usemap: " << usemap.string() << endl;
        HTMLMapElementImpl* map;
    	if ( (map = HTMLMapElementImpl::getMap(usemap))!=0)
	{
	    //kdDebug( 6030 ) << "have map" << endl;
            return map->mapMouseEvent(_x-renderer()->xPos()-_tx,
                                      _y-renderer()->yPos()-_ty,
                                      renderer()->width(), renderer()->height(), button, type, url);
	}
    }
    return HTMLElementImpl::mouseEvent(_x, _y, button, type, _tx, _ty, url, innerNode, offset);
}

void HTMLImageElementImpl::parseAttribute(AttrImpl *attr)
{
    switch (attr->attrId)
    {
    case ATTR_SRC:
	imageURL = attr->value();
	break;
    case ATTR_WIDTH:
	addCSSLength(CSS_PROP_WIDTH, attr->value(), false);
	break;
    case ATTR_HEIGHT:
	addCSSLength(CSS_PROP_HEIGHT, attr->value(), false);
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
    case ATTR_USEMAP:
	if ( attr->value()[0] == '#' )
	{
	    usemap = attr->value();
	}
	else
	{
	    //KURL u( HTMLWidget->getBaseURL(), attr->value() );
	    //usemap = u.url();
	}
    case ATTR_ISMAP:
	ismap = true;
	break;
    case ATTR_ALT:
	alt = attr->value();
	break;	
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLImageElementImpl::attach(KHTMLView *w)
{
    //kdDebug( 6030 ) << "HTMLImageImpl::attach" << endl;
    m_style = document->styleSelector()->styleForElement(this);
    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	RenderImage *renderImage = new RenderImage();
	renderImage->setStyle(m_style);
	renderImage->setAlt(alt);
	renderImage->setImageUrl(imageURL, static_cast<HTMLDocumentImpl *>(document)->baseURL(),
	                         static_cast<HTMLDocumentImpl *>(document)->docLoader());
	m_render = renderImage;
	m_render->ref();
	if(m_render) r->addChild(m_render, _next ? _next->renderer() : 0);
	
    }
    NodeBaseImpl::attach( w );
}

void HTMLImageElementImpl::applyChanges(bool top, bool force)
{
    kdDebug(0) << "Image::applyChanges(" << top << ", " << force <<")" << endl;
    if((force || changed()) && m_render) {
	static_cast<RenderImage *>(m_render)
	    ->setImageUrl(imageURL, static_cast<HTMLDocumentImpl *>(document)->baseURL(),
	                  static_cast<HTMLDocumentImpl *>(document)->docLoader());
    }
    HTMLElementImpl::applyChanges(top,force);
    setChanged(false);
}

// -------------------------------------------------------------------------

QMap<QString,HTMLMapElementImpl*> *HTMLMapElementImpl::mapMap = 0;

HTMLMapElementImpl::HTMLMapElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    if(!mapMap) mapMap = new QMap<QString, HTMLMapElementImpl *>();
}

HTMLMapElementImpl::~HTMLMapElementImpl()
{
    HTMLMapElementImpl::mapMap->remove(name);
}

const DOMString HTMLMapElementImpl::nodeName() const
{
    return "MAP";
}

ushort HTMLMapElementImpl::id() const
{
    return ID_MAP;
}

bool
HTMLMapElementImpl::mapMouseEvent(int x_, int y_, int width_, int height_,
    	int button_, MouseEventType type_, DOMString& url_)
{
    //cout << "map:mapMouseEvent " << endl;
    //cout << x_ << " " << y_ <<" "<< width_ <<" "<< height_ << endl;
    bool inside = false;

    QStack<NodeImpl> nodeStack;

    NodeImpl *current = firstChild();
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	    continue;
	}
	// ### What is the ID_P supposed to do here? This can
	// only lead to memory corruption or a segfault, since P does
	// not inherit form Area!
	if(current->id()==ID_AREA)// || current->id()==ID_P)
	{
	    //cout << "area found " << endl;
	    HTMLAreaElementImpl* area=static_cast<HTMLAreaElementImpl*>(current);
	    if (area->mapMouseEvent(x_,y_,width_,height_,button_,type_,url_))
	    {
	    	inside = true;
		if (!area->isDefault())
	    	    break;
	    }
	}
	NodeImpl *child = current->firstChild();
	if(child)
	{	
	    nodeStack.push(current);
	    current = child;
	}
	else
	{
	    current = current->nextSibling();
	}
    }

    return inside;
}

HTMLMapElementImpl*
HTMLMapElementImpl::getMap(const DOMString& _url)
{
    QString s;
    if(*_url.unicode() == '#')
	s = QString(_url.unicode()+1, _url.length()-1);
    else
	s = _url.string();
    if (mapMap && mapMap->contains(s))
	return (*mapMap)[s];
    else
	return 0;
}



void HTMLMapElementImpl::parseAttribute(AttrImpl *attr)
{
    switch (attr->attrId)
    {
    case ATTR_NAME:
    {
	DOMString s = attr->value();
	if(*s.unicode() == '#')
	    name = QString(s.unicode()+1, s.length()-1);
	else
	    name = s.string();
	(*mapMap)[name] = this;
	break;
    }
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLAreaElementImpl::HTMLAreaElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    coords=0L;
    href = 0;
    target = 0;
    shape = Unknown;
    has_tabindex=false;

     DOMString indexstring=((HTMLElementImpl*)this)->getAttribute(ATTR_TABINDEX);
     if (indexstring.length()) {
         has_tabindex=true;
         tabindex=atoi(indexstring.string().latin1());
     } else {
         has_tabindex=false;
         tabindex=0;
     }
}

HTMLAreaElementImpl::~HTMLAreaElementImpl()
{
    if( href ) href->deref();
    if( target ) target->deref();
    if (coords)
    	delete coords;
}

const DOMString HTMLAreaElementImpl::nodeName() const
{
    return "AREA";
}

ushort HTMLAreaElementImpl::id() const
{
    return ID_AREA;
}

long HTMLAreaElementImpl::tabIndex() const
{
  if (has_tabindex)
    return tabindex;
  else
    return -1;
}

void HTMLAreaElementImpl::setTabIndex( long _tabindex )
{
  has_tabindex=true;
  tabindex=_tabindex;
}

void HTMLAreaElementImpl::parseAttribute(AttrImpl *attr)
{
    DOMString indexstring;
    switch (attr->attrId)
    {
    case ATTR_SHAPE:
	if ( strcasecmp( attr->value(), "default" ) == 0 )
	    shape = Default;
	else if ( strcasecmp( attr->value(), "circle" ) == 0 )
	    shape = Circle;
	else if ( strcasecmp( attr->value(), "poly" ) == 0 )
	    shape = Poly;
	else if ( strcasecmp( attr->value(), "rect" ) == 0 )
	    shape = Rect;	
	break;
    case ATTR_COORDS:	
    	coords = attr->val()->toLengthList(); 		
	break;	
    case ATTR_NOHREF:
	break;	
    case ATTR_HREF:
    	href = attr->val();
	href->ref();
	break;	
    case ATTR_TARGET:
    	target = attr->val();
	target->ref();
	break;	
    case ATTR_ALT:
	break;
    case ATTR_TABINDEX:
        indexstring=((HTMLElementImpl*)this)->getAttribute(ATTR_TABINDEX);
        if (indexstring.length()) {
	  has_tabindex=true;
	  tabindex=atoi(indexstring.string().latin1());
	} else {
	  has_tabindex=false;
	  tabindex=0;
	}
	has_tabindex=true;
	break;	
    case ATTR_ACCESSKEY:
	break;	
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

bool
HTMLAreaElementImpl::mapMouseEvent(int x_, int y_, int width_, int height_,
				   int /*button_*/, MouseEventType /*type*/, DOMString& url_)
{
    //cout << "area:mapMouseEvent " << endl;
    bool inside = false;
    if (width_ != lastw || height_ != lasth)
    {
    	region=getRegion(width_, height_);
	lastw=width_; lasth=height_;
    }
    if (region.contains(QPoint(x_,y_)))
    {
        //cout << "region hit, url " << QConstString(href->s, href->l).string() << endl;
    	inside = true;
	if(target && href)
	{
	    DOMString s = DOMString("target://") + DOMString(target) + DOMString("/#") + DOMString(href);
	    url_ = s;
	}
	else
	    url_ = href;
    }
    return inside;
}	



QRegion HTMLAreaElementImpl::getRegion(int width_, int height_)
{
    QRegion region;
    if (!coords)
    	return region;

    // added broken HTML support (Dirk): some pages omit the SHAPE
    // attribute, so we try to guess by looking at the coords count
    // what the HTML author tried to tell us.

    if (shape==Poly || shape==Unknown && coords->count() > 4)
    {
        //cout << " poly " << endl;	
	bool xcoord=true;
	int xx = 0, yy = 0; // shut up egcs...
	int i=0;

	QListIterator<Length> it(*coords);
	QPointArray points(it.count()/2);
	for ( ; it.current(); ++it ) {
	    Length* len = it.current();
	    if (xcoord)
	    {
		xx = len->minWidth(width_);
		xcoord = false;
	    } else {
		yy = len->minWidth(height_);
		xcoord = true;
		points.setPoint(i,xx,yy);
		i++;
	    }	    	
	}
	region = QRegion(points);	
    }
    else if (shape==Circle && coords->count()>=3 || shape==Unknown && coords->count() == 3)
    {
    	//cout << " circle " << endl;
    	int cx = coords->at(0)->minWidth(width_);
	int cy = coords->at(1)->minWidth(height_);
	int r1 = coords->at(2)->minWidth(width_);
	int r2 = coords->at(2)->minWidth(height_);
	int r  = QMIN(r1, r2);

    	region = QRegion(cx-r, cy-r, 2*r, 2*r,QRegion::Ellipse);
    }
    else if (shape==Rect && coords->count()>=4 || shape==Unknown && coords->count() == 4)
    {
        //cout << " rect " << endl;
    	int x0 = coords->at(0)->minWidth(width_);
	int y0 = coords->at(1)->minWidth(height_);
	int x1 = coords->at(2)->minWidth(width_);
	int y1 = coords->at(3)->minWidth(height_);
    	region = QRegion(x0,y0,x1-x0,y1-y0);
    }
    else /*if (shape==Default || shape == Unknown)*/ {
        //cout << "default/unknown" << endl;
    	region = QRegion(0,0,width_,height_);
    }

    return region;
}

