/**
 * This file is part of the KDE project.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
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

#include "render_frames.h"
#include "html_baseimpl.h"
#include "html_objectimpl.h"
//#include "html_inlineimpl.h"
#include "htmltags.h"
#include "khtmlview.h"
#include "khtml_part.h"

#include <kapp.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qstringlist.h>

#include <assert.h>
#include <kdebug.h>

using namespace khtml;

RenderFrameSet::RenderFrameSet( HTMLFrameSetElementImpl *frameSet, KHTMLView *view,
			        QList<khtml::Length> *rows, QList<khtml::Length> *cols )
: RenderBox()
{
  m_frameset = frameSet;

  m_rows = rows;
  m_cols = cols;
  m_rowHeight = 0;
  m_colWidth = 0;

  m_resizing = false;

  m_hSplit = -1;
  m_vSplit = -1;

  m_hSplitVar = 0;
  m_vSplitVar = 0;

  m_view = view;
}

RenderFrameSet::~RenderFrameSet()
{
  if ( m_rowHeight ) delete [] m_rowHeight;
  if ( m_colWidth ) delete [] m_colWidth;

  if ( m_hSplitVar ) delete m_hSplitVar;
  if ( m_vSplitVar ) delete m_vSplitVar;
}

void RenderFrameSet::layout( bool deep )
{

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << nodeName().string() << "(FrameSet)::layout(" << deep << ") width=" << width << ", layouted=" << layouted() << endl;
#endif

    if ( strcmp( m_parent->renderName(), "RenderFrameSet" ) != 0 )
    {
      m_width = m_view->clipper()->width();
      m_height = m_view->clipper()->height();
    }

    int remainingWidth = m_width - (m_frameset->totalCols()-1)*m_frameset->border();
    if(remainingWidth<0) remainingWidth=0;
    int remainingHeight = m_height - (m_frameset->totalRows()-1)*m_frameset->border();
    if(remainingHeight<0) remainingHeight=0;
    int widthAvailable = remainingWidth;
    int heightAvailable = remainingHeight;

    if(m_rowHeight) delete [] m_rowHeight;
    if(m_colWidth) delete [] m_colWidth;
    m_rowHeight = new int[m_frameset->totalRows()];
    m_colWidth = new int[m_frameset->totalCols()];

    int i;
    int totalRelative = 0;
    int colsRelative = 0;
    int rowsRelative = 0;
    int remainingRelativeWidth = 0;
    // fixed rows first, then percent and then relative

    if(m_rows)
    {
	for(i = 0; i< m_frameset->totalRows(); i++)
	{
	    kdDebug( 6031 ) << "setting row " << i << endl;
	    if(m_rows->at(i)->type == Fixed || m_rows->at(i)->type == Percent)
	    {
		m_rowHeight[i] = m_rows->at(i)->width(heightAvailable);
		kdDebug( 6031 ) << "setting row height to " << m_rowHeight[i] << endl;
		remainingHeight -= m_rowHeight[i];
	    }
	    else if(m_rows->at(i)->type == Relative)
	    {
		totalRelative += m_rows->at(i)->value;
		rowsRelative++;
	    }
	}
	// ###
	if(remainingHeight < 0) remainingHeight = 0;

	if ( !totalRelative && rowsRelative )
	  remainingRelativeWidth = remainingHeight/rowsRelative;
	
	for(i = 0; i< m_frameset->totalRows(); i++)
	 {
	    if(m_rows->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  m_rowHeight[i] = m_rows->at(i)->value*remainingHeight/totalRelative;
		else
		  m_rowHeight[i] = remainingRelativeWidth;
	  	remainingHeight -= m_rowHeight[i];
		totalRelative--;
	    }
	}
	
	// support for totally broken frame declarations
	if(remainingHeight)
	{
	    // just distribute it over all columns...
	    int rows = m_frameset->totalRows();
	    for(i = 0; i< m_frameset->totalRows(); i++)
	    {
		int toAdd = remainingHeight/rows;
		rows--;
		m_rowHeight[i] += toAdd;
		remainingHeight -= toAdd;
	    }
	}	
    }
    else
	m_rowHeight[0] = m_height;

    if(m_cols)
    {
	totalRelative = 0;
	remainingRelativeWidth = 0;

	for(i = 0; i< m_frameset->totalCols(); i++)
	{
	    if(m_cols->at(i)->type == Fixed || m_cols->at(i)->type == Percent)
	    {
		m_colWidth[i] = m_cols->at(i)->width(widthAvailable);
		remainingWidth -= m_colWidth[i];
	    }
	    else if(m_cols->at(i)->type == Relative)
	    {
		totalRelative += m_cols->at(i)->value;
		colsRelative++;
	    }
	}
	// ###
	if(remainingWidth < 0) remainingWidth = 0;
	
	if ( !totalRelative && colsRelative )
	  remainingRelativeWidth = remainingWidth/colsRelative;

	for(i = 0; i < m_frameset->totalCols(); i++)
	{
	    if(m_cols->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  m_colWidth[i] = m_cols->at(i)->value*remainingWidth/totalRelative;
		else
		  m_colWidth[i] = remainingRelativeWidth;
		remainingWidth -= m_colWidth[i];
		totalRelative--;
	    }
	}

	// support for totally broken frame declarations
	if(remainingWidth)
	{
	    // just distribute it over all columns...
	    int cols = m_frameset->totalCols();
	    for(i = 0; i< m_frameset->totalCols(); i++)
	    {
		int toAdd = remainingHeight/cols;
		cols--;
		m_rowHeight[i] += toAdd;
		remainingHeight -= toAdd;
	    }
	}

    }
    else
	m_colWidth[0] = m_width;

    positionFrames(deep);

    RenderObject *child = firstChild();
    if ( !child )
      return;

    if(!m_hSplitVar && !m_vSplitVar)
    {
	kdDebug( 6031 ) << "calculationg fixed Splitters" << endl;
	if(!m_vSplitVar && m_frameset->totalCols() > 1)
	{
	    m_vSplitVar = new bool[m_frameset->totalCols()];
	    for(int i = 0; i < m_frameset->totalCols(); i++) m_vSplitVar[i] = true;
	}
	if(!m_hSplitVar && m_frameset->totalRows() > 1)
	{
	    m_hSplitVar = new bool[m_frameset->totalRows()];
	    for(int i = 0; i < m_frameset->totalRows(); i++) m_hSplitVar[i] = true;
	}

	for(int r = 0; r < m_frameset->totalRows(); r++)
	{
	    for(int c = 0; c < m_frameset->totalCols(); c++)
	    {
		bool fixed = false;
		
		if ( strcmp( child->renderName(), "RenderFrameSet" ) == 0 )
		  fixed = static_cast<RenderFrameSet *>(child)->frameSetImpl()->noResize();
		else
		  fixed = static_cast<RenderFrame *>(child)->frameImpl()->noResize();

		/*
		if(child->id() == ID_FRAMESET)
		    fixed = (static_cast<HTMLFrameSetElementImpl *>(child))->noResize();
		else if(child->id() == ID_FRAME)
		    fixed = (static_cast<HTMLFrameElementImpl *>(child))->noResize();
		*/

		if(fixed)
		{
		    kdDebug( 6031 ) << "found fixed cell " << r << "/" << c << "!" << endl;
		    if( m_frameset->totalCols() > 1)
		    {
			if(c>0) m_vSplitVar[c-1] = false;
			m_vSplitVar[c] = false;
		    }
		    if( m_frameset->totalRows() > 1)
		    {
			if(r>0) m_hSplitVar[r-1] = false;
			m_hSplitVar[r] = false;
		    }
		    child = child->nextSibling();
		    if(!child) goto end2;
		}		
		else
		    kdDebug( 6031 ) << "not fixed: " << r << "/" << c << "!" << endl;
	    }
	}

    }
 end2:
    setLayouted();
}

