 /**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
 *           (C) 2004-2006 Germain Garand (germain@ebooksfrance.org)
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
#include "render_layer.h"
#include "render_canvas.h"
#include "render_line.h"

#include "render_arena.h"

#include <assert.h>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QActionEvent>
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <kglobalsettings.h>
#include <kurlrequester.h>
#include <QtCore/QObject>
#include <QVector>
#include <QMatrix>
#include <QtGui/QPaintEngine>

#include "khtml_ext.h"
#include "khtmlview.h"
#include "xml/dom2_eventsimpl.h"
#include "khtml_part.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_position.h"
#include "misc/helper.h"
#include "misc/paintbuffer.h"
#include "css/cssvalues.h"
#include "misc/htmltags.h"
#include <kdebug.h>

bool khtml::allowWidgetPaintEvents = false;

using namespace khtml;
using namespace DOM;


RenderReplaced::RenderReplaced(DOM::NodeImpl* node)
    : RenderBox(node)
{
    // init RenderObject attributes
    setReplaced(true);

    m_intrinsicWidth = 300;
    m_intrinsicHeight = 150;
}

void RenderReplaced::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown());

#ifdef DEBUG_LAYOUT
    kDebug( 6040 ) << "RenderReplaced::calcMinMaxWidth() known=" << minMaxKnown();
#endif

    m_width = calcReplacedWidth() + borderLeft() + borderRight() + paddingLeft() + paddingRight();

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

FindSelectionResult RenderReplaced::checkSelectionPoint(int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int &offset, SelPointState &)
{
#if 0
    kDebug(6040) << "RenderReplaced::checkSelectionPoint(_x="<<_x<<",_y="<<_y<<",_tx="<<_tx<<",_ty="<<_ty<<")" << endl
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

long RenderReplaced::caretMinOffset() const 
{
    return 0;
}

// Returns 1 since a replaced element can have the caret positioned 
// at its beginning (0), or at its end (1).
long RenderReplaced::caretMaxOffset() const 
{
    return 1;
}

unsigned long RenderReplaced::caretMaxRenderedOffset() const
{
    return 1;
}

Position RenderReplaced::positionForCoordinates(int _x, int _y)
{
    InlineBox *box = placeHolderBox();
    if (!box)
        return Position(element(), 0);
  
    RootInlineBox *root = box->root();
  
    int absx, absy;
    containingBlock()->absolutePosition(absx, absy);
  
    int top = absy + root->topOverflow();
    int bottom = root->nextRootBox() ? absy + root->nextRootBox()->topOverflow() : absy + root->bottomOverflow();
  
    if (_y < top)
        return Position(element(), caretMinOffset()); // coordinates are above
        
    if (_y >= bottom)
        return Position(element(), caretMaxOffset()); // coordinates are below
        
    if (element()) {
        if (_x <= absx + xPos() + (width() / 2))
            return Position(element(), 0);
        return Position(element(), 1);
    }
  
    return RenderBox::positionForCoordinates(_x, _y);
}

// -----------------------------------------------------------------------------

RenderWidget::RenderWidget(DOM::NodeImpl* node)
        : RenderReplaced(node)
{
    m_widget = 0;
    m_underMouse = 0;
    m_buffer[0] = 0;
    m_buffer[1] = 0;
    // a widget doesn't support being anonymous
    assert(!isAnonymous());
    m_view  = node->document()->view();
    m_arena.reset(renderArena());
    m_resizePending = false;
    m_discardResizes = false;
    m_needsMask = false;
    m_ownsWidget = true;

    // this is no real reference counting, its just there
    // to make sure that we're not deleted while we're recursed
    // in an eventFilter of the widget
    ref();
}

void RenderWidget::detach()
{
    // warning: keep in sync with RenderObject::detach
    
    detachCounters();
    remove();

    if ( m_widget ) {
        if ( m_view ) {
            m_view->setWidgetVisible(this, false);
        }
        KHTMLWidget* k = dynamic_cast<KHTMLWidget*>(m_widget);
        if (k)
            k->m_kwp->setRenderWidget(0);
        m_widget->removeEventFilter( this );
        m_widget->setMouseTracking( false );
    }

    // make sure our DOM-node don't think we exist
    if ( node() && node()->renderer() == this)
        node()->setRenderer(0);

    setDetached();
    deref();
}

RenderWidget::~RenderWidget()
{
    KHTMLAssert( refCount() <= 0 );

    if(m_widget) {
        m_widget->hide();
        if (m_ownsWidget)
            m_widget->deleteLater();
    }
    delete m_buffer[0];
    delete m_buffer[1];
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
        if (isRedirectedWidget() && qobject_cast<KHTMLView*>(m_widget)) {
             m_widget->resize( w, h);
             if (!m_widget->isVisible()) {
                 // Emission of Resize event is delayed.
                 // we have to pre-call KHTMLView::resizeEvent
                 // so that viewport size change and subsequent layout update
                 // is effective synchronously, which is important for JS.
                 // This only work because m_widget is a redirected view,
                 // and thus has visibleWidth()/visibleHeight() that mirror this RenderWidget,
                 // rather than the effective widget size. - gg.
                 QResizeEvent e( QSize(w,h), QSize(m_widget->width(),m_widget->height()));
                 static_cast<KHTMLView*>(m_widget)->resizeEvent( &e );
             }
        } else {
            m_resizePending = isRedirectedWidget();
            ref();
            element()->ref();
            QApplication::postEvent( this, new QWidgetResizeEvent( w, h ) );
            element()->deref();
            deref();
        }
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

bool RenderWidget::isRedirectedWidget() const
{
    KHTMLWidget* k = dynamic_cast<KHTMLWidget*>(m_widget);
    return k ? k->m_kwp->isRedirected() : false;   
}

void RenderWidget::setQWidget(QWidget *widget)
{
    if (widget != m_widget)
    {
        if (m_widget) {
            m_widget->removeEventFilter(this);
            disconnect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            m_widget->hide();
            if (m_ownsWidget)
                m_widget->deleteLater(); //Might happen due to event on the widget, so be careful
            m_widget = 0;
        }
        m_widget = widget;
        if (m_widget) {
            KHTMLWidget* k = dynamic_cast<KHTMLWidget*>(m_widget);
            bool isRedirectedSubFrame = false;
            if (k) {
                k->m_kwp->setRenderWidget(this);
                // enable redirection of every sub-frame that is not a FRAME
                if (qobject_cast<KHTMLView*>(m_widget) && element() && element()->id() != ID_FRAME) {
                    k->m_kwp->setIsRedirected( true );
                    isRedirectedSubFrame = true;
                }
            }   
            m_widget->setParent(m_view->widget());
            if (isRedirectedSubFrame)
                static_cast<KHTMLView*>(m_widget)->setHasStaticBackground();
            connect( m_widget, SIGNAL( destroyed()), this, SLOT( slotWidgetDestructed()));
            m_widget->installEventFilter(this);
            if (isRedirectedWidget()) {
                if (!qobject_cast<QFrame*>(m_widget))
                    m_widget->setAttribute( Qt::WA_NoSystemBackground );
            }
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
        if ( m_widget ) {        
            m_widget->move(0, -500000);
            m_widget->hide();
        }
        m_resizePending = false;
    }
}

void RenderWidget::layout( )
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );
    if ( m_widget ) {
        resizeWidget( m_width-borderLeft()-borderRight()-paddingLeft()-paddingRight(),
                      m_height-borderTop()-borderBottom()-paddingTop()-paddingBottom() );
        if (!isRedirectedWidget() && !isFrame() && !m_needsMask) {
            m_needsMask = true;
            RenderLayer* rl = enclosingStackingContext();
            RenderLayer* el = enclosingLayer();
            while (rl && el && el != rl) {
                if (el->renderer()->style()->position() != STATIC) {
                    m_needsMask = false;
                    break;
                }
                el = el->parent();
            }
            if (m_needsMask) {
	        if (rl) rl->setHasOverlaidWidgets();
                canvas()->setNeedsWidgetMasks();
            }
        }
    }

    setNeedsLayout(false);
}

void RenderWidget::updateFromElement()
{
    if (m_widget && !qobject_cast<KHTMLView*>(m_widget)) {
        // Color:
        QColor color = style()->color();
        QColor backgroundColor = style()->backgroundColor();

        if (!backgroundColor.isValid() && !style()->htmlHacks())
            backgroundColor = QColor(0,0,0,0);

        // check if we have to paint our background and let it show through the widget
        bool trans = ( isRedirectedWidget() && style()->backgroundLayers() && 
                       style()->backgroundLayers()->hasImage() && !qobject_cast<KUrlRequester*>(m_widget) );

        QPalette pal(QApplication::palette(m_widget));
        // We need a non-transparent version for widgets with popups (e.g. kcombobox). The popups must not let
        // the background show through.
        QPalette non_trans_pal = pal;

        if (color.isValid() || backgroundColor.isValid() || trans) {
            int contrast_ = KGlobalSettings::contrast();
            int highlightVal = 100 + (2*contrast_+4)*16/10;
            int lowlightVal = 100 + (2*contrast_+4)*10;
            bool shouldChangeBgPal = true;

            if (!backgroundColor.isValid()) 
                backgroundColor = pal.color( widget()->backgroundRole() );
            else
                shouldChangeBgPal = !( (backgroundColor == colorForCSSValue(CSS_VAL_WINDOW)) ||
                                       (backgroundColor == colorForCSSValue(CSS_VAL_BUTTONFACE)) );
            if (shouldChangeBgPal || trans) {
                pal.setColor(widget()->backgroundRole(), trans ? QColor(0,0,0,0) : backgroundColor);
                non_trans_pal.setColor(widget()->backgroundRole(), backgroundColor);
                for ( int i = 0; i < QPalette::NColorGroups; ++i ) {
                    if (shouldChangeBgPal) {
                        pal.setColor( (QPalette::ColorGroup)i, QPalette::Window, backgroundColor );
                        pal.setColor( (QPalette::ColorGroup)i, QPalette::Light, backgroundColor.light(highlightVal) );
                        pal.setColor( (QPalette::ColorGroup)i, QPalette::Dark, backgroundColor.dark(lowlightVal) );
                        pal.setColor( (QPalette::ColorGroup)i, QPalette::Mid, backgroundColor.dark(120) );
                        pal.setColor( (QPalette::ColorGroup)i, QPalette::Midlight, backgroundColor.light(110) );
                        non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Window, backgroundColor );
                        non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Light, backgroundColor.light(highlightVal) );
                        non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Dark, backgroundColor.dark(lowlightVal) );
                        non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Mid, backgroundColor.dark(120) );
                        non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Midlight, backgroundColor.light(110) );
                    }
                    pal.setColor( (QPalette::ColorGroup)i, QPalette::Button, trans ? QColor(0,0,0,0):backgroundColor );
                    pal.setColor( (QPalette::ColorGroup)i, QPalette::Base, trans ? QColor(0,0,0,0):backgroundColor );
                    non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Button, backgroundColor );
                    non_trans_pal.setColor( (QPalette::ColorGroup)i, QPalette::Base, backgroundColor );
                }
            }
            if ( color.isValid() ) {
                struct ColorSet {
                    QPalette::ColorGroup cg;
                    QPalette::ColorRole cr;
                };
                const struct ColorSet toSet [] = {
                    { QPalette::Active, QPalette::Foreground },
                    { QPalette::Active, QPalette::ButtonText },
                    { QPalette::Active, QPalette::Text },
                    { QPalette::Inactive, QPalette::Foreground },
                    { QPalette::Inactive, QPalette::ButtonText },
                    { QPalette::Inactive, QPalette::Text },
                    { QPalette::NColorGroups, QPalette::NColorRoles },
                };
                const ColorSet *set = toSet;
                while( set->cg != QPalette::NColorGroups ) {
                    pal.setColor( set->cg, set->cr, color );
                    non_trans_pal.setColor( set->cg, set->cr, color );
                    ++set;
                }

		QColor disfg = color;
		int h, s, v;
		disfg.getHsv( &h, &s, &v );
		if (v > 128)
		    // dark bg, light fg - need a darker disabled fg
		    disfg = disfg.dark(lowlightVal);
		else if (v > 64)
		    // light bg, dark fg - need a lighter disabled fg - but only if !black
		    disfg = disfg.light(highlightVal);
		else
		    // for really dark fg - use darkgray disabled fg,
		    // as ::light is pretty useless in this range
		    disfg = Qt::darkGray;
		pal.setColor(QPalette::Disabled,QPalette::Foreground,disfg);
		pal.setColor(QPalette::Disabled,QPalette::Text,disfg);
		pal.setColor(QPalette::Disabled,QPalette::ButtonText,disfg);
                non_trans_pal.setColor(QPalette::Disabled,QPalette::Foreground,disfg);
                non_trans_pal.setColor(QPalette::Disabled,QPalette::Text,disfg);
                non_trans_pal.setColor(QPalette::Disabled,QPalette::ButtonText,disfg);
            }
        }
        if (qobject_cast<QComboBox*>(m_widget)) {
            m_widget->setPalette(pal);
            // mmh great, there's no accessor for the popup... 
            QList<QWidget*>l = qFindChildren<QWidget *>(m_widget, QString());
            foreach(QWidget* w, l) {
                if (qobject_cast<QAbstractScrollArea*>(w)) {
                    // we have the listview, climb up to reach its container.
                    assert( w->parentWidget() != m_widget );
                    if (w->parentWidget())
                        w->parentWidget()->setPalette(non_trans_pal);
                }
            }
        } else {
            m_widget->setPalette(pal);
        }

        // Border:
        if (shouldPaintBorder())
        {
            if (QFrame* frame = qobject_cast<QFrame*>(m_widget))
                frame->setFrameShape(QFrame::NoFrame);
            else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_widget))
                lineEdit->setFrame(false);
        }

        // Font:
        setStyle(style());
    }

    RenderReplaced::updateFromElement();
}

void RenderWidget::paintOneBackground(QPainter *p, const QColor& c, const BackgroundLayer* bgLayer, QRect clipr, int _tx, int _ty, int w, int height)
{
    bool fudge = !shouldPaintBorder();
    paintBackgroundExtended(p, c, bgLayer, clipr, _tx, _ty, w, height,
                                fudge ? 1 : borderLeft() , fudge ? 1 : borderRight(), paddingLeft(), paddingRight(),
                                fudge ? 1 : borderTop(), fudge ? 1 : borderBottom(), paddingTop(), paddingBottom());
}
                             

void RenderWidget::paintBoxDecorations(PaintInfo& paintInfo, int _tx, int _ty)
{
    QRect r = QRect(_tx, _ty, width(), height());
    QRect cr = r.intersected(paintInfo.r);

    if (qobject_cast<QAbstractScrollArea*>(m_widget) || (isRedirectedWidget() && 
            style()->backgroundLayers() && style()->backgroundLayers()->hasImage()))
    {
        paintAllBackgrounds(paintInfo.p, style()->backgroundColor(), style()->backgroundLayers(), 
            cr, r.x(), r.y(), r.width(), r.height());
    }

    if (shouldPaintBorder() && style()->hasBorder())
    {
        paintBorder(paintInfo.p, _tx, _ty, width(), height(), style());
    }
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
}

void RenderWidget::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    if (shouldPaintBackgroundOrBorder() && !qobject_cast<KUrlRequester*>(m_widget) &&
          (paintInfo.phase == PaintActionChildBackground || paintInfo.phase == PaintActionChildBackgrounds))
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

    bool khtmlw = isRedirectedWidget();
    int childw = m_widget->width();
    int childh = m_widget->height();
    if ( (childw == 2000 || childh == 3072) && m_widget->inherits( "KHTMLView" ) ) {
        KHTMLView *vw = static_cast<KHTMLView *>(m_widget);
        int cy = m_view->contentsY();
        int ch = m_view->visibleHeight();


        int childx = m_widget->pos().x();
        int childy = m_widget->pos().y();

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
    if (!khtmlw)
        m_view->addChild( m_widget, xPos, yPos );
    else
        m_view->addChild( m_widget, xPos, -500000 +yPos);
    m_widget->show();
    if (khtmlw) {
        if ( KHTMLView* v = qobject_cast<KHTMLView*>(m_widget) ) {
            // our buffers are dedicated to scrollbars.
            if (v->verticalScrollBar()->isVisible() && (!m_buffer[0] || v->verticalScrollBar()->size() != m_buffer[0]->size())) {
                delete m_buffer[0];
                m_buffer[0] = new QPixmap( v->verticalScrollBar()->size() );
            }
            if (v->horizontalScrollBar()->isVisible() && (!m_buffer[1] || v->horizontalScrollBar()->size() != m_buffer[1]->size())) {
                delete m_buffer[1];
                m_buffer[1] = new QPixmap( v->horizontalScrollBar()->size() );
            }
        } else if (!m_buffer[0] || (m_widget->size() != m_buffer[0]->size())) {
            assert(!m_buffer[1]);
            delete m_buffer[0];
            m_buffer[0] = new QPixmap( m_widget->size() );
        }
        paintWidget(paintInfo, m_widget, xPos, yPos, m_buffer);
    }
}

static void setInPaintEventFlag(QWidget* w, bool b = true, bool recurse=true)
{
      w->setAttribute(Qt::WA_WState_InPaintEvent, b);

      if (!recurse)
          return;
      if (qobject_cast<KHTMLView*>(w)) {
          setInPaintEventFlag(static_cast<KHTMLView*>(w)->widget(), b, false);
          setInPaintEventFlag(static_cast<KHTMLView*>(w)->horizontalScrollBar(), b, false);
          setInPaintEventFlag(static_cast<KHTMLView*>(w)->verticalScrollBar(), b, false);
          return;
      }

      foreach(QObject* o, w->children()) {
          QWidget* const cw = static_cast<QWidget*>(o);
          if (o->isWidgetType() && ! cw->isWindow()
                                && !(cw->windowModality() & Qt::ApplicationModal)) {
              setInPaintEventFlag(cw, b);
          }
      }
}

static void copyWidget(const QRect& r, QPainter *p, QWidget *widget, int tx, int ty, bool buffered = false, QPixmap* buffer = 0)
{
    if (r.isNull() || r.isEmpty() )
        return;
 
    QPoint thePoint(tx, ty);
    QTransform t = p->worldTransform();
    bool vte = p->viewTransformEnabled();
    bool wme = p->worldMatrixEnabled();
    QRect w = p->window();
    QRect v = p->viewport();
    if (!buffered && t.isTranslating()) {
        thePoint.setX( thePoint.x()+ static_cast<int>(t.dx()) );
        thePoint.setY( thePoint.y()+ static_cast<int>(t.dy()) );
    }
    QRegion rg = p->clipRegion();
    QPaintDevice *d = p->device();
    QPaintDevice *x = d;
    qreal op = p->opacity();
    QPixmap* pm = 0;
    QPen pen = p->pen();
    QBrush brush = p->brush();
    if (buffered) {
        if (!widget->size().isValid())
            return;
        // TT says Qt 4's widget painting hits an NVidia RenderAccel bug/shortcoming
        // resulting in pixmap buffers being unsuitable for reuse by more than one widget.
        //
        // Until a turnaround exist in Qt, we can't reliably use shared buffers.
        // ###  pm = PaintBuffer::grab(widget->size());
        assert( buffer );
        pm = buffer;
        if (!pm->hasAlphaChannel()) {
            pm->fill(Qt::transparent);
        } else {
            QPainter pp(pm);
            pp.setCompositionMode( QPainter::CompositionMode_Source );
            pp.fillRect(r, Qt::transparent);
        }
        d = pm;
    }
    // Qt 4.4 regression #1: 
    // can't let a painter active on the view as Qt thinks it is opened on the *pixmap*
    // and prints "paint device can only be painted by one painter at a time" warnings.
    //
    // Testcase: paintEvent(...) { QPainter p(this); aChildWidget->render( aPixmapTarget, ...); }
    //
    p->end();

    setInPaintEventFlag( widget, false );

    widget->render( d, (buffered ? QPoint(0,0) : thePoint) + r.topLeft(), r);

    setInPaintEventFlag( widget );

//    if (!buffered) {
        p->begin(x);
        p->setWorldTransform(t);
        p->setWindow(w);
        p->setViewport(v);
        p->setViewTransformEnabled( vte );
        p->setWorldMatrixEnabled( wme );
        if (!rg.isEmpty())
            p->setClipRegion(rg);
        if (op < 1.0f)
            p->setOpacity(op);
        p->setPen(pen);
        p->setBrush(brush);
//    } else {
    if (buffered) {
        // transfer results
        QPoint off(r.x(), r.y());
        p->drawPixmap(thePoint+off, static_cast<QPixmap&>(*d), r);
        // ### PaintBuffer::release(pm);
    }
}

void RenderWidget::paintWidget(PaintInfo& pI, QWidget *widget, int tx, int ty, QPixmap* buffer[])
{
    QPainter* const p = pI.p;
    allowWidgetPaintEvents = true;

    // Qt 4.4 regression #2: 
    //    can't use QWidget::render to directly paint widgets on the view anymore.
    //    Results are unreliable for subrects, leaving blank squares. (cf. kde #158607)
    //
    bool buffered = true; // p->combinedMatrix().m22() != 1.0 || (p->device()->devType() == QInternal::Printer);

    QRect rr = pI.r;
    rr.translate(-tx, -ty);
    const QRect r = widget->rect().intersect( rr );
    if ( KHTMLView* v = qobject_cast<KHTMLView*>( widget ) ) {
        QPoint thePoint(tx, ty);
        if (v->verticalScrollBar()->isVisible()) {
            QRect vbr = v->verticalScrollBar()->rect();
            QPoint of = v->verticalScrollBar()->mapTo(v, QPoint(vbr.x(), vbr.y()));
            vbr.translate( of );
            vbr &= r;
            vbr.translate( -of );
            if (vbr.isValid() && !vbr.isEmpty())
                copyWidget(vbr, p, v->verticalScrollBar(), tx+of.x(), ty+of.y(), buffered, buffer[0]);
        }
        if (v->horizontalScrollBar()->isVisible()) {
            QRect hbr = v->horizontalScrollBar()->rect();
            QPoint of = v->horizontalScrollBar()->mapTo(v, QPoint(hbr.x(), hbr.y()));
            hbr.translate( of );
            hbr &= r;
            hbr.translate( -of );
            if (hbr.isValid() && !hbr.isEmpty())
                copyWidget(hbr, p, v->horizontalScrollBar(), tx+ of.x(), ty+ of.y(), buffered, buffer[1]);
        }
        QRect vr = (r & v->viewport()->rect());
        if (vr.isValid() && !vr.isEmpty())
            v->render(p, vr, thePoint);
    } else {
        copyWidget(r, p, widget, tx, ty, buffered, buffer[0]);
    }
    allowWidgetPaintEvents = false;
}

bool RenderWidget::eventFilter(QObject* /*o*/, QEvent* e)
{
    // no special event processing if this is a frame (in which case KHTMLView handles it all)
    if ( qobject_cast<KHTMLView*>( m_widget ) || isRedirectedWidget() )
        return false;
    if ( !element() ) return true;


    static bool directToWidget = false;
    if (directToWidget) {
      //We're trying to get the event to the widget 
      //promptly. So get out of here..
      return false;
    }

    ref();
    element()->ref();

    bool filtered = false;

    //kDebug() << "RenderWidget::eventFilter type=" << e->type();
    switch(e->type()) {
    case QEvent::FocusOut:
        // First, forward it to the widget, so that Qt gets a precise
        // state of the focus before pesky JS can try changing it..
        directToWidget = true;
        QApplication::sendEvent(m_widget, e);
        directToWidget = false;
        filtered       = true; //We already delivered it!
        
        // Don't count popup as a valid reason for losing the focus
        // (example: opening the options of a select combobox shouldn't emit onblur)
        if ( static_cast<QFocusEvent*>(e)->reason() != Qt::PopupFocusReason )
            handleFocusOut();
        break;
    case QEvent::FocusIn:
        //As above, forward to the widget first...
        directToWidget = true;
        QApplication::sendEvent(m_widget, e);
        directToWidget = false;
        filtered       = true; //We already delivered it!

        //kDebug(6000) << "RenderWidget::eventFilter captures FocusIn";
        document()->setFocusNode(element());
//         if ( isEditable() ) {
//             KHTMLPartBrowserExtension *ext = static_cast<KHTMLPartBrowserExtension *>( element()->view->part()->browserExtension() );
//             if ( ext )  ext->editableWidgetFocused( m_widget );
//         }
        break;
    case QEvent::Wheel: {
       if (widget()->parentWidget() == view()->widget()) {
            bool vertical = ( static_cast<QWheelEvent*>(e)->orientation() == Qt::Vertical );
            // don't allow the widget to react to wheel event if
            // the view is being scrolled by mouse wheel
            // This does not apply if the webpage has no valid scroll range in the given wheel event orientation.
            if ( ((vertical && (view()->contentsHeight() > view()->visibleHeight()))  ||
                  (!vertical && (view()->contentsWidth() > view()->visibleWidth()))) &&
                   view()->isScrollingFromMouseWheel() )  {
                static_cast<QWheelEvent*>(e)->ignore();
                QApplication::sendEvent(view(), e);
                filtered = true;
            }
        }
        break;
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    // TODO this seems wrong - Qt events are not correctly translated to DOM ones,
    // like in KHTMLView::dispatchKeyEvent()
        if (element()->dispatchKeyEvent(static_cast<QKeyEvent*>(e),false))
            filtered = true;
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
    case QEvent::Wheel:
        wheelEvent ( static_cast<QWheelEvent *> (e) );
        break;
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
    case QEvent::FocusIn:
        focusInEvent(static_cast<QFocusEvent *>(e));
        break;
    case QEvent::FocusOut:
        focusOutEvent(static_cast<QFocusEvent *>(e));
        break;
    case QEvent::ContextMenu:
        contextMenuEvent(static_cast<QContextMenuEvent*>(e));
        break;
    default:
        break;
    }
}

bool RenderWidget::handleEvent(const DOM::EventImpl& ev)
{
    bool ret = false;
    
    switch(ev.id()) {
    case EventImpl::DOMFOCUSIN_EVENT: 
    case EventImpl::DOMFOCUSOUT_EVENT: {
          QFocusEvent e(ev.id() == EventImpl::DOMFOCUSIN_EVENT ? QEvent::FocusIn : QEvent::FocusOut);
          static_cast<EventPropagator *>(m_widget)->sendEvent(&e);
          ret = e.isAccepted();
          break;
    }
    case EventImpl::KHTML_MOUSEWHEEL_EVENT:
    case EventImpl::MOUSEDOWN_EVENT:
    case EventImpl::MOUSEUP_EVENT:
    case EventImpl::MOUSEMOVE_EVENT: {
        if (!ev.isMouseEvent()) break;

        const MouseEventImpl &me = static_cast<const MouseEventImpl &>(ev);
        QMouseEvent* const qme = me.qEvent();
        QMouseEvent::Type type;
        Qt::MouseButton button = Qt::NoButton;
        Qt::MouseButtons buttons = Qt::NoButton;
        Qt::KeyboardModifiers state = 0;
        Qt::Orientation orient = Qt::Vertical;

        if (qme) {
            buttons = qme->buttons();
            button = qme->button();
            state = qme->modifiers();
            type = qme->type();
        } else {
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

            buttons = button;

            switch(me.id())  {
            case EventImpl::MOUSEDOWN_EVENT:
                type = QMouseEvent::MouseButtonPress;
                break;
            case EventImpl::MOUSEUP_EVENT:
                type = QMouseEvent::MouseButtonRelease;
                break;
            case EventImpl::KHTML_MOUSEWHEEL_EVENT:
            case EventImpl::MOUSEMOVE_EVENT:
            default:
                type = QMouseEvent::MouseMove;
                button = Qt::NoButton;
                break;
            }
            
            if (me.orientation() == MouseEventImpl::OHorizontal)
                orient = Qt::Horizontal;

            if (me.ctrlKey())
                state |= Qt::ControlModifier;
            if (me.altKey())
                state |= Qt::AltModifier;
            if (me.shiftKey())
                state |= Qt::ShiftModifier;
            if (me.metaKey())
                state |= Qt::MetaModifier;
        }

        int absx = 0;
        int absy = 0;
        absolutePosition(absx, absy);
        absx += borderLeft()+paddingLeft();
        absy += borderTop()+paddingTop();

        QPoint p(me.clientX() - absx + m_view->contentsX(),
                 me.clientY() - absy + m_view->contentsY());

        QWidget* target = 0;
        target = m_widget->childAt(p);
        
        if ( includesPadding() && ::qobject_cast<QAbstractScrollArea*>(m_widget) )
        {
            QAbstractScrollArea *w = static_cast<QAbstractScrollArea*>(m_widget);
            // childAt returns the wrong child in this case?
            if (target == w->viewport())
                p -= QPoint(RenderWidget::paddingLeft(), RenderWidget::paddingTop());
        }
        
        if (m_underMouse != target && ev.id() == EventImpl::MOUSEMOVE_EVENT) {
            if (m_underMouse) {
                QEvent moe( QEvent::Leave );
                QApplication::sendEvent(m_underMouse, &moe);
//                qDebug() << "sending LEAVE to"<< m_underMouse;
                if (m_underMouse->testAttribute(Qt::WA_Hover)) {
                    QHoverEvent he( QEvent::HoverLeave, QPoint(-1,-1), QPoint(0,0) );
                    QApplication::sendEvent(m_underMouse, &he);
                }
            }
            if (target) {
	        QEvent moe( QEvent::Enter );
	        QApplication::sendEvent(target, &moe);
//                qDebug() << "sending ENTER to" << target;
                if (target->testAttribute(Qt::WA_Hover)) {
                    QHoverEvent he( QEvent::HoverEnter, QPoint(0,0), QPoint(-1,-1) );
                    QApplication::sendEvent(target, &he);
                }
                m_underMouse = target;
            }
        }

        if (ev.id() == EventImpl::MOUSEDOWN_EVENT) {
            if (!target || (!::qobject_cast<QScrollBar*>(target) && 
                            !::qobject_cast<KUrlRequester*>(m_widget)))
                target = m_widget;
            if ( button == Qt::LeftButton )
                view()->setMouseEventsTarget( target );
        } else {
            target = view()->mouseEventsTarget();
            if (target) {
                QWidget * parent = target;
                while (parent && parent != m_widget)
                    parent = parent->parentWidget();
                if (!parent) return false;
            } else {
                target = m_widget;
            }
        }

        p = target->mapFrom(m_widget, p);

        bool needContextMenuEvent = (type == QMouseEvent::MouseButtonPress && button == Qt::RightButton);
        bool isMouseWheel = (ev.id() == EventImpl::KHTML_MOUSEWHEEL_EVENT);

        if (isMouseWheel) {
            // don't allow the widget to react to wheel event if
            // a) the view is being scrolled by mouse wheel
            // b) it's an unfocused ComboBox (for extra security against unwanted changes to formulars)
            // This does not apply if the webpage has no valid scroll range in the given wheel event orientation.
            if ( ((orient == Qt::Vertical && (view()->contentsHeight() > view()->visibleHeight()))  ||
                  (orient == Qt::Horizontal && (view()->contentsWidth() > view()->visibleWidth()))) &&
                  ( view()->isScrollingFromMouseWheel() ||
                    (qobject_cast<QComboBox*>(m_widget) && 
                     (!document()->focusNode() || document()->focusNode()->renderer() != this) )))  {
                ret = false;
                break;
            }
        }

        QEvent *e = isMouseWheel ?
                    static_cast<QEvent*>(new QWheelEvent(p, -me.detail()*40, buttons, state, orient)) :
                    static_cast<QEvent*>(new QMouseEvent(type,    p, button, buttons, state));
        static_cast<EventPropagator *>(target)->sendEvent(e);

        ret = e->isAccepted();

        if (needContextMenuEvent) {
            QContextMenuEvent cme(QContextMenuEvent::Mouse, p);
            static_cast<EventPropagator *>(target)->sendEvent(&cme);
        } else if (type == QEvent::MouseMove && target->testAttribute(Qt::WA_Hover)) {
            QHoverEvent he( QEvent::HoverMove, p, p );
            QApplication::sendEvent(target, &he);
        }
        if (ev.id() == EventImpl::MOUSEUP_EVENT) {
            view()->setMouseEventsTarget( 0 );
        }
        delete e;
        break;
    }
    case EventImpl::KEYDOWN_EVENT:
        // do nothing; see the mapping table below
        break;
    case EventImpl::KEYUP_EVENT: {
        if (!ev.isKeyRelatedEvent()) break;

        const KeyEventBaseImpl& domKeyEv = static_cast<const KeyEventBaseImpl &>(ev);
        if (domKeyEv.isSynthetic() && !acceptsSyntheticEvents()) break;

        QKeyEvent* const ke = domKeyEv.qKeyEvent();
        static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        ret = ke->isAccepted();
        break;
    }
    case EventImpl::KEYPRESS_EVENT: {
        if (!ev.isKeyRelatedEvent()) break;

        const KeyEventBaseImpl& domKeyEv = static_cast<const KeyEventBaseImpl &>(ev);
        if (domKeyEv.isSynthetic() && !acceptsSyntheticEvents()) break;

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

        QKeyEvent* const ke = domKeyEv.qKeyEvent();
        if (ke->isAutoRepeat()) {
            QKeyEvent releaseEv( QEvent::KeyRelease, ke->key(), ke->modifiers(),
                               ke->text(), ke->isAutoRepeat(), ke->count() );
            static_cast<EventPropagator *>(m_widget)->sendEvent(&releaseEv);
        }
        static_cast<EventPropagator *>(m_widget)->sendEvent(ke);
        ret = ke->isAccepted();
	break;
    }
    case EventImpl::MOUSEOUT_EVENT: {
        QWidget* target = m_underMouse ? (QWidget*) m_underMouse : m_widget;
	QEvent moe( QEvent::Leave );
	QApplication::sendEvent(target, &moe);
//        qDebug() << "received MOUSEOUT, forwarding to" << target ;
	if (target->testAttribute(Qt::WA_Hover)) {
            QHoverEvent he( QEvent::HoverLeave, QPoint(-1,-1), QPoint(0,0) );
            QApplication::sendEvent(target, &he);
        }
        m_underMouse = 0;
	break;
    }
    case EventImpl::MOUSEOVER_EVENT: {
	QEvent moe( QEvent::Enter );
	QApplication::sendEvent(m_widget, &moe);
//        qDebug() << "received MOUSEOVER, forwarding to" << m_widget;
        if (m_widget->testAttribute(Qt::WA_Hover)) {
            QHoverEvent he( QEvent::HoverEnter, QPoint(0,0), QPoint(-1,-1) );
            QApplication::sendEvent(m_widget, &he);
        }
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
    if (!_ref) {
        if (attached()) {
            _ref++;
            detach(); // will perform the final deref.
            return;
        }
        SharedPtr<RenderArena> guard(m_arena); //Since delete on us gets called -first-,
                                               //before the arena free
        arenaDelete(m_arena.get());
    }
}

#ifdef ENABLE_DUMP
void RenderWidget::dump(QTextStream &stream, const QString &ind) const
{
    RenderReplaced::dump(stream,ind);
    if ( widget() )
        stream << " color=" << widget()->palette().color( widget()->foregroundRole() ).name()
               << " bg=" << widget()->palette().color( widget()->backgroundRole() ).name();
    else
        stream << " null widget";
}
#endif

// -----------------------------------------------------------------------------

QPoint KHTMLWidgetPrivate::absolutePos()
{
        if (!m_rw)
            return m_pos;
        int x, y;
        m_rw->absolutePosition(x, y);
        x += m_rw->borderLeft()+m_rw->paddingLeft();
        y += m_rw->borderTop()+m_rw->paddingTop();
        return QPoint(x, y);
}

KHTMLView* KHTMLWidgetPrivate::rootViewPos(QPoint& pos)
{
    if (!m_rw || !m_rw->widget()) {
        pos = QPoint();
        return 0;
    }
    pos = absolutePos();
    KHTMLView* v = m_rw->view();
    KHTMLView* last = 0;
    while (v) {
        last = v;
        pos.setX( pos.x() - v->contentsX() );
        pos.setY( pos.y() - v->contentsY() );
        KHTMLWidget*kw = dynamic_cast<KHTMLWidget*>(v);
        if (!kw || !kw->m_kwp->isRedirected()) 
            break;
        pos += kw->m_kwp->absolutePos();
        v = v->part()->parentPart() ? v->part()->parentPart()->view() : 0;
    }
    return last;
}

// -----------------------------------------------------------------------------

KHTMLWidget::KHTMLWidget() 
    : m_kwp(new KHTMLWidgetPrivate()) {}

KHTMLWidget::~KHTMLWidget() 
    { delete m_kwp; }

#include "render_replaced.moc"

