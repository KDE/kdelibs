/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000-2004 Dirk Mueller <mueller@kde.org>
 *                     2003 Leo Savernik <l.savernik@aon.at>
 *                     2003-2004 Apple Computer, Inc.
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "khtmlview.moc"

#include "khtmlview.h"

#include "khtml_part.h"
#include "khtml_events.h"

#include "html/html_documentimpl.h"
#include "html/html_inlineimpl.h"
#include "html/html_formimpl.h"
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
#include "css/csshelper.h"
#include "misc/htmlhashes.h"
#include "misc/helper.h"
#include "khtml_settings.h"
#include "khtml_printsettings.h"

#include "khtmlpart_p.h"

#ifndef KHTML_NO_CARET
#include "khtml_caret_p.h"
#include "xml/dom2_rangeimpl.h"
#endif

#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kstringhandler.h>
#include <kurldrag.h>

#include <qbitmap.h>
#include <qlabel.h>
#include <qobjectlist.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qptrdict.h>
#include <qtooltip.h>
#include <qstring.h>
#include <qstylesheet.h>
#include <qtimer.h>
#include <qvaluevector.h>

//#define DEBUG_NO_PAINT_BUFFER

//#define DEBUG_FLICKER

//#define DEBUG_PIXEL

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#define PAINT_BUFFER_HEIGHT 128

#if 0
namespace khtml {
    void dumpLineBoxes(RenderFlow *flow);
}
#endif

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

class KHTMLViewPrivate {
    friend class KHTMLToolTip;
public:

    enum PseudoFocusNodes {
	PFNone,
	PFTop,
	PFBottom
    };

    enum CompletedState {
        CSNone = 0,
        CSFull,
        CSActionPending
    };

    KHTMLViewPrivate()
        : underMouse( 0 ), underMouseNonShared( 0 )
    {
#ifndef KHTML_NO_CARET
	m_caretViewContext = 0;
	m_editorContext = 0;
#endif // KHTML_NO_CARET
        postponed_autorepeat = NULL;
        reset();
        vmode = QScrollView::Auto;
 	hmode = QScrollView::Auto;
        tp=0;
        paintBuffer=0;
        vertPaintBuffer=0;
        formCompletions=0;
        prevScrollbarVisible = true;
	tooltip = 0;
        possibleTripleClick = false;
        emitCompletedAfterRepaint = CSNone;
	cursor_icon_widget = NULL;
        m_mouseScrollTimer = 0;
        m_mouseScrollIndicator = 0;
    }
    ~KHTMLViewPrivate()
    {
        delete formCompletions;
        delete tp; tp = 0;
        delete paintBuffer; paintBuffer =0;
        delete vertPaintBuffer;
        delete postponed_autorepeat;
        if (underMouse)
	    underMouse->deref();
        if (underMouseNonShared)
	    underMouseNonShared->deref();
	delete tooltip;
#ifndef KHTML_NO_CARET
	delete m_caretViewContext;
	delete m_editorContext;
#endif // KHTML_NO_CARET
        delete cursor_icon_widget;
        delete m_mouseScrollTimer;
        delete m_mouseScrollIndicator;
    }
    void reset()
    {
        if (underMouse)
	    underMouse->deref();
	underMouse = 0;
        if (underMouseNonShared)
	    underMouseNonShared->deref();
	underMouseNonShared = 0;
        linkPressed = false;
        useSlowRepaints = false;
	tabMovePending = false;
	lastTabbingDirection = true;
	pseudoFocusNode = PFNone;
#ifndef KHTML_NO_SCROLLBARS
        //We don't turn off the toolbars here
	//since if the user turns them
	//off, then chances are they want them turned
	//off always - even after a reset.
#else
        vmode = QScrollView::AlwaysOff;
        hmode = QScrollView::AlwaysOff;
#endif
#ifdef DEBUG_PIXEL
        timer.start();
        pixelbooth = 0;
        repaintbooth = 0;
#endif
        scrollBarMoved = false;
        contentsMoving = false;
        ignoreWheelEvents = false;
	borderX = 30;
	borderY = 30;
        paged = false;
	clickX = -1;
	clickY = -1;
        prevMouseX = -1;
        prevMouseY = -1;
	clickCount = 0;
	isDoubleClick = false;
	scrollingSelf = false;
        delete postponed_autorepeat;
        postponed_autorepeat = NULL;
	layoutTimerId = 0;
        repaintTimerId = 0;
        scrollTimerId = 0;
        scrollSuspended = false;
        scrollSuspendPreActivate = false;
        complete = false;
        firstRelayout = true;
        needsFullRepaint = true;
        dirtyLayout = false;
        layoutSchedulingEnabled = true;
        painting = false;
        updateRegion = QRegion();
        m_dialogsAllowed = true;
#ifndef KHTML_NO_CARET
        if (m_caretViewContext) {
          m_caretViewContext->caretMoved = false;
	  m_caretViewContext->keyReleasePending = false;
        }/*end if*/
#endif // KHTML_NO_CARET
#ifndef KHTML_NO_TYPE_AHEAD_FIND
        typeAheadActivated = false;
#endif // KHTML_NO_TYPE_AHEAD_FIND
	accessKeysActivated = false;
	accessKeysPreActivate = false;

        // We ref/deref to ensure defaultHTMLSettings is available
        KHTMLFactory::ref();
        accessKeysEnabled = KHTMLFactory::defaultHTMLSettings()->accessKeysEnabled();
        KHTMLFactory::deref();

        emitCompletedAfterRepaint = CSNone;
    }
    void newScrollTimer(QWidget *view, int tid)
    {
        //kdDebug(6000) << "newScrollTimer timer " << tid << endl;
        view->killTimer(scrollTimerId);
        scrollTimerId = tid;
        scrollSuspended = false;
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
             (scrollDirection != oppositedir || scrollSuspended))) {
            scrollTiming = 6;
            scrollBy = timings[scrollTiming].pixels;
            scrollDirection = direction;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == direction &&
                   timings[scrollTiming+1].msec && !scrollSuspended) {
            scrollBy = timings[++scrollTiming].pixels;
            newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
        } else if (scrollDirection == oppositedir) {
            if (scrollTiming) {
                scrollBy = timings[--scrollTiming].pixels;
                newScrollTimer(view, view->startTimer(timings[scrollTiming].msec));
            }
        }
        scrollSuspended = false;
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

#ifdef DEBUG_PIXEL
    QTime timer;
    unsigned int pixelbooth;
    unsigned int repaintbooth;
#endif

    QPainter *tp;
    QPixmap  *paintBuffer;
    QPixmap  *vertPaintBuffer;
    NodeImpl *underMouse;
    NodeImpl *underMouseNonShared;

    bool tabMovePending:1;
    bool lastTabbingDirection:1;
    PseudoFocusNodes pseudoFocusNode:2;
    bool scrollBarMoved:1;
    bool contentsMoving:1;

    QScrollView::ScrollBarMode vmode;
    QScrollView::ScrollBarMode hmode;
    bool prevScrollbarVisible:1;
    bool linkPressed:1;
    bool useSlowRepaints:1;
    bool ignoreWheelEvents:1;

    int borderX, borderY;
    KSimpleConfig *formCompletions;

    bool paged;

    int clickX, clickY, clickCount;
    bool isDoubleClick;

    int prevMouseX, prevMouseY;
    bool scrollingSelf;
    int layoutTimerId;
    QKeyEvent* postponed_autorepeat;

    int repaintTimerId;
    int scrollTimerId;
    int scrollTiming;
    int scrollBy;
    ScrollDirection scrollDirection		:2;
    bool scrollSuspended			:1;
    bool scrollSuspendPreActivate		:1;
    bool complete				:1;
    bool firstRelayout				:1;
    bool layoutSchedulingEnabled		:1;
    bool needsFullRepaint			:1;
    bool painting				:1;
    bool possibleTripleClick			:1;
    bool dirtyLayout				:1;
    bool m_dialogsAllowed			:1;
    QRegion updateRegion;
    KHTMLToolTip *tooltip;
    QPtrDict<QWidget> visibleWidgets;
#ifndef KHTML_NO_CARET
    CaretViewContext *m_caretViewContext;
    EditorContext *m_editorContext;
#endif // KHTML_NO_CARET
#ifndef KHTML_NO_TYPE_AHEAD_FIND
    QString findString;
    QTimer timer;
    bool findLinksOnly;
    bool typeAheadActivated;
#endif // KHTML_NO_TYPE_AHEAD_FIND
    bool accessKeysEnabled;
    bool accessKeysActivated;
    bool accessKeysPreActivate;
    CompletedState emitCompletedAfterRepaint;

    QWidget* cursor_icon_widget;

    // scrolling activated by MMB
    int m_mouseScroll_byX : 4;
    int m_mouseScroll_byY : 4;
    QTimer *m_mouseScrollTimer;
    QWidget *m_mouseScrollIndicator;
};

#ifndef QT_NO_TOOLTIP

/** calculates the client-side image map rectangle for the given image element
 * @param img image element
 * @param scrollOfs scroll offset of viewport in content coordinates
 * @param p position to be probed in viewport coordinates
 * @param r returns the bounding rectangle in content coordinates
 * @param s returns the title string
 * @return true if an appropriate area was found -- only in this case r and
 *	s are valid, false otherwise
 */
static bool findImageMapRect(HTMLImageElementImpl *img, const QPoint &scrollOfs,
			const QPoint &p, QRect &r, QString &s)
{
    HTMLMapElementImpl* map;
    if (img && img->getDocument()->isHTMLDocument() &&
        (map = static_cast<HTMLDocumentImpl*>(img->getDocument())->getMap(img->imageMap()))) {
        RenderObject::NodeInfo info(true, false);
        RenderObject *rend = img->renderer();
        int ax, ay;
        if (!rend || !rend->absolutePosition(ax, ay))
            return false;
        // we're a client side image map
        bool inside = map->mapMouseEvent(p.x() - ax + scrollOfs.x(),
                p.y() - ay + scrollOfs.y(), rend->contentWidth(),
                rend->contentHeight(), info);
        if (inside && info.URLElement()) {
            HTMLAreaElementImpl *area = static_cast<HTMLAreaElementImpl *>(info.URLElement());
            Q_ASSERT(area->id() == ID_AREA);
            s = area->getAttribute(ATTR_TITLE).string();
            QRegion reg = area->cachedRegion();
            if (!s.isEmpty() && !reg.isEmpty()) {
                r = reg.boundingRect();
                r.moveBy(ax, ay);
                return true;
            }
        }
    }
    return false;
}

