 /**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
 * Copyright (C) 2004 Germain Garand (germain@ebooksfrance.org)
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
 *
 */
#include "render_replaced.h"
#include "render_canvas.h"
#include "render_line.h"

#include "render_arena.h"

#include <assert.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <kglobalsettings.h>
#include <qobject.h>
#include <q3valuevector.h>
#include <QVector>
#include <qpaintengine.h>

#include "khtml_ext.h"
#include "khtmlview.h"
#include "xml/dom2_eventsimpl.h"
#include "khtml_part.h"
#include "xml/dom_docimpl.h"
#include <kdebug.h>

bool khtml::allowWidgetPaintEvents = false;

using namespace khtml;
using namespace DOM;


RenderReplaced::RenderReplaced(DOM::NodeImpl* node)
    : RenderBox(node)
{
    // init RenderObject attributes
    setReplaced(true);

    m_intrinsicWidth = 200;
    m_intrinsicHeight = 150;
}

void RenderReplaced::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    m_width = calcReplacedWidth();
    m_width = calcBoxWidth( m_width );

    if ( style()->width().isPercent() || style()->height().isPercent() || 
		    style()->maxWidth().isPercent() || style()->maxHeight().isPercent() ||
		    style()->minWidth().isPercent() || style()->minHeight().isPercent() ) {
        m_minWidth = 0;
        m_maxWidth = m_width;
    }
    else
        m_minWidth = m_maxWidth = m_width;

    setMinMaxKnown();
}

void RenderReplaced::position(InlineBox* box, int /*from*/, int /*len*/, bool /*reverse*/)
{
    setPos( box->xPos(), box->yPos() );
}

// -----------------------------------------------------------------------------

RenderWidget::RenderWidget(DOM::NodeImpl* node)
        : RenderReplaced(node)
{
    m_widget = 0;
    // a widget doesn't support being anonymous
    assert(!isAnonymous());
    m_view = node->getDocument()->view();
    m_resizePending = false;
    m_discardResizes = false;

    // this is no real reference counting, its just there
    // to make sure that we're not deleted while we're recursed
    // in an eventFilter of the widget
    ref();
}

void RenderWidget::detach()
{
    remove();
    deleteInlineBoxes();

    if ( m_widget ) {
        if ( m_view ) {
            m_view->setWidgetVisible(this, false);
            m_view->removeChild( m_widget );
        }

        m_widget->removeEventFilter( this );
        m_widget->setMouseTracking( false );
    }

    deref();
}

RenderWidget::~RenderWidget()
{
    KHTMLAssert( refCount() <= 0 );

    if(m_widget) {
        m_widget->hide();
        m_widget->deleteLater();
    }
}

class QWidgetResizeEvent : public QEvent
{
public:
    enum { Type = QEvent::User + 0xbee };
    QWidgetResizeEvent( int _w,  int _h ) :
        QEvent( ( QEvent::Type ) Type ),  w( _w ), h( _h ) {}
    int w;
    int h;
};

void  RenderWidget::resizeWidget( int w, int h )
{
    // ugly hack to limit the maximum size of the widget ( as X11 has problems if
    // its bigger )
    h = qMin( h, 3072 );
    w = qMin( w, 2000 );

    if (m_widget->width() != w || m_widget->height() != h) {
        m_resizePending = !strcmp(m_widget->name(), "__khtml");
        ref();
        element()->ref();
        QApplication::postEvent( this, new QWidgetResizeEvent( w, h ) );
        element()->deref();
        deref();
    }
}

void RenderWidget::cancelPendingResize()
{
    if (!m_widget)
        return;
    m_discardResizes = true;
    QApplication::sendPostedEvents(this, QWidgetResizeEvent::Type);
    m_discardResizes = false;
}

bool RenderWidget::event( QEvent *e )
{
    if ( m_widget && (e->type() == (QEvent::Type)QWidgetResizeEvent::Type) ) {
        m_resizePending = false;
        if (m_discardResizes)
            return true;
        QWidgetResizeEvent *re = static_cast<QWidgetResizeEvent *>(e);
        m_widget->resize( re->w,  re->h );
        repaint();
    }
    // eat all events - except if this is a frame (in which case KHTMLView handles it all)
    if ( qobject_cast<KHTMLView*>( m_widget ) )
        return QObject::event( e );
    return true;
}

void RenderWidget::flushWidgetResizes() //static
{
    QApplication::sendPostedEvents( 0, QWidgetResizeEvent::Type );
}

