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

QPixmap* KHTMLView::paintBuffer = 0L;

class KHTMLViewPrivate {
public:
    KHTMLViewPrivate()
    {
        underMouse = 0;
        linkPressed = false;
        useSlowRepaints = false;
        currentNode = 0;
        originalNode= 0;
        tabIndex=0;

    }
    NodeImpl *underMouse;

    NodeImpl *currentNode;
    NodeImpl *originalNode;

    bool linkPressed;
    bool useSlowRepaints;
    short tabIndex;
};


KHTMLView::KHTMLView( KHTMLPart *part, QWidget *parent, const char *name)
    : QScrollView( parent, name, WResizeNoErase | WRepaintNoErase )
{
    m_part = part;

    // initialize QScrollview

    enableClipper(true);

    viewport()->setMouseTracking(true);
    //viewport()->setBackgroundMode(PaletteBase);
    viewport()->setBackgroundMode(NoBackground);

    KImageIO::registerFormats();

    setCursor(arrowCursor);
    init();

    viewport()->show();

    setVScrollBarMode(Auto);
    setHScrollBarMode(Auto);

    d = new KHTMLViewPrivate;
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

    setFocusPolicy(QWidget::StrongFocus);
  viewport()->setFocusPolicy( QWidget::WheelFocus );

  _marginWidth = -1; // undefined
  _marginHeight = -1;
  _width = 0;
  _height = 0;

  resizeContents(visibleWidth(), visibleHeight());
}

void KHTMLView::clear()
{
    resizeContents(visibleWidth(), visibleHeight());
    viewport()->erase();

    setVScrollBarMode(Auto);
    setHScrollBarMode(Auto);

    if(d->useSlowRepaints) {
        delete paintBuffer;
        paintBuffer = 0;
        setStaticBackground(false);
    }

    delete d;
    d = new KHTMLViewPrivate();
}

void KHTMLView::resizeEvent (QResizeEvent* e)
{
    //kdDebug() << "KHTMLView::resizeEvent" << endl;
    int oldWidth = visibleWidth();
    
    QScrollView::resizeEvent(e);

    int w = visibleWidth();
    int h = visibleHeight();

    // only relayout if the width has changed
    if(w != oldWidth)
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
        p->fillRect(ex, ey, ew, eh, palette().normal().brush(QColorGroup::Background));
        return;
    }

    //kdDebug( 6000 ) << "drawContents x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;

    if(!paintBuffer)
        paintBuffer = new QPixmap( visibleWidth(),PAINT_BUFFER_HEIGHT );
    if ( paintBuffer->width() < visibleWidth() )
        paintBuffer->resize(visibleWidth(),PAINT_BUFFER_HEIGHT);

    //QTime qt;
    //   qt.start();

    QPainter tp( paintBuffer );
    tp.translate(-ex, -ey);

    int py=0;
    while (py < eh) {
        int ph = eh-py < PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;

        // ### fix this for frames...
        tp.fillRect(ex, ey+py, ew, ph, palette().normal().brush(QColorGroup::Background));
        m_part->docImpl()->renderer()->print(&tp, ex, ey+py, ew, ph, 0, 0);

        p->drawPixmap(ex, ey+py, *paintBuffer, 0, 0, ew, ph);
        py += PAINT_BUFFER_HEIGHT;

        tp.translate(0, -PAINT_BUFFER_HEIGHT);
    }
//    kdDebug(0) << "repaint time=" << qt.elapsed() << endl;
}

