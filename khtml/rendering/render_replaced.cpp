/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2000-2003 Dirk Mueller (mueller@kde.org)
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
#include <qobjectlist.h>

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

void RenderReplaced::paint( QPainter *p, int _x, int _y, int _w, int _h,
                            int _tx, int _ty, PaintAction paintAction)
{
    if (paintAction != PaintActionForeground)
        return;

    // not visible or not even once layouted?
    if (style()->visibility() != VISIBLE || m_y <=  -500000)  return;

    _tx += m_x;
    _ty += m_y;

    if((_ty > _y + _h) || (_ty + m_height < _y)) return;

    if(shouldPaintBackgroundOrBorder())
        paintBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    paintObject(p, _x, _y, _w, _h, _tx, _ty, paintAction);
}

void RenderReplaced::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    int width = calcReplacedWidth();

    if (!isWidget())
        width += paddingLeft() + paddingRight() + borderLeft() + borderRight();

    if ( style()->width().isPercent() || style()->height().isPercent() ) {
        m_minWidth = 0;
        m_maxWidth = width;
    }
    else
        m_minWidth = m_maxWidth = width;

    setMinMaxKnown();
}

void RenderReplaced::position(InlineBox* box, int /*from*/, int /*len*/, bool /*reverse*/, int)
{
    setPos( box->xPos() + marginLeft(), box->yPos() + marginTop() );
}

// -----------------------------------------------------------------------------

RenderWidget::RenderWidget(DOM::NodeImpl* node)
        : RenderReplaced(node)
{
    m_widget = 0;
    // a widget doesn't support being anonymous
    assert(!isAnonymous());
    m_view = node->getDocument()->view();

    // this is no real reference counting, its just there
    // to make sure that we're not deleted while we're recursed
    // in an eventFilter of the widget
    ref();
}

void RenderWidget::detach()
{
    kdDebug( 6040 ) << "RenderWidget::detach( " << this << " )" << endl;
    remove();

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
    h = kMin( h, 3072 );
    w = kMin( w, 2000 );

    if (m_widget->width() != w || m_widget->height() != h) {
        ref();
        element()->ref();
        QApplication::postEvent( this, new QWidgetResizeEvent( w, h ) );
        element()->deref();
        deref();
    }
}

bool RenderWidget::event( QEvent *e )
{
    if ( m_widget && (e->type() == (QEvent::Type)QWidgetResizeEvent::Type) ) {
        QWidgetResizeEvent *re = static_cast<QWidgetResizeEvent *>(e);
        m_widget->resize( re->w,  re->h );
    }
    // eat all events - except if this is a frame (in which case KHTMLView handles it all)
    if ( ::qt_cast<KHTMLView *>( m_widget ) )
        return QObject::event( e );
    return true;
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

            if ( !strcmp(m_widget->name(), "__khtml"))
                m_widget->setBackgroundMode( QWidget::NoBackground );

            if (m_widget->focusPolicy() > QWidget::StrongFocus)
                m_widget->setFocusPolicy(QWidget::StrongFocus);
            // if we're already layouted, apply the calculated space to the
            // widget immediately
            if (layouted()) {
                // ugly hack to limit the maximum size of the widget (as X11 has problems if it's bigger)
                resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                              m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
            }
            else
                setPos(xPos(), -500000);
        }
        m_view->setWidgetVisible(this, false);
        m_view->addChild( m_widget, 0, -500000);
    }
}

void RenderWidget::layout( )
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );
    if ( m_widget )
        resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );

    setLayouted();
}

void RenderWidget::updateFromElement()
{
    if (m_widget) {
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
                for ( int i = 0; i < QPalette::NColorGroups; i++ ) {
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
            m_widget->hide();
        }
    }

    // do not paint background or borders for widgets
    setShouldPaintBackgroundOrBorder(false);
}

void RenderWidget::paintObject(QPainter* p, int x, int y, int w, int h, int _tx, int _ty,
                               PaintAction paintAction)
{
    if (!m_widget || !m_view || paintAction != PaintActionForeground)
        return;

    // not visible or not even once layouted
    if (style()->visibility() != VISIBLE || m_y <=  -500000) {
        m_widget->hide();
        return;
    }

    // add offset for relative positioning
    if(isRelPositioned())
        relativePositionOffset(_tx, _ty);

    int xPos = _tx+borderLeft()+paddingLeft();
    int yPos = _ty+borderTop()+paddingTop();

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
        yNew = kMin( yNew, yPos + m_height - childh );
        yNew = kMax( yNew, yPos );
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
    paintWidget(p, m_widget, x, y, w, h, _tx, _ty);
}

#include <private/qinternal_p.h>

