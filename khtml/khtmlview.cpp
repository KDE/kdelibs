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
#include "khtml_events.h"

#include "html/html_documentimpl.h"
#include "html/html_inlineimpl.h"
#include "rendering/render_object.h"
#include "rendering/render_root.h"
#include "rendering/render_style.h"
#include "misc/htmlhashes.h"
#include "khtml_settings.h"

#include <kcursor.h>

#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qevent.h>
#include <qdatetime.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qtimer.h>
#include <kapp.h>

#include <kimageio.h>
#include <kdebug.h>

#define PAINT_BUFFER_HEIGHT 150

template class QList<KHTMLView>;

QList<KHTMLView> *KHTMLView::lstViews = 0L;

using namespace DOM;
using namespace khtml;

class KHTMLViewPrivate {
public:
    KHTMLViewPrivate()
    {
        reset();
        tp=0;
        paintBuffer=0;
    }
    ~KHTMLViewPrivate()
    {
        delete tp; tp = 0;
        delete paintBuffer; paintBuffer =0;
    }
    void reset()
    {
        underMouse = 0;
        linkPressed = false;
        useSlowRepaints = false;
        currentNode = 0;
        originalNode = 0;
        vmode = QScrollView::Auto;
        hmode = QScrollView::Auto;
    }

    QPainter *tp;
    QPixmap  *paintBuffer;
    NodeImpl *underMouse;

    HTMLElementImpl *currentNode;
    HTMLElementImpl *originalNode;

    QScrollView::ScrollBarMode vmode;
    QScrollView::ScrollBarMode hmode;
    bool linkPressed;
    bool useSlowRepaints;

};


KHTMLView::KHTMLView( KHTMLPart *part, QWidget *parent, const char *name)
    : QScrollView( parent, name, WResizeNoErase | WRepaintNoErase )
{
    m_part = part;
    d = new KHTMLViewPrivate;
    QScrollView::setVScrollBarMode(d->vmode);
    QScrollView::setHScrollBarMode(d->hmode);

    // initialize QScrollview
    enableClipper(true);
    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    KImageIO::registerFormats();

    setCursor(arrowCursor);

    init();

    viewport()->show();

}

KHTMLView::~KHTMLView()
{
    if (m_part)
    {
	//WABA: Is this Ok? Do I need to deref it as well?
	//Does this need to be done somewhere else?
	DOM::HTMLDocumentImpl *doc = m_part->docImpl();
	if (doc && doc->body())
	    doc->detach();
    }
    lstViews->removeRef( this );
    if(lstViews->isEmpty())
    {
	delete lstViews;
	lstViews = 0;
    }

    delete d;
}

void KHTMLView::init()
{
    if ( lstViews == 0L )
        lstViews = new QList<KHTMLView>;
    lstViews->setAutoDelete( FALSE );
    lstViews->append( this );

    if(!d->paintBuffer) d->paintBuffer = new QPixmap(PAINT_BUFFER_HEIGHT, PAINT_BUFFER_HEIGHT);
   if(!d->tp) d->tp = new QPainter(d->paintBuffer);

    setFocusPolicy(QWidget::StrongFocus);
    viewport()->setFocusPolicy( QWidget::WheelFocus );

  _marginWidth = 0;
  _marginHeight = 0;
  _width = 0;
  _height = 0;

  resizeContents(visibleWidth(), visibleHeight());
}

void KHTMLView::clear()
{
    if (d->vmode==Auto)
        QScrollView::setVScrollBarMode(AlwaysOn);
    else
        QScrollView::setVScrollBarMode(d->vmode);
    QScrollView::setHScrollBarMode(d->hmode);    
    
    resizeContents(visibleWidth(), visibleHeight());
    viewport()->erase();

    if(d->useSlowRepaints) {
//         why do we need to delete the paintBuffer here ?
//         delete d->tp;
//         tp = 0;
//         delete paintBuffer;
//         paintBuffer = 0;
        setStaticBackground(false);
    }

    d->reset();
    emit cleared();
}

void KHTMLView::resizeEvent (QResizeEvent* e)
{
    QScrollView::resizeEvent(e);

    int w = visibleWidth();
    int h = visibleHeight();

    layout();

    //  this is to make sure we get the right width even if the scrolbar has dissappeared
    // due to the size change.
    if(visibleHeight() != h || visibleWidth() != w)
        layout();

    KApplication::sendPostedEvents(viewport(), QEvent::Paint);
}

