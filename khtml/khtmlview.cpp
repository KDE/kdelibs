/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Dirk Mueller <mueller@kde.org>
 *                     2003 Leo Savernik <l.savernik@aon.at>
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

#include "khtmlview.h"

#include "khtml_part.h"
#include "khtml_events.h"

#include "html/html_documentimpl.h"
#include "html/html_inlineimpl.h"
#include "rendering/render_arena.h"
#include "rendering/render_canvas.h"
#include "rendering/render_frames.h"
#include "rendering/render_replaced.h"
#include "rendering/render_layer.h"
#include "rendering/render_line.h"
#include "rendering/render_table.h"
// removeme
#define protected public
#include "rendering/render_text.h"
#undef protected
#include "xml/dom2_eventsimpl.h"
#include "css/cssstyleselector.h"
#include "misc/htmlhashes.h"
#include "misc/helper.h"
#include "khtml_settings.h"
#include "khtml_printsettings.h"

#include "khtmlpart_p.h"

#include <kcursor.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <kstandarddirs.h>
#include <kprinter.h>
#include <klocale.h>

#include <qtooltip.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>
#include <kapplication.h>

#include <kimageio.h>
#include <kdebug.h>
#include <kurldrag.h>
#include <qobjectlist.h>
#include <qtimer.h>
#include <kdialogbase.h>
#include <qptrdict.h>

#define PAINT_BUFFER_HEIGHT 128

using namespace DOM;
using namespace khtml;
class KHTMLToolTip;

#ifndef QT_NO_TOOLTIP

class KHTMLToolTip : public QToolTip
{
public:
    KHTMLToolTip(KHTMLView *view,  KHTMLViewPrivate* vp) : QToolTip(view->viewport())
    {
        m_view = view;
        m_viewprivate = vp;
    };

protected:
    virtual void maybeTip(const QPoint &);

private:
    KHTMLView *m_view;
    KHTMLViewPrivate* m_viewprivate;
};

#endif

#ifndef KHTML_NO_CARET
/** contextual information about the caret which is related to the view.
 * An object of this class is only instantiated when it is needed.
 */
struct CaretViewContext {
    int freqTimerId;		// caret blink frequency timer id
    int x, y;			// caret position in viewport coordinates
    				// (y specifies the top, not the baseline)
    int width;			// width of caret in pixels
    int height;			// height of caret in pixels
    bool visible;		// true if currently visible.
    bool displayed;		// true if caret is to be displayed at all.
    bool caretMoved;		// set to true once caret has been moved in page

    /** For natural traversal of lines, the original x position is saved, and
     * the actual x is set to the first character whose x position is
     * greater than origX.
     *
     * origX is reset to x whenever the caret is moved horizontally or placed
     * by the mouse.
     */
    int origX;

    CaretViewContext() : freqTimerId(-1), x(0), y(0), width(1), height(16),
    	visible(true), displayed(false), caretMoved(false), origX(0)
    {}
};

/** contextual information about the editing state.
 * An object of this class is only instantiated when it is needed.
 */
struct EditorContext {
    bool override;		// true if typed characters should override
    				// the existing ones.

    EditorContext() : override(false)
    {}
};
#endif // KHTML_NO_CARET

class KHTMLViewPrivate {
    friend class KHTMLToolTip;
public:
    KHTMLViewPrivate()
        : underMouse( 0 )
    {
#ifndef KHTML_NO_CARET
	m_caretViewContext = 0;
	m_editorContext = 0;
#endif // KHTML_NO_CARET
        reset();
        tp=0;
        paintBuffer=0;
        vertPaintBuffer=0;
        formCompletions=0;
        prevScrollbarVisible = true;
	tooltip = 0;
        possibleTripleClick = false;
    }
    ~KHTMLViewPrivate()
    {
        delete formCompletions;
        delete tp; tp = 0;
        delete paintBuffer; paintBuffer =0;
        delete vertPaintBuffer;
        if (underMouse)
	    underMouse->deref();
	delete tooltip;
#ifndef KHTML_NO_CARET
	delete m_caretViewContext;
	delete m_editorContext;
#endif // KHTML_NO_CARET
    }
    void reset()
    {
        if (underMouse)
	    underMouse->deref();
	underMouse = 0;
        linkPressed = false;
        useSlowRepaints = false;
        originalNode = 0;
	borderTouched = false;
#ifndef KHTML_NO_SCROLLBARS
        vmode = QScrollView::Auto;
        hmode = QScrollView::Auto;
#else
        vmode = QScrollView::AlwaysOff;
        hmode = QScrollView::AlwaysOff;
#endif
        scrollBarMoved = false;
        ignoreWheelEvents = false;
	borderX = 30;
	borderY = 30;
	clickX = -1;
	clickY = -1;
        prevMouseX = -1;
        prevMouseY = -1;
	clickCount = 0;
	isDoubleClick = false;
	scrollingSelf = false;
	layoutTimerId = 0;
        repaintTimerId = 0;
        scrollTimerId = 0;
        complete = false;
        firstRelayout = true;
        dirtyLayout = false;
        layoutSchedulingEnabled = true;
        repaintLayout = false;
        updateRect = QRect();
        m_dialogsAllowed = true;
#ifndef KHTML_NO_CARET
        if (m_caretViewContext) {
          m_caretViewContext->caretMoved = false;
        }/*end if*/
#endif // KHTML_NO_CARET
    }
    void newScrollTimer(QWidget *view, int tid)
    {
        kdDebug() << "newScrollTimer timer" << tid << endl;
        view->killTimer(scrollTimerId);
        scrollTimerId = tid;
    }
    enum ScrollDirection { ScrollLeft, ScrollRight, ScrollUp, ScrollDown };

    void adjustScroller(QWidget *view, ScrollDirection direction, ScrollDirection oppositedir)
    {
        static const struct { int msec, pixels; } timings [] = {
            {320,1}, {224,1}, {160,1}, {112,1}, {80,1}, {56,1}, {40,1},
            {28,1}, {20,1}, {20,2}, {20,3}, {20,4}, {20,6}, {20,8}, {0,0}
        };
        if (!scrollTimerId ||
            (scrollDirection != direction &&
             scrollDirection != oppositedir)) {
            scrollTiming = 6;
            scrollBy = timings[scrollTiming].pixels;
            scrollDirection = direction;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == direction &&
                   timings[scrollTiming+1].msec) {
            scrollBy = timings[++scrollTiming].pixels;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == oppositedir) {
            if (scrollTiming) {
                scrollBy = timings[--scrollTiming].pixels;
                newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
            }
        }
    }

#ifndef KHTML_NO_CARET
    /** this function returns an instance of the caret view context. If none
     * exists, it will be instantiated.
     */
    CaretViewContext *caretViewContext() {
      if (!m_caretViewContext) m_caretViewContext = new CaretViewContext();
      return m_caretViewContext;
    }
    /** this function returns an instance of the editor context. If none
     * exists, it will be instantiated.
     */
    EditorContext *editorContext() {
      if (!m_editorContext) m_editorContext = new EditorContext();
      return m_editorContext;
    }
#endif // KHTML_NO_CARET

    QPainter *tp;
    QPixmap  *paintBuffer;
    QPixmap  *vertPaintBuffer;
    NodeImpl *underMouse;

    // the node that was selected when enter was pressed
    NodeImpl *originalNode;

    bool borderTouched:1;
    bool borderStart:1;
    bool scrollBarMoved:1;

    QScrollView::ScrollBarMode vmode;
    QScrollView::ScrollBarMode hmode;
    bool prevScrollbarVisible;
    bool linkPressed;
    bool useSlowRepaints;
    bool ignoreWheelEvents;

    int borderX, borderY;
    KSimpleConfig *formCompletions;

    int clickX, clickY, clickCount;
    bool isDoubleClick;

    int prevMouseX, prevMouseY;
    bool scrollingSelf;
    int layoutTimerId;

    int repaintTimerId;
    int scrollTimerId;
    int scrollTiming;
    int scrollBy;
    ScrollDirection scrollDirection;
    bool complete;
    bool firstRelayout;
    bool layoutSchedulingEnabled;
    bool possibleTripleClick;
    bool dirtyLayout;
    bool repaintLayout;
    bool m_dialogsAllowed;
    QRect updateRect;
    KHTMLToolTip *tooltip;
    QPtrDict<QWidget> visibleWidgets;
#ifndef KHTML_NO_CARET
    CaretViewContext *m_caretViewContext;
    EditorContext *m_editorContext;
#endif // KHTML_NO_CARET
};

// == caret-related helper functions

#ifndef KHTML_NO_CARET

// defined in khtml_part.cpp
bool isBeforeNode(DOM::Node node1, DOM::Node node2);

namespace khtml {

/** Make sure the given node is a leaf node. */
inline void ensureLeafNode(NodeImpl *&node) {
  if (node && node->hasChildNodes()) node = node->nextLeafNode();
}

/** Finds the next node that has a renderer.
 *
 * Note that if the initial @p node has a renderer, this will be returned,
 * regardless of being a leaf node.
 * Otherwise, for the next nodes, only leaf nodes are considered.
 * @param node node to start with, will be updated accordingly
 * @return renderer or 0 if no following node has a renderer.
 */
inline RenderObject *findRenderer(NodeImpl *&node) {
  if (!node) return 0;
  RenderObject *r = node->renderer();
  while (!r) {
    node = node->nextLeafNode();
    if (!node) break;
    r = node->renderer();
  }
  return r;
}

/** Bring caret information position into a sane state */
static void sanitizeCaretState(NodeImpl *&caretNode, long &offset) {
  ensureLeafNode(caretNode);

  // FIXME: this leaves caretNode untouched if there are no more renderers.
  // It better should search backwards then.
  // This still won't solve the problem what to do if *no* element has a
  // renderer.
  NodeImpl *tmpNode = caretNode;
  if (findRenderer(tmpNode)) caretNode = tmpNode;

  long max = caretNode->maxOffset();
  long min = caretNode->minOffset();
  if (offset < min) offset = min;
  else if (offset > max) offset = max;
}

/** Returns the previous render object that is a leaf object. */
static RenderObject *prevLeafRenderObject(RenderObject *r) {
  RenderObject *n = r->objectAbove();
  while (n && n == r->parent()) {
    if (n->previousSibling()) return n->objectAbove();
    r = n;
    n = r->parent();
  }
  return n;
}

/** Returns the following render object that is a leaf object. */
static RenderObject *nextLeafRenderObject(RenderObject *r) {
  RenderObject *n = r->objectBelow();
  r = n;
  while (n) r = n, n = n->firstChild();
  return r;
}

/** Seeks the next leaf inline box.
 */
inline InlineBox *seekLeafInlineBox(InlineBox *box) {
  while (box && box->isInlineFlowBox()) {
//    if (box->isInlineFlowBox()) {
      box = static_cast<InlineFlowBox *>(box)->firstChild();
//    else if (box->object()->isFlow())
//      box = static_cast<RenderFlow *>(box->object())->firstLineBox();
//    else
//      break;
  }/*wend*/
  return box;
}

/** Seeks the next leaf inline box, beginning from the end.
 */
inline InlineBox *seekLeafInlineBoxFromEnd(InlineBox *box) {
  while (box && box->isInlineFlowBox()) {
      box = static_cast<InlineFlowBox *>(box)->lastChild();
  }/*wend*/
  kdDebug(6200) << "seekLeafFromEnd: box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;
  return box;
}

#if 0
/** Traverses up to the next leaf inline box.
 * @return leaf or 0 if there are no more
 */
static InlineBox *nextLeafInlineBox(InlineBox *box) {
  return 0;
}

/** Traverses up to the previous leaf inline box.
 * @return leaf or 0 if there are no more
 */
static InlineBox *prevLeafInlineBox(InlineBox *box) {
  return 0;
}
#endif

}/*end namespace*/

// == end caret-related
#endif // KHTML_NO_CARET

#ifndef QT_NO_TOOLTIP

void KHTMLToolTip::maybeTip(const QPoint& p)
{
    DOM::NodeImpl *node = m_viewprivate->underMouse;
    QRect region;
    while ( node ) {
        if ( node->isElementNode() ) {
            QString s = static_cast<DOM::ElementImpl*>( node )->getAttribute( ATTR_TITLE ).string();
            region |= QRect( m_view->contentsToViewport( node->getRect().topLeft() ), node->getRect().size() );
            if ( !s.isEmpty() ) {
                tip( region, QStyleSheet::convertFromPlainText( s, QStyleSheetItem::WhiteSpaceNormal ) );
                break;
            }
        }
        node = node->parentNode();
    }
}
#endif

KHTMLView::KHTMLView( KHTMLPart *part, QWidget *parent, const char *name)
    : QScrollView( parent, name, WResizeNoErase | WRepaintNoErase )
{
    m_medium = "screen";

    m_part = part;
    d = new KHTMLViewPrivate;
    QScrollView::setVScrollBarMode(d->vmode);
    QScrollView::setHScrollBarMode(d->hmode);
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteChanged()));
    connect(this, SIGNAL(contentsMoving(int, int)), this, SLOT(slotScrollBarMoved()));

    // initialize QScrollView
    enableClipper(true);
    // hack to get unclipped painting on the viewport.
    static_cast<KHTMLView *>(static_cast<QWidget *>(viewport()))->setWFlags(WPaintUnclipped);

    setResizePolicy(Manual);
    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    KImageIO::registerFormats();

#ifndef QT_NO_TOOLTIP
    d->tooltip = new KHTMLToolTip( this, d );
#endif

    init();

    viewport()->show();
}

KHTMLView::~KHTMLView()
{
    closeChildDialogs();
    if (m_part)
    {
        //WABA: Is this Ok? Do I need to deref it as well?
        //Does this need to be done somewhere else?
        DOM::DocumentImpl *doc = m_part->xmlDocImpl();
        if (doc)
            doc->detach();
    }
    delete d; d = 0;
}

void KHTMLView::init()
{
    if(!d->paintBuffer) d->paintBuffer = new QPixmap(PAINT_BUFFER_HEIGHT, PAINT_BUFFER_HEIGHT);
    if(!d->vertPaintBuffer)
        d->vertPaintBuffer = new QPixmap(10, PAINT_BUFFER_HEIGHT);
    if(!d->tp) d->tp = new QPainter();

    setFocusPolicy(QWidget::StrongFocus);
    viewport()->setFocusPolicy( QWidget::WheelFocus );
    viewport()->setFocusProxy(this);

    _marginWidth = -1; // undefined
    _marginHeight = -1;
    _width = 0;
    _height = 0;

    installEventFilter(this);

    setAcceptDrops(true);
    QSize s = viewportSize(4095, 4095);
    resizeContents(s.width(), s.height());
}

void KHTMLView::clear()
{
    // work around QScrollview's unbelievable bugginess
    setStaticBackground(true);
#ifndef KHTML_NO_CARET
    if (!m_part->isCaretMode() && !m_part->isEditable()) caretOff();
#endif

    d->reset();
    killTimers();
    emit cleared();

    QScrollView::setHScrollBarMode(d->hmode);
    QScrollView::setVScrollBarMode(d->vmode);
}

void KHTMLView::hideEvent(QHideEvent* e)
{
    QScrollView::hideEvent(e);
}

void KHTMLView::showEvent(QShowEvent* e)
{
    QScrollView::showEvent(e);
}

void KHTMLView::resizeEvent (QResizeEvent* e)
{
    QScrollView::resizeEvent(e);

    if ( m_part && m_part->xmlDocImpl() )
        m_part->xmlDocImpl()->dispatchWindowEvent( EventImpl::RESIZE_EVENT, false, false );
}

void KHTMLView::viewportResizeEvent (QResizeEvent* e)
{
    QScrollView::viewportResizeEvent(e);

    //int w = visibleWidth();
    //int h = visibleHeight();

    if (d->layoutSchedulingEnabled)
        layout();
#ifndef KHTML_NO_CARET
    else {
        hideCaret();
        recalcAndStoreCaretPos();
	showCaret();
    }/*end if*/
#endif

    KApplication::sendPostedEvents(viewport(), QEvent::Paint);
}

// this is to get rid of a compiler virtual overload mismatch warning. do not remove
void KHTMLView::drawContents( QPainter*)
{
}

void KHTMLView::drawContents( QPainter *p, int ex, int ey, int ew, int eh )
{
//    kdDebug( 6000 ) << "drawContents x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;
    if(!m_part || !m_part->xmlDocImpl() || !m_part->xmlDocImpl()->renderer()) {
        p->fillRect(ex, ey, ew, eh, palette().active().brush(QColorGroup::Base));
        return;
    }

    if (eh > PAINT_BUFFER_HEIGHT && ew <= 10) {
        if ( d->vertPaintBuffer->height() < visibleHeight() )
            d->vertPaintBuffer->resize(10, visibleHeight());
        d->tp->begin(d->vertPaintBuffer);
        d->tp->translate(-ex, -ey);
        d->tp->fillRect(ex, ey, ew, eh, palette().active().brush(QColorGroup::Base));
        m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, ex, ey, ew, eh, 0, 0);
        d->tp->end();
        p->drawPixmap(ex, ey, *d->vertPaintBuffer, 0, 0, ew, eh);
    }
    else {
        if ( d->paintBuffer->width() < visibleWidth() )
            d->paintBuffer->resize(visibleWidth(),PAINT_BUFFER_HEIGHT);

        int py=0;
        while (py < eh) {
            int ph = eh-py < PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;
            d->tp->begin(d->paintBuffer);
            d->tp->translate(-ex, -ey-py);
            d->tp->fillRect(ex, ey+py, ew, ph, palette().active().brush(QColorGroup::Base));
            m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, ex, ey+py, ew, ph, 0, 0);
#ifdef BOX_DEBUG
            if (m_part->xmlDocImpl()->focusNode())
            {
                d->tp->setBrush(Qt::NoBrush);
                d->tp->drawRect(m_part->xmlDocImpl()->focusNode()->getRect());
            }
#endif
            d->tp->end();

            p->drawPixmap(ex, ey+py, *d->paintBuffer, 0, 0, ew, ph);
            py += PAINT_BUFFER_HEIGHT;
        }
    }

#ifndef KHTML_NO_CARET
    if (d->m_caretViewContext && d->m_caretViewContext->visible) {
        QRect pos(d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width, d->m_caretViewContext->height);
        if (pos.intersects(QRect(ex, ey, ew, eh))) {
            p->setRasterOp(XorROP);
	    p->setPen(white);
	    if (pos.height() == 1)
              p->drawLine(pos.topLeft(), pos.bottomRight());
	    else {
	      p->fillRect(pos, white);
	    }/*end if*/
	}/*end if*/
    }/*end if*/
#endif // KHTML_NO_CARET

    khtml::DrawContentsEvent event( p, ex, ey, ew, eh );
    QApplication::sendEvent( m_part, &event );

}

void KHTMLView::setMarginWidth(int w)
{
    // make it update the rendering area when set
    _marginWidth = w;
}

void KHTMLView::setMarginHeight(int h)
{
    // make it update the rendering area when set
    _marginHeight = h;
}

