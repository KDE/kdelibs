/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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

#include "khtmlattrs.h"
#include "khtmlio.h"

#include "html_image.h"
#include "dom_string.h"
#include "html_documentimpl.h"

#include <stdio.h>

#include <iostream>

using namespace DOM;

#define UNDEFINED -1

// -------------------------------------------------------------------------



// -------------------------------------------------------------------------

HTMLImageElementImpl::HTMLImageElementImpl(DocumentImpl *doc) 
    : HTMLPositionedElementImpl(doc), HTMLImageRequester()
{
    ismap = false;
    valign = Bottom;

    pixmap = 0;
    bComplete = true;
    border = 0;
    imgHeight = 0;
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

bool HTMLImageElementImpl::isMap() const
{
    return ismap;
}

void HTMLImageElementImpl::setIsMap( bool b)
{
    ismap = b;
    // ### set Attribute too...
}

bool HTMLImageElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url)
{
    _x-=_tx;
    _y-=_ty;
    if (ismap)
    {
    	cout << "ismap: " << usemap.string() << endl;
        HTMLMapElementImpl* map;
    	if ( (map = HTMLMapElementImpl::getMap(usemap)) )
    	    return map->mapMouseEvent(_x-getXPos(), _y-getYPos()+ascent,
	    	getWidth(), getHeight(), button, type, url);
    }
    return false;
}

// other stuff...
void HTMLImageElementImpl::attach(KHTMLWidget *)
{
    printf("Image: Requesting URL=%s\n", imageURL.string().ascii() );
    imageURL = document->requestImage(this, imageURL);
}

void HTMLImageElementImpl::detach()
{
    KHTMLCache::free(imageURL, this);
}

void HTMLImageElementImpl::parseAttribute(Attribute *attr)
{
    switch (attr->id)
    {
    case ATTR_SRC:
	imageURL = attr->value();
	break;
    case ATTR_WIDTH:
	predefinedWidth = attr->val()->toLength();
	break;
    case ATTR_HEIGHT:
	predefinedHeight = attr->val()->toLength();
	break;
    case ATTR_BORDER:
	border = attr->val()->toInt();
	break;
    case ATTR_VSPACE:
	vspace = attr->val()->toLength();
	break;
    case ATTR_HSPACE:
	hspace = attr->val()->toLength();
	break;
    case ATTR_ALIGN:
	// vertical alignment with respect to the current baseline of the text
	// right or left means floating images
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	{
	    halign = Left;
	    valign = Top;
	}
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	{
	    halign = Right;
	    valign = Top;
	}
	else if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = Bottom;
	break;
    case ATTR_USEMAP:
	if ( attr->value()[0] == '#' )
	{
	    usemap = attr->value();
	    ismap = true;
	}
	else
	{
	    //KURL u( HTMLWidget->getBaseURL(), attr->value() );
	    //usemap = u.url();
	}
    case ATTR_ISMAP:
	ismap = true;
	break;
    default:
	HTMLPositionedElementImpl::parseAttribute(attr);
    }
}

// layout related stuff

void  HTMLImageElementImpl::setPixmap( QPixmap *p )
{
    pixmap = p;

    if (!minMaxKnown())
    {
        // Image dimensions have been changed, recalculate layout
	printf("Image: recalculating layout\n");
	calcMinMaxWidth();
	if(_parent) _parent->updateSize();
	layout();	
    }
    static_cast<HTMLDocumentImpl *>(document)->print(this);
}

void  HTMLImageElementImpl::pixmapChanged( QPixmap *p )
{
    if( p )
	setPixmap( p );
}

