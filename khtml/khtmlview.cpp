/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
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
 */

#include "khtmlview.moc"
#include "khtml_part.h"

#include "misc/loader.h"
#include "html/html_documentimpl.h"
#include "dom/dom2_range.h"
#include "rendering/render_object.h"
#include "misc/htmlhashes.h"

#include <qdragobject.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qevent.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kmimetype.h>
#include <kimgio.h>

#include <stdio.h>
#include <qlist.h>
#include <qrect.h>
#include <qscrollview.h>
#include <qwidget.h>


#define SCROLLBARWIDTH 16

#define PAINT_BUFFER_HEIGHT 150

template class QList<KHTMLView>;

QList<KHTMLView> *KHTMLView::lstViews = 0L;

using namespace DOM;

QPixmap* KHTMLView::paintBuffer = 0L;

class KHTMLViewPrivate {
public:
    KHTMLViewPrivate()
    {
	selectionStart = 0;
	selectionEnd = 0;
	startOffset = 0;
	endOffset = 0;
	startBeforeEnd = true;
    }
    NodeImpl *selectionStart;
    int startOffset;
    NodeImpl *selectionEnd;
    int endOffset;
    bool startBeforeEnd;
};


KHTMLView::KHTMLView( KHTMLPart *part, QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    m_part = part;

    // initialize QScrollview

    enableClipper(true);

    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    kimgioRegister();

    setCursor(arrowCursor);

    init();

    viewport()->show();

    d = new KHTMLViewPrivate;
}

KHTMLView::~KHTMLView()
{
  lstViews->removeRef( this );
  if(lstViews->isEmpty())
  {
      delete lstViews;
      lstViews = 0;
      if(paintBuffer) delete paintBuffer;
      paintBuffer = 0;
  }

  delete d;
}

void KHTMLView::init()
{
    if ( lstViews == 0L )
	lstViews = new QList<KHTMLView>;
    lstViews->setAutoDelete( FALSE );
    lstViews->append( this );

    if(!paintBuffer) paintBuffer = new QPixmap();

  viewport()->setFocusPolicy( QWidget::WheelFocus );

  _marginWidth = 5;
  _marginHeight = 5;
  _width = width()- SCROLLBARWIDTH - 2*marginWidth();

  resizeContents(clipper()->width(), clipper()->height());

  khtml::Cache::init();
}

void KHTMLView::clear()
{
    resizeContents(clipper()->width(), clipper()->height());

    pressed = false;

    setVScrollBarMode(Auto);
    setHScrollBarMode(Auto);

    d->selectionStart = 0;
    d->selectionEnd = 0;
    d->startOffset = 0;
    d->endOffset = 0;
}

/*
void KHTMLView::setFollowsLinks( bool follow )
{
    _followLinks = follow;
}

bool KHTMLView::followsLinks()
{
    return _followLinks;
}
*/


void KHTMLView::resizeEvent ( QResizeEvent * event )
{
//    printf("resizeEvent\n");
    layout();

    DOM::HTMLDocumentImpl *doc = m_part->docImpl();

    if ( doc && doc->body() )
      resizeContents( doc->renderer()->width(), doc->renderer()->height() );

    QScrollView::resizeEvent(event);
}



void KHTMLView::viewportPaintEvent ( QPaintEvent* pe  )
{
    QRect r = pe->rect();

    NodeImpl *body = 0;

    if( m_part->docImpl() )
	body = m_part->docImpl()->body();

    QRect rr(
	-viewport()->x(), -viewport()->y(),
	clipper()->width(), clipper()->height()
    );
    r &= rr;
    int ex = r.x() + viewport()->x() + contentsX();;
    int ey = r.y() + viewport()->y() + contentsY();;
    int ew = r.width();
    int eh = r.height();

    if(!body)
    {
	QPainter p(viewport());

	p.fillRect(r.x(), r.y(), ew, eh, kapp->palette().normal().brush(QColorGroup::Background));
	return;
    }
    //printf("viewportPaintEvent x=%d,y=%d,w=%d,h=%d\n",ex,ey,ew,eh);

    if ( paintBuffer->width() < width() )
    {
        paintBuffer->resize(width(),PAINT_BUFFER_HEIGHT);
	QPainter p(paintBuffer);
	p.fillRect(r.x(), r.y(), ew, eh, kapp->palette().normal().brush(QColorGroup::Background));
    }

    QTime qt;
    qt.start();

    int py=0;
    while (py < eh)
    {
	QPainter* tp = new QPainter;
	tp->begin( paintBuffer );
	tp->translate(-ex,-ey-py);

    	int ph = eh-py<PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;	

	// ### fix this for frames...

	tp->fillRect(ex, ey+py, ew, ph, kapp->palette().normal().brush(QColorGroup::Background));

	m_part->docImpl()->renderer()->print(tp, ex, ey+py, ew, ph, 0, 0);

	tp->end();
	delete tp;

    	//printf("bitBlt x=%d,y=%d,sw=%d,sh=%d\n",ex,ey+py,ew,ph);
	bitBlt(viewport(),r.x(),r.y()+py,paintBuffer,0,0,ew,ph,Qt::CopyROP);
	
	py += PAINT_BUFFER_HEIGHT;
    }

    if(d->selectionStart) {
	paintSelection();
    }

    //printf("TIME: print() dt=%d\n",qt.elapsed());
}