void RenderFrameSet::positionFrames( bool deep )
{
  int r;
  int c;

  RenderObject *child = firstChild();
  if ( !child )
    return;

  //  NodeImpl *child = _first;
  //  if(!child) return;

  int yPos = 0;

  for(r = 0; r < m_frameset->totalRows(); r++)
  {
    int xPos = 0;
    for(c = 0; c < m_frameset->totalCols(); c++)
    {
    //	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(child);
      child->setPos( xPos, yPos );
      child->setSize( m_colWidth[c], m_rowHeight[r] );

      //if ( deep )
      child->layout( deep );
	
      xPos += m_colWidth[c] + m_frameset->border();
      child = child->nextSibling();

      if ( !child )
        return;
    /*	
	    e->renderer()->setXPos(xPos);
	    e->renderer()->setYPos(yPos);
	    e->setWidth(colWidth[c]);
	    e->setAvailableWidth(colWidth[c]);
	    e->setDescent(rowHeight[r]);
	    if(deep)
		e->layout(deep);
	    xPos += colWidth[c] + border;
	    child = child->nextSibling();
	    if(!child) return;
    */
	}
	yPos += m_rowHeight[r] + m_frameset->border();
    }
}

bool RenderFrameSet::userResize( int _x, int _y, DOM::NodeImpl::MouseEventType type )
{
  bool res = false;

  if ( !m_resizing && type == DOM::NodeImpl::MouseMove || type == DOM::NodeImpl::MousePress )
  {
    kdDebug( 6031 ) << "mouseEvent:check" << endl;

    m_hSplit = -1;
    m_vSplit = -1;
    //bool resizePossible = true;

    // check if we're over a horizontal or vertical boundary
    int pos = m_colWidth[0];
    for(int c = 1; c < m_frameset->totalCols(); c++)
    {
      if(_x >= pos && _x <= pos+m_frameset->border())
      {
        if(m_vSplitVar && m_vSplitVar[c-1] == true) m_vSplit = c-1;
        kdDebug( 6031 ) << "vsplit!" << endl;
	res = true;
        break;
      }
      pos += m_colWidth[c] + m_frameset->border();
    }
	
    pos = m_rowHeight[0];
    for(int r = 1; r < m_frameset->totalRows(); r++)
    {
      if( _y >= pos && _y <= pos+m_frameset->border())
      {
        if(m_hSplitVar && m_hSplitVar[r-1] == true) m_hSplit = r-1;
        kdDebug( 6031 ) << "hsplitvar = " << m_hSplitVar << endl;
        kdDebug( 6031 ) << "hsplit!" << endl;
	res = true;
        break;
      }
      pos += m_rowHeight[r] + m_frameset->border();
    }
    kdDebug( 6031 ) << m_hSplit << "/" << m_vSplit << endl;

    QCursor cursor;
    if(m_hSplit != -1 && m_vSplit != -1)
    {
      cursor = Qt::sizeAllCursor;
    }
    else if( m_vSplit != -1 )
    {
      cursor = Qt::splitHCursor;
    }	
    else if( m_hSplit != -1 )
    {
      cursor = Qt::splitVCursor;
    }

    if(type == DOM::NodeImpl::MousePress)
    {
      m_resizing = true;
      KApplication::setOverrideCursor(cursor);
      m_vSplitPos = _x;
      m_hSplitPos = _y;
    }
    else
      static_cast<KHTMLView *>(m_view)->setCursor(cursor);

  }

  // ### need to draw a nice movin indicator for the resize.
  // ### check the resize is not going out of bounds.
  if(m_resizing && type == DOM::NodeImpl::MouseRelease)
  {
    m_resizing = false;
    KApplication::restoreOverrideCursor();

    if(m_vSplit != -1 )
    {
      kdDebug( 6031 ) << "split xpos=" << _x << endl;
      int delta = m_vSplitPos - _x;
      m_colWidth[m_vSplit] -= delta;
      m_colWidth[m_vSplit+1] += delta;
    }	
    if(m_hSplit != -1 )
    {
      kdDebug( 6031 ) << "split ypos=" << _y << endl;
      int delta = m_hSplitPos - _y;
      m_rowHeight[m_hSplit] -= delta;
      m_rowHeight[m_hSplit+1] += delta;
    }

    positionFrames( true );
  }

  return res;
}