void KHTMLView::layout()
{
    d->layoutSchedulingEnabled=false;

    if( m_part && m_part->xmlDocImpl() ) {
        DOM::DocumentImpl *document = m_part->xmlDocImpl();

        khtml::RenderCanvas* root = static_cast<khtml::RenderCanvas *>(document->renderer());
        if ( !root ) return;

         if (document->isHTMLDocument()) {
             NodeImpl *body = static_cast<HTMLDocumentImpl*>(document)->body();
             if(body && body->renderer() && body->id() == ID_FRAMESET) {
                 QScrollView::setVScrollBarMode(AlwaysOff);
                 QScrollView::setHScrollBarMode(AlwaysOff);
                 body->renderer()->setLayouted(false);
//                  if (d->tooltip) {
//                      delete d->tooltip;
//                      d->tooltip = 0;
//                  }
             }
             else if (!d->tooltip)
                 d->tooltip = new KHTMLToolTip( this, d );
         }

        _height = visibleHeight();
        _width = visibleWidth();
        //QTime qt;
        //qt.start();
        root->setMinMaxKnown(false);
        root->setLayouted(false);
        root->layout();
#ifndef KHTML_NO_CARET
        hideCaret();
        if ((m_part->isCaretMode() || m_part->isEditable())
        	&& !d->complete && d->m_caretViewContext
                && !d->m_caretViewContext->caretMoved) {
            initCaret();
        } else {
	    recalcAndStoreCaretPos();
	    showCaret();
        }/*end if*/
#endif
        if( d->repaintLayout )
          root->repaint();
        //kdDebug( 6000 ) << "TIME: layout() dt=" << qt.elapsed() << endl;
    }
    else
       _width = visibleWidth();

    killTimer(d->layoutTimerId);
    d->layoutTimerId = 0;
    d->layoutSchedulingEnabled=true;
    d->repaintLayout = false;
}

void KHTMLView::closeChildDialogs()
{
    QObjectList *dlgs = queryList("QDialog");
    for (QObject *dlg = dlgs->first(); dlg; dlg = dlgs->next())
    {
        KDialogBase* dlgbase = dynamic_cast<KDialogBase *>( dlg );
        if ( dlgbase ) {
            kdDebug(6000) << "closeChildDialogs: closing dialog " << dlgbase << endl;
            // close() ends up calling QButton::animateClick, which isn't immediate
            // we need something the exits the event loop immediately (#49068)
            dlgbase->cancel();
        }
        else
        {
            kdWarning() << "closeChildDialogs: not a KDialogBase! Don't use QDialogs in KDE! " << static_cast<QWidget*>(dlg) << endl;
            static_cast<QWidget*>(dlg)->hide();
        }
    }
    delete dlgs;
    d->m_dialogsAllowed = false;
}

bool KHTMLView::dialogsAllowed() {
    bool allowed = d->m_dialogsAllowed;
    KHTMLPart* p = m_part->parentPart();
    if (p && p->view())
        allowed &= p->view()->dialogsAllowed();
    return allowed;
}

void KHTMLView::closeEvent( QCloseEvent* ev )
{
    closeChildDialogs();
    QScrollView::closeEvent( ev );
}

//
// Event Handling
//
/////////////////

void KHTMLView::viewportMousePressEvent( QMouseEvent *_mouse )
{
    if(!m_part->xmlDocImpl()) return;
    if (d->possibleTripleClick)
    {
        viewportMouseDoubleClickEvent( _mouse ); // it handles triple clicks too
        return;
    }

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //kdDebug( 6000 ) << "\nmousePressEvent: x=" << xm << ", y=" << ym << endl;

    d->isDoubleClick = false;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MousePress );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    if (d->clickCount > 0 &&
        QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= QApplication::startDragDistance())
	d->clickCount++;
    else {
	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;
    }

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MousePress);
    if (mev.innerNode.handle())
	mev.innerNode.handle()->setPressed();

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	khtml::MousePressEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
	QApplication::sendEvent( m_part, &event );

	emit m_part->nodeActivated(mev.innerNode);
    }
}

void KHTMLView::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!m_part->xmlDocImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    kdDebug( 6000 ) << "mouseDblClickEvent: x=" << xm << ", y=" << ym << endl;

    d->isDoubleClick = true;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    // We do the same thing as viewportMousePressEvent() here, since the DOM does not treat
    // single and double-click events as separate (only the detail, i.e. number of clicks differs)
    if (d->clickCount > 0 && d->clickX == xm && d->clickY == ym) // ### support mouse threshold
	d->clickCount++;
    else {
	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;
    }
    kdDebug() << "KHTMLView::viewportMouseDoubleClickEvent clickCount=" << d->clickCount << endl;
    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MouseDblClick);

    if (mev.innerNode.handle())
	mev.innerNode.handle()->setPressed();

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	khtml::MouseDoubleClickEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode, d->clickCount );
	QApplication::sendEvent( m_part, &event );
    }

    d->possibleTripleClick=true;
    QTimer::singleShot(QApplication::doubleClickInterval(),this,SLOT(tripleClickTimeout()));
}

void KHTMLView::tripleClickTimeout()
{
    d->possibleTripleClick = false;
    d->clickCount = 0;
}

void KHTMLView::viewportMouseMoveEvent( QMouseEvent * _mouse )
{

    if(!m_part->xmlDocImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseMove );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEMOVE_EVENT,mev.innerNode.handle(),false,
                                           0,_mouse,true,DOM::NodeImpl::MouseMove);

    if (d->clickCount > 0 &&
        QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() > QApplication::startDragDistance()) {
	d->clickCount = 0;  // moving the mouse outside the threshold invalidates the click
    }

    // execute the scheduled script. This is to make sure the mouseover events come after the mouseout events
    m_part->executeScheduledScript();

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    khtml::RenderStyle* style = (r && r->style()) ? r->style() : 0;
    QCursor c;
    switch ( style ? style->cursor() : CURSOR_AUTO) {
    case CURSOR_AUTO:
        if ( mev.url.length() && m_part->settings()->changeCursor() )
            c = m_part->urlCursor();

        if (r && r->isFrameSet() && !static_cast<RenderFrameSet*>(r)->noResize())
            c = QCursor(static_cast<RenderFrameSet*>(r)->cursorShape());

        break;
    case CURSOR_CROSS:
        c = KCursor::crossCursor();
        break;
    case CURSOR_POINTER:
        c = m_part->urlCursor();
        break;
    case CURSOR_PROGRESS:
        c = KCursor::workingCursor();
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
        c = KCursor::whatsThisCursor();
        break;
    case CURSOR_DEFAULT:
        break;
    }

    if ( viewport()->cursor().handle() != c.handle() ) {
        if( c.handle() == KCursor::arrowCursor().handle()) {
            for (KHTMLPart* p = m_part; p; p = p->parentPart())
                p->view()->viewport()->unsetCursor();
        }
        else {
            viewport()->setCursor( c );
	}
    }
    if (r && r->isWidget()) {
// 	static_cast<RenderWidget *>(r)->widget()->setCursor(c);
	_mouse->ignore();
    }


    d->prevMouseX = xm;
    d->prevMouseY = ym;

    if (!swallowEvent) {
        khtml::MouseMoveEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
        QApplication::sendEvent( m_part, &event );
    }
}

void KHTMLView::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !m_part->xmlDocImpl() ) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    kdDebug( 6000 ) << "\nmouseReleaseEvent: x=" << xm << ", y=" << ym << endl;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseRelease );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

//     // Qt bug: sometimes Qt sends us events that should be sent
//     // to the widget instead
//     if ( mev.innerNode.handle() && mev.innerNode.handle()->renderer() &&
//          mev.innerNode.handle()->renderer()->isWidget() )
//         return;

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEUP_EVENT,mev.innerNode.handle(),true,
                                           d->clickCount,_mouse,false,DOM::NodeImpl::MouseRelease);

    if (d->clickCount > 0 &&
        QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= QApplication::startDragDistance())
	dispatchMouseEvent(EventImpl::CLICK_EVENT,mev.innerNode.handle(),true,
                           d->clickCount,_mouse,true,DOM::NodeImpl::MouseRelease);

    if (mev.innerNode.handle())
	mev.innerNode.handle()->setPressed(false);

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	khtml::MouseReleaseEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
	QApplication::sendEvent( m_part, &event );
    }
}

void KHTMLView::keyPressEvent( QKeyEvent *_ke )
{

#ifndef KHTML_NO_CARET
    if (m_part->isEditable() || m_part->isCaretMode()
        || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
	    && m_part->xmlDocImpl()->focusNode()->contentEditable())) {
      caretKeyPressEvent(_ke);
      return;
    }
#endif // KHTML_NO_CARET

    if (m_part->xmlDocImpl()) {
        if (m_part->xmlDocImpl()->focusNode())
            if (m_part->xmlDocImpl()->focusNode()->dispatchKeyEvent(_ke))
            {
                _ke->accept();
                return;
            }
        if (!_ke->text().isNull() && m_part->xmlDocImpl()->getHTMLEventListener(EventImpl::KHTML_KEYDOWN_EVENT))
            if (m_part->xmlDocImpl()->documentElement()->dispatchKeyEvent(_ke))
            {
                _ke->accept();
                return;
            }
    }

    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    if (_ke->state() & Qt::ShiftButton)
      switch(_ke->key())
        {
        case Key_Space:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else
                scrollBy( 0, -clipper()->height() - offs );
            break;

        case Key_Down:
        case Key_J:
            d->adjustScroller(this, KHTMLViewPrivate::ScrollDown, KHTMLViewPrivate::ScrollUp);
            break;

        case Key_Up:
        case Key_K:
            d->adjustScroller(this, KHTMLViewPrivate::ScrollUp, KHTMLViewPrivate::ScrollDown);
            break;

        case Key_Left:
        case Key_H:
            d->adjustScroller(this, KHTMLViewPrivate::ScrollLeft, KHTMLViewPrivate::ScrollRight);
            break;

        case Key_Right:
        case Key_L:
            d->adjustScroller(this, KHTMLViewPrivate::ScrollRight, KHTMLViewPrivate::ScrollLeft);
            break;
        }
    else
        switch ( _ke->key() )
        {
        case Key_Down:
        case Key_J:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, 10 );
            }
            break;

        case Key_Space:
        case Key_Next:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else
                scrollBy( 0, clipper()->height() - offs );
            break;

        case Key_Up:
        case Key_K:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, -10 );
            }
            break;

        case Key_Prior:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else
                scrollBy( 0, -clipper()->height() + offs );
            break;
        case Key_Right:
        case Key_L:
            if ( d->hmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 10, 0 );
            }
            break;
        case Key_Left:
        case Key_H:
            if ( d->hmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( -10, 0 );
            }
            break;
        case Key_Enter:
        case Key_Return:
	    // ### FIXME:
	    // move this code to HTMLAnchorElementImpl::setPressed(false),
	    // or even better to HTMLAnchorElementImpl::event()
            if (m_part->xmlDocImpl()) {
		NodeImpl *n = m_part->xmlDocImpl()->focusNode();
		if (n)
		    n->setActive();
		d->originalNode = n;
	    }
            break;
        case Key_Home:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else
                setContentsPos( 0, 0 );
            break;
        case Key_End:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else
                setContentsPos( 0, contentsHeight() - visibleHeight() );
            break;
        case Key_Shift:
            // what are you doing here?
	    _ke->ignore();
            return;
        default:
            if (d->scrollTimerId)
                d->newScrollTimer(this, 0);
	    _ke->ignore();
            return;
        }
    _ke->accept();
}

void KHTMLView::keyReleaseEvent(QKeyEvent *_ke)
{
    if(m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()) {
        // Qt is damn buggy. we receive release events from our child
        // widgets. therefore, do not support keyrelease event on generic
        // nodes for now until we found  a workaround for the Qt bugs. (Dirk)
//         if (m_part->xmlDocImpl()->focusNode()->dispatchKeyEvent(_ke)) {
//             _ke->accept();
//             return;
//         }
//        QScrollView::keyReleaseEvent(_ke);
        Q_UNUSED(_ke);
    }
}

void KHTMLView::contentsContextMenuEvent ( QContextMenuEvent * /*ce*/ )
{
// ### what kind of c*** is that ?
#if 0
    if (!m_part->xmlDocImpl()) return;
    int xm = _ce->x();
    int ym = _ce->y();

    DOM::NodeImpl::MouseEvent mev( _ce->state(), DOM::NodeImpl::MouseMove ); // ### not a mouse event!
    m_part->xmlDocImpl()->prepareMouseEvent( xm, ym, &mev );

    NodeImpl *targetNode = mev.innerNode.handle();
    if (targetNode && targetNode->renderer() && targetNode->renderer()->isWidget()) {
        int absx = 0;
        int absy = 0;
        targetNode->renderer()->absolutePosition(absx,absy);
        QPoint pos(xm-absx,ym-absy);

        QWidget *w = static_cast<RenderWidget*>(targetNode->renderer())->widget();
        QContextMenuEvent cme(_ce->reason(),pos,_ce->globalPos(),_ce->state());
        setIgnoreEvents(true);
        QApplication::sendEvent(w,&cme);
        setIgnoreEvents(false);
    }
#endif
}

bool KHTMLView::focusNextPrevChild( bool next )
{
    // Now try to find the next child
    if (m_part->xmlDocImpl()) {
        focusNextPrevNode(next);
        if (m_part->xmlDocImpl()->focusNode() != 0) {
	  kdDebug() << "focusNode.name: "
	  << m_part->xmlDocImpl()->focusNode()->nodeName().string() << endl;
            return true; // focus node found
	    }
    }

    // If we get here, there is no next/previous child to go to, so pass up to the next/previous child in our parent
    if (m_part->parentPart() && m_part->parentPart()->view())
        return m_part->parentPart()->view()->focusNextPrevChild(next);

    return QWidget::focusNextPrevChild(next);
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

bool KHTMLView::eventFilter(QObject *o, QEvent *e)
{
    if ( e->type() == QEvent::AccelOverride ) {
	QKeyEvent* ke = (QKeyEvent*) e;
//kdDebug(6200) << "QEvent::AccelAvailable" << endl;
	if (m_part->isEditable() || m_part->isCaretMode()
	    || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
		&& m_part->xmlDocImpl()->focusNode()->contentEditable())) {
//kdDebug(6200) << "editable/navigable" << endl;
	    if ( ke->state() & ControlButton ) {
		switch ( ke->key() ) {
		case Key_Left:
		case Key_Right:
		case Key_Up:
		case Key_Down:
		case Key_Home:
		case Key_End:
		    ke->accept();
//kdDebug(6200) << "eaten" << endl;
		    return true;
		default:
		    break;
		}/*end switch*/
	    }/*end if*/
	}/*end if*/
    }/*end if*/

    QWidget *view = viewport();

    if (o == view) {
	// we need to install an event filter on all children of the viewport to
	// be able to get correct stacking of children within the document.
	if(e->type() == QEvent::ChildInserted) {
	    QObject *c = static_cast<QChildEvent *>(e)->child();
	    if (c->isWidgetType()) {
		QWidget *w = static_cast<QWidget *>(c);
		// don't install the event filter on toplevels
		if (w->parentWidget(true) == view) {
		    if (w->inherits("QScrollView")) {
			QScrollView *sv = static_cast<QScrollView *>(w);
			sv->viewport()->installEventFilter(this);
			QWidget *clipper = sv->clipper();
			if (clipper != viewport())
			    clipper->installEventFilter(this);
		    } else {
			w->installEventFilter(this);
			w->unsetCursor();
		    }
		}
	    }
	}
    } else if (o->isWidgetType()) {
	QWidget *v = static_cast<QWidget *>(o);
	while (v && v != view) {
	    v = v->parentWidget(true);
	}
	if (v) {
	    bool block = false;
	    QWidget *w = static_cast<QWidget *>(o);
	    switch(e->type()) {
	    case QEvent::Paint:
		if (!allowWidgetPaintEvents) {
		    // eat the event. Like this we can control exactly when the widget
		    // get's repainted.
		    block = true;
		    int x, y;
		    viewportToContents( w->x(), w->y(), x, y );
		    QPaintEvent *pe = static_cast<QPaintEvent *>(e);
		    scheduleRepaint(x + pe->rect().x(), y + pe->rect().y(),
				    pe->rect().width(), pe->rect().height());
		}
		break;
	    case QEvent::Wheel:
		// don't allow the widget to react to wheel event unless its
		// currently focused. this avoids accidentally changing a select box
		// or something while wheeling a webpage.
		if (qApp->focusWidget() != w &&
		    w->focusPolicy() <= QWidget::StrongFocus)  {
		    static_cast<QWheelEvent*>(e)->ignore();
		    QApplication::sendEvent(this, e);
		    block = true;
		}
		break;
	    case QEvent::MouseMove:
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonRelease:
	    case QEvent::MouseButtonDblClick: {
		if (allowWidgetMouseEvents && !w->inherits("QScrollBar")) {
		    QMouseEvent *me = static_cast<QMouseEvent *>(e);
		    QPoint pt = (me->pos() + w->pos());
		    QMouseEvent me2(me->type(), pt, me->button(), me->state());

		    if (e->type() == QEvent::MouseMove)
			viewportMouseMoveEvent(&me2);
		    else if(e->type() == QEvent::MouseButtonPress)
			viewportMousePressEvent(&me2);
		    else if(e->type() == QEvent::MouseButtonRelease)
			viewportMouseReleaseEvent(&me2);
		    else
			viewportMouseDoubleClickEvent(&me2);
		    //block = me2.isAccepted();
                    block = true;
		}
		break;
	    }
	    default:
		break;
	    }
	    if (block) {
 		//qDebug("eating event");
		return true;
	    }
	}
    }

    return QScrollView::eventFilter(o, e);
}


DOM::NodeImpl *KHTMLView::nodeUnderMouse() const
{
    return d->underMouse;
}

bool KHTMLView::scrollTo(const QRect &bounds)
{
    d->scrollingSelf = true; // so scroll events get ignored

    int x, y, xe, ye;
    x = bounds.left();
    y = bounds.top();
    xe = bounds.right();
    ye = bounds.bottom();

    //kdDebug(6000)<<"scrolling coords: x="<<x<<" y="<<y<<" width="<<xe-x<<" height="<<ye-y<<endl;

    int deltax;
    int deltay;

    int curHeight = visibleHeight();
    int curWidth = visibleWidth();

    if (ye-y>curHeight-d->borderY)
	ye  = y + curHeight - d->borderY;

    if (xe-x>curWidth-d->borderX)
	xe = x + curWidth - d->borderX;

    // is xpos of target left of the view's border?
    if (x < contentsX() + d->borderX )
            deltax = x - contentsX() - d->borderX;
    // is xpos of target right of the view's right border?
    else if (xe + d->borderX > contentsX() + curWidth)
            deltax = xe + d->borderX - ( contentsX() + curWidth );
    else
        deltax = 0;

    // is ypos of target above upper border?
    if (y < contentsY() + d->borderY)
            deltay = y - contentsY() - d->borderY;
    // is ypos of target below lower border?
    else if (ye + d->borderY > contentsY() + curHeight)
            deltay = ye + d->borderY - ( contentsY() + curHeight );
    else
        deltay = 0;

    int maxx = curWidth-d->borderX;
    int maxy = curHeight-d->borderY;

    int scrollX,scrollY;

    scrollX = deltax > 0 ? (deltax > maxx ? maxx : deltax) : deltax == 0 ? 0 : (deltax>-maxx ? deltax : -maxx);
    scrollY = deltay > 0 ? (deltay > maxy ? maxy : deltay) : deltay == 0 ? 0 : (deltay>-maxy ? deltay : -maxy);

    if (contentsX() + scrollX < 0)
	scrollX = -contentsX();
    else if (contentsWidth() - visibleWidth() - contentsX() < scrollX)
	scrollX = contentsWidth() - visibleWidth() - contentsX();

    if (contentsY() + scrollY < 0)
	scrollY = -contentsY();
    else if (contentsHeight() - visibleHeight() - contentsY() < scrollY)
	scrollY = contentsHeight() - visibleHeight() - contentsY();

    scrollBy(scrollX, scrollY);



    // generate abs(scroll.)
    if (scrollX<0)
        scrollX=-scrollX;
    if (scrollY<0)
        scrollY=-scrollY;

    d->scrollingSelf = false;

    if ( (scrollX!=maxx) && (scrollY!=maxy) )
	return true;
    else return false;

}