void KHTMLToolTip::maybeTip(const QPoint& p)
{
    DOM::NodeImpl *node = m_viewprivate->underMouseNonShared;
    QRect region;
    while ( node ) {
        if ( node->isElementNode() ) {
            DOM::ElementImpl *e = static_cast<DOM::ElementImpl*>( node );
            QRect r;
            QString s;
            bool found = false;
            // for images, check if it is part of a client-side image map,
            // and query the <area>s' title attributes, too
            if (e->id() == ID_IMG && !e->getAttribute( ATTR_USEMAP ).isEmpty()) {
                found = findImageMapRect(static_cast<HTMLImageElementImpl *>(e),
                    		m_view->viewportToContents(QPoint(0, 0)), p, r, s);
            }
            if (!found) {
                s = e->getAttribute( ATTR_TITLE ).string();
                r = node->getRect();
            }
            region |= QRect( m_view->contentsToViewport( r.topLeft() ), r.size() );
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

#ifndef KHTML_NO_TYPE_AHEAD_FIND
    connect(&d->timer, SIGNAL(timeout()), this, SLOT(findTimeout()));
#endif // KHTML_NO_TYPE_AHEAD_FIND

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

#ifndef KHTML_NO_TYPE_AHEAD_FIND
    if( d->typeAheadActivated )
        findTimeout();
#endif
    if (d->accessKeysEnabled && d->accessKeysActivated)
        accessKeysTimeout();
    viewport()->unsetCursor();
    if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();
    d->reset();
    killTimers();
    emit cleared();

    QScrollView::setHScrollBarMode(d->hmode);
    QScrollView::setVScrollBarMode(d->vmode);
    verticalScrollBar()->setEnabled( false );
    horizontalScrollBar()->setEnabled( false );
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
    int dw = e->oldSize().width() - e->size().width();
    int dh = e->oldSize().height() - e->size().height();

    // if we are shrinking the view, don't allow the content to overflow
    // before the layout occurs - we don't know if we need scrollbars yet
    dw = dw>0 ? kMax(0, contentsWidth()-dw) : contentsWidth();
    dh = dh>0 ? kMax(0, contentsHeight()-dh) : contentsHeight();

    resizeContents(dw, dh);

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
#ifdef DEBUG_PIXEL

    if ( d->timer.elapsed() > 5000 ) {
        qDebug( "drawed %d pixels in %d repaints the last %d milliseconds",
                d->pixelbooth, d->repaintbooth,  d->timer.elapsed() );
        d->timer.restart();
        d->pixelbooth = 0;
        d->repaintbooth = 0;
    }
    d->pixelbooth += ew*eh;
    d->repaintbooth++;
#endif

    //kdDebug( 6000 ) << "drawContents this="<< this <<" x=" << ex << ",y=" << ey << ",w=" << ew << ",h=" << eh << endl;
    if(!m_part || !m_part->xmlDocImpl() || !m_part->xmlDocImpl()->renderer()) {
        p->fillRect(ex, ey, ew, eh, palette().active().brush(QColorGroup::Base));
        return;
    } else if ( d->complete && static_cast<RenderCanvas*>(m_part->xmlDocImpl()->renderer())->needsLayout() ) {
        // an external update request happens while we have a layout scheduled
        unscheduleRelayout();
        layout();
    }

    if (d->painting) {
        kdDebug( 6000 ) << "WARNING: drawContents reentered! " << endl;
        return;
    }
    d->painting = true;

    QPoint pt = contentsToViewport(QPoint(ex, ey));
    QRegion cr = QRect(pt.x(), pt.y(), ew, eh);

    //kdDebug(6000) << "clip rect: " << QRect(pt.x(), pt.y(), ew, eh) << endl;
    for (QPtrDictIterator<QWidget> it(d->visibleWidgets); it.current(); ++it) {
	QWidget *w = it.current();
	RenderWidget* rw = static_cast<RenderWidget*>( it.currentKey() );
        if (strcmp(w->name(), "__khtml")) {
            int x, y;
            rw->absolutePosition(x, y);
            contentsToViewport(x, y, x, y);
            cr -= QRect(x, y, rw->width(), rw->height());
        }
    }

#if 0
    // this is commonly the case with framesets. we still do
    // want to paint them, otherwise the widgets don't get placed.
    if (cr.isEmpty()) {
        d->painting = false;
	return;
    }
#endif

#ifndef DEBUG_NO_PAINT_BUFFER
    p->setClipRegion(cr);

    if (eh > PAINT_BUFFER_HEIGHT && ew <= 10) {
        if ( d->vertPaintBuffer->height() < visibleHeight() )
            d->vertPaintBuffer->resize(10, visibleHeight());
        d->tp->begin(d->vertPaintBuffer);
        d->tp->translate(-ex, -ey);
        d->tp->fillRect(ex, ey, ew, eh, palette().active().brush(QColorGroup::Base));
        m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, QRect(ex, ey, ew, eh));
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
            m_part->xmlDocImpl()->renderer()->layer()->paint(d->tp, QRect(ex, ey+py, ew, ph));
            d->tp->end();

	    p->drawPixmap(ex, ey+py, *d->paintBuffer, 0, 0, ew, ph);
            py += PAINT_BUFFER_HEIGHT;
        }
    }
#else // !DEBUG_NO_PAINT_BUFFER
static int cnt=0;
	ex = contentsX(); ey = contentsY();
	ew = visibleWidth(); eh = visibleHeight();
	QRect pr(ex,ey,ew,eh);
	kdDebug() << "[" << ++cnt << "]" << " clip region: " << pr << endl;
//	p->setClipRegion(QRect(0,0,ew,eh));
//        p->translate(-ex, -ey);
        p->fillRect(ex, ey, ew, eh, palette().active().brush(QColorGroup::Base));
        m_part->xmlDocImpl()->renderer()->layer()->paint(p, pr);
#endif // DEBUG_NO_PAINT_BUFFER

#ifndef KHTML_NO_CARET
    if (d->m_caretViewContext && d->m_caretViewContext->visible) {
        QRect pos(d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width, d->m_caretViewContext->height);
        if (pos.intersects(QRect(ex, ey, ew, eh))) {
            p->setRasterOp(XorROP);
	    p->setPen(white);
	    if (pos.width() == 1)
              p->drawLine(pos.topLeft(), pos.bottomRight());
	    else {
	      p->fillRect(pos, white);
	    }/*end if*/
	}/*end if*/
    }/*end if*/
#endif // KHTML_NO_CARET

//    p->setPen(QPen(magenta,0,DashDotDotLine));
//    p->drawRect(dbg_paint_rect);

    khtml::DrawContentsEvent event( p, ex, ey, ew, eh );
    QApplication::sendEvent( m_part, &event );

    d->painting = false;
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
    if( m_part && m_part->xmlDocImpl() ) {
        DOM::DocumentImpl *document = m_part->xmlDocImpl();

        khtml::RenderCanvas* root = static_cast<khtml::RenderCanvas *>(document->renderer());
        if ( !root ) return;

        d->layoutSchedulingEnabled=false;

        if (document->isHTMLDocument()) {
             NodeImpl *body = static_cast<HTMLDocumentImpl*>(document)->body();
             if(body && body->renderer() && body->id() == ID_FRAMESET) {
                 QScrollView::setVScrollBarMode(AlwaysOff);
                 QScrollView::setHScrollBarMode(AlwaysOff);
                 body->renderer()->setNeedsLayout(true);
//                  if (d->tooltip) {
//                      delete d->tooltip;
//                      d->tooltip = 0;
//                  }
             }
             else if (!d->tooltip)
                 d->tooltip = new KHTMLToolTip( this, d );
        }
        d->needsFullRepaint = d->firstRelayout;
        if (_height !=  visibleHeight() || _width != visibleWidth()) {;
            d->needsFullRepaint = true;
            _height = visibleHeight();
            _width = visibleWidth();
        }
        //QTime qt;
        //qt.start();
        root->layout();

        emit finishedLayout();
        if (d->firstRelayout) {
            // make sure firstRelayout is set to false now in case this layout
            // wasn't scheduled
            d->firstRelayout = false;
            verticalScrollBar()->setEnabled( true );
            horizontalScrollBar()->setEnabled( true );
        }
#if 0
    ElementImpl *listitem = m_part->xmlDocImpl()->getElementById("__test_element__");
    if (listitem) kdDebug(6000) << "after layout, before repaint" << endl;
    if (listitem) dumpLineBoxes(static_cast<RenderFlow *>(listitem->renderer()));
#endif
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
        if (d->accessKeysEnabled && d->accessKeysActivated) {
            emit hideAccessKeys();
            displayAccessKeys();
        }
        //kdDebug( 6000 ) << "TIME: layout() dt=" << qt.elapsed() << endl;
    }
    else
       _width = visibleWidth();

    killTimer(d->layoutTimerId);
    d->layoutTimerId = 0;
    d->layoutSchedulingEnabled=true;
}

void KHTMLView::closeChildDialogs()
{
    QObjectList *dlgs = queryList("QDialog");
    for (QObject *dlg = dlgs->first(); dlg; dlg = dlgs->next())
    {
        KDialogBase* dlgbase = dynamic_cast<KDialogBase *>( dlg );
        if ( dlgbase ) {
            if ( dlgbase->testWFlags( WShowModal ) ) {
                kdDebug(6000) << "closeChildDialogs: closing dialog " << dlgbase << endl;
                // close() ends up calling QButton::animateClick, which isn't immediate
                // we need something the exits the event loop immediately (#49068)
                dlgbase->cancel();
            }
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
    if (!m_part->xmlDocImpl()) return;
    if (d->possibleTripleClick && ( _mouse->button() & MouseButtonMask ) == LeftButton)
    {
        viewportMouseDoubleClickEvent( _mouse ); // it handles triple clicks too
        return;
    }

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);
    //kdDebug( 6000 ) << "mousePressEvent: viewport=("<<_mouse->x()<<"/"<<_mouse->y()<<"), contents=(" << xm << "/" << ym << ")\n";

    d->isDoubleClick = false;

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MousePress );
    m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

    //kdDebug(6000) << "innerNode="<<mev.innerNode.nodeName().string()<<endl;

    if ( (_mouse->button() == MidButton) &&
          !m_part->d->m_bOpenMiddleClick && !d->m_mouseScrollTimer &&
          mev.url.isNull() && (mev.innerNode.elementId() != ID_INPUT) ) {
        QPoint point = mapFromGlobal( _mouse->globalPos() );

        d->m_mouseScroll_byX = 0;
        d->m_mouseScroll_byY = 0;

        d->m_mouseScrollTimer = new QTimer( this );
        connect( d->m_mouseScrollTimer, SIGNAL(timeout()), this, SLOT(slotMouseScrollTimer()) );

        if ( !d->m_mouseScrollIndicator ) {
            QPixmap pixmap, icon;
            pixmap.resize( 48, 48 );
            pixmap.fill( QColor( qRgba( 127, 127, 127, 127 ) ) );

            QPainter p( &pixmap );
            icon = KGlobal::iconLoader()->loadIcon( "1uparrow", KIcon::Small );
            p.drawPixmap( 16, 0, icon );
            icon = KGlobal::iconLoader()->loadIcon( "1leftarrow", KIcon::Small );
            p.drawPixmap( 0, 16, icon );
            icon = KGlobal::iconLoader()->loadIcon( "1downarrow", KIcon::Small );
            p.drawPixmap( 16, 32,icon  );
            icon = KGlobal::iconLoader()->loadIcon( "1rightarrow", KIcon::Small );
            p.drawPixmap( 32, 16, icon );
            p.drawEllipse( 23, 23, 2, 2 );

            d->m_mouseScrollIndicator = new QWidget( this, 0 );
            d->m_mouseScrollIndicator->setFixedSize( 48, 48 );
            d->m_mouseScrollIndicator->setPaletteBackgroundPixmap( pixmap );
        }
        d->m_mouseScrollIndicator->move( point.x()-24, point.y()-24 );

        bool hasHorBar = visibleWidth() < contentsWidth();
        bool hasVerBar = visibleHeight() < contentsHeight();

        KConfig *config = KGlobal::config();
        KConfigGroupSaver saver( config, "HTML Settings" );
        if ( config->readBoolEntry( "ShowMouseScrollIndicator", true ) ) {
            d->m_mouseScrollIndicator->show();
            d->m_mouseScrollIndicator->unsetCursor();

            QBitmap mask = d->m_mouseScrollIndicator->paletteBackgroundPixmap()->createHeuristicMask( true );

	    if ( hasHorBar && !hasVerBar ) {
                QBitmap bm( 16, 16, true );
                bitBlt( &mask, 16,  0, &bm, 0, 0, -1, -1 );
                bitBlt( &mask, 16, 32, &bm, 0, 0, -1, -1 );
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeHorCursor );
            }
            else if ( !hasHorBar && hasVerBar ) {
                QBitmap bm( 16, 16, true );
                bitBlt( &mask,  0, 16, &bm, 0, 0, -1, -1 );
                bitBlt( &mask, 32, 16, &bm, 0, 0, -1, -1 );
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeVerCursor );
            }
            else
                d->m_mouseScrollIndicator->setCursor( KCursor::SizeAllCursor );

            d->m_mouseScrollIndicator->setMask( mask );
        }
        else {
            if ( hasHorBar && !hasVerBar )
                viewport()->setCursor( KCursor::SizeHorCursor );
            else if ( !hasHorBar && hasVerBar )
                viewport()->setCursor( KCursor::SizeVerCursor );
            else
                viewport()->setCursor( KCursor::SizeAllCursor );
        }

        return;
    }
    else if ( d->m_mouseScrollTimer ) {
        delete d->m_mouseScrollTimer;
        d->m_mouseScrollTimer = 0;

        if ( d->m_mouseScrollIndicator )
            d->m_mouseScrollIndicator->hide();
    }

	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MousePress);

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    if (r && r->isWidget())
	_mouse->ignore();

    if (!swallowEvent) {
	emit m_part->nodeActivated(mev.innerNode);

	khtml::MousePressEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
        QApplication::sendEvent( m_part, &event );
        // we might be deleted after this
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
    if (d->clickCount > 0 &&
        QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= QApplication::startDragDistance())
	d->clickCount++;
    else { // shouldn't happen, if Qt has the same criterias for double clicks.
	d->clickCount = 1;
	d->clickX = xm;
	d->clickY = ym;
    }
    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEDOWN_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                           d->clickCount,_mouse,true,DOM::NodeImpl::MouseDblClick);

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

static inline void forwardPeripheralEvent(khtml::RenderWidget* r, QMouseEvent* me, int x, int y)
{
    int absx = 0;
    int absy = 0;
    r->absolutePosition(absx, absy);
    QPoint p(x-absx, y-absy);
    QMouseEvent fw(me->type(), p, me->button(), me->state());
    QWidget* w = r->widget();
    if(w)
        static_cast<khtml::RenderWidget::EventPropagator*>(w)->sendEvent(&fw);
}

void KHTMLView::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if ( d->m_mouseScrollTimer ) {
        QPoint point = mapFromGlobal( _mouse->globalPos() );

        int deltaX = point.x() - d->m_mouseScrollIndicator->x() - 24;
        int deltaY = point.y() - d->m_mouseScrollIndicator->y() - 24;

        (deltaX > 0) ? d->m_mouseScroll_byX = 1 : d->m_mouseScroll_byX = -1;
        (deltaY > 0) ? d->m_mouseScroll_byY = 1 : d->m_mouseScroll_byY = -1;

        int adX = abs( deltaX );
        int adY = abs( deltaY );

        if (adX > 100) d->m_mouseScroll_byX *= 7;
        else if (adX > 75) d->m_mouseScroll_byX *= 4;
        else if (adX > 50) d->m_mouseScroll_byX *= 2;
        else if (adX > 25) d->m_mouseScroll_byX *= 1;
        else d->m_mouseScroll_byX = 0;

        if (adY > 100) d->m_mouseScroll_byY *= 7;
        else if (adY > 75) d->m_mouseScroll_byY *= 4;
        else if (adY > 50) d->m_mouseScroll_byY *= 2;
        else if (adY > 25) d->m_mouseScroll_byY *= 1;
        else d->m_mouseScroll_byY = 0;

        if (d->m_mouseScroll_byX == 0 && d->m_mouseScroll_byY == 0) {
            d->m_mouseScrollTimer->stop();
        }
        else if (!d->m_mouseScrollTimer->isActive()) {
            d->m_mouseScrollTimer->changeInterval( 20 );
        }
    }

    if(!m_part->xmlDocImpl()) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseMove );
    // Do not modify :hover/:active state while mouse is pressed.
    m_part->xmlDocImpl()->prepareMouseEvent( _mouse->state() & Qt::MouseButtonMask /*readonly ?*/, xm, ym, &mev );