void KHTMLView::paintSelection()
{
    if(d->selectionStart == d->selectionEnd && d->startOffset == d->endOffset) return;

    QPainter* p = new QPainter;
    p->begin(this);
    if(d->startBeforeEnd)
	d->selectionStart->renderer()->printSelection(p, d->startOffset,
						      d->selectionEnd->renderer(), d->endOffset);
    else
	d->selectionEnd->renderer()->printSelection(p, d->endOffset,
						    d->selectionStart->renderer(), d->startOffset);
    delete p;
}


void KHTMLView::layout(bool force)
{
    //### take care of frmaes (hide scrollbars,...)

    if( m_part->docImpl() )
    {	
        DOM::HTMLDocumentImpl *document = m_part->docImpl();

	NodeImpl *body = document->body();
	if(body && body->id() == ID_FRAMESET)
	{
	    setVScrollBarMode(AlwaysOff);
	    setHScrollBarMode(AlwaysOff);
	    _width = width();
	
	    document->renderer()->setMinWidth(_width);
	    document->renderer()->layout(true);
	    return;
	}

	int w = width() - SCROLLBARWIDTH - 2*marginWidth();

//	if(w < _width-5 || w > _width + 5)
    	if (w != _width || force)
	{
	    //printf("layouting document\n");

	    _width = w;

	    QTime qt;
	    qt.start();

	    document->renderer()->setMinWidth(_width);
	    document->renderer()->layout(true);
	    resizeContents(document->renderer()->width(), document->renderer()->height());
	    printf("TIME: layout() dt=%d\n",qt.elapsed());

	    viewport()->repaint(false);
	}
    }
    else
    {
	_width = width() - SCROLLBARWIDTH - 2*marginWidth();
    }
}

void KHTMLView::paintElement( khtml::RenderObject *o, int xPos, int yPos )
{
    int yOff = contentsY();
    if(yOff > yPos + o->height() ||
       yOff + visibleHeight() < yPos)
	return;

    QWidget *vp = viewport();
    QPainter p(vp);
    int xOff = contentsX()+vp->x();
    yOff += vp->y();
    p.translate( -xOff, -yOff );

    o->printObject( &p , xOff, yOff, vp->width(), vp->height(),
		    xPos , yPos );
}

//
// Event Handling
//
/////////////////

void KHTMLView::viewportMousePressEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;

    if(m_part->mousePressHook(_mouse)) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //printf("\nmousePressEvent: x=%d, y=%d\n", xm, ym);


    // Make this frame the active one
    // ### need some visual indication for the active frame.
    /* ### use PartManager (Simon)
    if ( _isFrame && !_isSelected )
    {
	printf("activating frame!\n");
	topView()->setFrameSelected(this);
    }*/

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url, innerNode, offset );

    if(url != 0)
    {
	//printf("mouseEvent: overURL %s\n", url.string().latin1());
	m_strSelectedURL = url.string();
    }
    else
	m_strSelectedURL = QString::null;

    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
    	pressed = TRUE;
	if(_mouse->button() == LeftButton) {
    	    if(innerNode) {
		d->selectionStart = innerNode;
		d->startOffset = offset;
		d->selectionEnd = innerNode;
		d->endOffset = offset;
		printf("setting start of selection to %p/%d\n", innerNode, offset);
	    } else {
		d->selectionStart = 0;
		d->selectionEnd = 0;
	    }
	    // ### emit some signal
	}
    }

    if( _mouse->button() == RightButton )
    {
	m_part->popupMenu( m_strSelectedURL );
    }
}

void KHTMLView::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;
    if(m_part->mouseDoubleClickHook(_mouse)) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseDblClickEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url, innerNode, offset );

    // ###
    //if ( url.length() )
    //emit doubleClick( url.string(), _mouse->button() );
}