void KHTMLView::focusNextPrevNode(bool next)
{
    // Sets the focus node of the document to be the node after (or if next is false, before) the current focus node.
    // Only nodes that are selectable (i.e. for which isSelectable() returns true) are taken into account, and the order
    // used is that specified in the HTML spec (see DocumentImpl::nextFocusNode() and DocumentImpl::previousFocusNode()
    // for details).

    DocumentImpl *doc = m_part->xmlDocImpl();
    NodeImpl *oldFocusNode = doc->focusNode();
    NodeImpl *newFocusNode;

    // Find the next/previous node from the current one
    if (next)
        newFocusNode = doc->nextFocusNode(oldFocusNode);
    else
        newFocusNode = doc->previousFocusNode(oldFocusNode);

    // If there was previously no focus node and the user has scrolled the document, then instead of picking the first
    // focusable node in the document, use the first one that lies within the visible area (if possible).
    if (!oldFocusNode && newFocusNode && d->scrollBarMoved) {

      kdDebug(6000) << " searching for visible link" << endl;

        bool visible = false;
        NodeImpl *toFocus = newFocusNode;
        while (!visible && toFocus) {
            QRect focusNodeRect = toFocus->getRect();
            if ((focusNodeRect.left() > contentsX()) && (focusNodeRect.right() < contentsX() + visibleWidth()) &&
                (focusNodeRect.top() > contentsY()) && (focusNodeRect.bottom() < contentsY() + visibleHeight())) {
                // toFocus is visible in the contents area
                visible = true;
            }
            else {
                // toFocus is _not_ visible in the contents area, pick the next node
                if (next)
                    toFocus = doc->nextFocusNode(toFocus);
                else
                    toFocus = doc->previousFocusNode(toFocus);
            }
        }

        if (toFocus)
            newFocusNode = toFocus;
    }

    d->scrollBarMoved = false;

    if (!newFocusNode)
      {
        // No new focus node, scroll to bottom or top depending on next
        if (next)
            scrollTo(QRect(contentsX()+visibleWidth()/2,contentsHeight(),0,0));
        else
            scrollTo(QRect(contentsX()+visibleWidth()/2,0,0,0));
    }
    else
    // Scroll the view as necessary to ensure that the new focus node is visible
    {
#ifndef KHTML_NO_CARET
        // if it's an editable element, activate the caret
        if (!m_part->isCaretMode() && !m_part->isEditable()
		&& newFocusNode->contentEditable()) {
	    d->caretViewContext();
	    moveCaretTo(newFocusNode, 0L, true);
        } else {
	    caretOff();
	}
#endif // KHTML_NO_CARET

      if (oldFocusNode)
	{
	  if (!scrollTo(newFocusNode->getRect()))
	    return;
	}
      else
	{
	  ensureVisible(contentsX(), next?0:contentsHeight());
	  //return;
	}

    }

    // Set focus node on the document
    m_part->xmlDocImpl()->setFocusNode(newFocusNode);
    emit m_part->nodeActivated(Node(newFocusNode));
}

void KHTMLView::setMediaType( const QString &medium )
{
    m_medium = medium;
}

QString KHTMLView::mediaType() const
{
    return m_medium;
}

void KHTMLView::setWidgetVisible(RenderWidget* w, bool vis)
{
    if (vis) {
        d->visibleWidgets.replace(w, w->widget());
    }
    else
        d->visibleWidgets.remove(w);
}

void KHTMLView::print()
{
    print( false );
}

void KHTMLView::print(bool quick)
{
    if(!m_part->xmlDocImpl()) return;
    khtml::RenderCanvas *root = static_cast<khtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer());
    if(!root) return;

    // this only works on Unix - we assume 72dpi
    KPrinter *printer = new KPrinter(true, QPrinter::PrinterResolution);
    printer->addDialogPage(new KHTMLPrintSettings());
    QString docname = m_part->xmlDocImpl()->URL();
    if ( !docname.isEmpty() )
        docname = KStringHandler::csqueeze(docname, 80);
    if(quick || printer->setup(this, i18n("Print %1").arg(docname))) {
        viewport()->setCursor( waitCursor ); // only viewport(), no QApplication::, otherwise we get the busy cursor in kdeprint's dialogs
        // set up KPrinter
        printer->setFullPage(false);
        printer->setCreator("KDE 3.0 HTML Library");
        printer->setDocName(docname);

        QPainter *p = new QPainter;
        p->begin( printer );
        khtml::setPrintPainter( p );

        m_part->xmlDocImpl()->setPaintDevice( printer );
        QString oldMediaType = mediaType();
        setMediaType( "print" );
        // We ignore margin settings for html and body when printing
        // and use the default margins from the print-system
        // (In Qt 3.0.x the default margins are hardcoded in Qt)
        m_part->xmlDocImpl()->setPrintStyleSheet( printer->option("app-khtml-printfriendly") == "true" ?
                                                  "* { background-image: none !important;"
                                                  "    background-color: white !important;"
                                                  "    color: black !important; }"
						  "body { margin: 0px !important; }"
						  "html { margin: 0px !important; }" :
						  "body { margin: 0px !important; }"
						  "html { margin: 0px !important; }"
						  );

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

        m_part->xmlDocImpl()->styleSelector()->computeFontSizes(&metrics, 100);
        m_part->xmlDocImpl()->updateStyleSelector();
        root->setPrintImages( printer->option("app-khtml-printimages") == "true");
        root->setLayouted( false );
        root->setMinMaxKnown( false );
        root->layout();

        bool printHeader = (printer->option("app-khtml-printheader") == "true");

        int headerHeight = 0;
        QFont headerFont("helvetica", 8);

        QString headerLeft = KGlobal::locale()->formatDate(QDate::currentDate(),true);
        QString headerMid = docname;
        QString headerRight;

        if (printHeader)
        {
           p->setFont(headerFont);
           headerHeight = (p->fontMetrics().lineSpacing() * 3) / 2;
        }

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
        p->setClipRect(0,0, pageWidth, pageHeight);

        pageHeight -= headerHeight;

        bool scalePage = false;
        double scale = 0.0;
#ifndef QT_NO_TRANSFORMATIONS
        if(root->docWidth() > metrics.width()) {
            scalePage = true;
            scale = ((double) metrics.width())/((double) root->docWidth());
            pageHeight = (int) (pageHeight/scale);
            pageWidth = (int) (pageWidth/scale);
            headerHeight = (int) (headerHeight/scale);
        }
#endif
        kdDebug(6000) << "printing: scaled html width = " << pageWidth
                      << " height = " << pageHeight << endl;

        // Squeeze header to make it it on the page.
        if (printHeader)
        {
            int available_width = metrics.width() - 10 -
                2 * QMAX(p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), Qt::AlignLeft, headerLeft).width(),
                         p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), Qt::AlignLeft, headerRight).width());
            if (available_width < 150)
               available_width = 150;
            int mid_width;
            int squeeze = 120;
            do {
                headerMid = KStringHandler::csqueeze(docname, squeeze);
                mid_width = p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), Qt::AlignLeft, headerMid).width();
                squeeze -= 10;
            } while (mid_width > available_width);
        }

        int top = 0;
        int page = 1;
        while(top < root->docHeight()) {
            if(top > 0) printer->newPage();
            if (printHeader)
            {
                int dy = p->fontMetrics().lineSpacing();
                p->setPen(Qt::black);
                p->setFont(headerFont);

                headerRight = QString("#%1").arg(page);

                p->drawText(0, 0, metrics.width(), dy, Qt::AlignLeft, headerLeft);
                p->drawText(0, 0, metrics.width(), dy, Qt::AlignHCenter, headerMid);
                p->drawText(0, 0, metrics.width(), dy, Qt::AlignRight, headerRight);
            }

#ifndef QT_NO_TRANSFORMATIONS
            if (scalePage)
                p->scale(scale, scale);
#endif
            p->translate(0, headerHeight-top);

            root->setTruncatedAt(top+pageHeight);

            root->layer()->paint(p, 0, top, pageWidth, pageHeight, 0, 0);
            if (top + pageHeight >= root->docHeight())
                break; // Stop if we have printed everything

            top = root->truncatedAt();
            p->resetXForm();
            page++;
        }

        p->end();
        delete p;

        // and now reset the layout to the usual one...
        root->setPrintingMode(false);
        khtml::setPrintPainter( 0 );
        setMediaType( oldMediaType );
        m_part->xmlDocImpl()->setPaintDevice( this );
        m_part->xmlDocImpl()->styleSelector()->computeFontSizes(m_part->xmlDocImpl()->paintDeviceMetrics(), m_part->zoomFactor());
        m_part->xmlDocImpl()->updateStyleSelector();
        viewport()->unsetCursor();
    }
    delete printer;
}

void KHTMLView::slotPaletteChanged()
{
    if(!m_part->xmlDocImpl()) return;
    DOM::DocumentImpl *document = m_part->xmlDocImpl();
    if (!document->isHTMLDocument()) return;
    khtml::RenderCanvas *root = static_cast<khtml::RenderCanvas *>(document->renderer());
    if(!root) return;
    root->style()->resetPalette();
    NodeImpl *body = static_cast<HTMLDocumentImpl*>(document)->body();
    if(!body) return;
    body->setChanged(true);
    body->recalcStyle( NodeImpl::Force );
}

void KHTMLView::paint(QPainter *p, const QRect &rc, int yOff, bool *more)
{
    if(!m_part->xmlDocImpl()) return;
    khtml::RenderCanvas *root = static_cast<khtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer());
    if(!root) return;

    m_part->xmlDocImpl()->setPaintDevice(p->device());
    root->setPrintingMode(true);
    root->setWidth(rc.width());

    p->save();
    p->setClipRect(rc);
    p->translate(rc.left(), rc.top());
    double scale = ((double) rc.width()/(double) root->docWidth());
    int height = (int) ((double) rc.height() / scale);
#ifndef QT_NO_TRANSFORMATIONS
    p->scale(scale, scale);
#endif

    root->layer()->paint(p, 0, yOff, root->docWidth(), height, 0, 0);
    if (more)
        *more = yOff + height < root->docHeight();
    p->restore();

    root->setPrintingMode(false);
    m_part->xmlDocImpl()->setPaintDevice( this );
}


void KHTMLView::useSlowRepaints()
{
    kdDebug(0) << "slow repaints requested" << endl;
    d->useSlowRepaints = true;
    setStaticBackground(true);
}


void KHTMLView::setVScrollBarMode ( ScrollBarMode mode )
{
#ifndef KHTML_NO_SCROLLBARS
    d->vmode = mode;
    QScrollView::setVScrollBarMode(mode);
#else
    Q_UNUSED( mode );
#endif
}

void KHTMLView::setHScrollBarMode ( ScrollBarMode mode )
{
#ifndef KHTML_NO_SCROLLBARS
    d->hmode = mode;
    QScrollView::setHScrollBarMode(mode);
#else
    Q_UNUSED( mode );
#endif
}

void KHTMLView::restoreScrollBar()
{
    int ow = visibleWidth();
    QScrollView::setVScrollBarMode(d->vmode);
    if (visibleWidth() != ow)
        layout();
    d->prevScrollbarVisible = verticalScrollBar()->isVisible();
}

QStringList KHTMLView::formCompletionItems(const QString &name) const
{
    if (!m_part->settings()->isFormCompletionEnabled())
        return QStringList();
    if (!d->formCompletions)
        d->formCompletions = new KSimpleConfig(locateLocal("data", "khtml/formcompletions"));
    return d->formCompletions->readListEntry(name);
}

void KHTMLView::clearCompletionHistory(const QString& name)
{
    if (!d->formCompletions)
    {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "khtml/formcompletions"));
    }
    d->formCompletions->writeEntry(name, "");
    d->formCompletions->sync();
}

void KHTMLView::addFormCompletionItem(const QString &name, const QString &value)
{
    if (!m_part->settings()->isFormCompletionEnabled())
        return;
    // don't store values that are all numbers or just numbers with
    // dashes or spaces as those are likely credit card numbers or
    // something similar
    bool cc_number(true);
    for (unsigned int i = 0; i < value.length(); ++i)
    {
      QChar c(value[i]);
      if (!c.isNumber() && c != '-' && !c.isSpace())
      {
        cc_number = false;
        break;
      }
    }
    if (cc_number)
      return;
    QStringList items = formCompletionItems(name);
    if (!items.contains(value))
        items.prepend(value);
    while ((int)items.count() > m_part->settings()->maxFormCompletionItems())
        items.remove(items.fromLast());
    d->formCompletions->writeEntry(name, items);
}

bool KHTMLView::dispatchMouseEvent(int eventId, DOM::NodeImpl *targetNode, bool cancelable,
				   int detail,QMouseEvent *_mouse, bool setUnder,
				   int mouseEventType)
{
    if (d->underMouse)
	d->underMouse->deref();
    d->underMouse = targetNode;
    if (d->underMouse)
	d->underMouse->ref();

    int exceptioncode = 0;
    int mx, my;
    viewportToContents(_mouse->x(), _mouse->y(), mx, my);
    // clientX and clientY are in viewport coordinates
    // At least the JS code wants event.[xy]/event.client[XY] to be in viewport coords.
    // [that's not the same as _mouse->[xy](), since we use the clipper]
    int clientX = mx - contentsX();
    int clientY = my - contentsY();
    int screenX = _mouse->globalX();
    int screenY = _mouse->globalY();
    int button = -1;
    switch (_mouse->button()) {
	case LeftButton:
	    button = 0;
	    break;
	case MidButton:
	    button = 1;
	    break;
	case RightButton:
	    button = 2;
	    break;
	default:
	    break;
    }
    bool ctrlKey = (_mouse->state() & ControlButton);
    bool altKey = (_mouse->state() & AltButton);
    bool shiftKey = (_mouse->state() & ShiftButton);
    bool metaKey = (_mouse->state() & MetaButton);

    // mouseout/mouseover
    if (setUnder && (d->prevMouseX != mx || d->prevMouseY != my)) {

        // ### this code sucks. we should save the oldUnder instead of calculating
        // it again. calculating is expensive! (Dirk)
        NodeImpl *oldUnder = 0;
	if (d->prevMouseX >= 0 && d->prevMouseY >= 0) {
	    NodeImpl::MouseEvent mev( _mouse->stateAfter(), static_cast<NodeImpl::MouseEventType>(mouseEventType));
	    m_part->xmlDocImpl()->prepareMouseEvent( true, d->prevMouseX, d->prevMouseY, &mev );
	    oldUnder = mev.innerNode.handle();
	}
// 	qDebug("oldunder=%p (%s), target=%p (%s) x/y=%d/%d", oldUnder, oldUnder ? oldUnder->renderer()->renderName() : 0, targetNode,  targetNode ? targetNode->renderer()->renderName() : 0, _mouse->x(), _mouse->y());
	if (oldUnder != targetNode) {
	    // send mouseout event to the old node
	    if (oldUnder){
		oldUnder->ref();
		MouseEventImpl *me = new MouseEventImpl(EventImpl::MOUSEOUT_EVENT,
							true,true,m_part->xmlDocImpl()->defaultView(),
							0,screenX,screenY,clientX,clientY,
							ctrlKey,altKey,shiftKey,metaKey,
							button,targetNode);
		me->ref();
		oldUnder->dispatchEvent(me,exceptioncode,true);
		me->deref();
	    }

	    // send mouseover event to the new node
	    if (targetNode) {
		MouseEventImpl *me = new MouseEventImpl(EventImpl::MOUSEOVER_EVENT,
							true,true,m_part->xmlDocImpl()->defaultView(),
							0,screenX,screenY,clientX,clientY,
							ctrlKey,altKey,shiftKey,metaKey,
							button,oldUnder);

		me->ref();
		targetNode->dispatchEvent(me,exceptioncode,true);
		me->deref();
	    }

            if (oldUnder)
                oldUnder->deref();
        }
    }

    bool swallowEvent = false;

    if (targetNode) {
	// send the actual event
	MouseEventImpl *me = new MouseEventImpl(static_cast<EventImpl::EventId>(eventId),
						true,cancelable,m_part->xmlDocImpl()->defaultView(),
						detail,screenX,screenY,clientX,clientY,
						ctrlKey,altKey,shiftKey,metaKey,
						button,0);
	me->ref();
	targetNode->dispatchEvent(me,exceptioncode,true);
        if (me->defaultHandled() || me->defaultPrevented())
            swallowEvent = true;
	me->deref();

	if (eventId == EventImpl::MOUSEDOWN_EVENT) {
            if (targetNode->isSelectable())
                m_part->xmlDocImpl()->setFocusNode(targetNode);
            else
                m_part->xmlDocImpl()->setFocusNode(0);
	}
    }

    return swallowEvent;
}

void KHTMLView::setIgnoreWheelEvents( bool e )
{
    d->ignoreWheelEvents = e;
}

#ifndef QT_NO_WHEELEVENT

void KHTMLView::viewportWheelEvent(QWheelEvent* e)
{
    if ( ( e->state() & ShiftButton ) == ShiftButton )
    {
        emit zoomView( e->delta() );
        e->accept();
    }
    else if ( ( (d->ignoreWheelEvents && !verticalScrollBar()->isVisible())
    			|| e->delta() > 0 && contentsY() <= 0
                        || e->delta() < 0 && contentsY() >= contentsHeight() - visibleHeight())
                && m_part->parentPart() ) {
       kdDebug(6000) << this << " cz " << contentsY() << " ch " << contentsHeight() << " vh " << visibleHeight() << endl;
        if ( m_part->parentPart()->view() )
            m_part->parentPart()->view()->wheelEvent( e );
        kdDebug(6000) << "sent" << endl;
        e->ignore();
    }
    else if ( d->vmode == QScrollView::AlwaysOff ) {
        e->accept();
    }
    else {
        d->scrollBarMoved = true;
        QScrollView::viewportWheelEvent( e );

        QMouseEvent *tempEvent = new QMouseEvent( QEvent::MouseMove, QPoint(-1,-1), QPoint(-1,-1), Qt::NoButton, e->state() );
        emit viewportMouseMoveEvent ( tempEvent );
        delete tempEvent;
    }

}
#endif