//     kdDebug(6000) << "mouse move: " << _mouse->pos()
// 		  << " button " << _mouse->button()
// 		  << " state " << _mouse->state() << endl;

    bool swallowEvent = dispatchMouseEvent(EventImpl::MOUSEMOVE_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),false,
                                           0,_mouse,true,DOM::NodeImpl::MouseMove);

    if (d->clickCount > 0 &&
        QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() > QApplication::startDragDistance()) {
	d->clickCount = 0;  // moving the mouse outside the threshold invalidates the click
    }

    // execute the scheduled script. This is to make sure the mouseover events come after the mouseout events
    m_part->executeScheduledScript();

    DOM::NodeImpl* fn = m_part->xmlDocImpl()->focusNode();
    if (fn && fn != mev.innerNode.handle() &&
        fn->renderer() && fn->renderer()->isWidget()) {
        forwardPeripheralEvent(static_cast<khtml::RenderWidget*>(fn->renderer()), _mouse, xm, ym);
    }

    khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
    khtml::RenderStyle* style = (r && r->style()) ? r->style() : 0;
    QCursor c;
    bool mailtoCursor = false;
    switch ( style ? style->cursor() : CURSOR_AUTO) {
    case CURSOR_AUTO:
        if ( r && r->isText() )
            c = KCursor::ibeamCursor();
        if ( mev.url.length() && m_part->settings()->changeCursor() ) {
            c = m_part->urlCursor();
	    if (mev.url.string().startsWith("mailto:") && mev.url.string().find('@')>0)
                mailtoCursor = true;
        }

        if (r && r->isFrameSet() && !static_cast<RenderFrameSet*>(r)->noResize())
            c = QCursor(static_cast<RenderFrameSet*>(r)->cursorShape());

        break;
    case CURSOR_CROSS:
        c = KCursor::crossCursor();
        break;
    case CURSOR_POINTER:
        c = m_part->urlCursor();
	if (mev.url.string().startsWith("mailto:") && mev.url.string().find('@')>0)
            mailtoCursor = true;
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

    if ( mailtoCursor && isVisible() && hasFocus() ) {
#ifdef Q_WS_X11
        if( !d->cursor_icon_widget ) {
            QPixmap icon_pixmap = KGlobal::iconLoader()->loadIcon( "mail_generic", KIcon::Small, 0, KIcon::DefaultState, 0, true );
            d->cursor_icon_widget = new QWidget( NULL, NULL, WX11BypassWM );
            XSetWindowAttributes attr;
            attr.save_under = True;
            XChangeWindowAttributes( qt_xdisplay(), d->cursor_icon_widget->winId(), CWSaveUnder, &attr );
            d->cursor_icon_widget->resize( icon_pixmap.width(), icon_pixmap.height());
            if( icon_pixmap.mask() )
                d->cursor_icon_widget->setMask( *icon_pixmap.mask());
            else
                d->cursor_icon_widget->clearMask();
            d->cursor_icon_widget->setBackgroundPixmap( icon_pixmap );
            d->cursor_icon_widget->erase();
        }
        QPoint c_pos = QCursor::pos();
        d->cursor_icon_widget->move( c_pos.x() + 15, c_pos.y() + 15 );
        XRaiseWindow( qt_xdisplay(), d->cursor_icon_widget->winId());
        QApplication::flushX();
        d->cursor_icon_widget->show();
#endif
    }
    else if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();

    if (r && r->isWidget()) {
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
    bool swallowEvent = false;
    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);
    DOM::NodeImpl::MouseEvent mev( _mouse->stateAfter(), DOM::NodeImpl::MouseRelease );

    if ( m_part->xmlDocImpl() )
    {
        m_part->xmlDocImpl()->prepareMouseEvent( false, xm, ym, &mev );

        swallowEvent = dispatchMouseEvent(EventImpl::MOUSEUP_EVENT,mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                                          d->clickCount,_mouse,false,DOM::NodeImpl::MouseRelease);

        if (d->clickCount > 0 &&
            QPoint(d->clickX-xm,d->clickY-ym).manhattanLength() <= QApplication::startDragDistance()) {
            QMouseEvent me(d->isDoubleClick ? QEvent::MouseButtonDblClick : QEvent::MouseButtonRelease,
                           _mouse->pos(), _mouse->button(), _mouse->state());
            dispatchMouseEvent(EventImpl::CLICK_EVENT, mev.innerNode.handle(),mev.innerNonSharedNode.handle(),true,
                               d->clickCount, &me, true, DOM::NodeImpl::MouseRelease);
        }

        DOM::NodeImpl* fn = m_part->xmlDocImpl()->focusNode();
        if (fn && fn != mev.innerNode.handle() &&
            fn->renderer() && fn->renderer()->isWidget() &&
            _mouse->button() != MidButton) {
            forwardPeripheralEvent(static_cast<khtml::RenderWidget*>(fn->renderer()), _mouse, xm, ym);
        }

        khtml::RenderObject* r = mev.innerNode.handle() ? mev.innerNode.handle()->renderer() : 0;
        if (r && r->isWidget())
            _mouse->ignore();
    }

    if (!swallowEvent) {
	khtml::MouseReleaseEvent event( _mouse, xm, ym, mev.url, mev.target, mev.innerNode );
	QApplication::sendEvent( m_part, &event );
    }
}

// returns true if event should be swallowed
bool KHTMLView::dispatchKeyEvent( QKeyEvent *_ke )
{
    if (!m_part->xmlDocImpl())
        return false;
    // Pressing and releasing a key should generate keydown, keypress and keyup events
    // Holding it down should generated keydown, keypress (repeatedly) and keyup events
    // The problem here is that Qt generates two autorepeat events (keyrelease+keypress)
    // for autorepeating, while DOM wants only one autorepeat event (keypress), so one
    // of the Qt events shouldn't be passed to DOM, but it should be still filtered
    // out if DOM would filter the autorepeat event. Additional problem is that Qt keyrelease
    // events don't have text() set (Qt bug?), so DOM often would ignore the keypress event
    // if it was created using Qt keyrelease, but Qt autorepeat keyrelease comes
    // before Qt autorepeat keypress (i.e. problem whether to filter it out or not).
    // The solution is to filter out and postpone the Qt autorepeat keyrelease until
    // the following Qt keypress event comes. If DOM accepts the DOM keypress event,
    // the postponed event will be simply discarded. If not, it will be passed to keyPressEvent()
    // again, and here it will be ignored.
    //
    //  Qt:      Press      | Release(autorepeat) Press(autorepeat) etc. |   Release
    //  DOM:   Down + Press |      (nothing)           Press             |     Up

    // It's also possible to get only Releases. E.g. the release of alt-tab,
    // or when the keypresses get captured by an accel.

    if( _ke == d->postponed_autorepeat ) // replayed event
    {
        return false;
    }

    if( _ke->type() == QEvent::KeyPress )
    {
        if( !_ke->isAutoRepeat())
        {
            bool ret = dispatchKeyEventHelper( _ke, false ); // keydown
            // don't send keypress even if keydown was blocked, like IE (and unlike Mozilla)
            if( !ret && dispatchKeyEventHelper( _ke, true )) // keypress
                ret = true;
            return ret;
        }
        else // autorepeat
        {
            bool ret = dispatchKeyEventHelper( _ke, true ); // keypress
            if( !ret && d->postponed_autorepeat )
                keyPressEvent( d->postponed_autorepeat );
            delete d->postponed_autorepeat;
            d->postponed_autorepeat = NULL;
            return ret;
        }
    }
    else // QEvent::KeyRelease
    {
        // Discard postponed "autorepeat key-release" events that didn't see
        // a keypress after them (e.g. due to QAccel)
        if ( d->postponed_autorepeat ) {
            delete d->postponed_autorepeat;
            d->postponed_autorepeat = 0;
        }

        if( !_ke->isAutoRepeat()) {
            return dispatchKeyEventHelper( _ke, false ); // keyup
        }
        else
        {
            d->postponed_autorepeat = new QKeyEvent( _ke->type(), _ke->key(), _ke->ascii(), _ke->state(),
                _ke->text(), _ke->isAutoRepeat(), _ke->count());
            if( _ke->isAccepted())
                d->postponed_autorepeat->accept();
            else
                d->postponed_autorepeat->ignore();
            return true;
        }
    }
}

// returns true if event should be swallowed
bool KHTMLView::dispatchKeyEventHelper( QKeyEvent *_ke, bool keypress )
{
    DOM::NodeImpl* keyNode = m_part->xmlDocImpl()->focusNode();
    if (keyNode) {
        return keyNode->dispatchKeyEvent(_ke, keypress);
    } else { // no focused node, send to document
        return m_part->xmlDocImpl()->dispatchKeyEvent(_ke, keypress);
    }
}

void KHTMLView::keyPressEvent( QKeyEvent *_ke )
{
#ifndef KHTML_NO_TYPE_AHEAD_FIND
	if(d->typeAheadActivated)
	{
		// type-ahead find aka find-as-you-type
		if(_ke->key() == Key_BackSpace)
		{
			d->findString = d->findString.left(d->findString.length() - 1);

			if(!d->findString.isEmpty())
			{
				findAhead(false);
			}
			else
			{
				findTimeout();
			}

			d->timer.start(3000, true);
			_ke->accept();
			return;
		}
		else if(_ke->key() == Key_Escape)
		{
			findTimeout();

			_ke->accept();
			return;
		}
		else if(_ke->key() == Key_Space || !_ke->text().stripWhiteSpace().isEmpty())
		{
			d->findString += _ke->text();

			findAhead(true);

			d->timer.start(3000, true);
			_ke->accept();
			return;
		}
	}
#endif // KHTML_NO_TYPE_AHEAD_FIND

#ifndef KHTML_NO_CARET
    if (m_part->isEditable() || m_part->isCaretMode()
        || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
	    && m_part->xmlDocImpl()->focusNode()->contentEditable())) {
      d->caretViewContext()->keyReleasePending = true;
      caretKeyPressEvent(_ke);
      return;
    }
#endif // KHTML_NO_CARET

    // If CTRL was hit, be prepared for access keys
    if (d->accessKeysEnabled && _ke->key() == Key_Control && _ke->state()==0 && !d->accessKeysActivated)
    {
        d->accessKeysPreActivate=true;
        _ke->accept();
        return;
    }

    if (_ke->key() == Key_Shift && _ke->state()==0)
	    d->scrollSuspendPreActivate=true;

    // accesskey handling needs to be done before dispatching, otherwise e.g. lineedits
    // may eat the event

    if (d->accessKeysEnabled && d->accessKeysActivated)
    {
        int state = ( _ke->state() & ( ShiftButton | ControlButton | AltButton | MetaButton ));
        if ( state==0 || state==ShiftButton) {
	if (_ke->key() != Key_Shift) accessKeysTimeout();
        handleAccessKey( _ke );
        _ke->accept();
        return;
    	}
	accessKeysTimeout();
    }

    if ( dispatchKeyEvent( _ke )) {
        // If either keydown or keypress was accepted by a widget, or canceled by JS, stop here.
        _ke->accept();
        return;
    }

    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    if (_ke->state() & Qt::ShiftButton)
      switch(_ke->key())
        {
        case Key_Space:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                scrollBy( 0, -clipper()->height() + offs );
                if(d->scrollSuspended)
                    d->newScrollTimer(this, 0);
            }
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
                if (!d->scrollTimerId || d->scrollSuspended)
                    scrollBy( 0, 10 );
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
            }
            break;

        case Key_Space:
        case Key_Next:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                scrollBy( 0, clipper()->height() - offs );
                if(d->scrollSuspended)
                    d->newScrollTimer(this, 0);
            }
            break;

        case Key_Up:
        case Key_K:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (!d->scrollTimerId || d->scrollSuspended)
                    scrollBy( 0, -10 );
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
            }
            break;

        case Key_Prior:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                scrollBy( 0, -clipper()->height() + offs );
                if(d->scrollSuspended)
                    d->newScrollTimer(this, 0);
            }
            break;
        case Key_Right:
        case Key_L:
            if ( d->hmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (!d->scrollTimerId || d->scrollSuspended)
                    scrollBy( 10, 0 );
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
            }
            break;
        case Key_Left:
        case Key_H:
            if ( d->hmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                if (!d->scrollTimerId || d->scrollSuspended)
                    scrollBy( -10, 0 );
                if (d->scrollTimerId)
                    d->newScrollTimer(this, 0);
            }
            break;
        case Key_Enter:
        case Key_Return:
	    // ### FIXME:
	    // or even better to HTMLAnchorElementImpl::event()
            if (m_part->xmlDocImpl()) {
		NodeImpl *n = m_part->xmlDocImpl()->focusNode();
		if (n)
		    n->setActive();
	    }
            break;
        case Key_Home:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                setContentsPos( 0, 0 );
                if(d->scrollSuspended)
                    d->newScrollTimer(this, 0);
            }
            break;
        case Key_End:
            if ( d->vmode == QScrollView::AlwaysOff )
                _ke->accept();
            else {
                setContentsPos( 0, contentsHeight() - visibleHeight() );
                if(d->scrollSuspended)
                    d->newScrollTimer(this, 0);
            }
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

void KHTMLView::findTimeout()
{
#ifndef KHTML_NO_TYPE_AHEAD_FIND
	d->typeAheadActivated = false;
	d->findString = "";
	m_part->setStatusBarText(i18n("Find stopped."), KHTMLPart::BarDefaultText);
	m_part->enableFindAheadActions( true );
#endif // KHTML_NO_TYPE_AHEAD_FIND
}

#ifndef KHTML_NO_TYPE_AHEAD_FIND
void KHTMLView::startFindAhead( bool linksOnly )
{
	if( linksOnly )
	{
		d->findLinksOnly = true;
		m_part->setStatusBarText(i18n("Starting -- find links as you type"),
		                         KHTMLPart::BarDefaultText);
	}
	else
	{
		d->findLinksOnly = false;
		m_part->setStatusBarText(i18n("Starting -- find text as you type"),
		                         KHTMLPart::BarDefaultText);
	}

	m_part->findTextBegin();
	d->typeAheadActivated = true;
        // disable, so that the shortcut ( / or ' by default ) doesn't interfere
	m_part->enableFindAheadActions( false );
	d->timer.start(3000, true);
}

void KHTMLView::findAhead(bool increase)
{
	QString status;

	if(d->findLinksOnly)
	{
		m_part->findText(d->findString, KHTMLPart::FindNoPopups |
		                 KHTMLPart::FindLinksOnly, this);
		if(m_part->findTextNext())
		{
			status = i18n("Link found: \"%1\".");
		}
		else
		{
			if(increase) KNotifyClient::beep();
			status = i18n("Link not found: \"%1\".");
		}
	}
	else
	{
		m_part->findText(d->findString, KHTMLPart::FindNoPopups, this);
		if(m_part->findTextNext())
		{
			status = i18n("Text found: \"%1\".");
		}
		else
		{
			if(increase) KNotifyClient::beep();
			status = i18n("Text not found: \"%1\".");
		}
	}

	m_part->setStatusBarText(status.arg(d->findString.lower()),
	                         KHTMLPart::BarDefaultText);
}

void KHTMLView::updateFindAheadTimeout()
{
    if( d->typeAheadActivated )
        d->timer.start( 3000, true );
}