void KHTMLView::drawContents( QPainter *p, int ex, int ey, int ew, int eh )
{
   NodeImpl *body = 0;

    if( m_part->docImpl() )
        body = m_part->docImpl()->body();

    if(!body) {
        p->fillRect(ex, ey, ew, eh, palette().normal().brush(QColorGroup::Base));
        return;
    }

    //kdDebug( 6000 ) << "drawContents x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;

    if ( d->paintBuffer->width() < visibleWidth() )
    {
        delete d->tp;
        d->paintBuffer->resize(visibleWidth(),PAINT_BUFFER_HEIGHT);
        d->tp = new QPainter(d->paintBuffer);
    }

    d->tp->save();
    d->tp->translate(-ex, -ey);

    int py=0;
    while (py < eh) {
        int ph = eh-py < PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;

        // ### fix this for frames...
        d->tp->fillRect(ex, ey+py, ew, ph, palette().normal().brush(QColorGroup::Base));
        m_part->docImpl()->renderer()->print(d->tp, ex, ey+py, ew, ph, 0, 0);

        p->drawPixmap(ex, ey+py, *d->paintBuffer, 0, 0, ew, ph);
        py += PAINT_BUFFER_HEIGHT;

        d->tp->translate(0, -PAINT_BUFFER_HEIGHT);
    }
    d->tp->restore();
}

void KHTMLView::layout(bool)
{
    //### take care of frmaes (hide scrollbars,...)
    if( m_part->docImpl() ) {
        DOM::HTMLDocumentImpl *document = m_part->docImpl();

        khtml::RenderRoot* root = static_cast<khtml::RenderRoot *>(document->renderer());

        NodeImpl *body = document->body();
        if(body && body->id() == ID_FRAMESET) {
            QScrollView::setVScrollBarMode(AlwaysOff);
            QScrollView::setHScrollBarMode(AlwaysOff);
            _width = visibleWidth();
            body->renderer()->setLayouted(false);
            body->renderer()->layout();
	    root->layout();
            return;
        }

        _height = visibleHeight();
        _width = visibleWidth();

        //QTime qt;
        //qt.start();
            root->layout();
            //kdDebug( 6000 ) << "TIME: layout() dt=" << qt.elapsed() << endl;
    } else {
        _width = visibleWidth();
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

    o->printObject( &p , xOff, yOff, visibleWidth(), visibleHeight(),
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

    //kdDebug( 6000 ) << "\nmousePressEvent: x=" << xm << ", y=" << ym << endl;


    // Make this frame the active one
    // ### need some visual indication for the active frame.
    /* ### use PartManager (Simon)
       if ( _isFrame && !_isSelected )
       {
        kdDebug( 6000 ) << "activating frame!" << endl;
        topView()->setFrameSelected(this);
    }*/

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url, innerNode, offset );

    d->underMouse = innerNode;

    khtml::MousePressEvent event( _mouse, xm, ym, url, Node(innerNode), offset );
    QApplication::sendEvent( m_part, &event );
}

void KHTMLView::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    kdDebug( 6000 ) << "mouseDblClickEvent: x=" << xm << ", y=" << ym << endl;

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url, innerNode, offset );

    khtml::MouseDoubleClickEvent event( _mouse, xm, ym, url, Node(innerNode), offset );
    QApplication::sendEvent( m_part, &event );

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

    // execute the scheduled script. This is to make sure the mouseover events come after the mouseout events
    m_part->executeScheduledScript();

    d->underMouse = innerNode;

    QCursor c = KCursor::arrowCursor();
    if ( innerNode ) {
        switch( innerNode->style()->cursor() ) {
        case CURSOR_AUTO:
            if ( url.length() && const_cast<KHTMLSettings *>(m_part->settings())->changeCursor() )
                c = m_part->urlCursor();
            break;
        case CURSOR_CROSS:
            c = KCursor::crossCursor();
            break;
        case CURSOR_POINTER:
            c = m_part->urlCursor();
            break;
        case CURSOR_MOVE:
            c = KCursor::sizeAllCursor();
            break;
        case CURSOR_E_RESIZE:
        case CURSOR_W_RESIZE:
            c = KCursor::sizeHorCursor();
            break;
        case CURSOR_N_RESIZE:
        case CURSOR_S_RESIZE:
            c = KCursor::sizeVerCursor();
            break;
        case CURSOR_NE_RESIZE:
        case CURSOR_SW_RESIZE:
            c = KCursor::sizeBDiagCursor();
            break;
        case CURSOR_NW_RESIZE:
        case CURSOR_SE_RESIZE:
            c = KCursor::sizeFDiagCursor();
            break;
        case CURSOR_TEXT:
            c = KCursor::ibeamCursor();
            break;
        case CURSOR_WAIT:
            c = KCursor::waitCursor();
            break;
        case CURSOR_HELP:
        case CURSOR_DEFAULT:
            break;
        }
    }
    setCursor( c );


    khtml::MouseMoveEvent event( _mouse, xm, ym, url, Node(innerNode), offset );
    QApplication::sendEvent( m_part, &event );
}