void KHTMLView::dragEnterEvent( QDragEnterEvent* ev )
{
    // Handle drops onto frames (#16820)
    // Drops on the main html part is handled by Konqueror (and shouldn't do anything
    // in e.g. kmail, so not handled here).
    if ( m_part->parentPart() )
    {
    	QApplication::sendEvent(m_part->parentPart()->widget(), ev);
	return;
    }
    QScrollView::dragEnterEvent( ev );
}

void KHTMLView::dropEvent( QDropEvent *ev )
{
    // Handle drops onto frames (#16820)
    // Drops on the main html part is handled by Konqueror (and shouldn't do anything
    // in e.g. kmail, so not handled here).
    if ( m_part->parentPart() )
    {
    	QApplication::sendEvent(m_part->parentPart()->widget(), ev);
	return;
    }
    QScrollView::dropEvent( ev );
}

void KHTMLView::focusInEvent( QFocusEvent *e )
{
    showCaret();
    QScrollView::focusInEvent( e );
}

void KHTMLView::focusOutEvent( QFocusEvent *e )
{
    if(m_part) m_part->stopAutoScroll();
    hideCaret();
    QScrollView::focusOutEvent( e );
}

void KHTMLView::slotScrollBarMoved()
{
    if (!d->scrollingSelf)
        d->scrollBarMoved = true;
}

void KHTMLView::timerEvent ( QTimerEvent *e )
{
//    kdDebug() << "timer event " << e->timerId() << endl;
    if ( e->timerId() == d->scrollTimerId ) {
        switch (d->scrollDirection) {
            case KHTMLViewPrivate::ScrollDown:
                if (contentsY() + visibleHeight () >= contentsHeight())
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, d->scrollBy );
                break;
            case KHTMLViewPrivate::ScrollUp:
                if (contentsY() <= 0)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( 0, -d->scrollBy );
                break;
            case KHTMLViewPrivate::ScrollRight:
                if (contentsX() + visibleWidth () >= contentsWidth())
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( d->scrollBy, 0 );
                break;
            case KHTMLViewPrivate::ScrollLeft:
                if (contentsX() <= 0)
                    d->newScrollTimer(this, 0);
                else
                    scrollBy( -d->scrollBy, 0 );
                break;
        }
        return;
    }
    else if ( e->timerId() == d->layoutTimerId ) {
        d->firstRelayout = false;
        d->dirtyLayout = true;
        layout();
    }
#ifndef KHTML_NO_CARET
    else if (d->m_caretViewContext
    	     && e->timerId() == d->m_caretViewContext->freqTimerId) {
        d->m_caretViewContext->visible = !d->m_caretViewContext->visible;
	if (d->m_caretViewContext->displayed) {
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
			d->m_caretViewContext->width,
			d->m_caretViewContext->height);
	}/*end if*/
//	if (d->m_caretViewContext->visible) cout << "|" << flush;
//	else cout << "°" << flush;
	return;
    }
#endif

    if( m_part->xmlDocImpl() ) {
	DOM::DocumentImpl *document = m_part->xmlDocImpl();
	khtml::RenderCanvas* root = static_cast<khtml::RenderCanvas *>(document->renderer());

	if ( root && !root->layouted() ) {
	    killTimer(d->repaintTimerId);
	    d->repaintTimerId = 0;
	    scheduleRelayout(false);
	    return;
	}
    }

    setStaticBackground(d->useSlowRepaints);

//        kdDebug() << "scheduled repaint "<< d->repaintTimerId  << endl;
    killTimer(d->repaintTimerId);
    d->repaintTimerId = 0;

    updateContents( d->updateRect );
    d->updateRect = QRect();

    if (d->dirtyLayout && !d->visibleWidgets.isEmpty()) {
        QWidget* w;
        d->dirtyLayout = false;

        QRect visibleRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());
        QPtrList<RenderWidget> toRemove;
        for (QPtrDictIterator<QWidget> it(d->visibleWidgets); it.current(); ++it) {
            int xp = 0, yp = 0;
            w = it.current();
            RenderWidget* rw = static_cast<RenderWidget*>( it.currentKey() );
            if (!rw->absolutePosition(xp, yp) ||
                !visibleRect.intersects(QRect(xp, yp, w->width(), w->height())))
                toRemove.append(rw);
        }
        for (RenderWidget* r = toRemove.first(); r; r = toRemove.next())
            if ( (w = d->visibleWidgets.take(r) ) )
                addChild(w, 0, -500000);
    }
}

void KHTMLView::scheduleRelayout(bool dorepaint)
{
    if (!d->layoutSchedulingEnabled || d->layoutTimerId)
        return;

    d->layoutTimerId = startTimer( m_part->xmlDocImpl() && m_part->xmlDocImpl()->parsing()
                             ? 1000 : 0 );
    d->repaintLayout = dorepaint;
}

void KHTMLView::scheduleRepaint(int x, int y, int w, int h)
{
    bool parsing = !m_part->xmlDocImpl() || m_part->xmlDocImpl()->parsing();

//     kdDebug() << "parsing " << parsing << endl;
//     kdDebug() << "complete " << d->complete << endl;

    int time;

    // if complete...
    if (d->complete)
        // ...repaint immediately
        time = 20;
    else
    {
        if (parsing)
            // not complete and still parsing
            time = 300;
        else
            // not complete, not parsing, extend the timer if it exists
            // otherwise, repaint immediately
            time = d->repaintTimerId ? 400 : 20;
    }

    if (d->repaintTimerId) {
        killTimer(d->repaintTimerId);
        d->updateRect = d->updateRect.unite(QRect(x,y,w,h));
    } else
        d->updateRect = QRect(x,y,w,h);

//	kdDebug(6000) << "scheduled repaint for " << d->updateRect << endl;
    d->repaintTimerId = startTimer( time );

//     kdDebug() << "starting timer " << time << endl;
}

void KHTMLView::complete()
{
//     kdDebug() << "KHTMLView::complete()" << endl;

    d->complete = true;

    // is there a relayout pending?
    if (d->layoutTimerId)
    {
//         kdDebug() << "requesting relayout now" << endl;
        // do it now
        killTimer(d->layoutTimerId);
        d->layoutTimerId = startTimer( 0 );
    }

    // is there a repaint pending?
    if (d->repaintTimerId)
    {
//         kdDebug() << "requesting repaint now" << endl;
        // do it now
        killTimer(d->repaintTimerId);
        d->repaintTimerId = startTimer( 20 );
    }
}

#ifndef KHTML_NO_CARET
// == caret-related stuff ==

namespace khtml {

class LinearDocument;

static InlineFlowBox *findFlowBox(DOM::NodeImpl *node, long offset,
		RenderArena *arena, RenderFlow *&cb, InlineBox **ibox = 0);

/**
 * Iterates through the lines of a document.
 *
 * The line iterator becomes invalid when the associated LinearDocument object
 * is destroyed.
 * @since 3.2
 * @internal
 * @author Leo Savernik
 */
class LineIterator {
protected:
  LinearDocument *lines;	// associated document
  RenderFlow *cb;		// containing block
  InlineFlowBox *flowBox;	// the line itself

  static InlineBox *currentBox;	// current inline box

  // Note: cb == 0 indicates a position beyond the beginning or the
  // end of a document.

  /** Default constructor, only for internal use
   */
  LineIterator() {}

  /** Initializes a new iterator.
   *
   * Note: This constructor neither cares about the correctness of @p node
   * nor about @p offset. It is the responsibility of the caller to ensure
   * that both point to valid places.
   */
  LineIterator(LinearDocument *l, DOM::NodeImpl *node, long offset);

public:
  /** dereferences current line.
   *
   * Note: The returned inline flow box may be empty. This does <b>not<b>
   *	indicate the end
   *	of the document, but merely that the current paragraph does not contain
   *	any lines.
   */
  InlineFlowBox *operator *() const { return flowBox; }

  /** seek next line
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator ++();
  /** seek next line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator ++(int);

  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator --();
  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator --(int);

  /** does pointer arithmetic.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator operator +(int summand) const;
  /** does pointer arithmetic.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator operator -(int summand) const;

  /** does pointer arithmetic and assignment.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator &operator +=(int summand);
  /** does pointer arithmetic and assignment.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator &operator -=(int summand);

  /** compares two iterators. The comparator actually works only for
   * comparing arbitrary iterators to begin() and end().
   */
  bool operator ==(const LineIterator &it) const
  {
    return lines == it.lines
    		&& flowBox == it.flowBox && cb == it.cb;
  }

  /** compares two iterators
   */
  bool operator !=(const LineIterator &it) const
  {
    return !operator ==(it);
  }

  /** Whenever a new line iterator is created, it gets the inline box
   * it points to. For memory reasons, it's saved in a static instance,
   * thus making this function not thread-safe.
   *
   * This value can only be trusted immediately after having instantiated
   * a line iterator or one of its derivatives.
   * @return the corresponing inline box within the line represented by the
   *	last instantiation of a line iterator, or 0 if there was none.
   */
  static InlineBox *currentInlineBox() { return currentBox; }

protected:
  /** seeks next block.
   */
  void nextBlock();
  /** seeks previous block.
   */
  void prevBlock();

  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class LinearDocument;
};

InlineBox *LineIterator::currentBox;

/**
 * Represents the whole document in terms of lines.
 *
 * SGML documents are trees. But for navigation, this representation is
 * not practical. Therefore this class serves as a helper to represent the
 * document as a linear list of lines. Its usage somewhat resembles STL
 * semantics like @ref begin and @ref end as well as iterators.
 *
 * The lines itself are represented as pointers to InlineFlowBox objects.
 *
 * LinearDocument instances are not meant to be kept over the lifetime of their
 * associated document, but constructed from (node, offset) pairs whenever line
 * traversal is needed. This is because the underlying InlineFlowBox objects
 * may be destroyed and recreated (e. g. by resizing the window, adding/removing
 * elements).
 *
 * @author Leo Savernik
 * @since 3.2
 * @internal
 */
class LinearDocument {
public:
  typedef LineIterator Iterator;

  /**
   * Creates a new instance, and initializes it to the line specified by
   * the parameters below.
   *
   * Creation will fail if @p node is invisible or defect.
   * @param part part within which everything is taking place.
   * @param node document node with which to start
   * @param offset zero-based offset within this node.
   */
  LinearDocument(KHTMLPart *part, DOM::NodeImpl *node, long offset);

  virtual ~LinearDocument();

  /**
   * Tells whether this list contains any lines.
   *
   * @returns @p true if this document contains lines, @p false otherwise. Note
   *	that an empty document contains at least one line, so this method
   *	only returns @p false if the document could not be initialised for
   *	some reason.
   */
  bool isValid() const		// FIXME: not yet impl'd
  {
    return true;
  }

  /**
   * Returns the count of lines.
   *
   * Warning: This function is expensive. Call it once and cache the value.
   *
   * FIXME: It's not implemented yet (and maybe never will)
   */
  int count() const;

  /**
   * Returns a line iterator containing the current position as its starting
   * value.
   */
  Iterator current();

  /**
   * Returns a line iterator pointing right after the end of the document.
   */
  const Iterator &end() const { return _end; }

  /**
   * Returns a line iterator pointing to the very last line of the document.
   */
  Iterator preEnd();

  /**
   * Returns a line iterator pointing to the very first line of the document.
   */
  Iterator begin();

  /**
   * Returns a line iterator pointing just before the very first line of the
   * document (this is somewhat an emulation of reverse iterators).
   */
  const Iterator &preBegin() const { return _preBegin; }

protected:
  void initPreBeginIterator();
  void initEndIterator();

protected:
  RenderArena *arena;		// We need an arena for intermediate render
  				// objects that have no own inline box
  DOM::NodeImpl *node;
  long offset;

  Iterator _preBegin;
  Iterator _end;

  KHTMLPart *m_part;

  friend class LineIterator;
  friend class EditableLineIterator;
  friend class ErgonomicEditableLineIterator;
  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class EditableCharacterIterator;
};

/**
 * Iterates over the inner elements of an inline flow box.
 *
 * The given inline flow box must be a line box. The incrementor will
 * traverse all leaf inline boxes. It will also generate transient inline boxes
 * for those render objects that do not have one.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class InlineBoxIterator {
protected:
  RenderArena *arena;	// arena for allocating transient inline boxes
  InlineBox *box;	// currently traversed inline box

public:
  /** creates a new iterator, initialized with the given flow box.
   */
  InlineBoxIterator(RenderArena *arena, InlineFlowBox *flowBox, bool fromEnd = false) : arena(arena)
  {
    box = fromEnd ? seekLeafInlineBoxFromEnd(flowBox) : seekLeafInlineBox(flowBox);
  }

  /** creates a new iterator, initialized with the given line iterator,
   * initialized to the given inline box, if specified.
   */
  InlineBoxIterator(LineIterator &lit, bool fromEnd = false,
  		InlineBox *initBox = 0) : arena(lit.lines->arena)
  {
    if (initBox) box = initBox;
    else box = fromEnd ? seekLeafInlineBoxFromEnd(*lit) : seekLeafInlineBox(*lit);
  }

  /** empty constructor.
   */
  InlineBoxIterator() {}

  /** returns the current leaf inline box.
   *
   * @return the box or 0 if the end has been reached.
   */
  InlineBox *operator *() const { return box; }

  /** increments the iterator to point to the next inline box on this line box.
   */
  InlineBoxIterator &operator ++()
  {
    InlineBox *newBox = box->nextOnLine();

    if (newBox)
      box = seekLeafInlineBox(newBox);
    else {
      InlineFlowBox *flowBox = box->parent();
      box = 0;
      while (flowBox) {
        InlineBox *newBox2 = flowBox->nextOnLine();
	if (newBox2) {
	  box = seekLeafInlineBox(newBox2);
	  break;
	}/*end if*/

	flowBox = flowBox->parent();
      }/*wend*/
    }/*end if*/

    return *this;
  }

  /** decrements the iterator to point to the previous inline box on this
   * line box.
   */
  InlineBoxIterator &operator --()
  {
    InlineBox *newBox = box->prevOnLine();

    if (newBox)
      box = seekLeafInlineBoxFromEnd(newBox);
    else {
      InlineFlowBox *flowBox = box->parent();
      box = 0;
      while (flowBox) {
        InlineBox *newBox2 = flowBox->prevOnLine();
	if (newBox2) {
	  box = seekLeafInlineBoxFromEnd(newBox2);
	  break;
	}/*end if*/

	flowBox = flowBox->parent();
      }/*wend*/
    }/*end if*/

    return *this;
  }

};

/**
 * Iterates over the editable inner elements of an inline flow box.
 *
 * The given inline flow box must be a line box. The incrementor will
 * traverse all leaf inline boxes. In contrast to @p InlineBoxIterator this
 * iterator only regards inline boxes which are editable.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class EditableInlineBoxIterator : public InlineBoxIterator {
protected:
  KHTMLPart *m_part;
  bool adjacent;

public:
  /** creates a new iterator, initialized with the given flow box.
   * @param part part within which all actions are taking place.
   * @param arena arena for transient allocations
   * @param flowBox line box to be iterated
   * @param fromEnd @p true, start with last box in line
   */
  EditableInlineBoxIterator(KHTMLPart *part, RenderArena *arena,
  		InlineFlowBox *flowBox, bool fromEnd = false)
  	: InlineBoxIterator(arena, flowBox, fromEnd), m_part(part), adjacent(true)
  {
    if (box && !isEditable(box)) fromEnd ? --*this : ++*this;
  }

  /** initializes a new iterator from the given line iterator,
   * beginning with the given inline box, if specified.
   */
  EditableInlineBoxIterator(LineIterator &lit, bool fromEnd = false,
  		InlineBox *initBox = 0)
  		: InlineBoxIterator(lit, fromEnd, initBox), m_part(lit.lines->m_part)
  {
    if (box && !isEditable(box)) fromEnd ? --*this : ++*this;
  }

  /** empty constructor. Use only to copy another iterator into this one.
   */
  EditableInlineBoxIterator() {}

  /** returns @p true when the current inline box is visually adjacent to the
   * previous inline box, i. e. no intervening inline boxes.
   */
  bool isAdjacent() const { return adjacent; }

  /** increments the iterator to point to the next editable inline box
   * on this line box.
   */
  EditableInlineBoxIterator &operator ++()
  {
    adjacent = true;
    do {
      InlineBoxIterator::operator ++();
    } while (box && !isEditable(box));
    return *this;
  }

  /** decrements the iterator to point to the previous editable inline box
   * on this line box.
   */
  EditableInlineBoxIterator &operator --()
  {
    adjacent = true;
    do {
      InlineBoxIterator::operator --();
    } while (box && !isEditable(box));
    return *this;
  }

protected:
  /** finds out if the given box is editable.
   * @param b given inline box
   * @return @p true if box is editable
   */
  bool isEditable(InlineBox *b)
  {
    //if (m_part->isCaretMode() || m_part->isEditable()) return true;

    Q_ASSERT(b);
    RenderObject *r = b->object();
    if (b->isInlineFlowBox()) kdDebug(6200) << "b is inline flow box" << endl;
    kdDebug(6200) << "isEditable r" << r << ": " << (r ? r->renderName() : QString::null) << (r && r->isText() ? " contains \"" + QString(((RenderText *)r)->str->s, QMIN(((RenderText *)r)->str->l,15)) + "\"" : QString::null) << endl;
    // Must check caret mode or design mode *after* r && r->element(), otherwise
    // lines without a backing DOM node get regarded, leading to a crash.
    // ### check should actually be in InlineBoxIterator
    bool result = r && r->element()
    	&& (m_part->isCaretMode() || m_part->isEditable()
           	|| r->style()->userInput() == UI_ENABLED);
    if (!result) adjacent = false;
    kdDebug(6200) << result << endl;
    return result;
  }

};

/**
 * Iterates through the editable lines of a document.
 *
 * This iterator, opposing to @p LineIterator, only regards editable lines.
 * If either @ref KHTMLPart::editable or @ref KHTMLPart::caretMode are true,
 * this class works exactly as if it were a LineIterator.
 *
 * The iterator can be compared to normal LineIterators, especially to
 * @ref LinearDocument::preBegin and @ref LinearDocument::end
 *
 * The line iterator becomes invalid when the associated LinearDocument object
 * is destroyed.
 * @since 3.2
 * @internal
 * @author Leo Savernik
 */
class EditableLineIterator : public LineIterator {
public:
  /** Initializes a new iterator.
   *
   * The iterator is set to the first following editable line or to the
   * end if no editable line follows.
   * @param it a line iterator to initialize this from
   * @param fromEnd @p true, traverse towards the beginning in search of an
   *	editable line
   */
  EditableLineIterator(const LineIterator &it, bool fromEnd = false)
  		: LineIterator(it)
  {
    if (flowBox && !isEditable(*this)) fromEnd ? operator--() : operator ++();
    if (!flowBox || !cb) {
      kdDebug(6200) << "EditableLineIterator: findFlowBox failed" << endl;
      cb = 0;
    }/*end if*/
  }

