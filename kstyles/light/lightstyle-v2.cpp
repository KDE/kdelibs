/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include "lightstyle-v2.h"

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qframe.h"
#include "qpushbutton.h"
#include "qdrawutil.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qguardedptr.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qimage.h"
#include "qcombobox.h"
#include "qslider.h"
#include "qstylefactory.h"


class LightStyleV2Private
{
public:
    LightStyleV2Private()
	: ref(1)
    {
	basestyle = QStyleFactory::create( "Windows" );
	if ( ! basestyle )
	    basestyle = QStyleFactory::create( QStyleFactory::keys().first() );
	if ( ! basestyle )
	    qFatal( "LightStyle: couldn't find a basestyle!" );
    }

    ~LightStyleV2Private()
    {
	delete basestyle;
    }

    QStyle *basestyle;
    int ref;
};

static LightStyleV2Private *singleton = 0;


LightStyleV2::LightStyleV2()
    : QCommonStyle()
{
    if (! singleton)
	singleton = new LightStyleV2Private;
    else
	singleton->ref++;
}

LightStyleV2::~LightStyleV2()
{
    if (singleton && --singleton->ref <= 0) {
	delete singleton;
	singleton = 0;
    }
}

void LightStyleV2::polishPopupMenu( QPopupMenu * )
{
    // empty to satisy pure virtual requirements
}