/**************************************************************************************/

RenderPart::RenderPart( QScrollView *view )
: RenderWidget( view )
{
  m_view = view;
}

RenderPart::~RenderPart()
{
}

void RenderPart::setWidget( QWidget *widget )
{
  setQWidget(widget);
  if ( m_widget )
    m_widget->show();
  layout();
  repaint();
}

void RenderPart::layout( bool )
{
  if ( m_widget )
    m_widget->resize( m_width, m_height );
}

/***************************************************************************************/

RenderFrame::RenderFrame( QScrollView *view, DOM::HTMLFrameElementImpl *frame )
: RenderPart( view )
{
  m_frame = frame;
}

RenderFrame::~RenderFrame()
{
}

void RenderFrame::setWidget( QWidget *widget )
{
    kdDebug(6031) << "RenderFrame::setWidget()" << endl;
    if(widget->inherits("QScrollView")) {
	kdDebug(6031) << "frame is a scrollview!" << endl;
	QScrollView *view = static_cast<QScrollView *>(widget);
	if(!m_frame->frameBorder || !((static_cast<HTMLFrameSetElementImpl *>(m_frame->_parent))->frameBorder()))
	    view->setFrameStyle(QFrame::NoFrame);
	//	if(m_frame->scrolling == QScrollView::AlwaysOff)
	//    kdDebug(6031) << "no scrollbar"<<endl;
	view->setVScrollBarMode(m_frame->scrolling);
	view->setHScrollBarMode(m_frame->scrolling);
	if(view->inherits("KHTMLView")) {
	    kdDebug(6031) << "frame is a KHTMLview!" << endl;
	    KHTMLView *htmlView = static_cast<KHTMLView *>(view);
	    if(m_frame->marginWidth != -1) htmlView->setMarginWidth(m_frame->marginWidth);
	    if(m_frame->marginHeight != -1) htmlView->setMarginHeight(m_frame->marginHeight);
	}
    }
    RenderPart::setWidget(widget);
}