#endif // KHTML_NO_TYPE_AHEAD_FIND

void KHTMLView::keyReleaseEvent(QKeyEvent *_ke)
{
#ifndef KHTML_NO_TYPE_AHEAD_FIND
    if(d->typeAheadActivated) {
        _ke->accept();
        return;
    }
#endif
    if (d->m_caretViewContext && d->m_caretViewContext->keyReleasePending) {
        //caretKeyReleaseEvent(_ke);
	d->m_caretViewContext->keyReleasePending = false;
	return;
    }

    if( d->scrollSuspendPreActivate && _ke->key() != Key_Shift )
        d->scrollSuspendPreActivate = false;
    if( _ke->key() == Key_Shift && d->scrollSuspendPreActivate && _ke->state() == Qt::ShiftButton
        && !(KApplication::keyboardMouseState() & Qt::ShiftButton))
        if (d->scrollTimerId)
                d->scrollSuspended = !d->scrollSuspended;

    if (d->accessKeysEnabled) 
    {
        if (d->accessKeysPreActivate && _ke->key() != Key_Control) 
            d->accessKeysPreActivate=false;
        if (d->accessKeysPreActivate && _ke->state() == Qt::ControlButton && !(KApplication::keyboardMouseState() & Qt::ControlButton))
        {
	    displayAccessKeys();
	    m_part->setStatusBarText(i18n("Access Keys activated"),KHTMLPart::BarOverrideText);
	    d->accessKeysActivated = true;
	    d->accessKeysPreActivate = false;
            _ke->accept();
            return;
        }
	else if (d->accessKeysActivated) 
        {
            accessKeysTimeout();
            _ke->accept();
            return;
        }
    }

    // Send keyup event
    if ( dispatchKeyEvent( _ke ) )
    {
        _ke->accept();
        return;
    }

    QScrollView::keyReleaseEvent(_ke);
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
    if (m_part->xmlDocImpl() && focusNextPrevNode(next))
    {
	if (m_part->xmlDocImpl()->focusNode())
	    kdDebug() << "focusNode.name: "
		      << m_part->xmlDocImpl()->focusNode()->nodeName().string() << endl;
	return true; // focus node found
    }

    // If we get here, pass tabbing control up to the next/previous child in our parent
    d->pseudoFocusNode = KHTMLViewPrivate::PFNone;
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

#ifndef KHTML_NO_SELECTION
        // extend the selection while scrolling
	DOM::Node innerNode;
	if (m_part->isExtendingSelection()) {
            RenderObject::NodeInfo renderInfo(true/*readonly*/, false/*active*/);
            m_part->xmlDocImpl()->renderer()->layer()
				->nodeAtPoint(renderInfo, xm, ym);
            innerNode = renderInfo.innerNode();
	}/*end if*/

        if (innerNode.handle() && innerNode.handle()->renderer()) {
            int absX, absY;
            innerNode.handle()->renderer()->absolutePosition(absX, absY);

            m_part->extendSelectionTo(xm, ym, absX, absY, innerNode);
        }/*end if*/
#endif // KHTML_NO_SELECTION
    }
}


class HackWidget : public QWidget
{
 public:
    inline void setNoErase() { setWFlags(getWFlags()|WRepaintNoErase); }
};

bool KHTMLView::eventFilter(QObject *o, QEvent *e)
{
    if ( e->type() == QEvent::AccelOverride ) {
	QKeyEvent* ke = (QKeyEvent*) e;
//kdDebug(6200) << "QEvent::AccelOverride" << endl;
	if (m_part->isEditable() || m_part->isCaretMode()
	    || (m_part->xmlDocImpl() && m_part->xmlDocImpl()->focusNode()
		&& m_part->xmlDocImpl()->focusNode()->contentEditable())) {
//kdDebug(6200) << "editable/navigable" << endl;
	    if ( (ke->state() & ControlButton) || (ke->state() & ShiftButton) ) {
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
		}
	    }
	}
    }

    if ( e->type() == QEvent::Leave && d->cursor_icon_widget )
        d->cursor_icon_widget->hide();

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
		    if (!strcmp(w->name(), "__khtml")) {
			w->installEventFilter(this);
			w->unsetCursor();
			if (!::qt_cast<QFrame*>(w))
			    w->setBackgroundMode( QWidget::NoBackground );
			static_cast<HackWidget *>(w)->setNoErase();
			if (w->children()) {
			    QObjectListIterator it(*w->children());
			    for (; it.current(); ++it) {
				QWidget *widget = ::qt_cast<QWidget *>(it.current());
				if (widget && !widget->isTopLevel()) {
				    if (!::qt_cast<QFrame*>(w))
				        widget->setBackgroundMode( QWidget::NoBackground );
				    static_cast<HackWidget *>(widget)->setNoErase();
				    widget->installEventFilter(this);
				}
			    }
			}
		    }
		}
	    }
	}
    } else if (o->isWidgetType()) {
	QWidget *v = static_cast<QWidget *>(o);
        QWidget *c = v;
	while (v && v != view) {
            c = v;
	    v = v->parentWidget(true);
	}

	if (v && !strcmp(c->name(), "__khtml")) {
	    bool block = false;
	    QWidget *w = static_cast<QWidget *>(o);
	    switch(e->type()) {
	    case QEvent::Paint:
		if (!allowWidgetPaintEvents) {
		    // eat the event. Like this we can control exactly when the widget
		    // get's repainted.
		    block = true;
		    int x = 0, y = 0;
		    QWidget *v = w;
		    while (v && v != view) {
			x += v->x();
			y += v->y();
			v = v->parentWidget();
		    }
		    viewportToContents( x, y, x, y );
		    QPaintEvent *pe = static_cast<QPaintEvent *>(e);
		    bool asap = !d->contentsMoving && ::qt_cast<QScrollView *>(c);

		    // QScrollView needs fast repaints
		    if ( asap && !d->painting && m_part->xmlDocImpl() && m_part->xmlDocImpl()->renderer() &&
		         !static_cast<khtml::RenderCanvas *>(m_part->xmlDocImpl()->renderer())->needsLayout() ) {
		        repaintContents(x + pe->rect().x(), y + pe->rect().y(),
	                                        pe->rect().width(), pe->rect().height(), true);
                    } else {
 		        scheduleRepaint(x + pe->rect().x(), y + pe->rect().y(),
 				    pe->rect().width(), pe->rect().height(), asap);
                    }
		}
		break;
	    case QEvent::MouseMove:
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonRelease:
	    case QEvent::MouseButtonDblClick: {
		if (w->parentWidget() == view && !::qt_cast<QScrollBar *>(w)) {
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
		    block = true;
                }
		break;
	    }
	    case QEvent::KeyPress:
	    case QEvent::KeyRelease:
		if (w->parentWidget() == view && !::qt_cast<QScrollBar *>(w)) {
		    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
		    if (e->type() == QEvent::KeyPress)
			keyPressEvent(ke);
		    else
			keyReleaseEvent(ke);
		    block = true;
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

//    kdDebug(6000) <<"passing event on to sv event filter object=" << o->className() << " event=" << e->type() << endl;
    return QScrollView::eventFilter(o, e);
}


DOM::NodeImpl *KHTMLView::nodeUnderMouse() const
{
    return d->underMouse;
}

DOM::NodeImpl *KHTMLView::nonSharedNodeUnderMouse() const
{
    return d->underMouseNonShared;
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

    d->scrollingSelf = false;

    if ( (abs(deltax)<=maxx) && (abs(deltay)<=maxy) )
	return true;
    else return false;

}

bool KHTMLView::focusNextPrevNode(bool next)
{
    // Sets the focus node of the document to be the node after (or if
    // next is false, before) the current focus node.  Only nodes that
    // are selectable (i.e. for which isFocusable() returns true) are
    // taken into account, and the order used is that specified in the
    // HTML spec (see DocumentImpl::nextFocusNode() and
    // DocumentImpl::previousFocusNode() for details).

    DocumentImpl *doc = m_part->xmlDocImpl();
    NodeImpl *oldFocusNode = doc->focusNode();

#if 1
    // If the user has scrolled the document, then instead of picking
    // the next focusable node in the document, use the first one that
    // is within the visible area (if possible).
    if (d->scrollBarMoved)
    {
	NodeImpl *toFocus;
	if (next)
	    toFocus = doc->nextFocusNode(oldFocusNode);
	else
	    toFocus = doc->previousFocusNode(oldFocusNode);

	if (!toFocus && oldFocusNode)
	    if (next)
		toFocus = doc->nextFocusNode(NULL);
	    else
		toFocus = doc->previousFocusNode(NULL);

	while (toFocus && toFocus != oldFocusNode)
	{

	    QRect focusNodeRect = toFocus->getRect();
	    if ((focusNodeRect.left() > contentsX()) && (focusNodeRect.right() < contentsX() + visibleWidth()) &&
		(focusNodeRect.top() > contentsY()) && (focusNodeRect.bottom() < contentsY() + visibleHeight())) {
		{
		    QRect r = toFocus->getRect();
		    ensureVisible( r.right(), r.bottom());
		    ensureVisible( r.left(), r.top());
		    d->scrollBarMoved = false;
		    d->tabMovePending = false;
		    d->lastTabbingDirection = next;
		    d->pseudoFocusNode = KHTMLViewPrivate::PFNone;
		    m_part->xmlDocImpl()->setFocusNode(toFocus);
		    Node guard(toFocus);
		    if (!toFocus->hasOneRef() )
		    {
			emit m_part->nodeActivated(Node(toFocus));
		    }
		    return true;
		}
	    }
	    if (next)
		toFocus = doc->nextFocusNode(toFocus);
	    else
		toFocus = doc->previousFocusNode(toFocus);

	    if (!toFocus && oldFocusNode)
		if (next)
		    toFocus = doc->nextFocusNode(NULL);
		else
		    toFocus = doc->previousFocusNode(NULL);
	}

	d->scrollBarMoved = false;
    }
#endif

    if (!oldFocusNode && d->pseudoFocusNode == KHTMLViewPrivate::PFNone)
    {
	ensureVisible(contentsX(), next?0:contentsHeight());
	d->scrollBarMoved = false;
	d->pseudoFocusNode = next?KHTMLViewPrivate::PFTop:KHTMLViewPrivate::PFBottom;
	return true;
    }

    NodeImpl *newFocusNode = NULL;

    if (d->tabMovePending && next != d->lastTabbingDirection)
    {
	//kdDebug ( 6000 ) << " tab move pending and tabbing direction changed!\n";
	newFocusNode = oldFocusNode;
    }
    else if (next)
    {
	if (oldFocusNode || d->pseudoFocusNode == KHTMLViewPrivate::PFTop )
	    newFocusNode = doc->nextFocusNode(oldFocusNode);
    }
    else
    {
	if (oldFocusNode || d->pseudoFocusNode == KHTMLViewPrivate::PFBottom )
	    newFocusNode = doc->previousFocusNode(oldFocusNode);
    }

    bool targetVisible = false;
    if (!newFocusNode)
    {
	if ( next )
	{
	    targetVisible = scrollTo(QRect(contentsX()+visibleWidth()/2,contentsHeight()-d->borderY,0,0));
	}
	else
	{
	    targetVisible = scrollTo(QRect(contentsX()+visibleWidth()/2,d->borderY,0,0));
	}
    }
    else
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

	targetVisible = scrollTo(newFocusNode->getRect());
    }

    if (targetVisible)
    {
	//kdDebug ( 6000 ) << " target reached.\n";
	d->tabMovePending = false;

	m_part->xmlDocImpl()->setFocusNode(newFocusNode);
	if (newFocusNode)
	{
	    Node guard(newFocusNode);
	    if (!newFocusNode->hasOneRef() )
	    {
		emit m_part->nodeActivated(Node(newFocusNode));
	    }
	    return true;
	}
	else
	{
	    d->pseudoFocusNode = next?KHTMLViewPrivate::PFBottom:KHTMLViewPrivate::PFTop;
	    return false;
	}
    }
    else
    {
	if (!d->tabMovePending)
	    d->lastTabbingDirection = next;
	d->tabMovePending = true;
	return true;
    }
}

void KHTMLView::displayAccessKeys()
{
    QValueVector< QChar > taken;
    displayAccessKeys( NULL, this, taken, false );
    displayAccessKeys( NULL, this, taken, true );
}

void KHTMLView::displayAccessKeys( KHTMLView* caller, KHTMLView* origview, QValueVector< QChar >& taken, bool use_fallbacks )
{
    QMap< ElementImpl*, QChar > fallbacks;
    if( use_fallbacks )
        fallbacks = buildFallbackAccessKeys();
    for( NodeImpl* n = m_part->xmlDocImpl(); n != NULL; n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* en = static_cast< ElementImpl* >( n );
            DOMString s = en->getAttribute( ATTR_ACCESSKEY );
            QString accesskey;
            if( s.length() == 1 ) {
                QChar a = s.string()[ 0 ].upper();
                if( qFind( taken.begin(), taken.end(), a ) == taken.end()) // !contains
                    accesskey = a;
            }
            if( accesskey.isNull() && fallbacks.contains( en )) {
                QChar a = fallbacks[ en ].upper();
                if( qFind( taken.begin(), taken.end(), a ) == taken.end()) // !contains
                    accesskey = QString( "<qt><i>" ) + a + "</i></qt>";
            }
            if( !accesskey.isNull()) {
	        QRect rec=en->getRect();
	        QLabel *lab=new QLabel(accesskey,viewport(),0,Qt::WDestructiveClose);
	        connect( origview, SIGNAL(hideAccessKeys()), lab, SLOT(close()) );
	        connect( this, SIGNAL(repaintAccessKeys()), lab, SLOT(repaint()));
	        lab->setPalette(QToolTip::palette());
	        lab->setLineWidth(2);
	        lab->setFrameStyle(QFrame::Box | QFrame::Plain);
	        lab->setMargin(3);
	        lab->adjustSize();
	        addChild(lab,
                    KMIN(rec.left()+rec.width()/2, contentsWidth() - lab->width()),
                    KMIN(rec.top()+rec.height()/2, contentsHeight() - lab->height()));
	        showChild(lab);
                taken.append( accesskey[ 0 ] );
	    }
        }
    }
    if( use_fallbacks )
        return;
    QPtrList<KParts::ReadOnlyPart> frames = m_part->frames();
    for( QPtrListIterator<KParts::ReadOnlyPart> it( frames );
         it != NULL;
         ++it ) {
        if( !(*it)->inherits( "KHTMLPart" ))
            continue;
        KHTMLPart* part = static_cast< KHTMLPart* >( *it );
        if( part->view() && part->view() != caller )
            part->view()->displayAccessKeys( this, origview, taken, use_fallbacks );
    }
    // pass up to the parent
    if (m_part->parentPart() && m_part->parentPart()->view()
        && m_part->parentPart()->view() != caller)
        m_part->parentPart()->view()->displayAccessKeys( this, origview, taken, use_fallbacks );
}



void KHTMLView::accessKeysTimeout()
{
d->accessKeysActivated=false;
d->accessKeysPreActivate = false;
m_part->setStatusBarText(QString::null, KHTMLPart::BarOverrideText);
emit hideAccessKeys();
}

// Handling of the HTML accesskey attribute.
bool KHTMLView::handleAccessKey( const QKeyEvent* ev )
{
// Qt interprets the keyevent also with the modifiers, and ev->text() matches that,
// but this code must act as if the modifiers weren't pressed
    QChar c;
    if( ev->key() >= Key_A && ev->key() <= Key_Z )
        c = 'A' + ev->key() - Key_A;
    else if( ev->key() >= Key_0 && ev->key() <= Key_9 )
        c = '0' + ev->key() - Key_0;
    else {
        // TODO fake XKeyEvent and XLookupString ?
        // This below seems to work e.g. for eacute though.
        if( ev->text().length() == 1 )
            c = ev->text()[ 0 ];
    }
    if( c.isNull())
        return false;
    return focusNodeWithAccessKey( c );
}

bool KHTMLView::focusNodeWithAccessKey( QChar c, KHTMLView* caller )
{
    DocumentImpl *doc = m_part->xmlDocImpl();
    if( !doc )
        return false;
    ElementImpl* node = doc->findAccessKeyElement( c );
    if( !node ) {
        QPtrList<KParts::ReadOnlyPart> frames = m_part->frames();
        for( QPtrListIterator<KParts::ReadOnlyPart> it( frames );
             it != NULL;
             ++it ) {
            if( !(*it)->inherits( "KHTMLPart" ))
                continue;
            KHTMLPart* part = static_cast< KHTMLPart* >( *it );
            if( part->view() && part->view() != caller
                && part->view()->focusNodeWithAccessKey( c, this ))
                return true;
        }
        // pass up to the parent
        if (m_part->parentPart() && m_part->parentPart()->view()
            && m_part->parentPart()->view() != caller
            && m_part->parentPart()->view()->focusNodeWithAccessKey( c, this ))
            return true;
        if( caller == NULL ) { // the active frame (where the accesskey was pressed)
            QMap< ElementImpl*, QChar > fallbacks = buildFallbackAccessKeys();
            for( QMap< ElementImpl*, QChar >::ConstIterator it = fallbacks.begin();
                 it != fallbacks.end();
                 ++it )
                if( *it == c ) {
                    node = it.key();
                    break;
                }
        }
        if( node == NULL )
            return false;
    }

    // Scroll the view as necessary to ensure that the new focus node is visible
#ifndef KHTML_NO_CARET
    // if it's an editable element, activate the caret
    if (!m_part->isCaretMode() && !m_part->isEditable()
	&& node->contentEditable()) {
        d->caretViewContext();
        moveCaretTo(node, 0L, true);
    } else {
        caretOff();
    }
#endif // KHTML_NO_CARET

    QRect r = node->getRect();
    ensureVisible( r.right(), r.bottom());
    ensureVisible( r.left(), r.top());

    Node guard( node );
    if( node->isFocusable()) {
	if (node->id()==ID_LABEL) {
	    // if Accesskey is a label, give focus to the label's referrer.
	    node=static_cast<ElementImpl *>(static_cast< HTMLLabelElementImpl* >( node )->getFormElement());
	    if (!node) return true;
            guard = node;
	}
        // Set focus node on the document
        QFocusEvent::setReason( QFocusEvent::Shortcut );
        m_part->xmlDocImpl()->setFocusNode(node);
        QFocusEvent::resetReason();
        if( node != NULL && node->hasOneRef()) // deleted, only held by guard
            return true;
        emit m_part->nodeActivated(Node(node));
        if( node != NULL && node->hasOneRef())
            return true;
    }

    switch( node->id()) {
        case ID_A:
            static_cast< HTMLAnchorElementImpl* >( node )->click();
          break;
        case ID_INPUT:
            static_cast< HTMLInputElementImpl* >( node )->click();
          break;
        case ID_BUTTON:
            static_cast< HTMLButtonElementImpl* >( node )->click();
          break;
        case ID_AREA:
            static_cast< HTMLAreaElementImpl* >( node )->click();
          break;
        case ID_TEXTAREA:
	  break; // just focusing it is enough
        case ID_LEGEND:
            // TODO
          break;
    }
    return true;
}

static QString getElementText( NodeImpl* start, bool after )
{
    QString ret;             // nextSibling(), to go after e.g. </select>
    for( NodeImpl* n = after ? start->nextSibling() : start->traversePreviousNode();
         n != NULL;
         n = after ? n->traverseNextNode() : n->traversePreviousNode()) {
        if( n->isTextNode()) {
            if( after )
                ret += static_cast< TextImpl* >( n )->toString().string();
            else
                ret.prepend( static_cast< TextImpl* >( n )->toString().string());
        } else {
            switch( n->id()) {
                case ID_A:
                case ID_FONT:
                case ID_TT:
                case ID_U:
                case ID_B:
                case ID_I:
                case ID_S:
                case ID_STRIKE:
                case ID_BIG:
                case ID_SMALL:
                case ID_EM:
                case ID_STRONG:
                case ID_DFN:
                case ID_CODE:
                case ID_SAMP:
                case ID_KBD:
                case ID_VAR:
                case ID_CITE:
                case ID_ABBR:
                case ID_ACRONYM:
                case ID_SUB:
                case ID_SUP:
                case ID_SPAN:
                case ID_NOBR:
                case ID_WBR:
                    break;
                case ID_TD:
                    if( ret.stripWhiteSpace().isEmpty())
                        break;
                    // fall through
                default:
                    return ret.simplifyWhiteSpace();
            }
        }
    }
    return ret.simplifyWhiteSpace();
}

static QMap< NodeImpl*, QString > buildLabels( NodeImpl* start )
{
    QMap< NodeImpl*, QString > ret;
    for( NodeImpl* n = start;
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->id() == ID_LABEL ) {
            HTMLLabelElementImpl* label = static_cast< HTMLLabelElementImpl* >( n );
            NodeImpl* labelfor = label->getFormElement();
            if( labelfor )
                ret[ labelfor ] = label->innerText().string().simplifyWhiteSpace();
        }
    }
    return ret;
}