  /** empty constructor.
   *
   * Only use if you want to copy another iterator onto it later.
   */
  EditableLineIterator() {}

  /** seek next line
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  EditableLineIterator &operator ++()
  {
    // FIXME: MEGA-FLAW! editable empty inlines elements not
    // represented by an inline box aren't considered any more.
    do {
      LineIterator::operator ++();
    } while (cb && !isEditable(*this));
    return *this;
  }
  /** seek next line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  //EditableLineIterator operator ++(int);

  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  EditableLineIterator &operator --()
  {
    // FIXME: MEGA-FLAW! editable empty inlines not
    // represented by an inline box aren't considered any more.
    do {
      LineIterator::operator --();
    } while (cb && !isEditable(*this));
    return *this;
  }
  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  //EditableLineIterator operator --(int);

#if 0	// implement when it's needed
  /** does pointer arithmetic.
   * @param summand add these many lines
   */
  EditableLineIterator operator +(int summand) const;
  /** does pointer arithmetic.
   * @param summand add these many lines
   */
  EditableLineIterator operator -(int summand) const;

  /** does pointer arithmetic and assignment.
   * @param summand add these many lines
   */
  EditableLineIterator &operator +=(int summand);
  /** does pointer arithmetic and assignment.
   * @param summand add these many lines
   */
  EditableLineIterator &operator -=(int summand);
#endif

protected:
  /** finds out if the current line is editable.
   * @param it check flow box iterator points to
   * @return @p true if line is editable
   */
  bool isEditable(LineIterator &it)
  {
#if 0		// these shortcut evaluations are all invalid
    if (lines->m_part->isCaretMode() || lines->m_part->isEditable()) return true;

    // on dummy lines check the containing block itself for editability
    if (!(*it)->firstChild()) {
      kdDebug(6200) << "cb " << cb->renderName() << "[" << cb << "](" << (cb->element() ? cb->element()->nodeName().string() : QString::null) << ") editable? " << (cb->style()->userInput() == UI_ENABLED) << endl;
      return cb->style()->userInput() == UI_ENABLED;
    }/*end if*/
#endif

    EditableInlineBoxIterator fbit = it;
    return *fbit;
  }

};

/** Represents a render table as a linear list of rows.
 *
 * This iterator abstracts from table sections and treats tables as a linear
 * representation of all rows they contain.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class TableRowIterator {
protected:
  RenderTableSection *sec;	// current section
  int index;			// index of row within section
public:
  /** Constructs a new iterator.
   * @param table table to iterate through.
   * @param fromEnd @p true to iterate towards the beginning
   * @param startRow pointer to row to start with, 0 starts at the first/last
   *	row.
   */
  TableRowIterator(RenderTable *table, bool fromEnd = false,
  		RenderTableSection::RowStruct *row = 0);

  /** Constructs a new iterator.
   * @param section table section to begin with
   * @param index index within table section
   */
  TableRowIterator(RenderTableSection *section, int index)
  	: sec(section), index(index)
  {}

  /** empty constructor. This must be assigned another iterator before it is
   * useable.
   */
  TableRowIterator() {}

  /** returns the current table row.
   * @return the row or 0 if the end of the table has been reached.
   */
  RenderTableSection::RowStruct *operator *()
  {
    if (!sec) return 0;
    return &sec->grid[index];
  }

  /** advances to the next row
   */
  TableRowIterator &operator ++();

  /** advances to the previous row
   */
  TableRowIterator &operator --();

protected:
};

/** Iterates through the editable lines of a document, in a topological order.
 *
 * The differences between this and the EditableLineIterator lies in the way
 * lines are inquired. While the latter steps through the lines in document
 * order, the former takes into consideration ergonomics.
 *
 * This is especially useful for tables. EditableLineIterator traverses all
 * table cells from left to right, top to bottom, while this one will
 * actually snap to the cell in the right position, and traverse only
 * upwards/downwards, thus providing a more intuitive navigation.
 *
 * FIXME: This is not implemented yet, and might be superseded by some grandious
 * other idea of mine before it is.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class ErgonomicEditableLineIterator : public EditableLineIterator {
protected:
  int xCoor;		// x-coordinate to determine cell position with
public:
  /** Initializes a new ergonomic editable line iterator from the given one.
   * @param it line iterator
   * @param x absolute x-coordinate for cell determination
   */
  ErgonomicEditableLineIterator(const LineIterator &it, int x)
  	: EditableLineIterator(it), xCoor(x) {}

  /** Constructs an uninitialized iterator which must be assigned a line iterator before
   * it can be used.
   */
  ErgonomicEditableLineIterator() {}

  /** seek next line.
   *
   * The next line will be one that is visually situated below this line.
   */
  ErgonomicEditableLineIterator &operator ++();

  /** seek previous line.
   *
   * The previous line will be one that is visually situated above this line.
   */
  ErgonomicEditableLineIterator &operator --();

protected:
  /** determines the topologically next render object.
   * @param oldCell table cell the original object was under.
   * @param newObject object to determine whether and which transition
   *	between cells is to be handled. It need not be an object in the correct
   *	topological cell, a simple delivery from an editable line iterator suffices.
   * @param toBegin if @p true, iterate towards the beginning
   */
  void determineTopologicalElement(RenderTableCell *oldCell,
  		RenderObject *newObject, bool toBegin);

  /** initializes the iterator to point to the first previous/following editable
   * line.
   * @param newBlock take this as base block.
   * @param toBegin @p true, iterate towards beginning.
   */
  void calcAndStoreNewLine(RenderFlow *newBlock, bool toBegin);

#if 0
  /** compares whether two tables cells belong to the same table.
   *
   * If one or both cells are 0, @p false is returned.
   */
  static bool belongToSameTable(const RenderTableCell *t1, const RenderTableCell *t2)
  {
    return t1 && t2 && t1->table() == t2->table();
  }

  /** finds the cell corresponding to absolute x-coordinate @p x in the given
   * table section.
   *
   * If there is not direct cell, or the cell is not accessible, the function
   * will return the nearest suitable cell.
   * @param part part containing the document
   * @param x absolute x-coordinate
   * @param section table section to be searched
   * @param fromEnd @p true to begin search from end and work towards the
   *	beginning
   * @param startIndex start at this index (or < 0 to start with first/last row)
   * @return the cell, or 0 if no editable cell was found.
   */
  static RenderTableCell *findNearestTableCellInSection(KHTMLPart *part, int x,
  	RenderTableSection *section, bool fromEnd = false, int startIndex = -1);

  /** finds a suitable object beyond the given table.
   *
   * This method is to be called whenever there is no more row left
   * in the given table. It scans for a suitable line before/after the table,
   * or searches a table cell if another table happens to follow directly.
   * @param table table to start from
   * @param toBegin @p true, seek towards beginning, @p false, seek towards end.
   * @return a suitable, editable leaf render object, or 0 if the document
   *	boundary was reached.
   */
  RenderObject *findObjectBeyond(RenderTable *table, bool toBegin);
#endif
};

/**
 * Provides iterating through the document in terms of characters. Only the
 * editable characters are regarded.
 *
 * This iterator represents the document, which is structured as a tree itself,
 * as a linear stream of characters.
 */
class EditableCharacterIterator {
protected:
  LinearDocument *ld;
  EditableLineIterator _it;
  EditableInlineBoxIterator ebit;
  DOM::NodeImpl *_node;
  long _offset;
  int _char;

public:
  /** empty constructor.
   *
   * Only use if you want to assign another constructor as no fields will
   * be initialized.
   */
  EditableCharacterIterator() {}

  /** constructs a new iterator from the given linear document.
   *
   * @param ld linear representation of document.
   */
  EditableCharacterIterator(LinearDocument *ld)
  		: ld(ld), _it(ld->current()),
                ebit(_it, false, _it.currentInlineBox()), _char(-1)
  {
    _node = ld->node;
    _offset = ld->offset;

    // ### temporary fix for illegal nodes
    if (_it == ld->end()) { _node = 0; return; }

    // seeks the node's inline box
    // ### redundant, implement means to get it from ld or _it
    // ### if node is not there?
    EditableInlineBoxIterator copy = ebit;
    for (; *ebit; ++ebit) {
      copy = ebit;
      InlineBox *b = *ebit;

      if (b == _it.currentInlineBox() || b->object() == _node->renderer()) {
        _offset = QMIN(QMAX(_offset, b->minOffset()), b->maxOffset());
        break;
      }/*end if*/
    }/*next ebit*/
    // If no node is found, we take the last editable node. This is a very
    // feeble approximation as it sometimes makes the caret get stuck, or
    // iterate over the same element indefinitely,
    // but this covers up a case that should never happen in theory.
    if (!*ebit) {
      // this is a really bad hack but solves the caret-gets-stuck issue
      static long cache_offset = -1;
      ebit = copy;
      InlineBox *b = *ebit;
      _node = b->object()->element();
      long max_ofs = b->maxOffset();
      _offset = cache_offset == max_ofs ? b->minOffset() : max_ofs;
      cache_offset = _offset;
      kdDebug(6200) << "There was no node! Fixup applied!" << endl;
      if (cache_offset == max_ofs) kdDebug(6200) << "offset fixup applied as well" << endl;
    }/*end if*/

    initFirstChar();
  }

  /** returns the current character, or -1 if not on a text node, or beyond
   * the end.
   */
  int chr() const { return _char; }

  /** returns the current character as a unicode symbol, substituting
   * a blank for a non-text node.
   */
  QChar operator *() const { return QChar(_char >= 0 ? _char : ' '); }

  /** returns the current offset
   */
  long offset() const { return _offset; }
  /** returns the current node.
   *
   * If it's 0, then there are no more nodes.
   */
  DOM::NodeImpl *node() const { return _node; }
  /** returns the current inline box.
   *
   * May be 0 if the current element has none, or if the end has been reached.
   * Therefore, do *not* use this to test for the end condition, use node()
   * instead.
   */
  InlineBox *box() const { return *ebit; }
  /** moves to the next editable character.
   */
  EditableCharacterIterator &operator ++();