static void drawLightBevel(QPainter *p, const QRect &r, const QColorGroup &cg,
			   QStyle::SFlags flags, const QBrush *fill = 0)
{
    QRect br = r;
    bool sunken = (flags & (QStyle::Style_Down | QStyle::Style_On |
			    QStyle::Style_Sunken));

    p->setPen(cg.dark());
    p->drawRect(r);

    if (flags & (QStyle::Style_Down | QStyle::Style_On |
		 QStyle::Style_Sunken | QStyle::Style_Raised)) {
	// button bevel
	if (sunken)
	    p->setPen(cg.mid());
	else
	    p->setPen(cg.light());

	p->drawLine(r.x() + 1, r.y() + 2,
		    r.x() + 1, r.y() + r.height() - 3); // left
	p->drawLine(r.x() + 1, r.y() + 1,
		    r.x() + r.width() - 2, r.y() + 1); // top

	if (sunken)
	    p->setPen(cg.light());
	else
	    p->setPen(cg.mid());

	p->drawLine(r.x() + r.width() - 2, r.y() + 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
	p->drawLine(r.x() + 1, r.y() + r.height() - 2,
		    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom

	br.addCoords(2, 2, -2, -2);
    } else
	br.addCoords(1, 1, -1, -1);

    // fill
    if (fill) p->fillRect(br, *fill);
}

void LightStyleV2::drawPrimitive( PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				const QStyleOption &data ) const
{
    switch (pe) {
    case PE_HeaderSection:
	{
	    flags = ((flags | Style_Sunken) ^ Style_Sunken) | Style_Raised; 
	    	//Don't show pressed too often (as in light 3)
	    const QBrush *fill;
	    if (flags & QStyle::Style_Enabled) {
		    fill = &cg.brush(QColorGroup::Button);
	    } else
		fill = &cg.brush(QColorGroup::Background);

	    drawLightBevel(p, r, cg, flags, fill);
	    p->setPen( cg.buttonText() );
	    break;
	}
	
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
	{
	    const QBrush *fill;
	    if (flags & QStyle::Style_Enabled) {
		if (flags & (QStyle::Style_Down |
			     QStyle::Style_On |
			     QStyle::Style_Sunken))
		    fill = &cg.brush(QColorGroup::Midlight);
		else
		    fill = &cg.brush(QColorGroup::Button);
	    } else
		fill = &cg.brush(QColorGroup::Background);
	    drawLightBevel(p, r, cg, flags, fill);
	    break;
	}

    case PE_ButtonDropDown:
	{
	    QBrush thefill;
	    bool sunken =
		(flags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Sunken));

	    if (flags & QStyle::Style_Enabled) {
		if (sunken)
		    thefill = cg.brush(QColorGroup::Midlight);
		else
		    thefill = cg.brush(QColorGroup::Button);
	    } else
		thefill = cg.brush(QColorGroup::Background);

	    p->setPen(cg.dark());
	    p->drawLine(r.topLeft(),     r.topRight());
	    p->drawLine(r.topRight(),    r.bottomRight());
	    p->drawLine(r.bottomRight(), r.bottomLeft());

	    if (flags & (QStyle::Style_Down | QStyle::Style_On |
			 QStyle::Style_Sunken | QStyle::Style_Raised)) {
		// button bevel
		if (sunken)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());

		p->drawLine(r.x(), r.y() + 2,
			    r.x(), r.y() + r.height() - 3); // left
		p->drawLine(r.x(), r.y() + 1,
			    r.x() + r.width() - 2, r.y() + 1); // top

		if (sunken)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());

		p->drawLine(r.x() + r.width() - 2, r.y() + 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 3); // right
		p->drawLine(r.x() + 1, r.y() + r.height() - 2,
			    r.x() + r.width() - 2, r.y() + r.height() - 2); // bottom
	    }

	    p->fillRect(r.x() + 1, r.y() + 2, r.width() - 3, r.height() - 4, thefill);
	    break;
	}

    case PE_ButtonDefault:
	p->setPen(cg.dark());
	p->setBrush(cg.light());
	p->drawRect(r);
	break;

    case PE_Indicator:
	const QBrush *fill;
	if (! (flags & Style_Enabled))
	    fill = &cg.brush(QColorGroup::Background);
	else if (flags & Style_Down)
	    fill = &cg.brush(QColorGroup::Mid);
	else
	    fill = &cg.brush(QColorGroup::Base);
	drawLightBevel(p, r, cg, flags | Style_Sunken, fill);

	p->setPen(cg.text());
	if (flags & Style_NoChange) {
	    p->drawLine(r.x() + 3, r.y() + r.height() / 2,
			r.x() + r.width() - 4, r.y() + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() + 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() + 1 + r.height() / 2);
	    p->drawLine(r.x() + 3, r.y() - 1 + r.height() / 2,
			r.x() + r.width() - 4, r.y() - 1 + r.height() / 2);
	} else if (flags & Style_On) {
	    p->drawLine(r.x() + 4, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 5);
	    p->drawLine(r.x() + 3, r.y() + 3,
			r.x() + r.width() - 4, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + 4,
			r.x() + r.width() - 5, r.y() + r.height() - 4);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 5,
			r.x() + r.width() - 5, r.y() + 3);
	    p->drawLine(r.x() + 3, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 3);
	    p->drawLine(r.x() + 4, r.y() + r.height() - 4,
			r.x() + r.width() - 4, r.y() + 4);
	}

	break;

    case PE_ExclusiveIndicator:
	{
	    QRect br = r, // bevel rect
		  cr = r, // contents rect
		  ir = r; // indicator rect
	    br.addCoords(1, 1, -1, -1);
	    cr.addCoords(2, 2, -2, -2);
	    ir.addCoords(3, 3, -3, -3);

	    p->fillRect(r, cg.brush(QColorGroup::Background));

	    p->setPen(cg.dark());
	    p->drawArc(r, 0, 16*360);
	    p->setPen(cg.mid());
	    p->drawArc(br, 45*16, 180*16);
	    p->setPen(cg.light());
	    p->drawArc(br, 235*16, 180*16);

	    p->setPen(flags & Style_Down ? cg.mid() :
		      (flags & Style_Enabled ? cg.base() : cg.background()));
	    p->setBrush(flags & Style_Down ? cg.mid() :
			(flags & Style_Enabled ? cg.base() : cg.background()));
	    p->drawEllipse(cr);

	    if (flags & Style_On) {
		p->setBrush(cg.text());
		p->drawEllipse(ir);
	    }

	    break;
	}

    case PE_DockWindowHandle:
	{
	    QString title;
	    bool drawTitle = FALSE;
	    if ( p && p->device()->devType() == QInternal::Widget ) {
		QWidget *w = (QWidget *) p->device();
		QWidget *p = w->parentWidget();
		if (p->inherits("QDockWindow") && ! p->inherits("QToolBar")) {
		    drawTitle = TRUE;
		    title = p->caption();
		}
	    }

	    flags |= Style_Raised;
	    if (flags & Style_Horizontal) {
		if (drawTitle) {
		    QPixmap pm(r.height(), r.width());
		    QPainter p2(&pm);
		    p2.fillRect(0, 0, pm.width(), pm.height(),
				cg.brush(QColorGroup::Highlight));
		    p2.setPen(cg.highlightedText());
		    p2.drawText(0, 0, pm.width(), pm.height(), AlignCenter, title);
		    p2.end();

		    QWMatrix m;
		    m.rotate(270.0);
		    pm = pm.xForm(m);
		    p->drawPixmap(r.x(), r.y(), pm);
		} else {
		    p->fillRect(r, cg.background());
		    p->setPen(cg.mid().dark());
		    p->drawLine(r.right() - 6, r.top() + 2,
				r.right() - 6, r.bottom() - 2);
		    p->drawLine(r.right() - 3, r.top() + 2,
				r.right() - 3, r.bottom() - 2);
		    p->setPen(cg.light());
		    p->drawLine(r.right() - 5, r.top() + 2,
				r.right() - 5, r.bottom() - 2);
		    p->drawLine(r.right() - 2, r.top() + 2,
				r.right() - 2, r.bottom() - 2);
		}
	    } else {
		if (drawTitle) {
		    p->fillRect(r, cg.brush(QColorGroup::Highlight));
		    p->setPen(cg.highlightedText());
		    p->drawText(r, AlignCenter, title);
		} else {
		    p->fillRect(r, cg.background());
		    p->setPen(cg.mid().dark());
		    p->drawLine(r.left() + 2,  r.bottom() - 6,
				r.right() - 2, r.bottom() - 6);
		    p->drawLine(r.left() + 2,  r.bottom() - 3,
				r.right() - 2, r.bottom() - 3);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 2,  r.bottom() - 5,
				r.right() - 2, r.bottom() - 5);
		    p->drawLine(r.left() + 2,  r.bottom() - 2,
				r.right() - 2, r.bottom() - 2);
		}
	    }
	    break;
	}

    case PE_DockWindowSeparator:
	{
	    if (r.width() > 20 || r.height() > 20) {
		if (flags & Style_Horizontal) {
		    p->setPen(cg.mid().dark(120));
		    p->drawLine(r.left() + 1, r.top() + 6, r.left() + 1, r.bottom() - 6);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 2, r.top() + 6, r.left() + 2, r.bottom() - 6);
		} else {
		    p->setPen(cg.mid().dark(120));
		    p->drawLine(r.left() + 6, r.top() + 1, r.right() - 6, r.top() + 1);
		    p->setPen(cg.light());
		    p->drawLine(r.left() + 6, r.top() + 2, r.right() - 6, r.top() + 2);
		}
	    } else
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_Splitter:
	if (flags & Style_Horizontal)
	    flags &= ~Style_Horizontal;
	else
	    flags |= Style_Horizontal;
	// fall through intended

    case PE_DockWindowResizeHandle:
	{
	    p->fillRect(r, cg.background());
	    if (flags & Style_Horizontal) {
		p->setPen(cg.highlight().light());
		p->drawLine(r.left() + 1, r.top() + 1, r.right() - 1, r.top() + 1);
		p->setPen(cg.highlight());
		p->drawLine(r.left() + 1, r.top() + 2, r.right() - 1, r.top() + 2);
		p->setPen(cg.highlight().dark());
		p->drawLine(r.left() + 1, r.top() + 3, r.right() - 1, r.top() + 3);
	    } else {
		p->setPen(cg.highlight().light());
		p->drawLine(r.left() + 1, r.top() + 1, r.left() + 1, r.bottom() - 1);
		p->setPen(cg.highlight());
		p->drawLine(r.left() + 2, r.top() + 1, r.left() + 2, r.bottom() - 1);
		p->setPen(cg.highlight().dark());
		p->drawLine(r.left() + 3, r.top() + 1, r.left() + 3, r.bottom() - 1);
	    }
	    break;
	}

    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_DefaultFrameWidth) : data.lineWidth();

	    if ( ! ( flags & Style_Sunken ) )
		flags |= Style_Raised;
	    if (lw == 2)
		drawLightBevel(p, r, cg, flags);
	    else
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_PanelDockWindow:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_DockWindowFrameWidth) : data.lineWidth();

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags | Style_Raised,
			       &cg.brush(QColorGroup::Button));
	    else
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_PanelMenuBar:
	{
	    int lw = data.isDefault() ?
		     pixelMetric(PM_MenuBarFrameWidth) : data.lineWidth();

	    if (lw == 2)
		drawLightBevel(p, r, cg, flags, &cg.brush(QColorGroup::Button));
	    else
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	    break;
	}

    case PE_ScrollBarSubLine:
	{
	    QRect fr = r, ar = r;;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowLeft;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowUp;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Background));
	    drawPrimitive(pe, p, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarAddLine:
	{
	    QRect fr = r, ar = r;
	    PrimitiveElement pe;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		fr.addCoords(0, 1, 0, 0);
		ar.addCoords(0, 1, 0, 0);
		pe = PE_ArrowRight;
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		fr.addCoords(1, 0, 0, 0);
		ar.addCoords(2, 0, 0, 0);
		pe = PE_ArrowDown;
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Background));
	    drawPrimitive(pe, p, ar, cg, flags);
	    break;
	}

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
	{
	    QRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		fr.addCoords(0, 2, 0, 0);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, 0, 0);
	    }

	    p->fillRect(fr, cg.brush((flags & Style_Down) ?
				     QColorGroup::Midlight :
				     QColorGroup::Mid));
	    break;
	}

    case PE_ScrollBarSlider:
	{
	    QRect fr = r;

	    p->setPen(cg.dark());
	    if (flags & Style_Horizontal) {
		p->drawLine(r.topLeft(), r.topRight());
		p->setPen(cg.background());
		p->drawLine(r.left(), r.top() + 1, r.right(), r.top() + 1);
		fr.addCoords(0, 2, 0, -1);
	    } else {
		p->drawLine(r.topLeft(), r.bottomLeft());
		p->setPen(cg.background());
		p->drawLine(r.left() + 1, r.top(), r.left() + 1, r.bottom());
		fr.addCoords(2, 0, -1, 0);
	    }

	    drawLightBevel(p, fr, cg, ((flags | Style_Down) ^ Style_Down) |
			   ((flags & Style_Enabled) ? Style_Raised : Style_Default),
			   &cg.brush(QColorGroup::Button));
	    break;
	}

    case PE_FocusRect:
	{
	    p->setBrush(NoBrush);
	    if (flags & Style_FocusAtBorder)
		p->setPen(cg.shadow());
	    else
		p->setPen(cg.dark());
	    p->drawRect(r);
	    break;
	}

    case PE_ProgressBarChunk:
	p->fillRect(r.x(), r.y() + 2, r.width(), r.height() - 4, cg.highlight());
	break;

    default:
	if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) {
	    QPointArray a;

	    switch ( pe ) {
	    case PE_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PE_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PE_ArrowRight:
		a.setPoints( 7, -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0 );
		break;

	    case PE_ArrowLeft:
		a.setPoints( 7, 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    p->save();
	    if ( flags & Style_Enabled ) {
		a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
		p->setPen( cg.buttonText() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    } else {
		a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
		p->setPen( cg.light() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
		a.translate( -1, -1 );
		p->setPen( cg.mid() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    }
	    p->restore();
	} else
	    QCommonStyle::drawPrimitive(pe, p, r, cg, flags, data);
	break;
    }
}

void LightStyleV2::drawControl( ControlElement control,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      SFlags flags,
			      const QStyleOption &data ) const
{
    switch (control) {
    case CE_TabBarTab:
	{
	    const QTabBar* tb = static_cast<const QTabBar*>(widget);
	    bool below = false;
	    QRect tr(r);
	    QRect fr(r);

	    tr.addCoords(0, 0,  0, -1);
	    fr.addCoords(2, 2, -2, -2);
	    
	    if ( tb->shape() == QTabBar::RoundedBelow || tb->shape() == QTabBar::TriangularBelow) {
		tr = r; tr.addCoords(0, 1, 0, 0);
		fr = r; fr.addCoords(2, 2,-2, -4);
		below = true;
	    }
		
	    if (! (flags & Style_Selected)) {
		if (below) {
		    tr.addCoords(0, 0, 0, -1);
		    tr.addCoords(0, 0, 0, -1);
		} else {
		    tr.addCoords(0, 1, 0, 0);
		    fr.addCoords(0, 1, 0, 0);
		}

		p->setPen(cg.dark());
		p->drawRect(tr);

		if (tr.left() == 0)
		    if (below) 
			p->drawPoint(tr.left(), tr.top() - 1);
		    else
			p->drawPoint(tr.left(), tr.bottom() + 1);

		p->setPen(cg.light());
		if (below) {
		    p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.left() + 1, tr.bottom() - 2);
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
			    tr.right() - 1, tr.bottom() - 1);
		} else {
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
			    tr.left() + 1, tr.top() + 2);
		    p->drawLine(tr.left() + 1, tr.top() + 1,
			    tr.right() - 1, tr.top() + 1);
		}
		
		if (below) {
		    if (tr.left() == 0)
			p->drawLine(tr.left() + 1, tr.top() - 1,
				    tr.right(), tr.top() - 1);
		    else
		    {
			p->setPen(cg.mid()); //To match lower border of the frame
			p->drawLine(tr.left(), tr.top() - 1,
				    tr.right(), tr.top() - 1);
		    }
		} else {
		    if (tr.left() == 0)
			p->drawLine(tr.left() + 1, tr.bottom() + 1,
				    tr.right(), tr.bottom() + 1);
		    else
			p->drawLine(tr.left(), tr.bottom() + 1,
				    tr.right(), tr.bottom() + 1);
		}
		
		p->setPen(cg.mid());
		
		if (below) {
		    p->drawLine(tr.right() - 1, tr.bottom() - 2,
				tr.right() - 1, tr.top() + 1);
		} else {
		    p->drawLine(tr.right() - 1, tr.top() + 2,
				tr.right() - 1, tr.bottom() - 1);
		}
	    } else {
		p->setPen(cg.dark());
		if (tr.left() == 0)
		    if (below)
			p->drawLine(tr.left(), tr.top() - 1,
				    tr.left(), tr.bottom() - 1);
		    else
			p->drawLine(tr.left(), tr.bottom() + 1,
				    tr.left(), tr.top() + 1);
		else
		    if (below)
			p->drawLine(tr.left(), tr.bottom(),
				    tr.left(), tr.top() + 1);
		    else
			p->drawLine(tr.left(), tr.bottom(),
				    tr.left(), tr.top() + 1);
				    
		if (below) {
		    p->drawLine(tr.left(), tr.bottom(),
				tr.right(), tr.bottom());
		    p->drawLine(tr.right(), tr.bottom() - 1,
				tr.right(), tr.top());

		} else {
		    p->drawLine(tr.left(), tr.top(),
				tr.right(), tr.top());
		    p->drawLine(tr.right(), tr.top() + 1,
				tr.right(), tr.bottom());
		}

		p->setPen(cg.light());
		if (tr.left() == 0)
		    if (below)
			p->drawLine(tr.left() + 1, tr.top() - 2,
				    tr.left() + 1, tr.bottom() - 2);
		    else
			p->drawLine(tr.left() + 1, tr.bottom() + 2,
				    tr.left() + 1, tr.top() + 2);
		else {
		    if (below) {
			p->drawLine(tr.left() + 1, tr.top(),
				    tr.left() + 1, tr.bottom() - 2);
			p->drawPoint(tr.left(), tr.top() - 1);

		    } else {
			p->drawLine(tr.left() + 1, tr.bottom(),
				    tr.left() + 1, tr.top() + 2);
			p->drawPoint(tr.left(), tr.bottom() + 1);
		    }
		}
		
		if (below) {
		    p->drawLine(tr.left() + 1, tr.bottom() - 1,
				tr.right() - 1, tr.bottom() - 1);
		    p->drawPoint(tr.right(), tr.top() - 1);

		    p->setPen(cg.mid());
		    p->drawLine(tr.right() - 1, tr.bottom() - 2,
				tr.right() - 1, tr.top());
		} else {
		    p->drawLine(tr.left() + 1, tr.top() + 1,
				tr.right() - 1, tr.top() + 1);
		    p->drawPoint(tr.right(), tr.bottom() + 1);

		    p->setPen(cg.mid());
		    p->drawLine(tr.right() - 1, tr.top() + 2,
				tr.right() - 1, tr.bottom());
		}
	    }

	    p->fillRect(fr, ((flags & Style_Selected) ?
			     cg.background() : cg.mid()));
	    break;
	}

    case CE_PopupMenuItem:
	{
	    if (! widget || data.isDefault())
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = data.menuItem();
	    int tab = data.tabWidth();
	    int maxpmw = data.maxIconWidth();

	    if ( mi && mi->isSeparator() ) {
		// draw separator
		p->fillRect(r, cg.brush(QColorGroup::Button));
		p->setPen(cg.mid().dark(120));
		p->drawLine(r.left() + 12,  r.top() + 1,
			    r.right() - 12, r.top() + 1);
		p->setPen(cg.light());
		p->drawLine(r.left() + 12,  r.top() + 2,
			    r.right() - 12, r.top() + 2);
		break;
	    }

	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, TRUE, 1,
				&cg.brush(QColorGroup::Midlight));
	    else
		p->fillRect(r, cg.brush(QColorGroup::Button));

	    if ( !mi )
		break;

	    maxpmw = QMAX(maxpmw, 16);

	    QRect cr, ir, tr, sr;
	    // check column
	    cr.setRect(r.left(), r.top(), maxpmw, r.height());
	    // submenu indicator column
	    sr.setCoords(r.right() - maxpmw, r.top(), r.right(), r.bottom());
	    // tab/accelerator column
	    tr.setCoords(sr.left() - tab - 4, r.top(), sr.left(), r.bottom());
	    // item column
	    ir.setCoords(cr.right() + 4, r.top(), tr.right() - 4, r.bottom());

	    bool reverse = QApplication::reverseLayout();
	    if ( reverse ) {
		cr = visualRect( cr, r );
		sr = visualRect( sr, r );
		tr = visualRect( tr, r );
		ir = visualRect( ir, r );
	    }

	    if (mi->isChecked() &&
		! (flags & Style_Active) &
		(flags & Style_Enabled))
		qDrawShadePanel(p, cr, cg, TRUE, 1, &cg.brush(QColorGroup::Midlight));

	    if (mi->iconSet()) {
		QIconSet::Mode mode =
		    (flags & Style_Enabled) ? QIconSet::Normal : QIconSet::Disabled;
		if ((flags & Style_Active) && (flags & Style_Enabled))
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if (popupmenu->isCheckable() && mi->isChecked())
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap =
			mi->iconSet()->pixmap( QIconSet::Small, mode );
		QRect pmr(QPoint(0, 0), pixmap.size());
		pmr.moveCenter(cr.center());
		p->setPen(cg.text());
		p->drawPixmap(pmr.topLeft(), pixmap);
	    } else if (popupmenu->isCheckable() && mi->isChecked())
		drawPrimitive(PE_CheckMark, p, cr, cg,
			      (flags & Style_Enabled) | Style_On);

	    QColor textcolor;
	    QColor embosscolor;
	    if (flags & Style_Active) {
		if (! (flags & Style_Enabled))
		    textcolor = cg.midlight().dark();
		else
		    textcolor = cg.buttonText();
		embosscolor = cg.midlight().light();
	    } else if (! (flags & Style_Enabled)) {
		textcolor = cg.text();
		embosscolor = cg.light();
	    } else
		textcolor = embosscolor = cg.buttonText();
	    p->setPen(textcolor);

	    if (mi->custom()) {
		p->save();
		if (! (flags & Style_Enabled)) {
		    p->setPen(cg.light());
		    mi->custom()->paint(p, cg, flags & Style_Active,
					flags & Style_Enabled,
					ir.x() + 1, ir.y() + 1,
					ir.width() - 1, ir.height() - 1);
		    p->setPen(textcolor);
		}
		mi->custom()->paint(p, cg, flags & Style_Active,
				    flags & Style_Enabled,
				    ir.x(), ir.y(),
				    ir.width(), ir.height());
		p->restore();
	    }

	    QString text = mi->text();
	    if (! text.isNull()) {
		int t = text.find('\t');

		// draw accelerator/tab-text
		if (t >= 0) {
		    int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		    alignFlag |= ( reverse ? AlignLeft : AlignRight );
		    if (! (flags & Style_Enabled)) {
			p->setPen(embosscolor);
			tr.moveBy(1, 1);
			p->drawText(tr, alignFlag, text.mid(t + 1));
			tr.moveBy(-1, -1);
			p->setPen(textcolor);
		    }

		    p->drawText(tr, alignFlag, text.mid(t + 1));
		}

		int alignFlag = AlignVCenter | ShowPrefix | DontClip | SingleLine;
		alignFlag |= ( reverse ? AlignRight : AlignLeft );

		if (! (flags & Style_Enabled)) {
		    p->setPen(embosscolor);
		    ir.moveBy(1, 1);
		    p->drawText(ir, alignFlag, text, t);
		    ir.moveBy(-1, -1);
		    p->setPen(textcolor);
		}

		p->drawText(ir, alignFlag, text, t);
	    } else if (mi->pixmap()) {
		QPixmap pixmap = *mi->pixmap();
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(OpaqueMode);
		p->drawPixmap(ir.x(), (ir.height() - pixmap.height()) / 2, pixmap);
		if (pixmap.depth() == 1)
		    p->setBackgroundMode(TransparentMode);
	    }

	    if (mi->popup())
		drawPrimitive( (reverse ? PE_ArrowLeft : PE_ArrowRight), p, sr, cg, flags);
	    break;
	}
	
    case CE_MenuBarEmptyArea:
	{
	    p->fillRect(r, cg.brush(QColorGroup::Button));
	    break;
	}
	
    case CE_DockWindowEmptyArea:
	{
	    p->fillRect(r, cg.brush(QColorGroup::Button));
	    break;
	}


    case CE_MenuBarItem:
	{
	    if (flags & Style_Active)
		qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Midlight));
	    else
		p->fillRect(r, cg.brush(QColorGroup::Button));

	    if (data.isDefault())
		break;

	    QMenuItem *mi = data.menuItem();
	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
		     flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		     &cg.buttonText());
	    break;
	}

    case CE_ProgressBarGroove:
	drawLightBevel(p, r, cg, Style_Sunken, &cg.brush(QColorGroup::Background));
	break;

    default:
	QCommonStyle::drawControl(control, p, widget, r, cg, flags, data);
	break;
    }
}