namespace khtml {
struct AccessKeyData {
    ElementImpl* element;
    QString text;
    QString url;
    int priority; // 10(highest) - 0(lowest)
};
}

QMap< ElementImpl*, QChar > KHTMLView::buildFallbackAccessKeys() const
{
    // build a list of all possible candidate elements that could use an accesskey
    QValueList< AccessKeyData > data;
    QMap< NodeImpl*, QString > labels = buildLabels( m_part->xmlDocImpl());
    for( NodeImpl* n = m_part->xmlDocImpl();
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* element = static_cast< ElementImpl* >( n );
            if( element->getAttribute( ATTR_ACCESSKEY ).length() == 1 )
                continue; // has accesskey set, ignore
            if( element->renderer() == NULL )
                continue; // not visible
            QString text;
            QString url;
            int priority = 0;
            bool ignore = false;
            bool text_after = false;
            bool text_before = false;
            switch( element->id()) {
                case ID_A:
                    url = khtml::parseURL(element->getAttribute(ATTR_HREF)).string();
                    if( url.isEmpty()) // doesn't have href, it's only an anchor
                        continue;
                    text = static_cast< HTMLElementImpl* >( element )->innerText().string().simplifyWhiteSpace();
                    priority = 2;
                    break;
                case ID_INPUT: {
                    HTMLInputElementImpl* in = static_cast< HTMLInputElementImpl* >( element );
                    switch( in->inputType()) {
                        case HTMLInputElementImpl::SUBMIT:
                            text = in->value().string();
                            if( text.isEmpty())
                                text = i18n( "Submit" );
                            priority = 7;
                            break;
                        case HTMLInputElementImpl::IMAGE:
                            text = in->altText().string();
                            priority = 7;
                            break;
                        case HTMLInputElementImpl::BUTTON:
                            text = in->value().string();
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::RESET:
                            text = in->value().string();
                            if( text.isEmpty())
                                text = i18n( "Reset" );
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::HIDDEN:
                            ignore = true;
                            break;
                        case HTMLInputElementImpl::CHECKBOX:
                        case HTMLInputElementImpl::RADIO:
                            text_after = true;
                            priority = 5;
                            break;
                        case HTMLInputElementImpl::TEXT:
                        case HTMLInputElementImpl::PASSWORD:
                        case HTMLInputElementImpl::FILE:
                            text_before = true;
                            priority = 5;
                            break;
                        default:
                            priority = 5;
                            break;
                    }
                    break;
                }
                case ID_BUTTON:
                    text = static_cast< HTMLElementImpl* >( element )->innerText().string().simplifyWhiteSpace();
                    switch( static_cast< HTMLButtonElementImpl* >( element )->buttonType()) {
                        case HTMLButtonElementImpl::SUBMIT:
                            if( text.isEmpty())
                                text = i18n( "Submit" );
                            priority = 7;
                            break;
                        case HTMLButtonElementImpl::RESET:
                            if( text.isEmpty())
                                text = i18n( "Reset" );
                            priority = 5;
                            break;
                        default:
                            priority = 5;
                            break;
                    break;
                    }
                case ID_SELECT: // these don't have accesskey attribute, but quick access may be handy
                    text_before = true;
                    text_after = true;
                    priority = 5;
                    break;
                case ID_FRAME:
                    ignore = true;
                    break;
                default:
                    ignore = !element->isFocusable();
                    priority = 2;
                    break;
            }
            if( ignore )
                continue;
            if( text.isNull() && labels.contains( element ))
                text = labels[ element ];
            if( text.isNull() && text_before )
                text = getElementText( element, false );
            if( text.isNull() && text_after )
                text = getElementText( element, true );
            text = text.stripWhiteSpace();
            // increase priority of items which have explicitly specified accesskeys in the config
            QValueList< QPair< QString, QChar > > priorities
                = m_part->settings()->fallbackAccessKeysAssignments();
            for( QValueList< QPair< QString, QChar > >::ConstIterator it = priorities.begin();
                 it != priorities.end();
                 ++it ) {
                if( text == (*it).first )
                    priority = 10;
            }
            AccessKeyData tmp = { element, text, url, priority };
            data.append( tmp );
        }
    }

    QValueList< QChar > keys;
    for( char c = 'A'; c <= 'Z'; ++c )
        keys << c;
    for( char c = '0'; c <= '9'; ++c )
        keys << c;
    for( NodeImpl* n = m_part->xmlDocImpl();
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* en = static_cast< ElementImpl* >( n );
            DOMString s = en->getAttribute( ATTR_ACCESSKEY );
            if( s.length() == 1 ) {
                QChar c = s.string()[ 0 ].upper();
                keys.remove( c ); // remove manually assigned accesskeys
            }
        }
    }

    QMap< ElementImpl*, QChar > ret;
    for( int priority = 10;
         priority >= 0;
         --priority ) {
        for( QValueList< AccessKeyData >::Iterator it = data.begin();
             it != data.end();
             ) {
            if( (*it).priority != priority ) {
                ++it;
                continue;
            }
            if( keys.isEmpty())
                break;
            QString text = (*it).text;
            QChar key;
            if( key.isNull() && !text.isEmpty()) {
                QValueList< QPair< QString, QChar > > priorities
                    = m_part->settings()->fallbackAccessKeysAssignments();
                for( QValueList< QPair< QString, QChar > >::ConstIterator it = priorities.begin();
                     it != priorities.end();
                     ++it )
                    if( text == (*it).first && keys.contains( (*it).second )) {
                        key = (*it).second;
                        break;
                    }
            }
            // try first to select the first character as the accesskey,
            // then first character of the following words,
            // and then simply the first free character
            if( key.isNull() && !text.isEmpty()) {
                QStringList words = QStringList::split( ' ', text );
                for( QStringList::ConstIterator it = words.begin();
                     it != words.end();
                     ++it ) {
                    if( keys.contains( (*it)[ 0 ].upper())) {
                        key = (*it)[ 0 ].upper();
                        break;
                    }
                }
            }
            if( key.isNull() && !text.isEmpty()) {
                for( unsigned int i = 0;
                     i < text.length();
                     ++i ) {
                    if( keys.contains( text[ i ].upper())) {
                        key = text[ i ].upper();
                        break;
                    }
                }
            }
            if( key.isNull())
                key = keys.front();
            ret[ (*it).element ] = key;
            keys.remove( key );
            QString url = (*it).url;
            it = data.remove( it );
            // assign the same accesskey also to other elements pointing to the same url
            if( !url.isEmpty() && !url.startsWith( "javascript:", false )) {
                for( QValueList< AccessKeyData >::Iterator it2 = data.begin();
                     it2 != data.end();
                     ) {                   
                    if( (*it2).url == url ) {
                        ret[ (*it2).element ] = key;
                        if( it == it2 )
                            ++it;
                        it2 = data.remove( it2 );
                    } else
                        ++it2;
                }
            }
        }
    }
    return ret;
}

void KHTMLView::setMediaType( const QString &medium )
{
    m_medium = medium;
}

QString KHTMLView::mediaType() const
{
    return m_medium;
}

bool KHTMLView::pagedMode() const
{
    return d->paged;
}

void KHTMLView::setWidgetVisible(RenderWidget* w, bool vis)
{
    if (vis) {
        d->visibleWidgets.replace(w, w->widget());
    }
    else
        d->visibleWidgets.remove(w);
}