  /** moves to the previous editable character.
   */
  EditableCharacterIterator &operator --();

protected:
  /** initializes the _char member by reading the character at the current
   * offset, peeking ahead as necessary.
   */
  void initFirstChar();
  /** reads ahead the next node and updates the data structures accordingly
   */
  void peekNext()
  {
    EditableInlineBoxIterator copy = ebit;
    ++copy;
    InlineBox *b = *copy;
    if (b && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->str->s[b->minOffset()].unicode();
    else
      _char = -1;
  }
  /** reads ahead the previous node and updates the data structures accordingly
   */
  void peekPrev()
  {
    --ebit;
//    _peekPrev = *ebit;
  }

};

// == caret-related helper functions that need the above defined classes

/** generates a transient inline flow box.
 *
 * Empty blocks don't have inline flow boxes constructed. This function
 * constructs a dummy flow box which contains a single empty inline box with
 * coordinate information to be able to display the caret correctly.
 *
 * You can find out whether an inline flow box is transient by checking its
 * render object and the render object of its only child. If both are equal,
 * it's transient.
 *
 * @param arena render arena in which to create the flow box
 * @param cb containing block which to create the inline flow box for.
 * @return the constructed inline flow box.
 */
static InlineFlowBox* generateDummyFlowBox(RenderArena *arena, RenderFlow *cb)
{
  InlineFlowBox *flowBox = new(arena) InlineFlowBox(cb);
  int width = cb->width();
  // FIXME: this does neither take into regard :first-line nor :first-letter
  // However, as soon as some content is entered, the line boxes will be
  // constructed properly and this kludge is not called any more. So only
  // the caret size of an empty :first-line'd block is wrong, but I think we
  // can live with that.
  int height = cb->style()->fontMetrics().height();
  flowBox->setWidth(0);
  flowBox->setHeight(height);

  // Add single child at the right position
  InlineBox *child = new(arena) InlineBox(cb);
  // ### regard direction
  switch (cb->style()->textAlign()) {
    case LEFT:
    case TAAUTO:	// ### find out what this does
    case JUSTIFY:
      child->setXPos(0);
      break;
    case CENTER:
    case KONQ_CENTER:
      child->setXPos(width / 2);
      break;
    case RIGHT:
      child->setXPos(width);
      break;
  }/*end switch*/
  child->setYPos(0);
  child->setWidth(1);
  child->setHeight(height);

  flowBox->setXPos(child->xPos());
  flowBox->setYPos(child->yPos());
  flowBox->addToLine(child);
  //kdDebug(6200) << "generateDummyFlowBox: " << flowBox << " with child " << child << endl;
  return flowBox;
}

/** generates a dummy block for elements whose containing block is not a flow.
 * @param arena render arena in which to create the flow
 * @param cb block which to create the flow for.
 * @return the constructed flow.
 */
static RenderFlow* generateDummyBlock(RenderArena *arena, RenderObject *cb)
{
  RenderFlow *result = new(arena) RenderFlow(cb->element());
  result->setParent(cb->parent());
  result->setPreviousSibling(cb->previousSibling());
  result->setNextSibling(cb->nextSibling());

  result->setOverhangingContents(cb->overhangingContents());
  result->setPositioned(cb->isPositioned());
  result->setRelPositioned(cb->isRelPositioned());
  result->setFloating(cb->isFloating());
  result->setInline(cb->isInline());
  result->setMouseInside(cb->mouseInside());

  result->setStyle(cb->style());	// ### will fail if positioned

  result->setPos(cb->xPos(), cb->yPos());
  result->setWidth(cb->width());
  result->setHeight(cb->height());

  return result;
}

/** determines the inline flow box that contains the given node.
 *
 * If the node does not map to an inline flow box, the function will snap to
 * the next inline box following it.
 *
 * @param node node to begin with
 * @param offset zero-based offset within node.
 * @param arena sometimes the function must create a temporary inline flow box
 *	therefore it needs a render arena.
 * @param cb returns the containing block
 * @param ibox returns the inline box that contains the node.
 * @return the found inlineFlowBox or 0 if either the node is 0 or
 *	there is no inline flow box containing this node. The containing block
 *	will still be set. If it is 0 too, @p node was invalid.
 */
static InlineFlowBox* findFlowBox(DOM::NodeImpl *node, long offset,
		RenderArena *arena, RenderFlow *&cb, InlineBox **ibox)
{
  RenderObject *r = findRenderer(node);
  if (!r) { cb = 0; return 0; }
  kdDebug(6200) << "=================== findFlowBox" << endl;
  kdDebug(6200) << "node " << node << " r " << r->renderName() << "[" << r << "].node " << r->element()->nodeName().string() << "[" << r->element() << "]" << " offset: " << offset << endl;

  // If we have a totally empty render block, we simply construct a
  // transient inline flow box, and be done with it.
  // This case happens only when the render block is a leaf object itself.
  if (r->isFlow() && !static_cast<RenderFlow *>(r)->firstLineBox()) {
    cb = static_cast<RenderFlow *>(r);
  kdDebug(6200) << "=================== end findFlowBox (dummy)" << endl;
    InlineFlowBox *fb = generateDummyFlowBox(arena, cb);
    if (ibox) *ibox = fb;
    return fb;
  }/*end if*/

  // There are two strategies to find the correct line box.
  // (A) First, if node's renderer is a RenderText, we only traverse its text
  // runs and return the root line box (saves much time for long blocks).
  // This should be the case 99% of the time.
  // (B) Otherwise, we iterate linearly through all line boxes in order to find
  // the renderer. (A reverse mapping would be favourable, but needs memory)
  if (r->isText()) do {
    RenderText *t = static_cast<RenderText *>(r);
    int dummy;
    InlineBox *b = t->findInlineTextBox(offset, dummy, true);
    // Actually b should never be 0, but some render texts don't have text
    // boxes, so we insert the last run as an error correction.
    // If there is no last run, we resort to (B)
    if (!b) {
      if (t->m_lines.count() > 0)
        b = t->m_lines[t->m_lines.count() - 1];
      else
        break;
    }/*end if*/
    Q_ASSERT(b);
    if (ibox) *ibox = b;
    while (b->parent()) {	// seek root line box
      b = b->parent();
    }/*wend*/
    // FIXME: replace with isRootInlineBox after full WebCore merge.
    //Q_ASSERT(b->isRootInlineBox());
    Q_ASSERT(b->isInlineFlowBox());
    cb = static_cast<RenderFlow *>(b->object());
    Q_ASSERT(cb->isFlow());
  kdDebug(6200) << "=================== end findFlowBox (renderText)" << endl;
    return static_cast<InlineFlowBox *>(b);
  } while(false);/*end if*/

  cb = static_cast<RenderFlow *>(r->containingBlock());
  if (!cb->isFlow()) {
    cb = generateDummyBlock(arena, cb);
    kdDebug(6200) << "dummy block created: " << cb << endl;
  }/*end if*/

  InlineFlowBox *flowBox = cb->firstLineBox();
  // This case strikes when there are children but none of it is represented
  // by an inline box (for example, all of them are empty:
  // <div><b></b><i></i></div>)
  if (!flowBox) {
    flowBox = generateDummyFlowBox(arena, cb);
    if (ibox) *ibox = flowBox;
  kdDebug(6200) << "=================== end findFlowBox (2)" << endl;
    return flowBox;
  }/*end if*/

  // We iterate the inline flow boxes of the containing block until
  // we find the given node. This has one major flaw: it is linear, and therefore
  // painfully slow for really large blocks.
  for (; flowBox; flowBox = static_cast<InlineFlowBox *>(flowBox->nextLineBox())) {
    kdDebug(6200) << "[scan line]" << endl;

    // Iterate children, and look for node
    InlineBox *box;
    InlineBoxIterator it(arena, flowBox);
    for (; (box = *it) != 0; ++it) {
      RenderObject *br = box->object();
      if (!br) continue;

      kdDebug(6200) << "box->obj " << br->renderName() << "[" << br << "]" << " minOffset: " << box->minOffset() << " maxOffset: " << box->maxOffset() << endl;
      if (br == r && offset >= box->minOffset() && offset <= box->maxOffset())
        break;	// If Dijkstra hadn't brainwashed me, I'd have used a goto here
    }/*next it*/
    if (box) {
      if (ibox) *ibox = box;
      break;
    }

  }/*next flowBox*/

  // no inline flow box found, approximate to nearest following node.
  // Danger: this is O(n^2). It's only called to recover from
  // errors, that means, theoretically, never. (Practically, far too often :-( )
  if (!flowBox) flowBox = findFlowBox(node->nextLeafNode(), 0, arena, cb, ibox);

  kdDebug(6200) << "=================== end findFlowBox" << endl;
  return flowBox;
}

/** finds the innermost table object @p r is contained within, but no
 * farther than @p cb.
 * @param r leaf element to begin with
 * @param cb bottom element where to stop search at least.
 * @return the table object or 0 if none found.
 */
inline RenderTable *findTableUpTo(RenderObject *r, RenderFlow *cb)
{
  while (r && r != cb && !r->isTable()) r = r->parent();
  return r && r->isTable() ? static_cast<RenderTable *>(r) : 0;
}

/** checks whether @p r is a descendant of @p cb.
 */
inline bool isDescendant(RenderObject *r, RenderObject *cb)
{
  while (r && r != cb) r = r->parent();
  return r;
}

/** checks whether the given block contains at least one editable element.
 *
 * Warning: This function has linear complexity, and therefore is expensive.
 * Use it sparingly, and cache the result.
 * @param part part
 * @param cb block to be searched
 * @param table returns the nested table if there is one directly at the beginning
 *	or at the end.
 * @param fromEnd begin search from end (default: begin from beginning)
 */
static bool containsEditableElement(KHTMLPart *part, RenderFlow *cb,
	RenderTable *&table, bool fromEnd = false)
{
  RenderObject *r = cb;
  if (fromEnd)
    while (r->lastChild()) r = r->lastChild();
  else
    while (r->firstChild()) r = r->firstChild();

  RenderTable *tempTable = 0;
  table = 0;
  bool withinCb;
  do {
    tempTable = findTableUpTo(r, cb);
    withinCb = isDescendant(r, cb);

    kdDebug(6201) << "r " << (r ? r->renderName() : QString::null) << "@" << r << endl;
    if (r && withinCb && r->element()
    	&& (part->isCaretMode() || part->isEditable()
		|| r->style()->userInput() == UI_ENABLED)) {
      table = tempTable;
      return true;
    }/*end if*/

    r = fromEnd ? prevLeafRenderObject(r) : nextLeafRenderObject(r);
  } while (r && withinCb);
  return false;
}

/** checks whether the given block contains at least one editable child
 * element, beginning with but excluding @p start.
 *
 * Warning: This function has linear complexity, and therefore is expensive.
 * Use it sparingly, and cache the result.
 * @param part part
 * @param cb block to be searched
 * @param table returns the nested table if there is one directly before/after
 *	the start object.
 * @param fromEnd begin search from end (default: begin from beginning)
 * @param start object after which to begin search.
 */
static bool containsEditableChildElement(KHTMLPart *part, RenderFlow *cb,
	RenderTable *&table, bool fromEnd, RenderObject *start)
{
  RenderObject *r = start;
  if (fromEnd)
    while (r->firstChild()) r = r->firstChild();
  else
    while (r->lastChild()) r = r->lastChild();

  if (!r) return false;

  RenderTable *tempTable = 0;
  table = 0;
  bool withinCb = false;
  do {
    r = fromEnd ? prevLeafRenderObject(r) : nextLeafRenderObject(r);
    if (!r) break;

    withinCb = isDescendant(r, cb) && r != cb;
    tempTable = findTableUpTo(r, cb);

    kdDebug(6201) << "r " << (r ? r->renderName() : QString::null) << "@" << r << endl;
    if (r && withinCb && r->element()
    	&& (part->isCaretMode() || part->isEditable()
		|| r->style()->userInput() == UI_ENABLED)) {
      table = tempTable;
      return true;
    }/*end if*/

  } while (withinCb);
  return false;
}

// == class LinearDocument implementation

LinearDocument::LinearDocument(KHTMLPart *part, NodeImpl *node, long offset)
	: arena(0), node(node), offset(offset), m_part(part)
{
  if (node == 0) return;
  sanitizeCaretState(this->node, this->offset);

  arena = new RenderArena(512);

  initPreBeginIterator();
  initEndIterator();
  //m_part = node->getDocument()->view()->part();
}

LinearDocument::~LinearDocument()
{
  delete arena;
}

int LinearDocument::count() const
{
  // FIXME: not implemented
  return 1;
}

LinearDocument::Iterator LinearDocument::current()
{
  return LineIterator(this, node, offset);
}

LinearDocument::Iterator LinearDocument::begin()
{
  DocumentImpl *doc = node ? node->getDocument() : 0;
  if (!doc) return end();

  NodeImpl *firstLeaf = doc->nextLeafNode();
  if (!firstLeaf) return end();		// must be empty document (is this possible?)
  return LineIterator(this, firstLeaf, firstLeaf->minOffset());
}

LinearDocument::Iterator LinearDocument::preEnd()
{
  DocumentImpl *doc = node ? node->getDocument() : 0;
  if (!doc) return preBegin();

  NodeImpl *lastLeaf = doc;
  while (lastLeaf->lastChild()) lastLeaf = lastLeaf->lastChild();

  if (!lastLeaf) return preBegin();	// must be empty document (is this possible?)
  return LineIterator(this, lastLeaf, lastLeaf->maxOffset());
}

void LinearDocument::initPreBeginIterator()
{
  _preBegin = LineIterator(this, 0, 0);
}

void LinearDocument::initEndIterator()
{
  _end = LineIterator(this, 0, 1);
}

// == class LineIterator implementation

LineIterator::LineIterator(LinearDocument *l, DOM::NodeImpl *node, long offset)
		: lines(l)
{
//  kdDebug(6200) << "LineIterator: node " << node << " offset " << offset << endl;
  flowBox = findFlowBox(node, offset, lines->arena, cb, &currentBox);
  if (!flowBox) {
    kdDebug(6200) << "LineIterator: findFlowBox failed" << endl;
    cb = 0;
  }/*end if*/
}

void LineIterator::nextBlock()
{
  RenderObject *r = cb;
  RenderObject *n = r->lastChild();
  while (n) r = n, n = r->lastChild();
  r = nextLeafRenderObject(r);
  kdDebug(6200) << "++: r " << r << "[" << (r?r->renderName():QString::null) << "]" << endl;
  if (!r) {
    cb = 0;
    return;
  }/*end if*/

  // If we hit a leaf block (which can happen on empty blocks), use this
  // as its containing block
  if (r->isFlow())
    cb = static_cast<RenderFlow *>(r);
  else {
    cb = static_cast<RenderFlow *>(r->containingBlock());
    if (!cb->isFlow()) {
      kdDebug(6200) << "dummy cb created " << cb << endl;
      cb = generateDummyBlock(lines->arena, r);
    }/*end if*/
  }/*end if*/
  flowBox = cb->firstLineBox();
  kdDebug(6200) << "++: flowBox " << flowBox << endl;

  if (!flowBox) flowBox = generateDummyFlowBox(lines->arena, cb);
  if (!cb->firstLineBox()) kdDebug(6200) << "++: dummy flowBox " << flowBox << endl;
}

inline LineIterator &LineIterator::operator ++()
{
  flowBox = static_cast<InlineFlowBox *>(flowBox->nextLineBox());

  // if there are no more lines in this block, begin with first line of
  // next block
  if (!flowBox) nextBlock();

  return *this;
}

inline LineIterator LineIterator::operator ++(int)
{
  LineIterator it(*this);
  operator ++();
  return it;
}

void LineIterator::prevBlock()
{
  RenderObject *r = cb;
  RenderObject *n = r->firstChild();
  while (n) r = n, n = r->firstChild();
  r = prevLeafRenderObject(r);
  if (!r) {
    cb = 0;
    return;
  }/*end if*/

  // If we hit a leaf block (which can happen on empty blocks), use this
  // as its containing block
  if (r->isFlow())
    cb = static_cast<RenderFlow *>(r);
  else {
    cb = static_cast<RenderFlow *>(r->containingBlock());
    if (!cb->isFlow()) {
      kdDebug(6200) << "dummy cb created " << cb << endl;
      cb = generateDummyBlock(lines->arena, r);
    }/*end if*/
  }/*end if*/
  flowBox = cb->lastLineBox();

  if (!flowBox) flowBox = generateDummyFlowBox(lines->arena, cb);
}

inline LineIterator &LineIterator::operator --()
{
  flowBox = static_cast<InlineFlowBox *>(flowBox->prevLineBox());

  // if there are no more lines in this block, begin with last line of
  // previous block
  if (!flowBox) prevBlock();

  return *this;
}

inline LineIterator LineIterator::operator --(int)
{
  LineIterator it(*this);
  operator --();
  return it;
}

#if 0 // not implemented because it's not needed
LineIterator LineIterator::operator +(int /*summand*/) const
{
  // FIXME: not implemented
  return LineIterator();
}

LineIterator LineIterator::operator -(int /*summand*/) const
{
  // FIXME: not implemented
  return LineIterator();
}
#endif

LineIterator &LineIterator::operator +=(int summand)
{
  if (summand > 0)
    while (summand-- && *this != lines->end()) ++*this;
  else if (summand < 0)
    operator -=(-summand);
  return *this;
}

LineIterator &LineIterator::operator -=(int summand)
{
  if (summand > 0)
    while (summand-- && *this != lines->preBegin()) --*this;
  else if (summand < 0)
    operator +=(-summand);
  return *this;
}

// == class EditableCharacterIterator implementation

void EditableCharacterIterator::initFirstChar()
{
  InlineBox *b = *ebit;
  if (b) {
    if (_offset == b->maxOffset())
      peekNext();
    else if (b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->str->s[_offset].unicode();
    else
      _char = -1;
  }/*end if*/
}

EditableCharacterIterator &EditableCharacterIterator::operator ++()
{
  _offset++;

  InlineBox *b = *ebit;
  RenderObject *r = b->object();
  // BRs have no extent, so their maximum offset must be their minimum.
  // A block element can only be the target if it is empty -- in this case
  // its extent is zero, too.
  long maxofs = r->isBR() || r->isFlow() ? b->minOffset() : b->maxOffset();
  kdDebug(6200) << "b->maxOffset() " << b->maxOffset() << " b->minOffset() " << b->minOffset() << endl;
  if (_offset == maxofs) {
kdDebug(6200) << "_offset == maxofs: " << _offset << " == " << maxofs << endl;
//    _peekPrev = b;
    peekNext();
  } else if (_offset > maxofs) {
kdDebug(6200) << "_offset > maxofs: " << _offset << " > " << maxofs /*<< " _peekNext: " << _peekNext*/ << endl;
    if (/*!_peekNext*/true) {
      if (*ebit)
        ++ebit;
      if (!*ebit) {		// end of line reached, go to next line
        ++_it;
kdDebug(6200) << "++_it" << endl;
        if (_it != ld->end()) {
	  ebit = _it;
          b = *ebit;
kdDebug(6200) << "b " << b << " isText " << b->isInlineTextBox() << endl;
	  _node = b->object()->element();
kdDebug(6200) << "_node " << _node << ":" << _node->nodeName().string() << endl;
	  _offset = b->minOffset();
kdDebug(6200) << "_offset " << _offset << endl;
	} else {
	  _node = 0;
	  b = 0;
	}/*end if*/
        goto readchar;
      }/*end if*/
    }/*end if*/
    bool adjacent = ebit.isAdjacent();
    // Jump over element if this one is not a text node.
    if (adjacent && !(*ebit)->isInlineTextBox()) {
      EditableInlineBoxIterator copy = ebit;
      ++ebit;
      if (*ebit && (*ebit)->isInlineTextBox()) adjacent = false;
      else ebit = copy;
    }/*end if*/
    _node = (*ebit)->object()->element();
    _offset = (*ebit)->minOffset() + adjacent;
    //_peekNext = 0;
    b = *ebit;
    goto readchar;
  } else {
readchar:
    // get character
    if (b && b->isInlineTextBox() && _offset < b->maxOffset())
      _char = static_cast<RenderText *>(b->object())->str->s[_offset].unicode();
    else
      _char = -1;
  }/*end if*/
kdDebug(6200) << "_offset: " << _offset /*<< " _peekNext: " << _peekNext*/ << " char '" << (char)_char << "'" << endl;

  if (*ebit) {
    InlineBox *box = *ebit;
    kdDebug(6200) << "echit++(1): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << " node " << (_node ? _node->nodeName().string() : QString("<nil>")) << endl;
  }
  return *this;
}

EditableCharacterIterator &EditableCharacterIterator::operator --()
{
  _offset--;
  //kdDebug(6200) << "--: _offset=" << _offset << endl;

  InlineBox *b = *ebit;
  InlineBox *_peekPrev = 0;
  InlineBox *_peekNext = 0;
  long minofs = b ? b->minOffset() : _offset + 1;
  kdDebug(6200) << "b->maxOffset() " << b->maxOffset() << " b->minOffset() " << b->minOffset() << endl;
  if (_offset == minofs) {
kdDebug(6200) << "_offset == minofs: " << _offset << " == " << minofs << endl;
    _peekNext = b;
    // get character
    if (b && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->text()[_offset].unicode();
    else
      _char = -1;

    //peekPrev();
    bool do_prev = false;
    {
      EditableInlineBoxIterator copy = ebit;
      --ebit;
      _peekPrev = *ebit;
      // Jump to end of previous element if it's adjacent, and a text box
      if (ebit.isAdjacent() && *ebit && (*ebit)->isInlineTextBox())
        //operator --();
	do_prev = true;
      else
        ebit = copy;
    }
    if (do_prev) goto prev;
  } else if (_offset < minofs) {
prev:
kdDebug(6200) << "_offset < minofs: " << _offset << " < " << minofs /*<< " _peekNext: " << _peekNext*/ << endl;
    if (!_peekPrev) {
      _peekNext = *ebit;
      if (*ebit)
        --ebit;
      if (!*ebit) {		// end of line reached, go to previous line
        --_it;
kdDebug(6200) << "--_it" << endl;
        if (_it != ld->preBegin()) {
//	  kdDebug(6200) << "begin from end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	  ebit = EditableInlineBoxIterator(_it, true);
	  RenderObject *r = (*ebit)->object();
kdDebug(6200) << "b " << *ebit << " isText " << (*ebit)->isInlineTextBox() << endl;
	  _node = r->element();
	  _offset = r->isBR() ? (*ebit)->minOffset() : (*ebit)->maxOffset();
	  _char = -1;
          {InlineBox *box = *ebit; kdDebug(6200) << "echit--(2): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;}
	} else
	  _node = 0;
	return *this;
      }/*end if*/
    }/*end if*/

    bool adjacent = ebit.isAdjacent();
    // Ignore this box if it isn't a text box, but the previous box was
    kdDebug(6200) << "adjacent " << adjacent << " _peekNext " << _peekNext << " _peekNext->isInlineTextBox: " << (_peekNext ? _peekNext->isInlineTextBox() : false) << " !((*ebit)->isInlineTextBox): " << (*ebit ? !(*ebit)->isInlineTextBox() : true) << endl;
    if (adjacent && _peekNext && _peekNext->isInlineTextBox()
    	&& !(*ebit)->isInlineTextBox()) {
      EditableInlineBoxIterator copy = ebit;
      --ebit;
      if (!*ebit) /*adjacent = false;
      else */ebit = copy;
    }/*end if*/
    kdDebug(6200) << "(*ebit)->obj " << (*ebit)->object()->renderName() << "[" << (*ebit)->object() << "]" << " minOffset: " << (*ebit)->minOffset() << " maxOffset: " << (*ebit)->maxOffset() << endl;
    _node = (*ebit)->object()->element();
kdDebug(6200) << "_node " << _node << ":" << _node->nodeName().string() << endl;
    _offset = (*ebit)->maxOffset()/* - adjacent*/;
kdDebug(6200) << "_offset " << _offset << endl;
    _peekPrev = 0;
  } else {
kdDebug(6200) << "_offset: " << _offset << " _peekNext: " << _peekNext << endl;
    // get character
    if (_peekNext && _offset >= b->maxOffset() && _peekNext->isInlineTextBox())
      _char = static_cast<RenderText *>(_peekNext->object())->text()[_peekNext->minOffset()].unicode();
    else if (b && _offset < b->maxOffset() && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->text()[_offset].unicode();
    else
      _char = -1;
  }/*end if*/

  if (*ebit) {
    InlineBox *box = *ebit;
    kdDebug(6200) << "echit--(1): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;
  }
  return *this;
}

// == class TableRowIterator implementation

/** iterates through the sections of a table.
 * @param table table
 * @param section section to start with, 0 if beyond table
 * @param toBegin @p true if iterate towards beginning, @p false if iterate
 *	towards end.
 * @return the next section or 0 if end of table has been reached.
 */
static RenderTableSection *iterateTableSections(RenderTable *table,
		RenderTableSection *section, bool toBegin)
{
  if (!section) {
    if (toBegin)
      section = static_cast<RenderTableSection *>(table->lastChild());	// ### are sections direct children of RenderTable?
    else
      section = table->firstBodySection();
    if (section->isTableSection()) return section;
  }/*end if*/

  do {
    section = static_cast<RenderTableSection *>(toBegin
  		? section->previousSibling() : section->nextSibling());
  } while (section && !section->isTableSection());
  return section;
}

TableRowIterator::TableRowIterator(RenderTable *table, bool fromEnd,
  		RenderTableSection::RowStruct *row)
{
  sec = iterateTableSections(table, 0, fromEnd);

  // set index
  if (sec) {
    if (fromEnd) index = sec->grid.size() - 1;
    else index = 0;
  }/*end if*/

  // initialize with given row
  if (row && sec) {
    while (operator *() != row)
      if (fromEnd) operator --(); else operator ++();
  }/*end if*/
}

TableRowIterator &TableRowIterator::operator ++()
{
  index++;

  if (index >= (int)sec->grid.size()) {
    sec = iterateTableSections(0, sec, false);

    if (sec) index = 0;
  }/*end if*/
  return *this;
}

TableRowIterator &TableRowIterator::operator --()
{
  index--;

  if (index < 0) {
    sec = iterateTableSections(0, sec, true);

    if (sec) index = sec->grid.size() - 1;
  }/*end if*/
  return *this;
}

// == class ErgonomicEditableLineIterator implementation

// some decls
static RenderTableCell *findNearestTableCellInRow(KHTMLPart *part, int x,
		RenderTableSection::RowStruct *row, bool fromEnd);

/** finds the cell corresponding to absolute x-coordinate @p x in the given
 * table.
 *
 * If there is no direct cell, or the cell is not accessible, the function
 * will return the nearest suitable cell.
 * @param part part containing the document
 * @param x absolute x-coordinate
 * @param it table row iterator, will be adapted accordingly as more rows are
 *	investigated.
 * @param fromEnd @p true to begin search from end and work towards the
 *	beginning
 * @return the cell, or 0 if no editable cell was found.
 */
inline RenderTableCell *findNearestTableCell(KHTMLPart *part, int x,
		TableRowIterator &it, bool fromEnd)
{
  RenderTableCell *result = 0;

  while (*it) {
    result = findNearestTableCellInRow(part, x, *it, fromEnd);
    if (result) break;

    if (fromEnd) --it; else ++it;
  }/*wend*/

  return result;
}

/** finds the nearest editable cell around the given absolute x-coordinate
 *
 * It will dive into nested tables as necessary to provide seamless navigation.
 *
 * If the cell at @p x is not editable, its left neighbour is tried, then its
 * right neighbour, then the left neighbour's left neighbour etc. If no
 * editable cell can be found, 0 is returned.
 * @param part khtml part
 * @param x absolute x-coordinate
 * @param row table row to be searched
 * @param fromEnd @p true, begin from end (applies only for nested tables)
 * @return the found cell or 0 if no editable cell was found
 */
static RenderTableCell *findNearestTableCellInRow(KHTMLPart *part, int x,
		RenderTableSection::RowStruct *row, bool fromEnd)
{
  // First pass. Find spatially nearest cell.
  int n = (int)row->row->size();
  int i;
  for (i = 0; i < n; i++) {
    RenderTableCell *cell = row->row->at(i);
    if (!cell || (int)cell == -1) continue;

    int absx, absy;
    cell->absolutePosition(absx, absy, false); // ### position: fixed?
    kdDebug(6201) << "i/n " << i << "/" << n << " absx " << absx << " absy " << absy << endl;

    // I rely on the assumption that all cells are in ascending visual order
    // ### maybe this assumption is wrong for bidi?
    kdDebug(6201) << "x " << x << " < " << (absx + cell->width()) << "?" << endl;
    if (x < absx + cell->width()) break;
  }/*next i*/
  if (i >= n) i = n - 1;

  // Second pass. Find editable cell, beginning with the currently found,
  // extending to the left, and to the right, alternating.
  for (int cnt = 0; cnt < 2*n; cnt++) {
    int index = i - ((cnt >> 1) + 1)*(cnt & 1) + (cnt >> 1)*!(cnt & 1);
    if (index < 0 || index >= n) continue;

    RenderTableCell *cell = row->row->at(index);
    if (!cell || (int)cell == -1) continue;

    kdDebug(6201) << "index " << index << " cell " << cell << endl;
    RenderTable *nestedTable;
    if (containsEditableElement(part, cell, nestedTable, fromEnd)) {

      if (nestedTable) {
        TableRowIterator it(nestedTable, fromEnd);
	while (*it) {
	  cell = findNearestTableCell(part, x, it, fromEnd);
	  if (cell) break;
	  if (fromEnd) --it; else ++it;
	}/*wend*/
      }/*end if*/

      return cell;
    }/*end if*/
  }/*next i*/
  return 0;
}

/** returns the nearest common ancestor of two objects that is a table cell,
 * a table section, or 0 if not inside a common table.
 *
 * If @p r1 and @p r2 belong to the same table, but different sections, @p r1's
 * section is returned.
 */
static RenderObject *commonAncestorTableSectionOrCell(RenderObject *r1,
		RenderObject *r2)
{
  if (!r1 || !r2) return 0;
  RenderTableSection *sec = 0;
  int start_depth=0, end_depth=0;
  // First we find the depths of the two objects in the tree (start_depth, end_depth)
  RenderObject *n = r1;
  while (n->parent()) {
    n = n->parent();
    start_depth++;
  }/*wend*/
  n = r2;
  while( n->parent()) {
    n = n->parent();
    end_depth++;
  }/*wend*/
  // here we climb up the tree with the deeper object, until both objects have equal depth
  while (end_depth > start_depth) {
    r2 = r2->parent();
    end_depth--;
  }/*wend*/
  while (start_depth > end_depth) {
    r1 = r1->parent();
//    if (r1->isTableSection()) sec = static_cast<RenderTableSection *>(r1);
    start_depth--;
  }/*wend*/
  // Climb the tree with both r1 and r2 until they are the same
  while (r1 != r2){
    r1 = r1->parent();
    if (r1->isTableSection()) sec = static_cast<RenderTableSection *>(r1);
    r2 = r2->parent();
  }/*wend*/

  // At this point, we found the most approximate common ancestor. Now climb
  // up until the condition of the function return value is satisfied.
  while (r1 && !r1->isTableCell() && !r1->isTableSection() && !r1->isTable())
    r1 = r1->parent();

  return r1 && r1->isTable() ? sec : r1;
}

/** Finds the row that contains the given cell, directly, or indirectly
 * @param section section to be searched
 * @param cell table cell
 * @param row returns the row
 * @param directCell returns the direct cell that contains @p cell
 * @return the index of the row.
 */
static int findRowInSection(RenderTableSection *section, RenderTableCell *cell,
		RenderTableSection::RowStruct *&row, RenderTableCell *&directCell)
{
  // Seek direct cell
  RenderObject *r = cell;
  while (r != section) {
    if (r->isTableCell()) directCell = static_cast<RenderTableCell *>(r);
    r = r->parent();
  }/*wend*/

  // So, and this is really nasty: As we have no indices, we have to do a
  // linear comparison. Oh, that sucks so much for long tables, you can't
  // imagine.
  int n = section->numRows();
  for (int i = 0; i < n; i++) {
    row = &section->grid[i];

    // check for cell
    int m = row->row->size();
    for (int j = 0; j < m; j++) {
      RenderTableCell *c = row->row->at(j);
      if (c == directCell) return i;
    }/*next j*/

  }/*next i*/
  Q_ASSERT(false);
  return -1;
}

/** finds the table that is the first direct or indirect descendant of @p block.
 * @param leaf object to begin search from.
 * @param block object to search to, or 0 to search up to top.
 * @return the table or 0 if there were none.
 */
inline RenderTable *findFirstDescendantTable(RenderObject *leaf, RenderFlow *block)
{
  RenderTable *result = 0;
  while (leaf && leaf != block) {
    if (leaf->isTable()) result = static_cast<RenderTable *>(leaf);
    leaf = leaf->parent();
  }/*wend*/
  return result;
}

/** looks for the table cell the given object @p r is contained within.
 * @return the table cell or 0 if not contained in any table.
 */
inline RenderTableCell *containingTableCell(RenderObject *r)
{
  while (r && !r->isTableCell()) r = r->parent();
  return static_cast<RenderTableCell *>(r);
}

inline void ErgonomicEditableLineIterator::calcAndStoreNewLine(
			RenderFlow *newBlock, bool toBegin)
{
  // take the first/last editable element in the found cell as the new
  // value for the iterator
  cb = newBlock;
  if (toBegin) prevBlock(); else nextBlock();

  if (!cb) {
    flowBox = 0;
    return;
  }/*end if*/

  if (!isEditable(*this)) {
    if (toBegin) EditableLineIterator::operator --();
    else EditableLineIterator::operator ++();
  }/*end if*/
}

void ErgonomicEditableLineIterator::determineTopologicalElement(
		RenderTableCell *oldCell, RenderObject *newObject, bool toBegin)
{
  // When we arrive here, a transition between cells has happened.
  // Now determine the type of the transition. This can be
  // (1) a transition from this cell into a table inside this cell.
  // (2) a transition from this cell into another cell of this table

  TableRowIterator it;

  RenderObject *commonAncestor = commonAncestorTableSectionOrCell(oldCell, newObject);
  kdDebug(6201) << " ancestor " << commonAncestor << endl;

  // The whole document is treated as a table cell.
  if (!commonAncestor || commonAncestor->isTableCell()) {	// (1)

    RenderTableCell *cell = static_cast<RenderTableCell *>(commonAncestor);
    RenderTable *table = findFirstDescendantTable(newObject, cell);

    kdDebug(6201) << "table cell: " << cell << endl;

    // if there is no table, we fell out of the previous table, and are now
    // in some table-less block. Therefore, done.
    if (!table) return;

    it = TableRowIterator(table, toBegin);

  } else if (commonAncestor->isTableSection()) {		// (2)

    RenderTableSection *section = static_cast<RenderTableSection *>(commonAncestor);
    RenderTableSection::RowStruct *row;
    int idx = findRowInSection(section, oldCell, row, oldCell);
    kdDebug(6201) << "table section: row idx " << idx << endl;

    it = TableRowIterator(section, idx);

    // advance rowspan rows
    int rowspan = oldCell->rowSpan();
    while (*it && rowspan--) {
      if (toBegin) --it; else ++it;
    }/*wend*/

  } else {
    kdError(6201) << "Neither common cell nor section! " << commonAncestor->renderName() << endl;
    // will crash on uninitialized table row iterator
  }/*end if*/

  RenderTableCell *cell = findNearestTableCell(lines->m_part, xCoor, it, toBegin);
  kdDebug(6201) << "findNearestTableCell result: " << cell << endl;

  RenderFlow *newBlock = cell;
  if (!cell) {
    Q_ASSERT(commonAncestor->isTableSection());
    RenderTableSection *section = static_cast<RenderTableSection *>(commonAncestor);
    cell = containingTableCell(section);
    kdDebug(6201) << "containing cell: " << cell << endl;

    RenderTable *nestedTable;
    bool editableChild = cell && containsEditableChildElement(lines->m_part,
    		cell, nestedTable, toBegin, section->table());

    if (cell && !editableChild) {
      kdDebug(6201) << "========= recursive invocation outer =========" << endl;
      determineTopologicalElement(cell, cell->section(), toBegin);
      kdDebug(6201) << "========= end recursive invocation outer =========" << endl;
      return;

    } else if (cell && nestedTable) {
      kdDebug(6201) << "========= recursive invocation inner =========" << endl;
      determineTopologicalElement(cell, nestedTable, toBegin);
      kdDebug(6201) << "========= end recursive invocation inner =========" << endl;
      return;

    } else {
      kdDebug(6201) << "newBlock is table: " << section->table() << endl;
      newBlock = section->table();
//      if (toBegin) prevBlock(); else nextBlock();
    }/*end if*/
  } else {
    // adapt cell so that prevBlock/nextBlock works as expected
    RenderObject *r = cell;
    if (toBegin) {
      while (r->lastChild()) r = r->lastChild();
      r = nextLeafRenderObject(r);
    } else
      r = prevLeafRenderObject(r);
    newBlock = static_cast<RenderFlow *>(!r || r->isFlow() ? r : r->containingBlock());
  }/*end if*/

  calcAndStoreNewLine(newBlock, toBegin);
}

ErgonomicEditableLineIterator &ErgonomicEditableLineIterator::operator ++()
{
  RenderTableCell *oldCell = containingTableCell(cb);

  EditableLineIterator::operator ++();
  if (*this == lines->end() || *this == lines->preBegin()) return *this;

  RenderTableCell *newCell = containingTableCell(cb);

  if (!newCell || newCell == oldCell) return *this;

  determineTopologicalElement(oldCell, newCell, false);

  return *this;
}

ErgonomicEditableLineIterator &ErgonomicEditableLineIterator::operator --()
{
  RenderTableCell *oldCell = containingTableCell(cb);

  EditableLineIterator::operator --();
  if (*this == lines->end() || *this == lines->preBegin()) return *this;

  RenderTableCell *newCell = containingTableCell(cb);

  if (!newCell || newCell == oldCell) return *this;

  determineTopologicalElement(oldCell, newCell, true);

  return *this;
}

}/*end namespace*/

void KHTMLView::initCaret(bool keepSelection)
{
  kdDebug(6200) << "begin initCaret" << endl;
  // save caretMoved state as moveCaretTo changes it
  if (m_part->xmlDocImpl()) {
    d->caretViewContext();
    bool cmoved = d->m_caretViewContext->caretMoved;
    if (m_part->d->caretNode().isNull()) {
      // set to document, position will be sanitized anyway
      m_part->d->caretNode() = m_part->document();
      m_part->d->caretOffset() = 0L;
      // This sanity check is necessary for the not so unlikely case that
      // setEditable or setCaretMode is called before any render objects have
      // been created.
      if (!m_part->d->caretNode().handle()->renderer()) return;
    }/*end if*/
//    kdDebug(6200) << "d->m_selectionStart " << m_part->d->m_selectionStart.handle()
//    		<< " d->m_selectionEnd " << m_part->d->m_selectionEnd.handle() << endl;
    // ### does not repaint the selection on keepSelection!=false
    moveCaretTo(m_part->d->caretNode().handle(), m_part->d->caretOffset(), !keepSelection);
//    kdDebug(6200) << "d->m_selectionStart " << m_part->d->m_selectionStart.handle()
//    		<< " d->m_selectionEnd " << m_part->d->m_selectionEnd.handle() << endl;
    d->m_caretViewContext->caretMoved = cmoved;
  }/*end if*/
  kdDebug(6200) << "end initCaret" << endl;
}

bool KHTMLView::caretOverrides()
{
    bool cm = m_part->isCaretMode();
    bool dm = m_part->isEditable();
    return cm && !dm ? false
    	: (dm || m_part->d->caretNode().handle()->contentEditable())
	  && d->editorContext()->override;
}

void KHTMLView::ensureNodeHasFocus(NodeImpl *node)
{
  if (m_part->isCaretMode() || m_part->isEditable()) return;
  if (node->focused()) return;

  // Find first ancestor whose "user-input" is "enabled"
  NodeImpl *firstAncestor = 0;
  while (node) {
    if (node->renderer()
       && node->renderer()->style()->userInput() != UI_ENABLED)
      break;
    firstAncestor = node;
    node = node->parentNode();
  }/*wend*/

  if (!node) firstAncestor = 0;

  // Set focus node on the document
  m_part->xmlDocImpl()->setFocusNode(firstAncestor);
  emit m_part->nodeActivated(Node(firstAncestor));
}

void KHTMLView::recalcAndStoreCaretPos(InlineBox *hintBox)
{
    if (!m_part || m_part->d->caretNode().isNull()) return;
    d->caretViewContext();
    NodeImpl *caretNode = m_part->d->caretNode().handle();
  kdDebug(6200) << "recalcAndStoreCaretPos: caretNode=" << caretNode << (caretNode ? " "+caretNode->nodeName().string() : QString::null) << " r@" << caretNode->renderer() << (caretNode->renderer() && caretNode->renderer()->isText() ? " \"" + QConstString(static_cast<RenderText *>(caretNode->renderer())->str->s, QMIN(static_cast<RenderText *>(caretNode->renderer())->str->l, 15)).string() + "\"" : QString::null) << endl;
    caretNode->getCaret(m_part->d->caretOffset(),
                caretOverrides(),
    		d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width,
		d->m_caretViewContext->height);

    if (hintBox && d->m_caretViewContext->x == -1) {
        kdDebug(6200) << "using hint inline box coordinates" << endl;
	RenderObject *r = caretNode->renderer();
	const QFontMetrics &fm = r->style()->fontMetrics();
        int absx, absy;
	r->containingBlock()->absolutePosition(absx, absy,
						false);	// ### what about fixed?
	d->m_caretViewContext->x = absx + hintBox->xPos();
	d->m_caretViewContext->y = absy + hintBox->yPos()
				+ hintBox->baseline() - fm.ascent();
	d->m_caretViewContext->width = 1;
	// ### firstline not regarded. But I think it can be savely neglected
	// as hint boxes are only used for empty lines.
	d->m_caretViewContext->height = fm.height();
    }/*end if*/

//    kdDebug(6200) << "freqTimerId: "<<d->m_caretViewContext->freqTimerId<<endl;
    kdDebug(6200) << "caret: ofs="<<m_part->d->caretOffset()<<" "
    	<<" x="<<d->m_caretViewContext->x<<" y="<<d->m_caretViewContext->y
	<<" h="<<d->m_caretViewContext->height<<endl;
}

void KHTMLView::caretOn()
{
    if (d->m_caretViewContext) {
        killTimer(d->m_caretViewContext->freqTimerId);
        d->m_caretViewContext->freqTimerId = startTimer(500);
        d->m_caretViewContext->visible = true;
        d->m_caretViewContext->displayed = true;
	updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
//        kdDebug(6200) << "caret on" << endl;
    }/*end if*/
}

void KHTMLView::caretOff()
{
    if (d->m_caretViewContext) {
        killTimer(d->m_caretViewContext->freqTimerId);
	d->m_caretViewContext->freqTimerId = -1;
        d->m_caretViewContext->displayed = false;
        if (d->m_caretViewContext->visible) {
            d->m_caretViewContext->visible = false;
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	}/*end if*/
//        kdDebug(6200) << "caret off" << endl;
    }/*end if*/
}

void KHTMLView::showCaret()
{
    if (d->m_caretViewContext) {
        d->m_caretViewContext->displayed = true;
        if (d->m_caretViewContext->visible) {
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
   	}/*end if*/
//        kdDebug(6200) << "caret shown" << endl;
    }/*end if*/
}

bool KHTMLView::foldSelectionToCaret(NodeImpl *startNode, long startOffset,
    				NodeImpl *endNode, long endOffset)
{
  m_part->d->m_selectionStart = m_part->d->m_selectionEnd = m_part->d->caretNode();
  m_part->d->m_startOffset = m_part->d->m_endOffset = m_part->d->caretOffset();
  m_part->d->m_extendAtEnd = true;

  bool folded = startNode != endNode || startOffset != endOffset;

  // Only clear the selection if there has been one.
  if (folded) {
    m_part->xmlDocImpl()->clearSelection();
  }/*end if*/

  return folded;
}

void KHTMLView::hideCaret()
{
    if (d->m_caretViewContext) {
        if (d->m_caretViewContext->visible) {
//            kdDebug(6200) << "redraw caret hidden" << endl;
	    d->m_caretViewContext->visible = false;
	    // force repaint, otherwise the event won't be handled
	    // before the focus leaves the window
	    repaintContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	    d->m_caretViewContext->visible = true;
	}/*end if*/
        d->m_caretViewContext->displayed = false;
//        kdDebug(6200) << "caret hidden" << endl;
    }/*end if*/
}

bool KHTMLView::placeCaret(InlineBox *hintBox)
{
  CaretViewContext *cv = d->caretViewContext();
  caretOff();
  recalcAndStoreCaretPos(hintBox);

  cv->origX = cv->x;

  NodeImpl *caretNode = m_part->d->caretNode().handle();
  // ### why is it sometimes null?
  if (!caretNode) return false;
  ensureNodeHasFocus(caretNode);
  if (m_part->isCaretMode() || m_part->isEditable()
     || caretNode->renderer()->style()->userInput() == UI_ENABLED) {
    caretOn();
    return true;
  }/*end if*/
  return false;
}

void KHTMLView::caretKeyPressEvent(QKeyEvent *_ke)
{
  NodeImpl *oldStartSel = m_part->d->m_selectionStart.handle();
  long oldStartOfs = m_part->d->m_startOffset;
  NodeImpl *oldEndSel = m_part->d->m_selectionEnd.handle();
  long oldEndOfs = m_part->d->m_endOffset;

  NodeImpl *oldCaretNode = m_part->d->caretNode().handle();
  long oldOffset = m_part->d->caretOffset();

  bool ctrl = _ke->state() & ControlButton;

// FIXME: this is that widely indented because I will write ifs around it.
      switch(_ke->key()) {
        case Key_Space:
          break;

        case Key_Down:
	  moveCaretNextLine(1);
          break;

        case Key_Up:
	  moveCaretPrevLine(1);
          break;

        case Key_Left:
	  moveCaretBy(false, ctrl ? CaretByWord : CaretByCharacter, 1);
          break;

        case Key_Right:
	  moveCaretBy(true, ctrl ? CaretByWord : CaretByCharacter, 1);
          break;

        case Key_Next:
	  moveCaretNextPage();
          break;

        case Key_Prior:
	  moveCaretPrevPage();
          break;

        case Key_Home:
	  if (ctrl)
	    moveCaretToDocumentBoundary(false);
	  else
	    moveCaretToLineBegin();
          break;

        case Key_End:
	  if (ctrl)
	    moveCaretToDocumentBoundary(true);
	  else
	    moveCaretToLineEnd();
          break;

      }/*end switch*/

  if ((m_part->d->caretNode().handle() != oldCaretNode
  	|| m_part->d->caretOffset() != oldOffset)
	// node should never be null, but faulty conditions may cause it to be
	&& !m_part->d->caretNode().isNull()) {

    d->m_caretViewContext->caretMoved = true;

    if (_ke->state() & ShiftButton) {	// extend selection

      if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd
          && m_part->d->m_startOffset == m_part->d->m_endOffset) {
        if (foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs))
          m_part->emitSelectionChanged();
        m_part->d->m_extendAtEnd = true;
      } else {
        // check if the extending end has passed the immobile end
        if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd) {
	  if (m_part->d->m_startOffset > m_part->d->m_endOffset) {
            long tmpOffset = m_part->d->m_startOffset;
            m_part->d->m_startOffset = m_part->d->m_endOffset;
            m_part->d->m_endOffset = tmpOffset;
            m_part->d->m_startBeforeEnd = true;
            m_part->d->m_extendAtEnd = !m_part->d->m_extendAtEnd;
	  }/*end if*/
	} else if (isBeforeNode(m_part->d->m_selectionEnd, m_part->d->m_selectionStart)) {
          DOM::Node tmpNode = m_part->d->m_selectionStart;
          long tmpOffset = m_part->d->m_startOffset;
          m_part->d->m_selectionStart = m_part->d->m_selectionEnd;
          m_part->d->m_startOffset = m_part->d->m_endOffset;
          m_part->d->m_selectionEnd = tmpNode;
          m_part->d->m_endOffset = tmpOffset;
          m_part->d->m_startBeforeEnd = true;
          m_part->d->m_extendAtEnd = !m_part->d->m_extendAtEnd;
	}/*end if*/
        m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset, m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset);
        m_part->emitSelectionChanged();
      }/*end if*/

      kdDebug(6200) << "extendAtEnd: " << m_part->d->m_extendAtEnd << endl;

    } else {			// clear any selection
      if (foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs))
        m_part->emitSelectionChanged();
    }/*end if*/