void LightStyleV2::drawControlMask( ControlElement control,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QStyleOption &data ) const
{
    switch (control) {
    case CE_PushButton:
	p->fillRect(r, color1);
	break;

    default:
	QCommonStyle::drawControlMask(control, p, widget, r, data);
	break;
    }
}

QRect LightStyleV2::subRect(SubRect subrect, const QWidget *widget) const
{
    QRect rect, wrect(widget->rect());

    switch (subrect) {
    case SR_PushButtonFocusRect:
 	{
 	    const QPushButton *button = (const QPushButton *) widget;
 	    int dbw1 = 0, dbw2 = 0;
 	    if (button->isDefault() || button->autoDefault()) {
 		dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
 		dbw2 = dbw1 * 2;
 	    }

 	    rect.setRect(wrect.x()      + 3 + dbw1,
 			 wrect.y()      + 3 + dbw1,
 			 wrect.width()  - 6 - dbw2,
 			 wrect.height() - 6 - dbw2);
 	    break;
 	}

    default:
	rect = QCommonStyle::subRect(subrect, widget);
    }

    return rect;
}

void LightStyleV2::drawComplexControl( ComplexControl control,
				     QPainter* p,
				     const QWidget* widget,
				     const QRect& r,
				     const QColorGroup& cg,
				     SFlags flags,
				     SCFlags controls,
				     SCFlags active,
				     const QStyleOption &data ) const
{
    switch (control) {
    case CC_ComboBox:
	{
	    const QComboBox *combobox = (const QComboBox *) widget;
	    QRect frame, arrow, field;
	    frame =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxFrame, data),
				   widget);
	    arrow =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxArrow, data),
				   widget);
	    field =
		QStyle::visualRect(querySubControlMetrics(CC_ComboBox, widget,
							  SC_ComboBoxEditField, data),
				   widget);

	    if ((controls & SC_ComboBoxFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Raised,
			       &cg.brush(QColorGroup::Button));

	    if ((controls & SC_ComboBoxArrow) && arrow.isValid()) {
		if (active == SC_ComboBoxArrow)
		    p->fillRect(arrow, cg.brush(QColorGroup::Mid));
		arrow.addCoords(4, 2, -2, -2);
		drawPrimitive(PE_ArrowDown, p, arrow, cg, flags);
	    }

	    if ((controls & SC_ComboBoxEditField) && field.isValid()) {
		p->setPen(cg.dark());
		if (combobox->editable()) {
		    field.addCoords(-1, -1, 1, 1);
		    p->drawRect(field);
		} else
		    p->drawLine(field.right() + 1, field.top(),
				field.right() + 1, field.bottom());

		if (flags & Style_HasFocus) {
		    if (! combobox->editable()) {
			p->fillRect( field, cg.brush( QColorGroup::Highlight ) );
			QRect fr =
			    QStyle::visualRect( subRect( SR_ComboBoxFocusRect, widget ),
						widget );
			drawPrimitive( PE_FocusRect, p, fr, cg,
				       flags | Style_FocusAtBorder,
				       QStyleOption(cg.highlight()));
		    }

		    p->setPen(cg.highlightedText());
		} else
		    p->setPen(cg.buttonText());
	    }

	    break;
	}

    case CC_SpinWidget:
	{
	    const QSpinWidget *spinwidget = (const QSpinWidget *) widget;
	    QRect frame, up, down;

	    frame = querySubControlMetrics(CC_SpinWidget, widget,
					   SC_SpinWidgetFrame, data);
	    up = spinwidget->upRect();
	    down = spinwidget->downRect();

	    if ((controls & SC_SpinWidgetFrame) && frame.isValid())
		drawLightBevel(p, frame, cg, flags | Style_Sunken,
			       &cg.brush(QColorGroup::Base));

	    if ((controls & SC_SpinWidgetUp) && up.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetUp;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetPlus;

		p->setPen(cg.dark());
		p->drawLine(up.topLeft(), up.bottomLeft());

		up.addCoords(1, 0, 0, 0);
		p->fillRect(up, cg.brush(QColorGroup::Button));
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(up.left(), up.top(),
			    up.right() - 1, up.top());
		p->drawLine(up.left(), up.top() + 1,
			    up.left(), up.bottom() - 1);
		if (active == SC_SpinWidgetUp)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(up.right(), up.top(),
			    up.right(), up.bottom());
		p->drawLine(up.left(), up.bottom(),
			    up.right() - 1, up.bottom());

		up.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, up, cg, flags |
			      ((active == SC_SpinWidgetUp) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    if ((controls & SC_SpinWidgetDown) && down.isValid()) {
		PrimitiveElement pe = PE_SpinWidgetDown;
		if ( spinwidget->buttonSymbols() == QSpinWidget::PlusMinus )
		    pe = PE_SpinWidgetMinus;

		p->setPen(cg.dark());
		p->drawLine(down.topLeft(), down.bottomLeft());

		down.addCoords(1, 0, 0, 0);
		p->fillRect(down, cg.brush(QColorGroup::Button));
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.mid());
		else
		    p->setPen(cg.light());
		p->drawLine(down.left(), down.top(),
			    down.right() - 1, down.top());
		p->drawLine(down.left(), down.top() + 1,
			    down.left(), down.bottom() - 1);
		if (active == SC_SpinWidgetDown)
		    p->setPen(cg.light());
		else
		    p->setPen(cg.mid());
		p->drawLine(down.right(), down.top(),
			    down.right(), down.bottom());
		p->drawLine(down.left(), down.bottom(),
			    down.right() - 1, down.bottom());

		down.addCoords(1, 0, 0, 0);
		drawPrimitive(pe, p, down, cg, flags |
			      ((active == SC_SpinWidgetDown) ?
			       Style_On | Style_Sunken : Style_Raised));
	    }

	    break;
	}

    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, subline2, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, data);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, data);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, data);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, data);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  data);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   data);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    data);

	    subline2 = addline;
	    if (scrollbar->orientation() == Qt::Horizontal)
		subline2.moveBy(-addline.width(), 0);
	    else
		subline2.moveBy(0, -addline.height());

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
		drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubLine) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		if (subline2.isValid())
		    drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg,
				  Style_Enabled | ((active == SC_ScrollBarSubLine) ?
						   Style_Down : Style_Default) |
				  ((scrollbar->orientation() == Qt::Horizontal) ?
				   Style_Horizontal : 0));
	    }
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			      Style_Enabled | ((active == SC_ScrollBarAddLine) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      Style_Enabled | ((active == SC_ScrollBarSubPage) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      Style_Enabled | ((active == SC_ScrollBarFirst) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      Style_Enabled | ((active == SC_ScrollBarLast) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			      Style_Enabled | ((active == SC_ScrollBarSlider) ?
					       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}

    case CC_Slider:
	{
	    const QSlider *slider = (const QSlider *) widget;
	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  data),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  data);

	    if ((controls & SC_SliderGroove) && groove.isValid()) {
		if (flags & Style_HasFocus)
		    drawPrimitive( PE_FocusRect, p, groove, cg );

		if (slider->orientation() == Qt::Horizontal) {
		    int dh = (groove.height() - 5) / 2;
		    groove.addCoords(0, dh, 0, -dh);
		} else {
		    int dw = (groove.width() - 5) / 2;
		    groove.addCoords(dw, 0, -dw, 0);
		}

		drawLightBevel(p, groove, cg, ((flags | Style_Raised) ^ Style_Raised) |
			       ((flags & Style_Enabled) ? Style_Sunken : Style_Default),
			       &cg.brush(QColorGroup::Midlight));
	    }

	    if ((controls & SC_SliderHandle) && handle.isValid()) {
		drawLightBevel(p, handle, cg, ((flags | Style_Down) ^ Style_Down) |
			       ((flags & Style_Enabled) ? Style_Raised : Style_Default),
			       &cg.brush(QColorGroup::Button));

	    }

	    if (controls & SC_SliderTickmarks)
		QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags,
						 SC_SliderTickmarks, active, data );
	    break;
	}

    case CC_ListView:
	// use the base style for CC_ListView
	singleton->basestyle->drawComplexControl(control, p, widget, r, cg, flags,
						 controls, active, data);
	break;

    default:
	QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags,
					 controls, active, data);
	break;
    }
}

