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
#include "xml/dom_textimpl.h"

#include <qdragobject.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qevent.h>
#include <qdatetime.h>
#include <qclipboard.h>

#include <kapp.h>
#include <kmimetype.h>
#include <kimgio.h>
#include <kdebug.h>

#include <qlist.h>
#include <qrect.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qapplication.h>


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
	underMouse = 0;
    }
    NodeImpl *selectionStart;
    int startOffset;
    NodeImpl *selectionEnd;
    int endOffset;
    bool startBeforeEnd;
    NodeImpl *underMouse;
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
    linkCursor = QCursor(PointingHandCursor);
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
    d->underMouse = 0;
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
//    kdDebug(300) << "resizeEvent" << endl;
    layout();

    DOM::HTMLDocumentImpl *doc = m_part->docImpl();

    if ( doc && doc->body() )
      resizeContents( doc->renderer()->width(), doc->renderer()->height() );

    QScrollView::resizeEvent(event);
}



void KHTMLView::viewportPaintEvent ( QPaintEvent* pe  )
{
    QRect r = pe->rect();

//    kdDebug(300) << "viewportPaintEvent r x=" << //    	r.x() << ",y=" << r.y() << ",w=" << r.width() << ",h=" << r.height() << endl;

    NodeImpl *body = 0;

    if( m_part->docImpl() )
	body = m_part->docImpl()->body();

    QRect rr(
	-viewport()->x(), -viewport()->y(),
	clipper()->width(), clipper()->height()
    );
    r &= rr;
    int ex = r.x() + viewport()->x() + contentsX();
    int ey = r.y() + viewport()->y() + contentsY();
    int ew = r.width();
    int eh = r.height();

    if (ew<0)	    	// events generated with repaint() are bit weird...
    	ew = pe->rect().width();
    if (eh<0)
    	eh = pe->rect().height();

    if(!body)
    {
	QPainter p(viewport());

	p.fillRect(r.x(), r.y(), ew, eh, kapp->palette().normal().brush(QColorGroup::Background));
	return;
    }
//    kdDebug(300) << "viewportPaintEvent x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;

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

	
	drawContents(tp,ex,ey+py,ew,ph); // in case someone want to extend the widget
	tp->end();

	delete tp;

    	//kdDebug(300) << "bitBlt x=" << ex << ",y=" << ey+py << ",sw=" << ew << ",sh=" << ph << endl;
	bitBlt(viewport(),r.x(),r.y()+py,paintBuffer,0,0,ew,ph,Qt::CopyROP);
	
	py += PAINT_BUFFER_HEIGHT;
    }

    //kdDebug(300) << "TIME: print() dt=" << qt.elapsed() << endl;
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
	    //kdDebug(300) << "layouting document" << endl;

	    _width = w;

	    QTime qt;
	    qt.start();

	    document->renderer()->setMinWidth(_width);
	    document->renderer()->layout(true);
	    resizeContents(document->renderer()->width(), document->renderer()->height());
	    kdDebug(300) << "TIME: layout() dt=" << qt.elapsed() << endl;

	    viewport()->repaint(false);
	}
	else
	{
	    document->renderer()->layout(false);
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


    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //kdDebug(300) << "\nmousePressEvent: x=" << xm << ", y=" << ym << endl;


    // Make this frame the active one
    // ### need some visual indication for the active frame.
    /* ### use PartManager (Simon)
    if ( _isFrame && !_isSelected )
    {
	kdDebug(300) << "activating frame!" << endl;
	topView()->setFrameSelected(this);
    }*/

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url, innerNode, offset );

    kdDebug(300) << "Her har vi long'n: " << offset << " " << endl;
    if(m_part->mousePressHook(_mouse, xm, ym, url, Node(innerNode), offset)) return;

    if(url != 0)
    {
	//kdDebug(300) << "mouseEvent: overURL " << url.string() << endl;
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
		m_part->docImpl()->clearSelection();
		kdDebug(300) << "setting start of selection to " << innerNode << "/" << offset << endl;
	    } else {
		d->selectionStart = 0;
		d->selectionEnd = 0;
	    }
	    // ### emit some signal
	    emit selectionChanged();
	}
    }

    if( _mouse->button() == RightButton )
    {
	m_part->popupMenu( m_strSelectedURL );
    }
    else if ( _mouse->button() == MidButton )
    {
      KURL u( m_part->url(), m_strSelectedURL );
      if ( !u.isMalformed() )
        emit m_part->browserExtension()->createNewWindow( u );
    }
}

void KHTMLView::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    kdDebug(300) << "\nmouseDblClickEvent: x=" << xm << ", y=" << ym << endl;

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url, innerNode, offset );

    if(m_part->mouseDoubleClickHook(_mouse, xm, ym, url, Node(innerNode), offset)) return;

    // ###
    //if ( url.length() )
    //emit doubleClick( url.string(), _mouse->button() );
}