void KHTMLView::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if(!m_part->docImpl()) return;
    if(m_part->mouseMoveHook(_mouse)) return;

    // drag of URL

    if(pressed && !m_strSelectedURL.isEmpty())
    {
	QStrList uris;
	KURL u( m_part->completeURL( m_strSelectedURL) );
	uris.append(u.url().ascii());
	QDragObject *d = new QUriDrag(uris, this);
	QPixmap p = KMimeType::pixmapForURL(u, 0, KIconLoader::Medium);
	if(p.isNull()) printf("null pixmap\n");
	d->setPixmap(p);
	d->drag();

	// when we finish our drag, we need to undo our mouse press
	pressed = false;
        m_strSelectedURL = "";
	return;
    }

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseMove, 0, 0, url, innerNode, offset );

    if ( !pressed && url.length() )
    {
	QString surl = url.string();
	if ( overURL.isEmpty() )
	{
	    setCursor( linkCursor );
	    overURL = surl;
	    m_part->overURL( overURL );
	    //	    emit onURL( overURL );
	}
	else if ( overURL != surl )
	{
	//	    emit onURL( surl );
	    m_part->overURL( overURL );
	    overURL = surl;
	}
	return;
    }
    else if( overURL.length() && !url.length() )
    {
	setCursor( arrowCursor );
	//	emit onURL( 0 );
	m_part->overURL( QString::null );
	overURL = "";
    }

    // selection stuff
    if( pressed ) {
	d->selectionEnd = innerNode;
	d->endOffset = offset;
	printf("setting end of selection to %p/%d\n", innerNode, offset);

	// we have to get to know if end is before start or not...
	NodeImpl *n = d->selectionStart;
	d->startBeforeEnd = false;
	while(n) {
	    if(n == d->selectionEnd) {
		d->startBeforeEnd = true;
		break;
	    }
	    NodeImpl *next = n->firstChild();
	    if(!next) n = n->nextSibling();
	    while(n && !next) {
		n = n->parentNode();
		next = n->nextSibling();
	    }
	    n = next;
	}
    }
}

void KHTMLView::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !m_part->docImpl() ) return;
    if(m_part->mouseReleaseHook(_mouse)) return;

    if ( pressed )
    {
	// in case we started an autoscroll in MouseMove event
	// ###
	//stopAutoScrollY();
	//disconnect( this, SLOT( slotUpdateSelectText(int) ) );
    }

    // Used to prevent mouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //printf("\nmouseReleaseEvent: x=%d, y=%d\n", xm, ym);

    DOMString url=0;
    NodeImpl *innerNode=0;
    long offset;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url, innerNode, offset );

    if ( m_strSelectedURL.isEmpty() )
	return;

    if ( _mouse->button() != RightButton )
    {
	KURL u(m_strSelectedURL);
	QString pressedTarget;
	if(u.protocol() == "target")
	{
	    m_strSelectedURL = u.ref();
	    pressedTarget = u.host();
	}	
	printf("m_strSelectedURL='%s' target=%s\n",m_strSelectedURL.data(), pressedTarget.latin1());

	m_part->urlSelected( m_strSelectedURL, _mouse->button(), pressedTarget );
   }

    printf("final range of selection to %p/%d --> %p/%d\n", d->selectionStart, d->startOffset,
	   innerNode, offset);
    d->selectionEnd = innerNode;
    d->endOffset = offset;

    // ### delete selection in case start and end position are at the same point
    if(d->selectionStart == d->selectionEnd && d->startOffset == d->endOffset) {
	d->selectionStart = 0;
	d->selectionEnd = 0;
	d->startOffset = 0;
	d->endOffset = 0;
    } else {
	// we have to get to know if end is before start or not...
	NodeImpl *n = d->selectionStart;
	d->startBeforeEnd = false;
	while(n) {
	    if(n == d->selectionEnd) {
		d->startBeforeEnd = true;
		break;
	    }
	    NodeImpl *next = n->firstChild();
	    if(!next) n = n->nextSibling();
	    while(n && !next) {
		n = n->parentNode();
		next = n->nextSibling();
	    }
	    n = next;
	}
    }
}

void KHTMLView::keyPressEvent( QKeyEvent *_ke )
{
    if(m_part->keyPressHook(_ke)) return;


    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    switch ( _ke->key() )
    {
    case Key_Down:
    case Key_J:
	scrollBy( 0, 10 );
	break;

    case Key_Next:
    case Key_Space:
	scrollBy( 0, clipper()->height() - offs );
	break;

    case Key_Up:
    case Key_K:
	scrollBy( 0, -10 );
	break;

    case Key_Prior:
    case Key_Backspace:
	scrollBy( 0, -clipper()->height() + offs );
	break;

    case Key_Right:
    case Key_L:
	scrollBy( 10, 0 );
	break;	

    case Key_Left:
    case Key_H:
	scrollBy( -10, 0 );
	break;

    default:
	QScrollView::keyPressEvent( _ke );
    }
}

void KHTMLView::keyReleaseEvent( QKeyEvent *_ke )
{
    if(m_part->keyReleaseHook(_ke)) return;
    QScrollView::keyReleaseEvent( _ke);
}

bool KHTMLView::focusNextPrevChild( bool next )
{
    printf("focusNextPrev %d\n",next);
    return true;    // ### temporary fix for qscrollview focus bug
    	    	    // as a side effect, disables tabbing between form elements
		    // -antti

//    return QScrollView::focusNextPrevChild( next );
}