QRect LightStyleV2::querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  const QStyleOption &data ) const
{
    QRect ret;

    switch (control) {
    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    int sliderstart = scrollbar->sliderStart();
	    int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	    int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
			  scrollbar->width() : scrollbar->height()) - (sbextent * 3);
	    int sliderlen;

	    // calculate slider length
	    if (scrollbar->maxValue() != scrollbar->minValue()) {
		uint range = scrollbar->maxValue() - scrollbar->minValue();
		sliderlen = (scrollbar->pageStep() * maxlen) /
			    (range + scrollbar->pageStep());

		int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
		if ( sliderlen < slidermin || range > INT_MAX / 2 )
		    sliderlen = slidermin;
		if ( sliderlen > maxlen )
		    sliderlen = maxlen;
	    } else
		sliderlen = maxlen;

	    switch (sc) {
	    case SC_ScrollBarSubLine:
		// top/left button
		ret.setRect(0, 0, sbextent, sbextent);
		break;

	    case SC_ScrollBarAddLine:
		// bottom/right button
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(scrollbar->width() - sbextent, 0, sbextent, sbextent);
		else
		    ret.setRect(0, scrollbar->height() - sbextent, sbextent, sbextent);
		break;

	    case SC_ScrollBarSubPage:
		// between top/left button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
		else
		    ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
		break;

	    case SC_ScrollBarAddPage:
		// between bottom/right button and slider
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart + sliderlen, 0,
				maxlen - sliderstart - sliderlen + sbextent, sbextent);
		else
		    ret.setRect(0, sliderstart + sliderlen,
				sbextent, maxlen - sliderstart - sliderlen + sbextent);
		break;

	    case SC_ScrollBarGroove:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sbextent, 0, scrollbar->width() - sbextent * 3,
				scrollbar->height());
		else
		    ret.setRect(0, sbextent, scrollbar->width(),
				scrollbar->height() - sbextent * 3);
		break;

	    case SC_ScrollBarSlider:
		if (scrollbar->orientation() == Qt::Horizontal)
		    ret.setRect(sliderstart, 0, sliderlen, sbextent);
		else
		    ret.setRect(0, sliderstart, sbextent, sliderlen);
		break;

	    default:
		break;
	    }

	    break;
	}

    default:
	ret = QCommonStyle::querySubControlMetrics(control, widget, sc, data);
	break;
    }

    return ret;
}