static QPixmap copyWidget(int tx, int ty, QPainter *p, QWidget *widget)
{
    QPixmap pm = QPixmap(widget->width(), widget->height());
    if ( p->device()->isExtDev() || ::qt_cast<QLineEdit *>(widget) ) {
	// even hackier!
	pm.fill(widget->colorGroup().base());
    } else {
	QPoint pt(tx, ty);
	pt = p->xForm(pt);
	bitBlt(&pm, 0, 0, p->device(), pt.x(), pt.y());
    }
    QPainter::redirect(widget, &pm);
    QPaintEvent e( widget->rect(), false );
    QApplication::sendEvent( widget, &e );
    QPainter::redirect(widget, 0);
    if (widget->children()) {
	QObjectListIterator it(*widget->children());
	for (; it.current(); ++it) {
	    QWidget *w = ::qt_cast<QWidget *>(it.current());
	    if (w && !w->isTopLevel()) {
		QPixmap cp = copyWidget(tx + w->x(), ty + w->y(), p, w);
		bitBlt(&pm, w->x(), w->y(), &cp, 0, 0);
	    }
	}
    }
    return pm;
}


void RenderWidget::paintWidget(QPainter *p, QWidget *widget, int, int, int, int, int tx, int ty)
{
    // We have some problems here, as we can't redirect some of the widgets.
    allowWidgetPaintEvents = true;

    if (!strcmp(widget->name(), "__khtml")) {
        bool dsbld = QSharedDoubleBuffer::isDisabled();
        QSharedDoubleBuffer::setDisabled(true);
	QPixmap pm = copyWidget(tx, ty, p, widget);
        QSharedDoubleBuffer::setDisabled(dsbld);
        p->drawPixmap(tx, ty, pm);
    } else {
        // QScrollview is difficult and I currently know of no way to get
        // the stuff on screen without flicker.
        //
        // This still doesn't work nicely for textareas. Probably need
        // to fix qtextedit for that.
        // KHTMLView::eventFilter()
#if 0
        QPaintEvent e( widget->rect(), false );
        QApplication::sendEvent( widget, &e );
        QScrollView *sv = static_cast<QScrollView *>(widget);
        sv->repaint(true);
        pm = QPixmap::grabWindow(widget->winId());
#endif
    }

    allowWidgetPaintEvents = false;
}

bool RenderWidget::eventFilter(QObject* /*o*/, QEvent* e)
{
    // no special event processing if this is a frame (in which case KHTMLView handles it all)
    if ( ::qt_cast<KHTMLView *>( m_widget ) )
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
        if ( QFocusEvent::reason() != QFocusEvent::Popup )
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
                widget()->focusPolicy() <= QWidget::StrongFocus)  {
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
    switch(ev.id()) {
    case EventImpl::MOUSEDOWN_EVENT:
    case EventImpl::MOUSEUP_EVENT:
    case EventImpl::MOUSEMOVE_EVENT: {
        const MouseEventImpl &me = static_cast<const MouseEventImpl &>(ev);
        QMouseEvent *qme = me.qEvent();

        int absx = 0;
        int absy = 0;

        absolutePosition(absx, absy);

        QPoint p(me.clientX() - absx + m_view->contentsX(),
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
                button = LeftButton;
                break;
            case 1:
                button = MidButton;
                break;
            case 2:
                button = RightButton;
                break;
            default:
                break;
            }
            if (me.ctrlKey())
                state |= ControlButton;
            if (me.altKey())
                state |= AltButton;
            if (me.shiftKey())
                state |= ShiftButton;
            if (me.metaKey())
                state |= MetaButton;
        }

//     kdDebug(6000) << "sending event to widget "
//                   << " pos=" << p << " type=" << type
//                   << " button=" << button << " state=" << state << endl;
        QMouseEvent e(type, p, button, state);
        static_cast<EventPropagator *>(m_widget)->sendEvent(&e);
        break;
    }
    case EventImpl::KHTML_KEYDOWN_EVENT:
    case EventImpl::KHTML_KEYUP_EVENT: {
        QKeyEvent *ke = static_cast<const TextEventImpl &>(ev).qKeyEvent;
        if (ke)
            static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        break;
    }
    case EventImpl::KHTML_KEYPRESS_EVENT: {

        // See KHTMLView::dispatchKeyEvent: autorepeat is just keypress in the DOM
        // but it's keyrelease+keypress in Qt. So here we do the inverse mapping as
        // the one done in KHTMLView: generate two events for one DOM auto-repeat keypress.
        // Similarly, DOM keypress events with non-autorepeat Qt event do nothing here,
        // because the matching Qt keypress event was already sent from DOM keydown event.

        // Reverse drawing as the one in KHTMLView:
        //  DOM:   Down     Press   |       Press                             |     Up
        //  Qt:    Press  (nothing) | Release(autorepeat) + Press(autorepeat) |   Release

        QKeyEvent *ke = static_cast<const TextEventImpl &>(ev).qKeyEvent;
        if (ke && ke->isAutoRepeat()) {
            QKeyEvent releaseEv( QEvent::KeyRelease, ke->key(), ke->ascii(), ke->state(),
                               ke->text(), ke->isAutoRepeat(), ke->count() );
            static_cast<EventPropagator *>(m_widget)->sendEvent(&releaseEv);
            static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        }

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
	break;
    }
    default:
        break;
    }
    return true;
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

#ifndef NDEBUG
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