void RenderWidget::setQWidget(QWidget *widget)
{
    if (widget != m_widget)
    {
        if (m_widget) {
            m_widget->removeEventFilter(this);
            disconnect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            delete m_widget;
            m_widget = 0;
        }
        m_widget = widget;
        if (m_widget) {
            connect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            m_widget->installEventFilter(this);

            if ( !strcmp(m_widget->name(), "__khtml") && !qobject_cast<Q3Frame*>(m_widget))
                m_widget->setBackgroundMode( Qt::NoBackground );

            if (m_widget->focusPolicy() > Qt::StrongFocus)
                m_widget->setFocusPolicy(Qt::StrongFocus);
            // if we've already received a layout, apply the calculated space to the
            // widget immediately, but we have to have really been full constructed (with a non-null
            // style pointer).
            if (!needsLayout() && style()) {
                resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                              m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
            }
            else
                setPos(xPos(), -500000);
        }
        m_view->setWidgetVisible(this, false);
        m_view->addChild( m_widget, 0, -500000);
        if ( m_widget ) m_widget->hide();
        m_resizePending = false;
    }
}

void RenderWidget::layout( )
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );
    if ( m_widget )
        resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );

    setNeedsLayout(false);
}

void RenderWidget::updateFromElement()
{
    if (m_widget) {
        // Color:
        QColor color = style()->color();
        QColor backgroundColor = style()->backgroundColor();

        if ( color.isValid() || backgroundColor.isValid() ) {
            QPalette pal(QApplication::palette(m_widget));

            int contrast_ = KGlobalSettings::contrast();
            int highlightVal = 100 + (2*contrast_+4)*16/10;
            int lowlightVal = 100 + (2*contrast_+4)*10;

            if (backgroundColor.isValid()) {
                if (strcmp(widget()->name(), "__khtml"))
                    widget()->setEraseColor(backgroundColor );
                for ( int i = 0; i < QPalette::NColorGroups; ++i ) {
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Background, backgroundColor );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Light, backgroundColor.light(highlightVal) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Dark, backgroundColor.dark(lowlightVal) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Mid, backgroundColor.dark(120) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Midlight, backgroundColor.light(110) );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Button, backgroundColor );
                    pal.setColor( (QPalette::ColorGroup)i, QColorGroup::Base, backgroundColor );
            }
            }
            if ( color.isValid() ) {
                struct ColorSet {
                    QPalette::ColorGroup cg;
                    QColorGroup::ColorRole cr;
                };
                const struct ColorSet toSet [] = {
                    { QPalette::Active, QColorGroup::Foreground },
                    { QPalette::Active, QColorGroup::ButtonText },
                    { QPalette::Active, QColorGroup::Text },
                    { QPalette::Inactive, QColorGroup::Foreground },
                    { QPalette::Inactive, QColorGroup::ButtonText },
                    { QPalette::Inactive, QColorGroup::Text },
                    { QPalette::Disabled,QColorGroup::ButtonText },
                    { QPalette::NColorGroups, QColorGroup::NColorRoles },
                };
                const ColorSet *set = toSet;
                while( set->cg != QPalette::NColorGroups ) {
                    pal.setColor( set->cg, set->cr, color );
                    ++set;
                }

                QColor disfg = color;
                int h, s, v;
                disfg.hsv( &h, &s, &v );
                if (v > 128)
                    // dark bg, light fg - need a darker disabled fg
                    disfg = disfg.dark(lowlightVal);
                else if (disfg != Qt::black)
                    // light bg, dark fg - need a lighter disabled fg - but only if !black
                    disfg = disfg.light(highlightVal);
                else
                    // black fg - use darkgray disabled fg
                    disfg = Qt::darkGray;
                pal.setColor(QPalette::Disabled,QColorGroup::Foreground,disfg);
            }

            m_widget->setPalette(pal);
        }
        else
            m_widget->unsetPalette();
        // Border:
        if (shouldPaintBackgroundOrBorder())
        {
            if (QFrame* frame = qobject_cast<QFrame*>(m_widget))
                frame->setFrameShape(QFrame::NoFrame);
            else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_widget))
                lineEdit->setFrame(false);
        }

    }

    RenderReplaced::updateFromElement();
}

void RenderWidget::slotWidgetDestructed()
{
    if (m_view)
       m_view->setWidgetVisible(this, false);
    m_widget = 0;
}