QStyle::SubControl LightStyleV2::querySubControl( ComplexControl control,
						const QWidget *widget,
						const QPoint &pos,
						const QStyleOption &data ) const
{
    QStyle::SubControl ret = QCommonStyle::querySubControl(control, widget, pos, data);

    // this is an ugly hack, but i really don't care, it's the quickest way to
    // enabled the third button
    if (control == CC_ScrollBar &&
	ret == SC_None)
	ret = SC_ScrollBarSubLine;

    return ret;
}

int LightStyleV2::pixelMetric( PixelMetric metric,
			     const QWidget *widget ) const
{
    int ret;

    switch (metric) {
    case PM_ButtonMargin:
	ret = 4;
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_ButtonDefaultIndicator:
    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 13;
	break;

    case PM_TabBarTabOverlap:
	ret = 0;
	break;

    case PM_ScrollBarExtent:
    case PM_ScrollBarSliderMin:
	ret = 14;
	break;

    case PM_MenuBarFrameWidth:
	ret = 1;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 2;
	break;

    case PM_DockWindowSeparatorExtent:
	ret = 4;
	break;
	
    case PM_SplitterWidth:
	ret = 6;
	break;


    case PM_SliderLength:
    case PM_SliderControlThickness:
	ret = singleton->basestyle->pixelMetric( metric, widget );
	break;

    case PM_MaximumDragDistance:
	ret = -1;
	break;

    default:
	ret = QCommonStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}

QSize LightStyleV2::sizeFromContents( ContentsType contents,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    const QStyleOption &data ) const
{
    QSize ret;

    switch (contents) {
    case CT_PushButton:
	{
	    const QPushButton *button = (const QPushButton *) widget;
	    ret = QCommonStyle::sizeFromContents( contents, widget, contentsSize, data );
	    int w = ret.width(), h = ret.height();

	    // only expand the button if we are displaying text...
	    if ( ! button->pixmap() ) {
		if ( button->isDefault() || button->autoDefault() ) {
		    // default button minimum size
		    if ( w < 80 )
			w = 80;
		    if ( h < 25 )
			h = 25;
		} else {
		    // regular button minimum size
		    if ( w < 76 )
			w = 76;
		    if ( h < 21 )
			h = 21;
		}
	    }

	    ret = QSize( w, h );
	    break;
	}

    case CT_PopupMenuItem:
	{
	    if (! widget || data.isDefault())
		break;

	    QMenuItem *mi = data.menuItem();
	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    int maxpmw = data.maxIconWidth();
	    int w = contentsSize.width(), h = contentsSize.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan() && h < 22)
		    h = 22;
	    } else if(mi->widget()) {
	    } else if (mi->isSeparator()) {
		w = 10;
		h = 4;
	    } else {
		// check is at least 16x16
		if (h < 16)
		    h = 16;
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height());
		else if (! mi->text().isNull())
		    h = QMAX(h, popupmenu->fontMetrics().height() + 2);
		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height());
		h += 2;
	    }

	    // check | 4 pixels | item | 8 pixels | accel | 4 pixels | check

	    // check is at least 16x16
	    maxpmw = QMAX(maxpmw, 16);
	    w += (maxpmw * 2) + 8;

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0)
		w += 8;

	    ret = QSize(w, h);
	    break;
	}

    default:
	ret = QCommonStyle::sizeFromContents(contents, widget, contentsSize, data);
	break;
    }

    return ret;
}

int LightStyleV2::styleHint( StyleHint stylehint,
			   const QWidget *widget,
			   const QStyleOption &option,
			   QStyleHintReturn* returnData ) const
{
    int ret;

    switch (stylehint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_PopupMenu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_PopupMenu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
	ret = 1;
	break;

    case SH_MainWindow_SpaceBelowMenuBar:
	ret = 0;
	break;

    default:
	ret = QCommonStyle::styleHint(stylehint, widget, option, returnData);
	break;
    }

    return ret;
}

QPixmap LightStyleV2::stylePixmap( StylePixmap stylepixmap,
				   const QWidget *widget,
				   const QStyleOption &data ) const
{
    return singleton->basestyle->stylePixmap( stylepixmap, widget, data );
}
#include "lightstyle-v2.moc"