void KHTMLView::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !m_part->docImpl() ) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //kdDebug( 6000 ) << "\nmouseReleaseEvent: x=" << xm << ", y=" << ym << endl;

    DOMString url=0;
    NodeImpl *innerNode=0;
    long offset = 0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url, innerNode, offset );

    khtml::MouseReleaseEvent event( _mouse, xm, ym, url, Node(innerNode), offset );
    QApplication::sendEvent( m_part, &event );
}

void KHTMLView::keyPressEvent( QKeyEvent *_ke )
{
//    if(m_part->keyPressHook(_ke)) return;

    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    if (_ke->state()&ShiftButton)
      switch(_ke->key())
	{
	case Key_Space:
	    scrollBy( 0, -clipper()->height() - offs );
	    break;
	}
    else
	switch ( _ke->key() )
	{
	case Key_Down:
	case Key_J:
	    scrollBy( 0, 10 );
	    break;

	case Key_Space:
	    if (d->currentNode)
	    {
	        toggleActLink(false);
	        break;
	    }
	    // no current Node? scroll...
	case Key_Next:
	    scrollBy( 0, clipper()->height() - offs );
	    break;

	case Key_Up:
	case Key_K:
	    scrollBy( 0, -10 );
	    break;

	case Key_Prior:
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
	case Key_Enter:
	case Key_Return:
	    toggleActLink(false);
	    break;
	case Key_Home:
	    setContentsPos( 0, 0 );
	    break;
	case Key_End:
	    setContentsPos( 0, contentsHeight() - height() );
	    break;
	default:
	    //	d->currentNode->keyPressEvent( _ke );
	    return;
	    break;
	}
    _ke->accept();
}

void KHTMLView::keyReleaseEvent( QKeyEvent *_ke )
{
    switch(_ke->key())
    {
    case Key_Enter:
    case Key_Return:
    case Key_Space:
        toggleActLink(true);
        return;
      break;
    }
    //    if(m_part->keyReleaseHook(_ke)) return;
    QScrollView::keyReleaseEvent( _ke);
}

bool KHTMLView::focusNextPrevChild( bool next )
{
    //    return (gotoLink(next) || QScrollView::focusNextPrevChild( next ));
  
    if (!gotoLink(next))
      {
	  bool retval = QScrollView::focusNextPrevChild( next );
	  viewport()->clearFocus();
	  return retval;
      }
    else
      return true;

}

void KHTMLView::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );

    int xm, ym;
    viewportToContents(pos.x(), pos.y(), xm, ym);

    pos = QPoint(pos.x() - viewport()->x(), pos.y() - viewport()->y());
    if ( (pos.y() < 0) || (pos.y() > visibleHeight()) ||
         (pos.x() < 0) || (pos.x() > visibleWidth()) )
    {
        ensureVisible( xm, ym, 0, 5 );
    }
}

DOM::NodeImpl *KHTMLView::nodeUnderMouse() const
{
    return d->underMouse;
}

bool KHTMLView::gotoLink()
{
  return gotoLink(d->currentNode);
}