    m_part->emitCaretPositionChanged(m_part->d->caretNode(), m_part->d->caretOffset());
  }/*end if*/

  _ke->accept();
}

bool KHTMLView::moveCaretTo(NodeImpl *node, long offset, bool clearSel)
{
  sanitizeCaretState(node, offset);
  // need to find out the node's inline box. If there is none, this function
  // will snap to the next node that has one. This is necessary to make the
  // caret visible in any case.
  RenderArena arena;
  RenderFlow *cb;
  InlineBox *box = 0;
  findFlowBox(node, offset, &arena, cb, &box);
  if (box && box->object() != node->renderer()) {
    node = box->object()->element(); // ### what if element is 0?
    offset = node->minOffset();
    kdDebug(6200) << "set new node " << node->nodeName().string() << "@" << node << endl;
  }

  NodeImpl *oldStartSel = m_part->d->m_selectionStart.handle();
  long oldStartOfs = m_part->d->m_startOffset;
  NodeImpl *oldEndSel = m_part->d->m_selectionEnd.handle();
  long oldEndOfs = m_part->d->m_endOffset;

  // test for position change
  bool posChanged = m_part->d->caretNode().handle() != node
  		|| m_part->d->caretOffset() != offset;
  bool folded = false;

  m_part->d->caretNode() = node;
  m_part->d->caretOffset() = offset;
  if (clearSel) {
    folded = foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
  }/*end if*/

  d->m_caretViewContext->caretMoved = true;

  bool visible_caret = placeCaret();

  // FIXME: if the old position was !visible_caret, and the new position is
  // also, then two caretPositionChanged signals with a null Node are
  // emitted in series.
  if (posChanged) {
    m_part->emitCaretPositionChanged(visible_caret ? node : 0, offset);
  }/*end if*/

  return folded;
}