void RenderWidget::setStyle(RenderStyle *_style)
{
    RenderReplaced::setStyle(_style);
    if(m_widget)
    {
        m_widget->setFont(style()->font());
        if (style()->visibility() != VISIBLE) {
            if (m_view)
                m_view->setWidgetVisible(this, false);
            m_widget->hide();
        }
    }

    // Don't paint borders if the border-style is native
    // or borders are not supported on this widget
    if (!canHaveBorder() ||
        (style()->borderLeftStyle()   == BNATIVE &&
         style()->borderRightStyle()  == BNATIVE &&
         style()->borderTopStyle()    == BNATIVE &&
         style()->borderBottomStyle() == BNATIVE))
    {
        setShouldPaintBackgroundOrBorder(false);
    }
}

void RenderWidget::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    if (shouldPaintBackgroundOrBorder() && paintInfo.phase != PaintActionOutline)
        paintBoxDecorations(paintInfo, _tx, _ty);

    if (!m_widget || !m_view || paintInfo.phase != PaintActionForeground)
        return;

    // not visible or not even once layouted
    if (style()->visibility() != VISIBLE || m_y <= -500000 || m_resizePending )
        return;

    if ( (_ty > paintInfo.r.bottom()) || (_ty + m_height <= paintInfo.r.top()) ||
         (_tx + m_width <= paintInfo.r.left()) || (_tx > paintInfo.r.right()) )
        return;

    int xPos = _tx+borderLeft()+paddingLeft();
    int yPos = _ty+borderTop()+paddingTop();

    bool khtmlw = !strcmp(m_widget->name(), "__khtml");
    int childw = m_widget->width();
    int childh = m_widget->height();
    if ( (childw == 2000 || childh == 3072) && m_widget->inherits( "KHTMLView" ) ) {
        KHTMLView *vw = static_cast<KHTMLView *>(m_widget);
        int cy = m_view->contentsY();
        int ch = m_view->visibleHeight();


        int childx = m_view->childX( m_widget );
        int childy = m_view->childY( m_widget );

        int xNew = xPos;
        int yNew = childy;

        //         qDebug("cy=%d, ch=%d, childy=%d, childh=%d", cy, ch, childy, childh );
        if ( childh == 3072 ) {
            if ( cy + ch > childy + childh ) {
                yNew = cy + ( ch - childh )/2;
            } else if ( cy < childy ) {
                yNew = cy + ( ch - childh )/2;
            }
//             qDebug("calculated yNew=%d", yNew);
        }
        yNew = qMin( yNew, yPos + m_height - childh );
        yNew = qMax( yNew, yPos );
        if ( yNew != childy || xNew != childx ) {
            if ( vw->contentsHeight() < yNew - yPos + childh )
                vw->resizeContents( vw->contentsWidth(), yNew - yPos + childh );
            vw->setContentsPos( xNew - xPos, yNew - yPos );
        }
        xPos = xNew;
        yPos = yNew;
    }
    m_view->setWidgetVisible(this, true);
    m_view->addChild(m_widget, xPos, yPos );
    m_widget->show();
    if (khtmlw)
        paintWidget(paintInfo, m_widget, xPos, yPos);
}

// The PaintBuffer class provides a shared buffer for widget painting.
//
// It will grow to encompass the biggest widget encountered, in order to avoid
// constantly resizing.
// When it grows over maxPixelBuffering, it periodically checks if such a size
// is still needed.  If not, it shrinks down to the biggest size < maxPixelBuffering
// that was requested during the overflow lapse.

class PaintBuffer: public QObject
{
public:
    static const int maxPixelBuffering = 320*200;
    static const int leaseTime = 20*1000;

    static QPixmap *grab( QSize s = QSize() ) {
        if (!m_inst)
            m_inst = new PaintBuffer;
        return m_inst->getBuf( s );
    }
    static void release() { m_inst->m_grabbed = false; }
protected:
    PaintBuffer(): m_overflow(false), m_grabbed(false),
                   m_timer(0), m_resetWidth(0), m_resetHeight(0) {};
    void timerEvent(QTimerEvent* e) {
        assert( m_timer == e->timerId() );
        if (m_grabbed)
            return;
        m_buf.resize(m_resetWidth, m_resetHeight);
        m_resetWidth = m_resetHeight = 0;
        killTimer( m_timer );
        m_timer = 0;
    }