bool KHTMLView::gotoLink(HTMLElementImpl *n)
{
    kdDebug(6000)<<"                :Link found:"<<n<<"\n";
    if(d->currentNode) d->currentNode->blur();
    if(!n)
	{
	    kdDebug(6000)<<"end of link list reached.\n";
	    d->currentNode = 0;
	    return false;
	}
    if (!n->isSelectable())
	{
	    d->currentNode = 0;
	    return false;
	}
    n->focus();
    if (d->linkPressed)
        n->setKeyboardFocus(DOM::ActivationActive);
    //calculate x- and ypos
    int x = 0, y = 0;
    n->getAnchorPosition(x,y);

    int xe = 0, ye = 0;
    n->getAnchorBounds(xe,ye);

    int deltax;
    int deltay;

    int borderX, borderY;

    borderX = borderY = 30;
    
    // is ypos of target above upper border?
    if (y < contentsY() + borderY)
	{
	    deltay = y - contentsY() - borderY;
	}
    // is ypos of target below lower border:
    else if (ye + borderY > contentsY() + height())
	{
	    deltay = ye + borderY - ( contentsY() + height() );
	}
    else
	deltay = 0;

    // is xpos of target left of the view's border?
    if (x - borderX - contentsX() < 0)
	{
	    deltax = x - contentsX() - borderX;
	}
    // is xpos of target right of the view's right border?
    else if (xe + borderX > contentsX() + width())
	{
	    deltax = xe + borderX - ( contentsX() + width() );
	}
    else
	deltax = 0;

    if (!d->currentNode)
    {
	scrollBy(deltax, deltay);
	d->currentNode = n;
	return true;
    }

    int maxx = width()-borderX;
    int maxy = height()-borderY;

    kdDebug(6000) << "contentsX: " << contentsX() <<" contentsY: "<< contentsY() << " x: " << x << " y: "<< y << " width: " << xe-x << " height: " << ye-y << " deltax: " << deltax << " deltay: " << deltay << " maxx: " << maxx << " maxy: " << maxy << "\n";

    int scrollX,scrollY;

    scrollX = deltax > 0 ? (deltax > maxx ? maxx : deltax) : deltax == 0 ? 0 : (deltax>-maxx ? deltax : -maxx);
    scrollY = deltay > 0 ? (deltay > maxy ? maxy : deltay) : deltay == 0 ? 0 : (deltay>-maxy ? deltay : -maxy);

    scrollBy(scrollX, scrollY);

    kdDebug(6000) << "scrollX:"<<scrollX<<" scrollY:"<<scrollY<<"\n";

    // generate abs(scroll.)
    if (scrollX<0)
	scrollX=-scrollX;
    if (scrollY<0)
	scrollY=-scrollY;

    // only set cursor to new node if scrolling could make
    // the link completely visible
    if ( (scrollX!=maxx) && (scrollY!=maxy) )
    {
	d->currentNode = n;
    }
    return true;
}

bool KHTMLView::gotoLink(bool forward)
{
    if (!m_part->docImpl())
	return false;

    int currentTabIndex =
	(d->currentNode?d->currentNode->tabIndex():-1);

    HTMLElementImpl *n=0;

    // search next link in current scope
    // (scope means either the links without tabindex or with tabindex)

    n = m_part->docImpl()->findLink(d->currentNode, forward, currentTabIndex);

    if (currentTabIndex!=-1 && (!n || n->tabIndex()!=currentTabIndex))
    {
	// found element with different tabindex or nothing:
	// redo search from the beginning matching the current tabindex
	HTMLElementImpl *m = m_part->docImpl()->findLink(0, forward, currentTabIndex);
	if (m && m!=d->currentNode && m->tabIndex()==currentTabIndex)
	    n = m;
    }
    
    if (!n)
    {
	//there is none, so we look for a different item in the whole document.
	kdDebug(6000)<<"reached document border while searching for link. restarting search...";
	int maxTabIndex;
	if (forward && maxTabIndex!=-1)
	    maxTabIndex = 0;
	else
	    maxTabIndex = m_part->docImpl()->findHighestTabIndex();

	if (maxTabIndex!=-1)
	    n = m_part->docImpl()->findLink(0, forward, maxTabIndex);
	else
	    n = 0;
    }

    if (!n)
	kdDebug(6000)<<"...without finding anything. will return false now.\n";
    return gotoLink(n);
}

bool KHTMLView::gotoNextLink()
{ return gotoLink(true); }

bool KHTMLView::gotoPrevLink()
{ return gotoLink(false); }