/** seeks the inline box which contains or is the nearest to @p x
 * @param it line iterator pointing to line to be searched
 * @param cv caret view context
 * @param x returns the cv->origX approximation, relatively positioned to the
 *	containing block.
 * @param absx returns absolute x-coordinate of containing block
 * @param absy returns absolute y-coordinate of containing block
 * @return the most suitable inline box
 */
static InlineBox *nearestInlineBox(LineIterator &it, CaretViewContext *cv,
	int &x, int &absx, int &absy)
{
  InlineFlowBox *fbox = *it;

  // Find containing block
  RenderObject *cb = fbox->object();

  if (cb) cb->absolutePosition(absx, absy);
  else absx = absy = 0;

  // Otherwise find out in which inline box the caret is to be placed.

  // this horizontal position is to be approximated
  x = cv->origX - absx;
  InlineBox *caretBox = 0; // Inline box containing the caret
//  NodeImpl *lastnode = 0;  // node of previously checked render object.
  int xPos;		   // x-coordinate of current inline box
  int oldXPos = -1;	   // x-coordinate of last inline box
  EditableInlineBoxIterator fbit = it;
  kdDebug(6200) << "*fbit = " << *fbit << endl;
  // Either iterate through all children or take the flow box itself as a
  // child if it has no children
  for (InlineBox *b; *fbit != 0; ++fbit) {
    b = *fbit;

//    RenderObject *r = b->object();
	if (b->isInlineFlowBox()) kdDebug(6200) << "b is inline flow box" << endl;
//	kdDebug(6200) << "approximate r" << r << ": " << (r ? r->renderName() : QString::null) << (r && r->isText() ? " contains \"" + QString(((RenderText *)r)->str->s, ((RenderText *)r)->str->l) + "\"" : QString::null) << endl;
//    NodeImpl *node = r->element();
    xPos = b->xPos();

    // the caret is before this box
    if (x < xPos) {
      // snap to nearest box
      if (oldXPos < 0 || x - (oldXPos + caretBox->width()) > xPos - x) {
	caretBox = b;	// current box is nearer
//        lastnode = node;
      }/*end if*/
      break;		// Otherwise, preceding box is implicitly used
    }

    caretBox = b;
//    lastnode = node;

    // the caret is within this box
    if (x >= xPos && x < xPos + caretBox->width())
      break;
    oldXPos = xPos;

    // the caret can only be after the last box which is automatically
    // contained in caretBox when we fall out of the loop.

    if (b == fbox) break;
  }/*next fbit*/

  return caretBox;
}

void KHTMLView::moveCaretByLine(bool next, int count)
{
  // FIXME: what if the node is removed whilst we access it?
  // Current solution: bail out
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  CaretViewContext *cv = d->caretViewContext();

  LinearDocument ld(m_part, caretNode, offset);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  // move count lines vertically
  while (count > 0 && it != ld.end() && it != ld.preBegin()) {
    count--;
    if (next) ++it; else --it;
  }/*wend*/

  // Nothing? Then leave everything as is.
  if (it == ld.end() || it == ld.preBegin()) return;

  int x, absx, absy;
  InlineBox *caretBox = nearestInlineBox(it, d->m_caretViewContext, x, absx, absy);

  placeCaretOnLine(caretBox, x, absx, absy);
}

void KHTMLView::placeCaretOnLine(InlineBox *caretBox, int x, int absx, int absy)
{
  // paranoia sanity check
  if (!caretBox) return;

  RenderObject *caretRender = caretBox->object();
  NodeImpl *caretNode = caretRender->element();

  kdDebug(6200) << "got valid caretBox " << caretBox << endl;
  kdDebug(6200) << "xPos: " << caretBox->xPos() << " yPos: " << caretBox->yPos()
  		<< " width: " << caretBox->width() << " height: " << caretBox->height() << endl;
  if (caretBox->isInlineTextBox()) { kdDebug(6200) << "contains \"" << QString(((RenderText *)((InlineTextBox *)caretBox)->object())->str->s + ((InlineTextBox *)caretBox)->m_start, ((InlineTextBox *)caretBox)->m_len) << "\"" << endl;}
  // inquire height of caret
  int caretHeight = caretBox->height();
  bool isText = caretBox->isInlineTextBox();
  int yOfs = 0;		// y-offset for text nodes
  if (isText) {
    // text boxes need extrawurst
    RenderText *t = static_cast<RenderText *>(caretRender);
    const QFontMetrics &fm = t->metrics(caretBox->m_firstLine);
    caretHeight = fm.height();
    yOfs = caretBox->baseline() - fm.ascent();
  }/*end if*/

  caretOff();

  // set new caret node
  m_part->d->caretNode() = caretNode;
  long &offset = m_part->d->caretOffset();

  // set all variables not needing special treatment
  d->m_caretViewContext->y = caretBox->yPos() + yOfs;
  d->m_caretViewContext->height = caretHeight;
  d->m_caretViewContext->width = 1; // FIXME: regard override

  int xPos = caretBox->xPos();
  int caretBoxWidth = caretBox->width();

  // before or at beginning of inline box -> place at beginning
  if (x <= xPos) {
    d->m_caretViewContext->x = xPos;
    offset = caretBox->minOffset();
  // somewhere within this block
  } else if (x > xPos && x <= xPos + caretBoxWidth) {
    if (isText) { // find out where exactly
      offset = static_cast<InlineTextBox *>(caretBox)->offsetForPoint(x,
      		d->m_caretViewContext->x);
      kdDebug(6200) << "deviation from origX " << d->m_caretViewContext->x - x << endl;
    } else {	// snap to nearest end
      if (xPos + caretBoxWidth - x < x - xPos) {
        d->m_caretViewContext->x = xPos + caretBoxWidth;
        offset = caretNode ? caretNode->maxOffset() : 1;
      } else {
        d->m_caretViewContext->x = xPos;
        offset = caretNode ? caretNode->minOffset() : 0;
      }/*end if*/
    }/*end if*/
  } else {		// after the inline box -> place at end
    d->m_caretViewContext->x = xPos + caretBoxWidth;
    offset = caretBox->maxOffset();
  }/*end if*/
      kdDebug(6200) << "new offset: " << offset << endl;

  d->m_caretViewContext->x += absx;
  d->m_caretViewContext->y += absy;

  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->scrollBarMoved = false;

  ensureNodeHasFocus(caretNode);
  caretOn();
}

void KHTMLView::moveCaretToLineBoundary(bool end)
{
  // FIXME: what if the node is removed whilst we access it?
  // Current solution: bail out
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  LinearDocument ld(m_part, caretNode, offset);

  EditableLineIterator it = ld.current();
  if (it == ld.end()) return;	// should not happen, but who knows

  EditableInlineBoxIterator fbit(it, end);
  InlineBox *b = *fbit;
  Q_ASSERT(b);

  RenderObject *cb = (*it)->object();
  int absx, absy;

  if (cb) cb->absolutePosition(absx,absy);
  else absx = absy = 0;

  int x = b->xPos() + (end ? b->width() : 0);
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

void KHTMLView::moveCaretToDocumentBoundary(bool end)
{
  // FIXME: what if the node is removed whilst we access it?
  // Current solution: bail out
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  LinearDocument ld(m_part, caretNode, offset);

  EditableLineIterator it(end ? ld.preEnd() : ld.begin(), end);
  if (it == ld.end() || it == ld.preBegin()) return;	// should not happen, but who knows

  EditableInlineBoxIterator fbit = it;
  InlineBox *b = *fbit;
  Q_ASSERT(b);

  RenderObject *cb = (*it)->object();
  int absx, absy;

  if (cb) cb->absolutePosition(absx, absy);
  else absx = absy = 0;

  int x = b->xPos()/* + (end ? b->width() : 0) reactivate for rtl*/;
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

/** moves the given iterator to the beginning of the next word.
 *
 * If the end is reached, the iterator will be positioned there.
 * @param it character iterator to be moved
 */
static void moveItToNextWord(EditableCharacterIterator &it)
{
  kdDebug(6200) << "%%%%%%%%%%%%%%%%%%%%% moveItToNextWord" << endl;
  EditableCharacterIterator copy;
  while (it.node() && !(*it).isSpace() && !(*it).isPunct()) {
    kdDebug(6200) << "reading1 '" << (*it).latin1() << "'" << endl;
    copy = it;
    ++it;
  }

  if (!it.node()) {
    it = copy;
    return;
  }/*end if*/

  while (it.node() && ((*it).isSpace() || (*it).isPunct())) {
    kdDebug(6200) << "reading2 '" << (*it).latin1() << "'" << endl;
    copy = it;
    ++it;
  }

  if (!it.node()) it = copy;
}

/** moves the given iterator to the beginning of the previous word.
 *
 * If the beginning is reached, the iterator will be positioned there.
 * @param it character iterator to be moved
 */
static void moveItToPrevWord(EditableCharacterIterator &it)
{
  if (!it.node()) return;

  kdDebug(6200) << "%%%%%%%%%%%%%%%%%%%%% moveItToPrevWord" << endl;
  EditableCharacterIterator copy;

  // Jump over all space and punctuation characters first
  do {
    copy = it;
    --it;
    if (it.node()) kdDebug(6200) << "reading1 '" << (*it).latin1() << "'" << endl;
  } while (it.node() && ((*it).isSpace() || (*it).isPunct()));

  if (!it.node()) {
    it = copy;
    return;
  }/*end if*/

  do {
    copy = it;
    --it;
    if (it.node()) kdDebug(6200) << "reading2 '" << (*it).latin1() << "'" << endl;
  } while (it.node() && !(*it).isSpace() && !(*it).isPunct());

  it = copy;
}

void KHTMLView::moveCaretBy(bool next, CaretMovement cmv, int count)
{
  if (!m_part) return;
  // FIXME: what if the node is removed whilst we access it?
  // Current solution: bail out
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long &offset = m_part->d->caretOffset();

  LinearDocument ld(m_part, caretNode, offset);

  EditableCharacterIterator it(&ld);
  while (it.node() && count > 0) {
    count--;
    if (cmv == CaretByCharacter) {
      if (next) ++it;
      else --it;
    } else if (cmv == CaretByWord) {
      if (next) moveItToNextWord(it);
      else moveItToPrevWord(it);
    }/*end if*/
  }/*wend*/
  if (it.node()) {
    caretNodeRef = it.node();
    offset = it.offset();
  kdDebug(6200) << "set by valid node. offset: " << offset << endl;
  } else {
    offset = next ? caretNode->maxOffset() : caretNode->minOffset();
  kdDebug(6200) << "set by INvalid node. offset: " << offset << endl;
  }/*end if*/
  placeCaretOnChar(it.box());
}

void KHTMLView::placeCaretOnChar(InlineBox *hintBox)
{
  caretOff();
  recalcAndStoreCaretPos(hintBox);
  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->m_caretViewContext->origX = d->m_caretViewContext->x;
  d->scrollBarMoved = false;
//  if (caretNode->isTextNode())  kdDebug(6200) << "text[0] = " << (int)*((TextImpl *)caretNode)->data().unicode() << " text :\"" << ((TextImpl *)caretNode)->data().string() << "\"" << endl;
  ensureNodeHasFocus(m_part->d->caretNode().handle());
  caretOn();
}

/** moves the iterator by one page.
 * @param ld linear document
 * @param it line iterator, will be updated accordingly
 * @param mindist minimum distance in pixel the iterator should be moved
 *	(if possible)
 * @param next @p true, move downward, @p false move upward
 */
static void moveIteratorByPage(LinearDocument &ld,
		ErgonomicEditableLineIterator &it, int mindist, bool next)
{
  if (it == ld.end() || it == ld.preBegin()) return;

  ErgonomicEditableLineIterator copy = it;
  kdDebug(6200) << " mindist: " << mindist << endl;

  InlineFlowBox *flowBox = *copy;
  int absx = 0, absy = 0;

  RenderFlow *lastcb = static_cast<RenderFlow *>(flowBox->object());
  Q_ASSERT(lastcb->isFlow());
  lastcb->absolutePosition(absx, absy, false);	// ### what about fixed?

  // ### actually flowBox->yPos() should suffice, but this is not ported
  // over yet from WebCore
  int lastfby = flowBox->firstChild()->yPos();
  int lastheight = 0;
  do {
    if (next) ++copy; else --copy;
    if (copy == ld.end() || copy == ld.preBegin()) break;

    // ### change to RootInlineBox after full WebCore merge
    flowBox = static_cast<InlineFlowBox *>(*copy);
    Q_ASSERT(flowBox->isInlineFlowBox());

    RenderFlow *cb = static_cast<RenderFlow *>(flowBox->object());
    Q_ASSERT(cb->isFlow());

    int diff = 0;
    // ### actually flowBox->yPos() should suffice, but this is not ported
    // over yet from WebCore
    int fby = flowBox->firstChild()->yPos();
    if (cb != lastcb) {
      if (next) {
        diff = absy + lastfby + lastheight;
        cb->absolutePosition(absx, absy, false);	// ### what about fixed?
        diff = absy - diff + fby;
        lastfby = 0;
      } else {
        diff = absy;
        cb->absolutePosition(absx, absy, false);	// ### what about fixed?
        diff -= absy + fby + lastheight;
	lastfby = fby - lastheight;
      }/*end if*/
      kdDebug(6200) << "absdiff " << diff << endl;
    } else {
      diff = QABS(fby - lastfby);
    }/*end if*/
    kdDebug(6200) << "flowBox->firstChild->yPos: " << fby << " diff " << diff << endl;

    mindist -= diff;

    lastheight = QABS(fby - lastfby);
    lastfby = fby;
    lastcb = cb;
    it = copy;
    kdDebug(6200) << " mindist: " << mindist << endl;
    // trick: actually the distance is always one line short, but we cannot
    // calculate the height of the first line (### WebCore will make it better)
    // Therefore, we simply approximate that excess line by using the last
    // caluculated line height.
  } while (mindist - lastheight > 0);
}

void KHTMLView::moveCaretByPage(bool next)
{
  // FIXME: what if the node is removed whilst we access it?
  // Current solution: bail out
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
  // Minimum distance the caret must be moved
  int mindist = clipper()->height() - offs;

  CaretViewContext *cv = d->caretViewContext();
//  int y = cv->y;		// we always measure the top border

  LinearDocument ld(m_part, caretNode, offset);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  moveIteratorByPage(ld, it, mindist, next);

  int x, absx, absy;
  InlineBox *caretBox = nearestInlineBox(it, d->m_caretViewContext, x, absx, absy);

  placeCaretOnLine(caretBox, x, absx, absy);
}

void KHTMLView::moveCaretPrevWord()
{
  moveCaretBy(false, CaretByWord, 1);
}

void KHTMLView::moveCaretNextWord()
{
  moveCaretBy(true, CaretByWord, 1);
}

void KHTMLView::moveCaretPrevLine(int n)
{
  moveCaretByLine(false, n);
}

void KHTMLView::moveCaretNextLine(int n)
{
  moveCaretByLine(true, n);
}

void KHTMLView::moveCaretPrevPage()
{
  moveCaretByPage(false);
}

void KHTMLView::moveCaretNextPage()
{
  moveCaretByPage(true);
}

void KHTMLView::moveCaretToLineBegin()
{
  moveCaretToLineBoundary(false);
}

void KHTMLView::moveCaretToLineEnd()
{
  moveCaretToLineBoundary(true);
}

#endif // KHTML_NO_CARET