void KHTMLView::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if(!m_part->docImpl()) return;


    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseMove, 0, 0, url, innerNode, offset );

    d->underMouse = innerNode;
    if(m_part->mouseMoveHook(_mouse, xm, ym, url, Node(innerNode), offset)) return;

    // drag of URL

    if(pressed && !m_strSelectedURL.isEmpty())
    {
	QStrList uris;
	KURL u( m_part->completeURL( m_strSelectedURL) );
	uris.append(u.url().ascii());
	QDragObject *d = new QUriDrag(uris, this);
	QPixmap p = KMimeType::pixmapForURL(u, 0, KIconLoader::Medium);
	if(p.isNull()) kdDebug(300) << "null pixmap" << endl;
	d->setPixmap(p);
	d->drag();

	// when we finish our drag, we need to undo our mouse press
	pressed = false;
        m_strSelectedURL = "";
	return;
    }



    if ( !pressed && url.length() )
    {
	QString surl = url.string();
	if ( overURL.isEmpty() )
	{
	    setCursor( linkCursor );
	    overURL = surl;
	    m_part->overURL( overURL );
	}
	else if ( overURL != surl )
	{
	    m_part->overURL( overURL );
	    overURL = surl;
	}
	return;
    }
    else if( overURL.length() && !url.length() )
    {
	setCursor( arrowCursor );
	m_part->overURL( QString::null );
	overURL = "";
    }

    // selection stuff
    if( pressed && innerNode->isTextNode()) {
	d->selectionEnd = innerNode;
	d->endOffset = offset;
	kdDebug(300) << "setting end of selection to " << innerNode << "/" << offset << endl;

	// we have to get to know if end is before start or not...
	NodeImpl *n = d->selectionStart;
	d->startBeforeEnd = false;
	while(n) {
	    if(n == d->selectionEnd) {
		d->startBeforeEnd = true;
		break;
	    }
	    NodeImpl *next = n->firstChild();
	    if(!next) next = n->nextSibling();
	    while( !next && (n = n->parentNode()) ) {
		next = n->nextSibling();
	    }
	    n = next;
	    //viewport()->repaint(false);
	}
	
	if (d->selectionEnd == d->selectionStart && d->endOffset < d->startOffset)
	     m_part->docImpl()
	    	->setSelection(d->selectionStart,d->endOffset,d->selectionEnd,d->startOffset);
	else if (d->startBeforeEnd)
	    m_part->docImpl()
	    	->setSelection(d->selectionStart,d->startOffset,d->selectionEnd,d->endOffset);
	else
	    m_part->docImpl()
	    	->setSelection(d->selectionEnd,d->endOffset,d->selectionStart,d->startOffset);
	
    }
}

void KHTMLView::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !m_part->docImpl() ) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //kdDebug(300) << "\nmouseReleaseEvent: x=" << xm << ", y=" << ym << endl;

    DOMString url=0;
    NodeImpl *innerNode=0;
    long offset;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url, innerNode, offset );

    if(m_part->mouseReleaseHook(_mouse, xm, ym, url, Node(innerNode), offset)) return;


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


    if ( !m_strSelectedURL.isEmpty() && _mouse->button() != RightButton )
    {
	KURL u(m_strSelectedURL);
	QString pressedTarget;
	if(u.protocol() == "target")
	{
	    m_strSelectedURL = u.ref();
	    pressedTarget = u.host();
	}	
	kdDebug(300) << "m_strSelectedURL='" << m_strSelectedURL << "' target=" << pressedTarget << endl;

	m_part->urlSelected( m_strSelectedURL, _mouse->button(), _mouse->state(), pressedTarget );
   }

    if(innerNode->isTextNode()) {
	kdDebug(300) << "final range of selection to " << d->selectionStart << "/" << d->startOffset << " --> " << innerNode << "/" << offset << endl;
	d->selectionEnd = innerNode;
	d->endOffset = offset;
    }

    // delete selection in case start and end position are at the same point
    if(d->selectionStart == d->selectionEnd && d->startOffset == d->endOffset) {
	d->selectionStart = 0;
	d->selectionEnd = 0;
	d->startOffset = 0;
	d->endOffset = 0;
	emit selectionChanged();
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
	    if(!next) next = n->nextSibling();
	    while( !next && (n = n->parentNode()) ) {
		next = n->nextSibling();
	    }	
	    n = next;
	}
	if(!d->startBeforeEnd)
	{
	    NodeImpl *tmpNode = d->selectionStart;
	    int tmpOffset = d->startOffset;
	    d->selectionStart = d->selectionEnd;
	    d->startOffset = d->endOffset;
	    d->selectionEnd = tmpNode;
	    d->endOffset = tmpOffset;
	    d->startBeforeEnd = true;
	}
	// get selected text and paste to the clipboard
	QString text = selectedText();
	QClipboard *cb = QApplication::clipboard();
	cb->setText(text);
	//kdDebug(300) << "selectedText = " << text << endl;
	emit selectionChanged();
    }
}

QString KHTMLView::selectedText() const
{
    QString text;
    NodeImpl *n = d->selectionStart;
    while(n) {
	if(n->isTextNode()) {
	    QString str = static_cast<TextImpl *>(n)->data().string();
	    if(n == d->selectionStart && n == d->selectionEnd)
		text = str.mid(d->startOffset, d->endOffset - d->startOffset);
	    else if(n == d->selectionStart)
		text = str.mid(d->startOffset);
	    else if(n == d->selectionEnd)
		text += str.left(d->endOffset);
	    else
		text += str;
	}
	else if(n->id() == ID_BR)
	    text += "\n";
	else if(n->id() == ID_P || n->id() == ID_TD)
	    text += "\n\n";
	if(n == d->selectionEnd) break;
	NodeImpl *next = n->firstChild();
	if(!next) next = n->nextSibling();
	while( !next && (n = n->parentNode()) ) {
	    next = n->nextSibling();
	}

	n = next;
    }
    return text;
}

bool KHTMLView::hasSelection() const
{
  return ( d->selectionStart != 0 && d->selectionEnd != 0 );
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
    kdDebug(300) << "focusNextPrev " << next << endl;
//    return true;    // ### temporary fix for qscrollview focus bug
    	    	    // as a side effect, disables tabbing between form elements
		    // -antti

//    return QScrollView::focusNextPrevChild( next );
        return QScrollView::focusNextPrevChild( next );
}

void KHTMLView::drawContents ( QPainter * p, int clipx, int clipy, int clipw, int cliph )
{
    m_part->drawContentsHook(p);
}


DOM::NodeImpl *KHTMLView::nodeUnderMouse() const
{
    return d->underMouse;
}