void HTMLImageElementImpl::print(QPainter *p, int _x, int _y, 
				 int _w, int _h, int _tx, int _ty)
{
   _tx += x;
   _ty += y;

    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void HTMLImageElementImpl::printObject(QPainter *p, int, int _y, 
				       int, int _h, int _tx, int _ty)
{
    //printf("%s(Image)::printObject()\n", nodeName().string().ascii());

    if((_ty - ascent > _y + _h) || (_ty + descent < _y)) return;

    QRect rect( 0, 0, width - border*2, getHeight() - border*2 );

#if 0
    if ( pixmap )
    {
	if ( predefinedWidth )
	    rect.setWidth( pixmap->width() );

	if ( predefinedHeight )
	    rect.setHeight( pixmap->height() );
    }
#endif

    if ( pixmap == 0 || pixmap->isNull() )
    {
	if ( predefinedWidth.type == Undefined &&
	     predefinedHeight.type == Undefined )
	{
	    QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, Qt::darkGray, Qt::gray,
		    Qt::black, Qt::white );
	    qDrawShadePanel( p, _tx, _ty - getHeight(), width, getHeight(),
		    colorGrp, true, 1 );
	}
    }
    else
    {
#if 0 // ###
	if ( isSelected() )
	{
	    QPainter painter( &pm );
//	    p.setRasterOp( NotEraseROP );
//	    p.fillRect( 0, 0, pm.width(), pm.height(), blue );
	    QBrush b( kapp->palette().normal().highlight(), Qt::Dense4Pattern );
	    painter.fillRect( 0, 0, pm.width(), pm.height(), b );
	}
#endif

	if ( (width - border*2 != pixmap->width() ||
	    getHeight() - border != pixmap->height() ) &&
	    pixmap->width() != 0 && pixmap->height() != 0 )
	{
	    printf("have to scale: width:%d<-->%d height %d<-->%d\n",
		   width - border*2, pixmap->width(),
		   getHeight() - border, pixmap->height() );
	    QWMatrix matrix;
	    matrix.scale( (float)(width-border*2)/pixmap->width(),
		    (float)(getHeight()-border)/pixmap->height() );
	    QPixmap tp = pixmap->xForm( matrix );
	    p->drawPixmap( QPoint( _tx + border,
				   _ty - ascent + border ), tp, rect );
	}
	else
	    p->drawPixmap( QPoint( _tx + border,
				   _ty - ascent + border), *pixmap, rect );
    }

    if ( border )
    {
	QPen pen( QColor("000000"));
	p->setPen( pen );
	QBrush brush;
	p->setBrush( brush );	// null brush
	for ( int i = 1; i <= border; i++ )
	    p->drawRect( _tx+border-i, _ty - ascent + border - i,
		(width - border*2) + i*2, getHeight() - border + i*2 );
    }
}

void HTMLImageElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(Image)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif
    setMinMaxKnown();
    
    switch(predefinedWidth.type)
    {
    case Fixed:
	width = predefinedWidth.value;
	minWidth = width;
	break;
    case Percent:
	width = predefinedWidth.value*availableWidth/100;
	minWidth = width;
	break;
    default:
	// we still don't know the width...
	if(!pixmap)
	{
	    width = 32;
	    setMinMaxKnown(false);
	}
	else
	{
	    width = pixmap->width();
	    // if it doesn't fit... make it fit
	    // NO! Images don't scale unless told to. Ever.  -AKo 
	    //if(availableWidth < width) width = availableWidth;
	    minWidth = width;
	    printf("IMG Width changed, width=%d\n",width);
	}
    }
    maxWidth = minWidth;


//    if(availableWidth && minWidth > availableWidth)
//	if(_parent) _parent->updateSize();
}

void HTMLImageElementImpl::layout(bool)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Image)::layout(???) width=%d, layouted=%d\n", nodeName().string().ascii(), width, layouted());
#endif

    calcMinMaxWidth(); // ### just to be sure here...

    ascent = descent = 0;

    switch(predefinedHeight.type)
    {
    case Fixed:
	imgHeight = predefinedHeight.value;
	break;
    case Percent:
	// ### is this correct????
	imgHeight = predefinedHeight.value*width/100;
	break;
    default:
	// we still don't know the height...
	if(!pixmap)
	    imgHeight = 32;
	else
	{
	    if(width == pixmap->width())
		imgHeight = pixmap->height();
	    else 
		imgHeight = pixmap->height()*width/pixmap->width();
	}
    }

    switch(valign)
    {
    case Bottom:
	ascent = imgHeight;
	break;
    case Top:
	descent = imgHeight;
	break;
    case VCenter:
	ascent = imgHeight/2;
	descent = imgHeight/2;
	break;
    }

    if(border)
    {
	ascent += border;
	descent += border;
    }
    

    //printf("HTMLIMage:: layout(): w/a/d = %d/%d/%d", width, ascent, descent);

    setLayouted();
}
// -------------------------------------------------------------------------