/****************************************************************************************/

RenderPartObject::RenderPartObject( QScrollView *view, DOM::HTMLElementImpl *o )
: RenderPart( view )
{
   m_obj = o;
}

RenderPartObject::~RenderPartObject()
{
}

void RenderPartObject::close()
{
  QString url;
  QString serviceType;

  QStringList params;

  if(m_obj->id() == ID_OBJECT) {
     // check for embed child object
     HTMLObjectElementImpl *o = static_cast<HTMLObjectElementImpl *>(m_obj);
     HTMLEmbedElementImpl *embed = 0;
     NodeImpl *child = o->firstChild();
     while ( child ) {
	if ( child->id() == ID_EMBED )
	   embed = static_cast<HTMLEmbedElementImpl *>( child );
	     
	child = child->nextSibling();
     }
      
     if ( !embed )
     {
	url = o->url;
	serviceType = o->serviceType;
	if(serviceType.isEmpty() || serviceType.isNull()) {
	   if(o->classId.contains(QString::fromLatin1("D27CDB6E-AE6D-11cf-96B8-444553540000"))) {
	      // Flash. set the mimetype
	      serviceType = "application/x-shockwave-flash";
	   }
	   // add more plugins here
	}	
	if((url.isEmpty() || url.isNull())) {
	   // look for a SRC attribute in the params
	   NodeImpl *child = o->firstChild();	
	   while ( child ) {
	      if ( child->id() == ID_PARAM ) {
		 HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>( child );

		 if ( p->name().lower()==QString::fromLatin1("src") || 
		      p->name().lower()==QString::fromLatin1("movie") ) 
		 {
		    url = p->value();
		    break;
		 }
	      }
	      child = child->nextSibling();
	   }
	}

	// add all <param>'s to the QStringList argument of the part
	NodeImpl *child = o->firstChild();
	while ( child ) {
	   if ( child->id() == ID_PARAM ) {
	      HTMLParamElementImpl *p = static_cast<HTMLParamElementImpl *>( child );

	      QString aStr = p->name();
	      aStr += QString::fromLatin1("=\"");
	      aStr += p->value();
	      aStr += QString::fromLatin1("\"");
	      params.append(aStr);
	   } 
	     
	   child = child->nextSibling();
	}
      
	if ( url.isEmpty() && serviceType.isEmpty() )
	   return; //ooops (-:

	static_cast<KHTMLView *>(m_view)->part()->requestObject( this, url, serviceType, params );
     } else
     {
	// render embed object
	url = embed->url;
	serviceType = embed->serviceType;

	if ( url.isEmpty() && serviceType.isEmpty() )
	   return; //ooops (-:

	kdDebug() << "<embed> - part()->requestObject( " << url << " )" << endl;
	static_cast<KHTMLView *>(m_view)->part()->requestObject( this, url, serviceType, 
								 embed->param );
     }
  } else if ( m_obj->id() == ID_EMBED ) {
     HTMLEmbedElementImpl *o = static_cast<HTMLEmbedElementImpl *>(m_obj);
     url = o->url;
     serviceType = o->serviceType;

     if ( url.isEmpty() && serviceType.isEmpty() )
	return; //ooops (-:

     kdDebug() << "<embed> - part()->requestObject( " << url << " )" << endl;
     static_cast<KHTMLView *>(m_view)->part()->requestObject( this, url, serviceType, params );
  } else {
      assert(m_obj->id() == ID_IFRAME);
      HTMLIFrameElementImpl *o = static_cast<HTMLIFrameElementImpl *>(m_obj);
      url = o->url.string();
      if( url.isEmpty()) return;
      KHTMLView *v = static_cast<KHTMLView *>(m_view);
      v->part()->requestFrame( this, url, o->name.string() );
  }

  layout();

  RenderPart::close();
}