    QPixmap *getBuf( QSize s ) {
        assert( !m_grabbed );
        if (s.isEmpty())
            return 0;

        m_grabbed = true;
        bool cur_overflow = false;

        int nw = qMax(m_buf.width(), s.width());
        int nh = qMax(m_buf.height(), s.height());

        if (!m_overflow && (nw*nh > maxPixelBuffering))
            cur_overflow = true;

        if (nw != m_buf.width() || nh != m_buf.height())
            m_buf.resize(nw, nh);

        if (cur_overflow) {
            m_overflow = true;
            m_timer = startTimer( leaseTime );
        } else if (m_overflow) {
            if( s.width()*s.height() > maxPixelBuffering ) {
                killTimer( m_timer );
                m_timer = startTimer( leaseTime );
            } else {
                if (s.width() > m_resetWidth)
                    m_resetWidth = s.width();
                if (s.height() > m_resetHeight)
                    m_resetHeight = s.height();
            }
        }
        return &m_buf;
    }
private:
    static PaintBuffer* m_inst;
    QPixmap m_buf;
    bool m_overflow;
    bool m_grabbed;
    int m_timer;
    int m_resetWidth;
    int m_resetHeight;
};

PaintBuffer *PaintBuffer::m_inst = 0;

static void copyWidget(const QRect& r, QPainter *p, QWidget *widget, int tx, int ty)
{
    if (r.isNull() || r.isEmpty() )
        return;
    QRegion blit(r);
    QVector<QWidget*> cw;
    QVector<QRect> cr;

    if (!widget->children().isEmpty()) {
        // build region
        QList<QObject*> list = widget->children();
	foreach ( QObject* it, list ) {
	    QWidget* w = qobject_cast<QWidget* >(it);
	    if ( w && !w->isTopLevel() && !w->isHidden()) {
	        QRect r2 = w->geometry();
		blit.subtract( r2 );
		r2 = r2.intersect( r );
		r2.moveBy(-w->x(), -w->y());
		cr.append(r2);
		cw.append(w);
	    }
        }
    }
    QVector<QRect> br = blit.rects();

    const int cnt = br.size();
    const bool external = p->device()->paintEngine()->type() >= QPaintEngine::PostScript;
    QPixmap* const pm = PaintBuffer::grab( widget->size() );
    if (!pm)
    {
        kdWarning(6040) << "Rendering widget [ " << widget->className() << " ] failed due to invalid size." << endl;
        return;
    }

    // fill background
    if ( external ) {
	// even hackier!
        QPainter pt( pm );
        const QColor c = widget->colorGroup().base();
        for (int i = 0; i < cnt; ++i)
            pt.fillRect( br[i], c );
    } else {
        QRect dr;
        for (int i = 0; i < cnt; ++i ) {
            dr = br[i];
	    dr.moveBy( tx, ty );
	    dr = p->xForm( dr );
	    bitBlt(pm, br[i].topLeft(), p->device(), dr);
        }
    }

    // send paint event
    QPainter::setRedirected(widget, pm);
    QPaintEvent e( r );
    QApplication::sendEvent(widget, &e);
    QPainter::restoreRedirected(widget);

    // transfer result
    for ( int i = 0; i < cnt; ++i )
        p->drawPixmap(QPoint(tx+br[i].x(), ty+br[i].y()), *pm, br[i]);

    // cleanup and recurse
    PaintBuffer::release();
    QVector<QWidget*>::iterator cwit = cw.begin();
    QVector<QWidget*>::iterator cwitEnd = cw.end();
    QVector<QRect>::const_iterator crit = cr.begin();
    for (; cwit != cwitEnd; ++cwit, ++crit)
        copyWidget(*crit, p, *cwit, tx+(*cwit)->x(), ty+(*cwit)->y());
}

void RenderWidget::paintWidget(PaintInfo& pI, QWidget *widget, int tx, int ty)
{
    QPainter* const p = pI.p;
    allowWidgetPaintEvents = true;

    QRect rr = pI.r;
    rr.moveBy(-tx, -ty);
    const QRect r = widget->rect().intersect( rr );
    copyWidget(r, p, widget, tx, ty);

    allowWidgetPaintEvents = false;
}