HTMLMapElementImpl::HTMLMapElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLMapElementImpl::~HTMLMapElementImpl()
{
    HTMLMapElementImpl::mapMap.remove(name);
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
    bool inside = false;
    
    QStack<NodeImpl> nodeStack;

    NodeImpl *current = firstChild();
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    continue;
	}
	if(current->id()==ID_AREA || current->id()==ID_P)
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

QMap<QString,HTMLMapElementImpl*> HTMLMapElementImpl::mapMap;

HTMLMapElementImpl* 
HTMLMapElementImpl::getMap(const DOMString& url_) 
{
    if (mapMap.contains(url_.string()))
	    return mapMap[url_.string()];
	else
	    return 0;
}



void HTMLMapElementImpl::parseAttribute(Attribute *attr)
{
    switch (attr->id)
    {
    case ATTR_NAME:
    	name = "#"+attr->value().string();
	HTMLMapElementImpl::mapMap[name] = this;
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}




/*HTMLMapStore* HTMLMapStore::instance_ = 0L;

HTMLMapStore* HTMLMapStore::instance()
{
    if (!instance_)
    	instance_ = new HTMLMapStore();	
    return instance_;	    
}*/

// -------------------------------------------------------------------------

HTMLAreaElementImpl::HTMLAreaElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    coords=0L;
    href = 0;
}

HTMLAreaElementImpl::~HTMLAreaElementImpl()
{
    if( href ) href->deref();
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

bool HTMLAreaElementImpl::noHref() const
{
    // ###
    return false;
}

void HTMLAreaElementImpl::setNoHref( const bool & )
{
    // ###
}

long HTMLAreaElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLAreaElementImpl::setTabIndex( const long & )
{
    // ###
}

void HTMLAreaElementImpl::parseAttribute(Attribute *attr)
{
    switch (attr->id)
    {
    case ATTR_SHAPE:
	if ( strcasecmp( attr->value(), "default" ) == 0 )
	    shape = Default;
	else if ( strcasecmp( attr->value(), "circle" ) == 0 )
	    shape = Circle;
	else if ( strcasecmp( attr->value(), "poly" ) == 0 )
	    shape = Poly;
	else 
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
    case ATTR_ALT:
	break;
    case ATTR_TABINDEX:
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
    if (!(width_==lastw && height_==lasth))
    {
    	region=getRegion(width_, height_);
	lastw=width_; lasth=height_;
    }
    if (region.contains(QPoint(x_,y_)))
    {
    	cout << "region hit, url " << QConstString(href->s, href->l).string() << endl;
    	inside = true;
    	url_ = href;
    }
    return inside;
}	



QRegion HTMLAreaElementImpl::getRegion(int width_, int height_)
{
    QRegion region;
    cout << "getting region" << endl;
    if (!coords)
    	return region;
    
    if (shape==Poly)
    {    
    	cout << " poly " << endl;	
	bool xcoord=true;
	int xx, yy;
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
    else if (shape==Rect && coords->count()>=4)
    {
    	cout << " rect " << endl;
    	int x0 = coords->at(0)->minWidth(width_);
	int y0 = coords->at(1)->minWidth(height_);
	int x1 = coords->at(2)->minWidth(width_);
	int y1 = coords->at(3)->minWidth(height_);
    	region = QRegion(x0,y0,x1-x0,y1-y0);
    }
    else if (shape==Circle && coords->count()>=3)
    {
    	cout << " circle " << endl;
    	int cx = coords->at(0)->minWidth(width_);
	int cy = coords->at(1)->minWidth(height_);
	int r1 = coords->at(2)->minWidth(width_);
	int r2 = coords->at(2)->minWidth(height_);
	int r;
	if (r1<r2) 
	    r = r1;
	else
	    r = r2;

    	region = QRegion(cx-r, cy-r, 2*r, 2*r,QRegion::Ellipse);
    } else if (shape==Default) {
    	region = QRegion(0,0,width_,height_);
    }
    
    return region;
}