void RenderPartObject::setWidget( QWidget *w )
{
  if(w->inherits("QScrollView") && m_obj->id() == ID_IFRAME) {
      kdDebug(6031) << "iframe is a scrollview!" << endl;
      QScrollView *view = static_cast<QScrollView *>(w);
      HTMLIFrameElementImpl *m_frame = static_cast<HTMLIFrameElementImpl *>(m_obj);
      if(!m_frame->frameBorder)
	  view->setFrameStyle(QFrame::NoFrame);
      view->setVScrollBarMode(m_frame->scrolling);
      view->setHScrollBarMode(m_frame->scrolling);
      if(view->inherits("KHTMLView")) {
	  kdDebug(6031) << "frame is a KHTMLview!" << endl;
	  KHTMLView *htmlView = static_cast<KHTMLView *>(view);
	  if(m_frame->marginWidth != -1) htmlView->setMarginWidth(m_frame->marginWidth);
	  if(m_frame->marginHeight != -1) htmlView->setMarginHeight(m_frame->marginHeight);
	}
  }
  if ( w && m_width == 0 && m_height == 0 ) {
      QSize hint = w->sizeHint();
      m_width = hint.width();
      m_height = hint.height();
  }

  RenderPart::setWidget( w );
}

bool RenderPartObject::isInline()
{
  return true;
}

void RenderPartObject::setSize( int w, int h )
{
  m_width = w;
  m_height = h;
  m_minWidth = m_maxWidth = w;
}

void RenderPartObject::layout( bool )
{
  setSize( m_style->width().width( m_view->clipper()->width() ),
	   m_style->height().width( m_view->clipper()->height() ) );
  RenderPart::layout();
}