bool RenderWidget::eventFilter(QObject* /*o*/, QEvent* e)
{
    // no special event processing if this is a frame (in which case KHTMLView handles it all)
    if ( qobject_cast<KHTMLView*>( m_widget ) )
        return false;
    if ( !element() ) return true;

    ref();
    element()->ref();

    bool filtered = false;

    //kdDebug() << "RenderWidget::eventFilter type=" << e->type() << endl;
    switch(e->type()) {
    case QEvent::FocusOut:
        // Don't count popup as a valid reason for losing the focus
        // (example: opening the options of a select combobox shouldn't emit onblur)
        if ( static_cast<QFocusEvent*>(e)->reason() != Qt::PopupFocusReason )
            handleFocusOut();
        break;
    case QEvent::FocusIn:
        //kdDebug(6000) << "RenderWidget::eventFilter captures FocusIn" << endl;
        element()->getDocument()->setFocusNode(element());
//         if ( isEditable() ) {
//             KHTMLPartBrowserExtension *ext = static_cast<KHTMLPartBrowserExtension *>( element()->view->part()->browserExtension() );
//             if ( ext )  ext->editableWidgetFocused( m_widget );
//         }
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    // TODO this seems wrong - Qt events are not correctly translated to DOM ones,
    // like in KHTMLView::dispatchKeyEvent()
        if (element()->dispatchKeyEvent(static_cast<QKeyEvent*>(e),false))
            filtered = true;
        break;

    case QEvent::Wheel:
        if (widget()->parentWidget() == view()->viewport()) {
            // don't allow the widget to react to wheel event unless its
            // currently focused. this avoids accidentally changing a select box
            // or something while wheeling a webpage.
            if (qApp->focusWidget() != widget() &&
                widget()->focusPolicy() <= Qt::StrongFocus)  {
                static_cast<QWheelEvent*>(e)->ignore();
                QApplication::sendEvent(view(), e);
                filtered = true;
            }
        }
        break;
    default:
        break;
    };

    element()->deref();

    // stop processing if the widget gets deleted, but continue in all other cases
    if (hasOneRef())
        filtered = true;
    deref();

    return filtered;
}

void RenderWidget::EventPropagator::sendEvent(QEvent *e) {
    switch(e->type()) {
    case QEvent::MouseButtonPress:
        mousePressEvent(static_cast<QMouseEvent *>(e));
        break;
    case QEvent::MouseButtonRelease:
        mouseReleaseEvent(static_cast<QMouseEvent *>(e));
        break;
    case QEvent::MouseButtonDblClick:
        mouseDoubleClickEvent(static_cast<QMouseEvent *>(e));
        break;
    case QEvent::MouseMove:
        mouseMoveEvent(static_cast<QMouseEvent *>(e));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(e));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(e));
        break;
    default:
        break;
    }
}