bool KHTMLView::needsFullRepaint() const
{
    return d->needsFullRepaint;
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

    KPrinter *printer = new KPrinter(true, QPrinter::ScreenResolution);
    printer->addDialogPage(new KHTMLPrintSettings());
    QString docname = m_part->xmlDocImpl()->URL().prettyURL();
    if ( !docname.isEmpty() )
        docname = KStringHandler::csqueeze(docname, 80);
    if(quick || printer->setup(this, i18n("Print %1").arg(docname))) {
        viewport()->setCursor( waitCursor ); // only viewport(), no QApplication::, otherwise we get the busy cursor in kdeprint's dialogs
        // set up KPrinter
        printer->setFullPage(false);
        printer->setCreator(QString("KDE %1.%2.%3 HTML Library").arg(KDE_VERSION_MAJOR).arg(KDE_VERSION_MINOR).arg(KDE_VERSION_RELEASE));
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

        kdDebug(6000) << "printing: physical page width = " << metrics.width()
                      << " height = " << metrics.height() << endl;
        root->setStaticMode(true);
        root->setPagedMode(true);
        root->setWidth(metrics.width());
//         root->setHeight(metrics.height());
        root->setPageTop(0);
        root->setPageBottom(0);
        d->paged = true;

        m_part->xmlDocImpl()->styleSelector()->computeFontSizes(&metrics, 100);
        m_part->xmlDocImpl()->updateStyleSelector();
        root->setPrintImages( printer->option("app-khtml-printimages") == "true");
        root->makePageBreakAvoidBlocks();

        root->setNeedsLayoutAndMinMaxRecalc();
        root->layout();
        khtml::RenderWidget::flushWidgetResizes(); // make sure widgets have their final size

        // check sizes ask for action.. (scale or clip)

        bool printHeader = (printer->option("app-khtml-printheader") == "true");

        int headerHeight = 0;
        QFont headerFont("Sans Serif", 8);

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
        int pageWidth = metrics.width();
        int pageHeight = metrics.height();
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

        root->setHeight(pageHeight);
        root->setPageBottom(pageHeight);
        root->setNeedsLayout(true);
        root->layoutIfNeeded();
//         m_part->slotDebugRenderTree();

        // Squeeze header to make it it on the page.
        if (printHeader)
        {
            int available_width = metrics.width() - 10 -
                2 * kMax(p->boundingRect(0, 0, metrics.width(), p->fontMetrics().lineSpacing(), Qt::AlignLeft, headerLeft).width(),
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
        int bottom = 0;
        int page = 1;
        while(top < root->docHeight()) {
            if(top > 0) printer->newPage();
            p->setClipRect(0, 0, pageWidth, headerHeight, QPainter::CoordDevice);
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

            p->setClipRect(0, headerHeight, pageWidth, pageHeight, QPainter::CoordDevice);
            p->translate(0, headerHeight-top);

            bottom = top+pageHeight;

            root->setPageTop(top);
            root->setPageBottom(bottom);
            root->setPageNumber(page);

            root->layer()->paint(p, QRect(0, top, pageWidth, pageHeight));
//             m_part->xmlDocImpl()->renderer()->layer()->paint(p, QRect(0, top, pageWidth, pageHeight));
//             root->repaint();
//             p->flush();
            kdDebug(6000) << "printed: page " << page <<" bottom At = " << bottom << endl;

            top = bottom;
            p->resetXForm();
            page++;
        }

        p->end();
        delete p;

        // and now reset the layout to the usual one...
        root->setPagedMode(false);
        root->setStaticMode(false);
        d->paged = false;
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
    root->setPagedMode(true);
    root->setStaticMode(true);
    root->setWidth(rc.width());

    p->save();
    p->setClipRect(rc);
    p->translate(rc.left(), rc.top());
    double scale = ((double) rc.width()/(double) root->docWidth());
    int height = (int) ((double) rc.height() / scale);
#ifndef QT_NO_TRANSFORMATIONS
    p->scale(scale, scale);
#endif
    root->setPageTop(yOff);
    root->setPageBottom(yOff+height);

    root->layer()->paint(p, QRect(0, yOff, root->docWidth(), height));
    if (more)
        *more = yOff + height < root->docHeight();
    p->restore();

    root->setPagedMode(false);
    root->setStaticMode(false);
    m_part->xmlDocImpl()->setPaintDevice( this );
}


void KHTMLView::useSlowRepaints()
{
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

void KHTMLView::addNonPasswordStorableSite(const QString& host)
{
    if (!d->formCompletions) {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "khtml/formcompletions"));
    }

    d->formCompletions->setGroup("NonPasswordStorableSites");
    QStringList sites = d->formCompletions->readListEntry("Sites");
    sites.append(host);
    d->formCompletions->writeEntry("Sites", sites);
    d->formCompletions->sync();
    d->formCompletions->setGroup(QString::null);//reset
}

bool KHTMLView::nonPasswordStorableSite(const QString& host) const
{
    if (!d->formCompletions) {
        d->formCompletions = new KSimpleConfig(locateLocal("data", "khtml/formcompletions"));
    }
    d->formCompletions->setGroup("NonPasswordStorableSites");
    QStringList sites =  d->formCompletions->readListEntry("Sites");
    d->formCompletions->setGroup(QString::null);//reset

    return (sites.find(host) != sites.end());
}

// returns true if event should be swallowed
bool KHTMLView::dispatchMouseEvent(int eventId, DOM::NodeImpl *targetNode,
				   DOM::NodeImpl *targetNodeNonShared, bool cancelable,
				   int detail,QMouseEvent *_mouse, bool setUnder,
				   int mouseEventType)
{
    // if the target node is a text node, dispatch on the parent node - rdar://4196646 (and #76948)
    if (targetNode && targetNode->isTextNode())
        targetNode = targetNode->parentNode();

    if (d->underMouse)
	d->underMouse->deref();
    d->underMouse = targetNode;
    if (d->underMouse)
	d->underMouse->ref();

    if (d->underMouseNonShared)
	d->underMouseNonShared->deref();
    d->underMouseNonShared = targetNodeNonShared;
    if (d->underMouseNonShared)
	d->underMouseNonShared->ref();

    int exceptioncode = 0;
    int pageX = 0;
    int pageY = 0;
    viewportToContents(_mouse->x(), _mouse->y(), pageX, pageY);
    int clientX = pageX - contentsX();
    int clientY = pageY - contentsY();
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
    if (d->accessKeysEnabled && d->accessKeysPreActivate && button!=-1)
    	d->accessKeysPreActivate=false;

    bool ctrlKey = (_mouse->state() & ControlButton);
    bool altKey = (_mouse->state() & AltButton);
    bool shiftKey = (_mouse->state() & ShiftButton);
    bool metaKey = (_mouse->state() & MetaButton);

    // mouseout/mouseover
    if (setUnder && (d->prevMouseX != pageX || d->prevMouseY != pageY)) {

        // ### this code sucks. we should save the oldUnder instead of calculating
        // it again. calculating is expensive! (Dirk)
        NodeImpl *oldUnder = 0;
	if (d->prevMouseX >= 0 && d->prevMouseY >= 0) {
	    NodeImpl::MouseEvent mev( _mouse->stateAfter(), static_cast<NodeImpl::MouseEventType>(mouseEventType));
	    m_part->xmlDocImpl()->prepareMouseEvent( true, d->prevMouseX, d->prevMouseY, &mev );
	    oldUnder = mev.innerNode.handle();

            if (oldUnder && oldUnder->isTextNode())
                oldUnder = oldUnder->parentNode();
	}
// 	qDebug("oldunder=%p (%s), target=%p (%s) x/y=%d/%d", oldUnder, oldUnder ? oldUnder->renderer()->renderName() : 0, targetNode,  targetNode ? targetNode->renderer()->renderName() : 0, _mouse->x(), _mouse->y());
	if (oldUnder != targetNode) {
	    // send mouseout event to the old node
	    if (oldUnder){
		oldUnder->ref();
		MouseEventImpl *me = new MouseEventImpl(EventImpl::MOUSEOUT_EVENT,
							true,true,m_part->xmlDocImpl()->defaultView(),
							0,screenX,screenY,clientX,clientY,pageX, pageY,
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
							0,screenX,screenY,clientX,clientY,pageX, pageY,
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
        bool dblclick = ( eventId == EventImpl::CLICK_EVENT &&
                          _mouse->type() == QEvent::MouseButtonDblClick );
        MouseEventImpl *me = new MouseEventImpl(static_cast<EventImpl::EventId>(eventId),
						true,cancelable,m_part->xmlDocImpl()->defaultView(),
						detail,screenX,screenY,clientX,clientY,pageX, pageY,
						ctrlKey,altKey,shiftKey,metaKey,
						button,0, _mouse, dblclick );
        me->ref();
        targetNode->dispatchEvent(me,exceptioncode,true);
	bool defaultHandled = me->defaultHandled();
        if (defaultHandled || me->defaultPrevented())
            swallowEvent = true;
        me->deref();

        if (eventId == EventImpl::MOUSEDOWN_EVENT) {
            // Focus should be shifted on mouse down, not on a click.  -dwh
            // Blur current focus node when a link/button is clicked; this
            // is expected by some sites that rely on onChange handlers running
            // from form fields before the button click is processed.
            DOM::NodeImpl* nodeImpl = targetNode;
            for ( ; nodeImpl && !nodeImpl->isFocusable(); nodeImpl = nodeImpl->parentNode());
            if (nodeImpl && nodeImpl->isMouseFocusable())
                m_part->xmlDocImpl()->setFocusNode(nodeImpl);
            else if (!nodeImpl || !nodeImpl->focused())
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
    if (d->accessKeysEnabled && d->accessKeysPreActivate) d->accessKeysPreActivate=false;

    if ( ( e->state() & ControlButton) == ControlButton )
    {
        emit zoomView( - e->delta() );
        e->accept();
    }
    else if (d->firstRelayout)
    {
        e->accept();
    }
    else if( (   (e->orientation() == Vertical &&
                   ((d->ignoreWheelEvents && !verticalScrollBar()->isVisible())
                     || e->delta() > 0 && contentsY() <= 0
                     || e->delta() < 0 && contentsY() >= contentsHeight() - visibleHeight()))
              ||
                 (e->orientation() == Horizontal &&
                    ((d->ignoreWheelEvents && !horizontalScrollBar()->isVisible())
                     || e->delta() > 0 && contentsX() <=0
                     || e->delta() < 0 && contentsX() >= contentsWidth() - visibleWidth())))
            && m_part->parentPart())
    {
        if ( m_part->parentPart()->view() )
            m_part->parentPart()->view()->wheelEvent( e );
        e->ignore();
    }
    else if ( (e->orientation() == Vertical && d->vmode == QScrollView::AlwaysOff) ||
              (e->orientation() == Horizontal && d->hmode == QScrollView::AlwaysOff) )
    {
        e->accept();
    }
    else
    {
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
#ifndef KHTML_NO_TYPE_AHEAD_FIND
    m_part->enableFindAheadActions( true );
#endif
    DOM::NodeImpl* fn = m_part->xmlDocImpl() ? m_part->xmlDocImpl()->focusNode() : 0;
    if (fn && fn->renderer() && fn->renderer()->isWidget() &&
        (e->reason() != QFocusEvent::Mouse) &&
        static_cast<khtml::RenderWidget*>(fn->renderer())->widget())
        static_cast<khtml::RenderWidget*>(fn->renderer())->widget()->setFocus();
#ifndef KHTML_NO_CARET
    // Restart blink frequency timer if it has been killed, but only on
    // editable nodes
    if (d->m_caretViewContext &&
        d->m_caretViewContext->freqTimerId == -1 &&
        fn) {
        if (m_part->isCaretMode()
		|| m_part->isEditable()
     		|| (fn && fn->renderer()
			&& fn->renderer()->style()->userInput()
				== UI_ENABLED)) {
            d->m_caretViewContext->freqTimerId = startTimer(500);
	    d->m_caretViewContext->visible = true;
        }/*end if*/
    }/*end if*/
    showCaret();
#endif // KHTML_NO_CARET
    QScrollView::focusInEvent( e );
}

void KHTMLView::focusOutEvent( QFocusEvent *e )
{
    if(m_part) m_part->stopAutoScroll();

#ifndef KHTML_NO_TYPE_AHEAD_FIND
    if(d->typeAheadActivated)
    {
        findTimeout();
    }
    m_part->enableFindAheadActions( false );
#endif // KHTML_NO_TYPE_AHEAD_FIND

#ifndef KHTML_NO_CARET
    if (d->m_caretViewContext) {
        switch (d->m_caretViewContext->displayNonFocused) {
	case KHTMLPart::CaretInvisible:
            hideCaret();
	    break;
	case KHTMLPart::CaretVisible: {
	    killTimer(d->m_caretViewContext->freqTimerId);
	    d->m_caretViewContext->freqTimerId = -1;
            NodeImpl *caretNode = m_part->xmlDocImpl()->focusNode();
	    if (!d->m_caretViewContext->visible && (m_part->isCaretMode()
		|| m_part->isEditable()
     		|| (caretNode && caretNode->renderer()
			&& caretNode->renderer()->style()->userInput()
				== UI_ENABLED))) {
	        d->m_caretViewContext->visible = true;
	        showCaret(true);
	    }/*end if*/
	    break;
	}
	case KHTMLPart::CaretBlink:
	    // simply leave as is
	    break;
	}/*end switch*/
    }/*end if*/
#endif // KHTML_NO_CARET

    if ( d->cursor_icon_widget )
        d->cursor_icon_widget->hide();

    QScrollView::focusOutEvent( e );
}

void KHTMLView::slotScrollBarMoved()
{
    if ( !d->firstRelayout && !d->complete && m_part->xmlDocImpl() &&
          d->layoutSchedulingEnabled) {
        // contents scroll while we are not complete: we need to check our layout *now*
        khtml::RenderCanvas* root = static_cast<khtml::RenderCanvas *>( m_part->xmlDocImpl()->renderer() );
        if (root && root->needsLayout()) {
            unscheduleRelayout();
            layout();
        }
    }
    if (!d->scrollingSelf) {
        d->scrollBarMoved = true;
        d->contentsMoving = true;
        // ensure quick reset of contentsMoving flag
        scheduleRepaint(0, 0, 0, 0);
    }
}

void KHTMLView::timerEvent ( QTimerEvent *e )
{
//    kdDebug() << "timer event " << e->timerId() << endl;
    if ( e->timerId() == d->scrollTimerId ) {
        if( d->scrollSuspended )
            return;
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
        d->dirtyLayout = true;
        layout();
        if (d->firstRelayout) {
            d->firstRelayout = false;
            verticalScrollBar()->setEnabled( true );
            horizontalScrollBar()->setEnabled( true );
        }
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
//	else cout << "" << flush;
	return;
    }
#endif

    d->contentsMoving = false;
    if( m_part->xmlDocImpl() ) {
	DOM::DocumentImpl *document = m_part->xmlDocImpl();
	khtml::RenderCanvas* root = static_cast<khtml::RenderCanvas *>(document->renderer());

	if ( root && root->needsLayout() ) {
	    killTimer(d->repaintTimerId);
	    d->repaintTimerId = 0;
	    scheduleRelayout();
	    return;
	}
    }

    setStaticBackground(d->useSlowRepaints);

//        kdDebug() << "scheduled repaint "<< d->repaintTimerId  << endl;
    killTimer(d->repaintTimerId);
    d->repaintTimerId = 0;

    QRect updateRegion;
    QMemArray<QRect> rects = d->updateRegion.rects();

    d->updateRegion = QRegion();

    if ( rects.size() )
        updateRegion = rects[0];

    for ( unsigned i = 1; i < rects.size(); ++i ) {
        QRect newRegion = updateRegion.unite(rects[i]);
        if (2*newRegion.height() > 3*updateRegion.height() )
        {
            repaintContents( updateRegion );
            updateRegion = rects[i];
        }
        else
            updateRegion = newRegion;
    }

    if ( !updateRegion.isNull() )
        repaintContents( updateRegion );

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
    emit repaintAccessKeys();
    if (d->emitCompletedAfterRepaint) {
        bool full = d->emitCompletedAfterRepaint == KHTMLViewPrivate::CSFull;
        d->emitCompletedAfterRepaint = KHTMLViewPrivate::CSNone;
        if ( full )
            emit m_part->completed();
        else
            emit m_part->completed(true);
    }
}

void KHTMLView::scheduleRelayout(khtml::RenderObject * /*clippedObj*/)
{
    if (!d->layoutSchedulingEnabled || d->layoutTimerId)
        return;

    d->layoutTimerId = startTimer( m_part->xmlDocImpl() && m_part->xmlDocImpl()->parsing()
                             ? 1000 : 0 );
}

void KHTMLView::unscheduleRelayout()
{
    if (!d->layoutTimerId)
        return;

    killTimer(d->layoutTimerId);
    d->layoutTimerId = 0;
}

void KHTMLView::unscheduleRepaint()
{
    if (!d->repaintTimerId)
        return;

    killTimer(d->repaintTimerId);
    d->repaintTimerId = 0;
}

void KHTMLView::scheduleRepaint(int x, int y, int w, int h, bool asap)
{
    bool parsing = !m_part->xmlDocImpl() || m_part->xmlDocImpl()->parsing();

//     kdDebug() << "parsing " << parsing << endl;
//     kdDebug() << "complete " << d->complete << endl;

    int time = parsing ? 300 : (!asap ? ( !d->complete ? 100 : 20 ) : 0);

#ifdef DEBUG_FLICKER
    QPainter p;
    p.begin( viewport() );

    int vx, vy;
    contentsToViewport( x, y, vx, vy );
    p.fillRect( vx, vy, w, h, Qt::red );
    p.end();
#endif

    d->updateRegion = d->updateRegion.unite(QRect(x,y,w,h));

    if (asap && !parsing)
        unscheduleRelayout();

    if ( !d->repaintTimerId )
        d->repaintTimerId = startTimer( time );

//     kdDebug() << "starting timer " << time << endl;
}

void KHTMLView::complete( bool pendingAction )
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
        d->emitCompletedAfterRepaint = pendingAction ?
            KHTMLViewPrivate::CSActionPending : KHTMLViewPrivate::CSFull;
    }

    // is there a repaint pending?
    if (d->repaintTimerId)
    {
//         kdDebug() << "requesting repaint now" << endl;
        // do it now
        killTimer(d->repaintTimerId);
        d->repaintTimerId = startTimer( 20 );
        d->emitCompletedAfterRepaint = pendingAction ?
            KHTMLViewPrivate::CSActionPending : KHTMLViewPrivate::CSFull;
    }

    if (!d->emitCompletedAfterRepaint)
    {
        if (!pendingAction)
	    emit m_part->completed();
        else
            emit m_part->completed(true);
    }

}

void KHTMLView::slotMouseScrollTimer()
{
    scrollBy( d->m_mouseScroll_byX, d->m_mouseScroll_byY );
}

#ifndef KHTML_NO_CARET

// ### the dependencies on static functions are a nightmare. just be
// hacky and include the implementation here. Clean me up, please.

#include "khtml_caret.cpp"

void KHTMLView::initCaret(bool keepSelection)
{
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "begin initCaret" << endl;
#endif
  // save caretMoved state as moveCaretTo changes it
  if (m_part->xmlDocImpl()) {
#if 0
    ElementImpl *listitem = m_part->xmlDocImpl()->getElementById("__test_element__");
    if (listitem) dumpLineBoxes(static_cast<RenderFlow *>(listitem->renderer()));
#endif
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
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "end initCaret" << endl;
#endif
}

bool KHTMLView::caretOverrides() const
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

  DocumentImpl *doc = m_part->xmlDocImpl();
  // ensure that embedded widgets don't lose their focus
  if (!firstAncestor && doc->focusNode() && doc->focusNode()->renderer()
  	&& doc->focusNode()->renderer()->isWidget())
    return;

  // Set focus node on the document
#if DEBUG_CARETMODE > 1
  kdDebug(6200) << k_funcinfo << "firstAncestor " << firstAncestor << ": "
  	<< (firstAncestor ? firstAncestor->nodeName().string() : QString::null) << endl;
#endif
  doc->setFocusNode(firstAncestor);
  emit m_part->nodeActivated(Node(firstAncestor));
}

void KHTMLView::recalcAndStoreCaretPos(CaretBox *hintBox)
{
    if (!m_part || m_part->d->caretNode().isNull()) return;
    d->caretViewContext();
    NodeImpl *caretNode = m_part->d->caretNode().handle();
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "recalcAndStoreCaretPos: caretNode=" << caretNode << (caretNode ? " "+caretNode->nodeName().string() : QString::null) << " r@" << caretNode->renderer() << (caretNode->renderer() && caretNode->renderer()->isText() ? " \"" + QConstString(static_cast<RenderText *>(caretNode->renderer())->str->s, kMin(static_cast<RenderText *>(caretNode->renderer())->str->l, 15u)).string() + "\"" : QString::null) << endl;
#endif
    caretNode->getCaret(m_part->d->caretOffset(), caretOverrides(),
    		d->m_caretViewContext->x, d->m_caretViewContext->y,
		d->m_caretViewContext->width,
		d->m_caretViewContext->height);

    if (hintBox && d->m_caretViewContext->x == -1) {
#if DEBUG_CARETMODE > 1
        kdDebug(6200) << "using hint inline box coordinates" << endl;
#endif
	RenderObject *r = caretNode->renderer();
	const QFontMetrics &fm = r->style()->fontMetrics();
        int absx, absy;
	r->containingBlock()->absolutePosition(absx, absy,
						false);	// ### what about fixed?
	d->m_caretViewContext->x = absx + hintBox->xPos();
	d->m_caretViewContext->y = absy + hintBox->yPos();
// 				+ hintBox->baseline() - fm.ascent();
	d->m_caretViewContext->width = 1;
	// ### firstline not regarded. But I think it can be safely neglected
	// as hint boxes are only used for empty lines.
	d->m_caretViewContext->height = fm.height();
    }/*end if*/

#if DEBUG_CARETMODE > 4
//    kdDebug(6200) << "freqTimerId: "<<d->m_caretViewContext->freqTimerId<<endl;
#endif
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "caret: ofs="<<m_part->d->caretOffset()<<" "
    	<<" x="<<d->m_caretViewContext->x<<" y="<<d->m_caretViewContext->y
	<<" h="<<d->m_caretViewContext->height<<endl;
#endif
}

void KHTMLView::caretOn()
{
    if (d->m_caretViewContext) {
        killTimer(d->m_caretViewContext->freqTimerId);

	if (hasFocus() || d->m_caretViewContext->displayNonFocused
			== KHTMLPart::CaretBlink) {
            d->m_caretViewContext->freqTimerId = startTimer(500);
	} else {
	    d->m_caretViewContext->freqTimerId = -1;
	}/*end if*/

        d->m_caretViewContext->visible = true;
        if ((d->m_caretViewContext->displayed = (hasFocus()
		|| d->m_caretViewContext->displayNonFocused
			!= KHTMLPart::CaretInvisible))) {
	    updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
	}/*end if*/
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

void KHTMLView::showCaret(bool forceRepaint)
{
    if (d->m_caretViewContext) {
        d->m_caretViewContext->displayed = true;
        if (d->m_caretViewContext->visible) {
	    if (!forceRepaint) {
	    	updateContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
			d->m_caretViewContext->height);
            } else {
	    	repaintContents(d->m_caretViewContext->x, d->m_caretViewContext->y,
	    		d->m_caretViewContext->width,
	    		d->m_caretViewContext->height);
	    }/*end if*/
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

int KHTMLView::caretDisplayPolicyNonFocused() const
{
  if (d->m_caretViewContext)
    return d->m_caretViewContext->displayNonFocused;
  else
    return KHTMLPart::CaretInvisible;
}

void KHTMLView::setCaretDisplayPolicyNonFocused(int policy)
{
  d->caretViewContext();
//  int old = d->m_caretViewContext->displayNonFocused;
  d->m_caretViewContext->displayNonFocused = (KHTMLPart::CaretDisplayPolicy)policy;

  // make change immediately take effect if not focused
  if (!hasFocus()) {
    switch (d->m_caretViewContext->displayNonFocused) {
      case KHTMLPart::CaretInvisible:
        hideCaret();
	break;
      case KHTMLPart::CaretBlink:
	if (d->m_caretViewContext->freqTimerId != -1) break;
	d->m_caretViewContext->freqTimerId = startTimer(500);
	// fall through
      case KHTMLPart::CaretVisible:
        d->m_caretViewContext->displayed = true;
        showCaret();
	break;
    }/*end switch*/
  }/*end if*/
}

bool KHTMLView::placeCaret(CaretBox *hintBox)
{
  CaretViewContext *cv = d->caretViewContext();
  caretOff();
  NodeImpl *caretNode = m_part->d->caretNode().handle();
  // ### why is it sometimes null?
  if (!caretNode || !caretNode->renderer()) return false;
  ensureNodeHasFocus(caretNode);
  if (m_part->isCaretMode() || m_part->isEditable()
     || caretNode->renderer()->style()->userInput() == UI_ENABLED) {
    recalcAndStoreCaretPos(hintBox);

    cv->origX = cv->x;

    caretOn();
    return true;
  }/*end if*/
  return false;
}

void KHTMLView::ensureCaretVisible()
{
  CaretViewContext *cv = d->m_caretViewContext;
  if (!cv) return;
  ensureVisible(cv->x, cv->y, cv->width, cv->height);
  d->scrollBarMoved = false;
}

bool KHTMLView::extendSelection(NodeImpl *oldStartSel, long oldStartOfs,
				NodeImpl *oldEndSel, long oldEndOfs)
{
  bool changed = false;
  if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd
      && m_part->d->m_startOffset == m_part->d->m_endOffset) {
    changed = foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
    m_part->d->m_extendAtEnd = true;
  } else do {
    changed = m_part->d->m_selectionStart.handle() != oldStartSel
    		|| m_part->d->m_startOffset != oldStartOfs
		|| m_part->d->m_selectionEnd.handle() != oldEndSel
		|| m_part->d->m_endOffset != oldEndOfs;
    if (!changed) break;

    // determine start position -- caret position is always at end.
    NodeImpl *startNode;
    long startOffset;
    if (m_part->d->m_extendAtEnd) {
      startNode = m_part->d->m_selectionStart.handle();
      startOffset = m_part->d->m_startOffset;
    } else {
      startNode = m_part->d->m_selectionEnd.handle();
      startOffset = m_part->d->m_endOffset;
      m_part->d->m_selectionEnd = m_part->d->m_selectionStart;
      m_part->d->m_endOffset = m_part->d->m_startOffset;
      m_part->d->m_extendAtEnd = true;
    }/*end if*/

    bool swapNeeded = false;
    if (!m_part->d->m_selectionEnd.isNull() && startNode) {
      swapNeeded = RangeImpl::compareBoundaryPoints(startNode, startOffset,
      			m_part->d->m_selectionEnd.handle(),
			m_part->d->m_endOffset) >= 0;
    }/*end if*/

    m_part->d->m_selectionStart = startNode;
    m_part->d->m_startOffset = startOffset;

    if (swapNeeded) {
      m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset, m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset);
    } else {
      m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset, m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset);
    }/*end if*/
  } while(false);/*end if*/
  return changed;
}

void KHTMLView::updateSelection(NodeImpl *oldStartSel, long oldStartOfs,
				NodeImpl *oldEndSel, long oldEndOfs)
{
  if (m_part->d->m_selectionStart == m_part->d->m_selectionEnd
      && m_part->d->m_startOffset == m_part->d->m_endOffset) {
    if (foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs)) {
      m_part->emitSelectionChanged();
    }/*end if*/
    m_part->d->m_extendAtEnd = true;
  } else {
    // check if the extending end has passed the immobile end
    if (!m_part->d->m_selectionEnd.isNull() && !m_part->d->m_selectionEnd.isNull()) {
      bool swapNeeded = RangeImpl::compareBoundaryPoints(
      			m_part->d->m_selectionStart.handle(), m_part->d->m_startOffset,
			m_part->d->m_selectionEnd.handle(), m_part->d->m_endOffset) >= 0;
      if (swapNeeded) {
        DOM::Node tmpNode = m_part->d->m_selectionStart;
        long tmpOffset = m_part->d->m_startOffset;
        m_part->d->m_selectionStart = m_part->d->m_selectionEnd;
        m_part->d->m_startOffset = m_part->d->m_endOffset;
        m_part->d->m_selectionEnd = tmpNode;
        m_part->d->m_endOffset = tmpOffset;
        m_part->d->m_startBeforeEnd = true;
        m_part->d->m_extendAtEnd = !m_part->d->m_extendAtEnd;
      }/*end if*/
    }/*end if*/

    m_part->xmlDocImpl()->setSelection(m_part->d->m_selectionStart.handle(),
		m_part->d->m_startOffset, m_part->d->m_selectionEnd.handle(),
		m_part->d->m_endOffset);
    m_part->emitSelectionChanged();
  }/*end if*/
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
      updateSelection(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
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
  if (!node) return false;
  ElementImpl *baseElem = determineBaseElement(node);
  RenderFlow *base = static_cast<RenderFlow *>(baseElem ? baseElem->renderer() : 0);
  if (!node) return false;

  // need to find out the node's inline box. If there is none, this function
  // will snap to the next node that has one. This is necessary to make the
  // caret visible in any case.
  CaretBoxLineDeleter cblDeleter;
//   RenderBlock *cb;
  long r_ofs;
  CaretBoxIterator cbit;
  CaretBoxLine *cbl = findCaretBoxLine(node, offset, &cblDeleter, base, r_ofs, cbit);
  if(!cbl) {
      kdWarning() << "KHTMLView::moveCaretTo - findCaretBoxLine() returns NULL" << endl;
      return false;
  }

#if DEBUG_CARETMODE > 3
  if (cbl) kdDebug(6200) << cbl->information() << endl;
#endif
  CaretBox *box = *cbit;
  if (cbit != cbl->end() && box->object() != node->renderer()) {
    if (box->object()->element()) {
      mapRenderPosToDOMPos(box->object(), r_ofs, box->isOutside(),
      			box->isOutsideEnd(), node, offset);
      //if (!outside) offset = node->minOffset();
#if DEBUG_CARETMODE > 1
      kdDebug(6200) << "set new node " << node->nodeName().string() << "@" << node << endl;
#endif
    } else {	// box has no associated element -> do not use
      // this case should actually never happen.
      box = 0;
      kdError(6200) << "Box contains no node! Crash imminent" << endl;
    }/*end if*/
  }

  NodeImpl *oldStartSel = m_part->d->m_selectionStart.handle();
  long oldStartOfs = m_part->d->m_startOffset;
  NodeImpl *oldEndSel = m_part->d->m_selectionEnd.handle();
  long oldEndOfs = m_part->d->m_endOffset;

  // test for position change
  bool posChanged = m_part->d->caretNode().handle() != node
  		|| m_part->d->caretOffset() != offset;
  bool selChanged = false;

  m_part->d->caretNode() = node;
  m_part->d->caretOffset() = offset;
  if (clearSel || !oldStartSel || !oldEndSel) {
    selChanged = foldSelectionToCaret(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
  } else {
    //kdDebug(6200) << "moveToCaret: extendSelection: m_extendAtEnd " << m_part->d->m_extendAtEnd << endl;
    //kdDebug(6200) << "selection: start(" << m_part->d->m_selectionStart.handle() << "," << m_part->d->m_startOffset << "), end(" << m_part->d->m_selectionEnd.handle() << "," << m_part->d->m_endOffset << "), caret(" << m_part->d->caretNode().handle() << "," << m_part->d->caretOffset() << ")" << endl;
    selChanged = extendSelection(oldStartSel, oldStartOfs, oldEndSel, oldEndOfs);
    //kdDebug(6200) << "after extendSelection: m_extendAtEnd " << m_part->d->m_extendAtEnd << endl;
    //kdDebug(6200) << "selection: start(" << m_part->d->m_selectionStart.handle() << "," << m_part->d->m_startOffset << "), end(" << m_part->d->m_selectionEnd.handle() << "," << m_part->d->m_endOffset << "), caret(" << m_part->d->caretNode().handle() << "," << m_part->d->caretOffset() << ")" << endl;
  }/*end if*/

  d->caretViewContext()->caretMoved = true;

  bool visible_caret = placeCaret(box);

  // FIXME: if the old position was !visible_caret, and the new position is
  // also, then two caretPositionChanged signals with a null Node are
  // emitted in series.
  if (posChanged) {
    m_part->emitCaretPositionChanged(visible_caret ? node : 0, offset);
  }/*end if*/

  return selChanged;
}

void KHTMLView::moveCaretByLine(bool next, int count)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  CaretViewContext *cv = d->caretViewContext();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  // move count lines vertically
  while (count > 0 && it != ld.end() && it != ld.preBegin()) {
    count--;
    if (next) ++it; else --it;
  }/*wend*/

  // Nothing? Then leave everything as is.
  if (it == ld.end() || it == ld.preBegin()) return;

  int x, absx, absy;
  CaretBox *caretBox = nearestCaretBox(it, d->m_caretViewContext, x, absx, absy);

  placeCaretOnLine(caretBox, x, absx, absy);
}

void KHTMLView::placeCaretOnLine(CaretBox *caretBox, int x, int absx, int absy)
{
  // paranoia sanity check
  if (!caretBox) return;

  RenderObject *caretRender = caretBox->object();

#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "got valid caretBox " << caretBox << endl;
  kdDebug(6200) << "xPos: " << caretBox->xPos() << " yPos: " << caretBox->yPos()
  		<< " width: " << caretBox->width() << " height: " << caretBox->height() << endl;
  InlineTextBox *tb = static_cast<InlineTextBox *>(caretBox->inlineBox());
  if (caretBox->isInlineTextBox()) { kdDebug(6200) << "contains \"" << QString(static_cast<RenderText *>(tb->object())->str->s + tb->m_start, tb->m_len) << "\"" << endl;}
#endif
  // inquire height of caret
  int caretHeight = caretBox->height();
  bool isText = caretBox->isInlineTextBox();
  int yOfs = 0;		// y-offset for text nodes
  if (isText) {
    // text boxes need extrawurst
    RenderText *t = static_cast<RenderText *>(caretRender);
    const QFontMetrics &fm = t->metrics(caretBox->inlineBox()->m_firstLine);
    caretHeight = fm.height();
    yOfs = caretBox->inlineBox()->baseline() - fm.ascent();
  }/*end if*/

  caretOff();

  // set new caret node
  NodeImpl *caretNode;
  long &offset = m_part->d->caretOffset();
  mapRenderPosToDOMPos(caretRender, offset, caretBox->isOutside(),
  		caretBox->isOutsideEnd(), caretNode, offset);

  // set all variables not needing special treatment
  d->m_caretViewContext->y = caretBox->yPos() + yOfs;
  d->m_caretViewContext->height = caretHeight;
  d->m_caretViewContext->width = 1; // FIXME: regard override

  int xPos = caretBox->xPos();
  int caretBoxWidth = caretBox->width();
  d->m_caretViewContext->x = xPos;

  if (!caretBox->isOutside()) {
    // before or at beginning of inline box -> place at beginning
    long r_ofs = 0;
    if (x <= xPos) {
      r_ofs = caretBox->minOffset();
  // somewhere within this block
    } else if (x > xPos && x <= xPos + caretBoxWidth) {
      if (isText) { // find out where exactly
        r_ofs = static_cast<InlineTextBox *>(caretBox->inlineBox())
      		->offsetForPoint(x, d->m_caretViewContext->x);
#if DEBUG_CARETMODE > 2
        kdDebug(6200) << "deviation from origX " << d->m_caretViewContext->x - x << endl;
#endif
#if 0
      } else {	// snap to nearest end
        if (xPos + caretBoxWidth - x < x - xPos) {
          d->m_caretViewContext->x = xPos + caretBoxWidth;
          r_ofs = caretNode ? caretNode->maxOffset() : 1;
        } else {
          d->m_caretViewContext->x = xPos;
          r_ofs = caretNode ? caretNode->minOffset() : 0;
        }/*end if*/
#endif
      }/*end if*/
    } else {		// after the inline box -> place at end
      d->m_caretViewContext->x = xPos + caretBoxWidth;
      r_ofs = caretBox->maxOffset();
    }/*end if*/
    offset = r_ofs;
  }/*end if*/
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "new offset: " << offset << endl;
#endif

  m_part->d->caretNode() = caretNode;
  m_part->d->caretOffset() = offset;

  d->m_caretViewContext->x += absx;
  d->m_caretViewContext->y += absy;

#if DEBUG_CARETMODE > 1
	kdDebug(6200) << "new caret position: x " << d->m_caretViewContext->x << " y " << d->m_caretViewContext->y << " w " << d->m_caretViewContext->width << " h " << d->m_caretViewContext->height << " absx " << absx << " absy " << absy << endl;
#endif

  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->scrollBarMoved = false;

  ensureNodeHasFocus(caretNode);
  caretOn();
}

void KHTMLView::moveCaretToLineBoundary(bool end)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  EditableLineIterator it = ld.current();
  if (it == ld.end()) return;	// should not happen, but who knows

  EditableCaretBoxIterator fbit(it, end);
  Q_ASSERT(fbit != (*it)->end() && fbit != (*it)->preBegin());
  CaretBox *b = *fbit;

  RenderObject *cb = b->containingBlock();
  int absx, absy;

  if (cb) cb->absolutePosition(absx,absy);
  else absx = absy = 0;

  int x = b->xPos() + (end && !b->isOutside() ? b->width() : 0);
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

void KHTMLView::moveCaretToDocumentBoundary(bool end)
{
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, IndicatedFlows, baseElem);

  EditableLineIterator it(end ? ld.preEnd() : ld.begin(), end);
  if (it == ld.end() || it == ld.preBegin()) return;	// should not happen, but who knows

  EditableCaretBoxIterator fbit = it;
  Q_ASSERT(fbit != (*it)->end() && fbit != (*it)->preBegin());
  CaretBox *b = *fbit;

  RenderObject *cb = (*it)->containingBlock();
  int absx, absy;

  if (cb) cb->absolutePosition(absx, absy);
  else absx = absy = 0;

  int x = b->xPos()/* + (end ? b->width() : 0) reactivate for rtl*/;
  d->m_caretViewContext->origX = absx + x;
  placeCaretOnLine(b, x, absx, absy);
}

void KHTMLView::moveCaretBy(bool next, CaretMovement cmv, int count)
{
  if (!m_part) return;
  Node &caretNodeRef = m_part->d->caretNode();
  if (caretNodeRef.isNull()) return;

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long &offset = m_part->d->caretOffset();

  ElementImpl *baseElem = determineBaseElement(caretNode);
  CaretAdvancePolicy advpol = cmv != CaretByWord ? IndicatedFlows : LeafsOnly;
  LinearDocument ld(m_part, caretNode, offset, advpol, baseElem);

  EditableCharacterIterator it(&ld);
  while (!it.isEnd() && count > 0) {
    count--;
    if (cmv == CaretByCharacter) {
      if (next) ++it;
      else --it;
    } else if (cmv == CaretByWord) {
      if (next) moveItToNextWord(it);
      else moveItToPrevWord(it);
    }/*end if*/
//kdDebug(6200) << "movecaret" << endl;
  }/*wend*/
  CaretBox *hintBox = 0;	// make gcc uninit warning disappear
  if (!it.isEnd()) {
    NodeImpl *node = caretNodeRef.handle();
    hintBox = it.caretBox();
//kdDebug(6200) << "hintBox = " << hintBox << endl;
//kdDebug(6200) << " outside " << hintBox->isOutside() << " outsideEnd " << hintBox->isOutsideEnd() << " r " << it.renderer() << " ofs " << it.offset() << " cb " << hintBox->containingBlock() << endl;
    mapRenderPosToDOMPos(it.renderer(), it.offset(), hintBox->isOutside(),
    		hintBox->isOutsideEnd(), node, offset);
//kdDebug(6200) << "mapRTD" << endl;
    caretNodeRef = node;
#if DEBUG_CARETMODE > 2
    kdDebug(6200) << "set by valid node " << node << " " << (node?node->nodeName().string():QString::null) << " offset: " << offset << endl;
#endif
  } else {
    offset = next ? caretNode->maxOffset() : caretNode->minOffset();
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "set by INvalid node. offset: " << offset << endl;
#endif
  }/*end if*/
  placeCaretOnChar(hintBox);
}

void KHTMLView::placeCaretOnChar(CaretBox *hintBox)
{
  caretOff();
  recalcAndStoreCaretPos(hintBox);
  ensureVisible(d->m_caretViewContext->x, d->m_caretViewContext->y,
  	d->m_caretViewContext->width, d->m_caretViewContext->height);
  d->m_caretViewContext->origX = d->m_caretViewContext->x;
  d->scrollBarMoved = false;
#if DEBUG_CARETMODE > 3
  //if (caretNode->isTextNode())  kdDebug(6200) << "text[0] = " << (int)*((TextImpl *)caretNode)->data().unicode() << " text :\"" << ((TextImpl *)caretNode)->data().string() << "\"" << endl;
#endif
  ensureNodeHasFocus(m_part->d->caretNode().handle());
  caretOn();
}

void KHTMLView::moveCaretByPage(bool next)
{
  Node &caretNodeRef = m_part->d->caretNode();

  NodeImpl *caretNode = caretNodeRef.handle();
//  kdDebug(6200) << ": caretNode=" << caretNode << endl;
  long offset = m_part->d->caretOffset();

  int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
  // Minimum distance the caret must be moved
  int mindist = clipper()->height() - offs;

  CaretViewContext *cv = d->caretViewContext();
//  int y = cv->y;		// we always measure the top border

  ElementImpl *baseElem = determineBaseElement(caretNode);
  LinearDocument ld(m_part, caretNode, offset, LeafsOnly, baseElem);

  ErgonomicEditableLineIterator it(ld.current(), cv->origX);

  moveIteratorByPage(ld, it, mindist, next);

  int x, absx, absy;
  CaretBox *caretBox = nearestCaretBox(it, d->m_caretViewContext, x, absx, absy);

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

#undef DEBUG_CARETMODE