void KHTMLView::print()
{
    if(!m_part->docImpl()) return;
    khtml::RenderRoot *root = static_cast<khtml::RenderRoot *>(m_part->docImpl()->renderer());
    if(!root) return;

    QPrinter *printer = new QPrinter;
    if(printer->setup(this)) {
        // set up QPrinter
        printer->setFullPage(false);
        printer->setCreator("KDE 2.0 HTML Library");
        //printer->setDocName(m_part->url());

        QPainter *p = new QPainter;
        p->begin( printer );

        QPaintDeviceMetrics metrics( printer );

        // this is a simple approximation... we layout the document
        // according to the width of the page, then just cut
        // pages without caring about the content. We should do better
        // in the future, but for the moment this is better than no
        // printing support
        kdDebug(6000) << "printing: physical page width = " << metrics.width()
                      << " height = " << metrics.height() << endl;
        root->setPrintingMode(true);
        root->setWidth(metrics.width());

        QValueList<int> oldSizes = m_part->fontSizes();

        const int printFontSizes[] = { 6, 7, 8, 10, 12, 14, 18, 24,
                                       28, 34, 40, 48, 56, 68, 82, 100, 0 };
        QValueList<int> fontSizes;
        for ( int i = 0; printFontSizes[i] != 0; i++ )
            fontSizes << printFontSizes[ i ];
        m_part->setFontSizes(fontSizes);
        m_part->docImpl()->applyChanges();

        // ok. now print the pages.
        kdDebug(6000) << "printing: html page width = " << root->docWidth()
                      << " height = " << root->docHeight() << endl;
        kdDebug(6000) << "printing: margins left = " << printer->margins().width()
                      << " top = " << printer->margins().height() << endl;
        kdDebug(6000) << "printing: paper width = " << metrics.width()
                      << " height = " << metrics.height() << endl;
        // if the width is too large to fit on the paper we just scale
        // the whole thing.
        int pageHeight = metrics.height();
        int pageWidth = metrics.width();
        // We print the bottom 'overlap' units again at the top of the next page.
        int overlap = 10;
        p->setClipRect(0,0, pageWidth, pageHeight);
        if(root->docWidth() > metrics.width()) {
            double scale = ((double) metrics.width())/((double) root->docWidth());
            p->scale(scale, scale);
            pageHeight = (int) (pageHeight/scale);
            pageWidth = (int) (pageWidth/scale);
            overlap = (int) (overlap/scale);
        }
        kdDebug(6000) << "printing: scaled html width = " << pageWidth
                      << " height = " << pageHeight << endl;
        int top = 0;
        while(top < root->docHeight()) {
            if(top > 0) printer->newPage();

            root->print(p, 0, top, pageWidth, pageHeight, 0, 0);
            p->translate(0,-(pageHeight-overlap));
            top += (pageHeight-overlap);
        }

        p->end();
        delete p;

        // and now reset the layout to the usual one...
        root->setPrintingMode(false);
        m_part->setFontSizes(oldSizes);
        m_part->docImpl()->applyChanges();
    }
    delete printer;
}

void KHTMLView::toggleActLink(bool actState)
{
    if ( d->currentNode )
    {
        //retrieve url
        HTMLElementImpl *e = static_cast<HTMLElementImpl *>(d->currentNode);
        if (!actState) // inactive->active
        {
            int x,y;
            d->currentNode->setKeyboardFocus(DOM::ActivationActive);
            d->originalNode=d->currentNode;
            d->linkPressed=true;
            e->getAnchorPosition(x,y);
            ensureVisible(x,y);
        }
        else //active->inactive
        {
            e->setKeyboardFocus(DOM::ActivationOff);
            d->linkPressed=false;
            if (d->currentNode==d->originalNode)
            {
	      if (e->id()==ID_A)
		{
		  HTMLAnchorElementImpl *a = static_cast<HTMLAnchorElementImpl *>(d->currentNode);
		  d->currentNode=0;
		  m_part->urlSelected( a->areaHref().string(),
				       LeftButton, 0,
				       a->targetRef().string() );
		}
            }
            d->originalNode=0;
        }
    }
}


void KHTMLView::useSlowRepaints()
{
    kdDebug(0) << "slow repaints requested" << endl;
    d->useSlowRepaints = true;
    setStaticBackground(true);
}


void KHTMLView::setVScrollBarMode ( ScrollBarMode mode )
{
    d->vmode = mode;
    QScrollView::setVScrollBarMode(mode);
}

void KHTMLView::setHScrollBarMode ( ScrollBarMode mode )
{
    d->hmode = mode;
    QScrollView::setHScrollBarMode(mode);
}

void KHTMLView::restoreScrollBar ( )
{
    int ow = visibleWidth();
    QScrollView::setVScrollBarMode(d->vmode);
    if (visibleWidth() != ow)
    {
        layout();
        updateContents(contentsX(),contentsY(),visibleWidth(),visibleHeight());
    }
}

void KHTMLView::setLinkCursor(DOM::HTMLElementImpl *n)
{
  if (lstViews)
  {
      lstViews->first();
      while(lstViews->next())
      {
	  KHTMLView * actView = lstViews->current();
	  if (!actView || !this)
	      kdFatal(6000)<<"no object / subject\n";
	  
	  if (actView != this)
	  {
	      if (actView->d->currentNode && actView->d->currentNode!=n)
		  actView->d->currentNode->blur();
	      actView->d->currentNode = 0;
	  }
	  
      }
  }

  if (d->currentNode != n)
  {
      if (d->currentNode)
	  d->currentNode->blur();
      d->currentNode = n;
      if (n)
      {
	  kdDebug(6000)<<"setLinkCursor to:"<<getTagName(n->id()).string()<<"\n";
	  n->setKeyboardFocus(DOM::ActivationPassive);
	  n->focus();
      }
  }
  d->linkPressed=false;
}