void KHTMLView::layout(bool)
{
    //### take care of frmaes (hide scrollbars,...)


    if( m_part->docImpl() ) {
        DOM::HTMLDocumentImpl *document = m_part->docImpl();

        khtml::RenderRoot* root = static_cast<khtml::RenderRoot *>(document->renderer());

        NodeImpl *body = document->body();
        if(body && body->id() == ID_FRAMESET) {
            setVScrollBarMode(AlwaysOff);
            setHScrollBarMode(AlwaysOff);
            _width = visibleWidth();

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
    case Key_N:
        gotoNextLink();
        break;
    case Key_P:
        gotoPrevLink();
        break;
    case Key_Enter:
    case Key_Return:
        if (!d->linkPressed)
          toggleActLink(false);
        else
          toggleActLink(true);
        break;
    case Key_Home:
        setContentsPos( 0, 0 );
        break;
    case Key_End:
        setContentsPos( 0, contentsHeight() - height() );
        break;
    default:
        QScrollView::keyPressEvent( _ke );
    }
}

void KHTMLView::keyReleaseEvent( QKeyEvent *_ke )
{
    switch(_ke->key())
    {
    case Key_Enter:
        toggleActLink(true);
        return;
      break;
    }
    //    if(m_part->keyReleaseHook(_ke)) return;
    QScrollView::keyReleaseEvent( _ke);
}

bool KHTMLView::focusNextPrevChild( bool next )
{
    return QScrollView::focusNextPrevChild( next );
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
    // let's ignore non anchors for the moment
    if(!d->currentNode || d->currentNode->id() != ID_A) return false;
    HTMLAnchorElementImpl *n=static_cast<HTMLAnchorElementImpl*>(d->currentNode);
    //kdDebug(6000)<<"current item:"<<n->areaHref().string().latin1()<<endl;

  if (d->linkPressed)
    n->setKeyboardFocus(DOM::ActivationActive);
  else
    n->setKeyboardFocus(DOM::ActivationPassive);

//calculate x- and ypos
  int x = 0, y = 0;
  n->getAnchorPosition(x,y);
  //  n->renderer()->absolutePosition();
  ensureVisible(x-30, y+30);
  if (!n->isInline())
    {
      x+=n->renderer()->width();
      y+=n->renderer()->height();
      ensureVisible(x, y);
    }
  return true;
}

bool KHTMLView::gotoNextLink()
{
    if(d->currentNode) d->currentNode->setKeyboardFocus(DOM::ActivationOff);
    if (!m_part->docImpl())
      return false;

    // find next link
    NodeImpl *n = d->currentNode;
    NodeImpl *begin = 0;

    //kdDebug( 6000 ) << "gotoNextLink: old tabindex: "<< d->tabIndex << "\n";

    if (d->tabIndex!=-1)
      {
        d->tabIndex++;
        if (d->tabIndex>m_part->docImpl()->findHighestTabIndex())
          {
            d->tabIndex=-1;
            n=0;
          }
      }

    //kdDebug( 6000 ) << "gotoNextLink: current tabindex: "<< d->tabIndex << "\n";

    if(!n) n = m_part->docImpl()->body();
    while(n) {
        // find next Node
        if(n->firstChild())
            n = n->firstChild();
        else if (n->nextSibling())
            n = n->nextSibling();
        else {
          NodeImpl *next = n->parentNode();
          bool wrap=true;
          while(next)
            {
              n=next;
              if (n->nextSibling())
                {
                  n=n->nextSibling();
                  next=0;
                  wrap=false;
                }
              else
                {
                  next=n->parentNode();
                }
            }
          if (wrap)
            {
              //        kdDebug(6000)<<"wrapped around document border.\n";
              if (d->tabIndex==-1)
                d->tabIndex++;
              else if (m_part->docImpl()->findHighestTabIndex()<d->tabIndex)
                  return false;
            }
        }
        //kdDebug( 6000 ) << "gotoPrevLink: in-between tabindex: "<< d->tabIndex << "\n";

        if(n->id() == ID_A && ((static_cast<HTMLElementImpl *>(n))->getAttribute(ATTR_HREF).length()))
          {
            //here, additional constraints for the previous link are checked.
            HTMLAreaElementImpl *a=static_cast<HTMLAreaElementImpl *>(n);
            if ((a->tabIndex()==d->tabIndex))
              {
                d->currentNode = n;
                //kdDebug( 6000 ) << "gotoNextLink: new tabindex: "<< d->tabIndex << "\n";

                return gotoLink();
              }
            else if (!begin)
              {
                begin=n;
              }
            else if (begin==n)
              {
                if (d->tabIndex<=m_part->docImpl()->findHighestTabIndex())
                  d->tabIndex++;
                if (d->tabIndex>m_part->docImpl()->findHighestTabIndex())
                  {
                    //kdDebug(6000) << "\n gotoNextLink: last tabindex "<<d->tabIndex<<" reached. now processing non-tabindex-elements\n";
                    d->tabIndex=-1;
                  }
              }
        }
    }
    return false;
}

bool KHTMLView::gotoPrevLink()
{
    if(d->currentNode) d->currentNode->setKeyboardFocus(DOM::ActivationOff);

    if (!(m_part->docImpl()))
      {
        //kdDebug(6000)<<"gotoPrevLink: No Document!!\n";
        return false;
      }

    //kdDebug( 6000 ) << "gotoPrevLink: old tabindex: "<< d->tabIndex << "\n";

    // find next link
    NodeImpl *n = d->currentNode;
    NodeImpl *begin=0;

    if(d->tabIndex!=-1)
      {
        d->tabIndex--;
        if (d->tabIndex==-1)
          {
            n=0;
            //kdDebug(6000)<< "tabindex wrapped backwards\n";
          }
      }

    //    kdDebug( 6000 ) << "gotoPrevLink: current tabindex: "<< d->tabIndex << "\n";


    if(!n) n = m_part->docImpl()->body();

    while(n) {
        // find next Node
        if(n->lastChild())
            n = n->lastChild();
        else if (n->previousSibling())
            n = n->previousSibling();
        else {
            NodeImpl *prev = n->parentNode();
            bool wrap=true;
            while(prev) {
                n=prev;
                if (n->previousSibling())
                  {
                    n=n->previousSibling();
                    prev=0;
                    wrap=false;
                  }
                else
                  prev=n->parentNode();
            }
            if (wrap)
              {
                //              kdDebug(6000)<<"wrapped from "<< d->tabIndex <<endl;
                if (d->tabIndex==-1)
                  {
                    d->tabIndex=m_part->docImpl()->findHighestTabIndex();
                    //              kdDebug"to "<< d->tabIndex<<endl;
                    if (d->tabIndex!=-1)
                      begin=0L;
                  }
              }
        }
        // ### add handling of form elements here!
        if((n->id() == ID_A)&&((static_cast<HTMLElementImpl *>(n)->getAttribute(ATTR_HREF).length()))) {
            //here, additional constraints for the previous link are checked.
            HTMLAreaElementImpl *a=static_cast<HTMLAreaElementImpl *>(n);
            if (a->tabIndex()==d->tabIndex)
              {
                d->currentNode = n;
                //kdDebug( 6000 ) << "gotoPrevLink: new tabindex: "<< d->tabIndex << "\n";

                return gotoLink();
              }
        }
        if (!begin)
          {
            begin=n;
            //    kdDebug(6000)<<"marked "<< d->tabIndex<<" as the beginning of search"<<endl;
          }
        else if (begin==n)
          {
            if (d->tabIndex>=0)
              {
                d->tabIndex--;
              }
            else
              {
                d->tabIndex=m_part->docImpl()->findHighestTabIndex();
                //      kdDebug(6000)<<"end of non-tabindex-link-mode. restarting search\n";
                if (d->tabIndex==-1)
                  return false;
              }
          }
    }
    return false;
}

void KHTMLView::print()
{
    if(!m_part->docImpl()) return;
    khtml::RenderRoot *root = static_cast<khtml::RenderRoot *>(m_part->docImpl()->renderer());
    if(!root) return;

    QPrinter *printer = new QPrinter;
    if(printer->setup(this)) {
        // set up QPrinter
        printer->setFullPage(true);
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
        kdDebug(6000) << "printing: html page width = " << root->width()
                      << " height = " << root->height() << endl;
        // if the width is too large to fit on the paper we just scale
        // the whole thing.
        int pageHeight = metrics.height();
        int pageWidth = metrics.width();
        if(root->width() > metrics.width()) {
            double scale = ((double) metrics.width())/((double) root->width());
            p->scale(scale, scale);
            pageHeight = (int) (pageHeight/scale);
            pageWidth = (int) (pageWidth/scale);
        }
        int top = 0;
        while(top < root->height()) {
            if(top > 0) printer->newPage();
            root->print(p, 0, top, pageWidth, pageHeight, 0, 0);
            p->translate(0,-pageHeight);
            top += pageHeight;
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
      HTMLAnchorElementImpl *n = static_cast<HTMLAnchorElementImpl *>(d->currentNode);
      if (!actState) // inactive->active
        {
          int x,y;
          d->currentNode->setKeyboardFocus(DOM::ActivationActive);
          d->originalNode=d->currentNode;
          d->linkPressed=true;
          n->getAnchorPosition(x,y);
          ensureVisible(x,y);
        }
      else //active->inactive
        {
          n->setKeyboardFocus(DOM::ActivationOff);
          d->linkPressed=false;
          if (d->currentNode==d->originalNode)
            {
              d->currentNode=0;
              m_part->urlSelected( n->areaHref().string(),
                                   LeftButton, 0,
                                   n->targetRef().string() );
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