bool RenderWidget::handleEvent(const DOM::EventImpl& ev)
{
    bool ret = false;
    switch(ev.id()) {
    case EventImpl::MOUSEDOWN_EVENT:
    case EventImpl::MOUSEUP_EVENT:
    case EventImpl::MOUSEMOVE_EVENT: {
        const MouseEventImpl &me = static_cast<const MouseEventImpl &>(ev);
        QMouseEvent* const qme = me.qEvent();

        int absx = 0;
        int absy = 0;

        absolutePosition(absx, absy);

        const QPoint p(me.clientX() - absx + m_view->contentsX(),
                 me.clientY() - absy + m_view->contentsY());
        QMouseEvent::Type type;
        int button = 0;
        int state = 0;

        if (qme) {
            button = qme->button();
            state = qme->state();
            type = qme->type();
        } else {
            switch(me.id())  {
            case EventImpl::MOUSEDOWN_EVENT:
                type = QMouseEvent::MouseButtonPress;
                break;
            case EventImpl::MOUSEUP_EVENT:
                type = QMouseEvent::MouseButtonRelease;
                break;
            case EventImpl::MOUSEMOVE_EVENT:
            default:
                type = QMouseEvent::MouseMove;
                break;
            }
            switch (me.button()) {
            case 0:
                button = Qt::LeftButton;
                break;
            case 1:
                button = Qt::MidButton;
                break;
            case 2:
                button = Qt::RightButton;
                break;
            default:
                break;
            }
            if (me.ctrlKey())
                state |= Qt::ControlModifier;
            if (me.altKey())
                state |= Qt::AltModifier;
            if (me.shiftKey())
                state |= Qt::ShiftModifier;
            if (me.metaKey())
                state |= Qt::MetaModifier;
        }

//     kdDebug(6000) << "sending event to widget "
//                   << " pos=" << p << " type=" << type
//                   << " button=" << button << " state=" << state << endl;
        QMouseEvent e(type, p, button, state);
        static_cast<EventPropagator *>(m_widget)->sendEvent(&e);
        ret = e.isAccepted();
        break;
    }
    case EventImpl::KEYDOWN_EVENT:
        // do nothing; see the mapping table below
        break;
    case EventImpl::KEYUP_EVENT: {
        QKeyEvent* const ke = static_cast<const TextEventImpl &>(ev).qKeyEvent();
        static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        ret = ke->isAccepted();
        break;
    }
    case EventImpl::KEYPRESS_EVENT: {

        // See KHTMLView::dispatchKeyEvent: autorepeat is just keypress in the DOM
        // but it's keyrelease+keypress in Qt. So here we do the inverse mapping as
        // the one done in KHTMLView: generate two events for one DOM auto-repeat keypress.
        // Similarly, DOM keypress events with non-autorepeat Qt event do nothing here,
        // because the matching Qt keypress event was already sent from DOM keydown event.

        // Reverse drawing as the one in KHTMLView:
        //  DOM:   Down      Press   |       Press                             |     Up
        //  Qt:    (nothing) Press   | Release(autorepeat) + Press(autorepeat) |   Release
        //
        // Qt::KeyPress is sent for DOM keypress and not DOM keydown to allow
        // sites to block a key with onkeypress, #99749

        QKeyEvent* const ke = static_cast<const TextEventImpl &>(ev).qKeyEvent();
        if (ke->isAutoRepeat()) {
            QKeyEvent releaseEv( QEvent::KeyRelease, ke->key(), ke->ascii(), ke->state(),
                               ke->text(), ke->isAutoRepeat(), ke->count() );
            static_cast<EventPropagator *>(m_widget)->sendEvent(&releaseEv);
        }
        static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        ret = ke->isAccepted();
	break;
    }
    case EventImpl::MOUSEOUT_EVENT: {
	QEvent moe( QEvent::Leave );
	QApplication::sendEvent(m_widget, &moe);
	break;
    }
    case EventImpl::MOUSEOVER_EVENT: {
	QEvent moe( QEvent::Enter );
	QApplication::sendEvent(m_widget, &moe);
	view()->part()->resetHoverText();
	break;
    }
    default:
        break;
    }
    return ret;
}

void RenderWidget::deref()
{
    if (_ref) _ref--;
//     qDebug( "deref(%p): width get count is %d", this, _ref);
    if (!_ref)
        arenaDelete(renderArena());
}

FindSelectionResult RenderReplaced::checkSelectionPoint(int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int &offset, SelPointState &)
{
#if 0
    kdDebug(6040) << "RenderReplaced::checkSelectionPoint(_x="<<_x<<",_y="<<_y<<",_tx="<<_tx<<",_ty="<<_ty<<")" << endl
                    << "xPos: " << xPos() << " yPos: " << yPos() << " width: " << width() << " height: " << height() << endl
                << "_ty + yPos: " << (_ty + yPos()) << " + height: " << (_ty + yPos() + height()) << "; _tx + xPos: " << (_tx + xPos()) << " + width: " << (_tx + xPos() + width()) << endl;
#endif
    node = element();
    offset = 0;

    if ( _y < _ty + yPos() )
        return SelectionPointBefore; // above -> before

    if ( _y > _ty + yPos() + height() ) {
        // below -> after
        // Set the offset to the max
        offset = 1;
        return SelectionPointAfter;
    }
    if ( _x > _tx + xPos() + width() ) {
        // to the right
        // ### how to regard bidi in replaced elements? (LS)
        offset = 1;
        return SelectionPointAfterInLine;
    }

    // The Y matches, check if we're on the left
    if ( _x < _tx + xPos() ) {
        // ### how to regard bidi in replaced elements? (LS)
        return SelectionPointBeforeInLine;
    }

    offset = _x > _tx + xPos() + width()/2;
    return SelectionPointInside;
}

#ifdef ENABLE_DUMP
void RenderWidget::dump(QTextStream &stream, const QString &ind) const
{
    RenderReplaced::dump(stream,ind);
    if ( widget() )
        stream << " color=" << widget()->foregroundColor().name()
               << " bg=" << widget()->backgroundColor().name();
    else
        stream << " null widget";
}
#endif

#include "render_replaced.moc"

