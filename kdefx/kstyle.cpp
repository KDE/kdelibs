/*
 * $Id$
 * 
 * KStyle
 * Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *
 * Many thanks to Bradley T. Hughes for the 3 button scrollbar code.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qapplication.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qscrollbar.h>
#include <qsettings.h>
#include <qslider.h>
#include <qstylefactory.h>
#include <qtabbar.h>
#include <qtoolbar.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include "kstyle.h"

#ifdef HAVE_XRENDER
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
extern bool qt_use_xrender;
#endif


// INTERNAL
enum TransparencyEngine { 
	Disabled = 0, 
	SoftwareTint, 
	SoftwareBlend, 
	XRender
};

class TransparencyHandler : public QObject
{
	public:
		TransparencyHandler(KStyle* style, TransparencyEngine tEngine, float menuOpacity);
		~TransparencyHandler();
		bool eventFilter(QObject* object, QEvent* event);

	protected:
		void blendToColor(const QColor &col);
		void blendToPixmap(const QColorGroup &cg);
#ifdef HAVE_XRENDER
		void XRenderBlendToPixmap(const QPopupMenu* p);
#endif
	private:
		float   opacity;
		QPixmap pix;
		KStyle* kstyle;
		TransparencyEngine te;
};


struct KStyle::Private
{
	bool  highcolor                : 1;
	bool  useFilledFrameWorkaround : 1;
	bool  etchDisabledText         : 1;
	bool  menuAltKeyNavigation     : 1;
	int   popupMenuDelay;
	float menuOpacity;

	TransparencyEngine   transparencyEngine;
	KStyleScrollBarType  scrollbarType;
	TransparencyHandler* menuHandler;
	QStyle* winstyle;		// ### REMOVE
	KStyleFlags flags;
};

// -----------------------------------------------------------------------------


KStyle::KStyle( KStyleFlags flags, KStyleScrollBarType sbtype )
	: QCommonStyle(), d(new Private)
{
	d->flags = flags;
	bool useMenuTransparency    = (flags & AllowMenuTransparency);
	d->useFilledFrameWorkaround = (flags & FilledFrameWorkaround);
	d->scrollbarType = sbtype;
	d->highcolor = QPixmap::defaultDepth() > 8;

	// Read style settings
	QSettings settings;
	d->popupMenuDelay       = settings.readNumEntry ("/KStyle/Settings/PopupMenuDelay", 256);
	d->etchDisabledText     = settings.readBoolEntry("/KStyle/Settings/EtchDisabledText", true);
	d->menuAltKeyNavigation = settings.readBoolEntry("/KStyle/Settings/MenuAltKeyNavigation", true);
	d->menuHandler = NULL;

//	if (d->highcolor && useMenuTransparency) {
	if (useMenuTransparency) {
		QString effectEngine = settings.readEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled");

#ifdef HAVE_XRENDER
		if (effectEngine == "XRender")
			d->transparencyEngine = XRender;
#else
		if (effectEngine == "XRender")
			d->transparencyEngine = SoftwareBlend;
#endif
		else if (effectEngine == "SoftwareBlend")
			d->transparencyEngine = SoftwareBlend;
		else if (effectEngine == "SoftwareTint")
			d->transparencyEngine = SoftwareTint;
		else
			d->transparencyEngine = Disabled;

		if (d->transparencyEngine != Disabled) {
			// Create an instance of the menu transparency handler
			d->menuOpacity = settings.readDoubleEntry("/KStyle/Settings/MenuOpacity", 0.90);
			d->menuHandler = new TransparencyHandler(this, d->transparencyEngine, d->menuOpacity);
		}
	}

	// ### Remove this ugly dependency!!!
	d->winstyle = QStyleFactory::create("Windows");
	if (!d->winstyle)
		d->winstyle = QStyleFactory::create( *(QStyleFactory::keys().begin()) );
}


KStyle::~KStyle()
{
	if (d->winstyle)
		delete d->winstyle;
	if (d->menuHandler)
		delete d->menuHandler;

	d->menuHandler = NULL;
	d->winstyle    = NULL;
	delete d;
}


void KStyle::polish( QWidget* widget )
{
	if ( d->useFilledFrameWorkaround )
	{
		if (widget->inherits("QToolBar"))
			widget->installEventFilter(this);
		if (widget->inherits("QMenuBar"))
			widget->installEventFilter(this);
	}
}


void KStyle::unPolish( QWidget* widget )
{
	if ( d->useFilledFrameWorkaround )
	{
		if (widget->inherits("QMenuBar"))
			widget->removeEventFilter(this);
		if (widget->inherits("QToolBar"))
			widget->removeEventFilter(this);
	}
}


// Style changes (should) always re-polish popups.
void KStyle::polishPopupMenu( QPopupMenu* p )
{
	if (!p->testWState( WState_Polished ))
		p->setCheckable(true);

	// Install transparency handler if the effect is enabled.
	if ( d->menuHandler && 
		(strcmp(p->name(), "tear off menu") != 0))
			p->installEventFilter(d->menuHandler);
}


// -----------------------------------------------------------------------------
// KStyle extensions
// -----------------------------------------------------------------------------

void KStyle::setScrollBarType(KStyleScrollBarType sbtype)
{
	d->scrollbarType = sbtype;
}

KStyle::KStyleFlags KStyle::styleFlags() const
{
	return d->flags;
}

void KStyle::renderMenuBlendPixmap( KPixmap &pix, const QColorGroup &cg ) const
{
	pix.fill(cg.button());	// Just tint as the default behaviour
}


void KStyle::drawKStylePrimitive( KStylePrimitive kpe,
								  QPainter* p,
								  const QWidget* widget,
								  const QRect &r,
								  const QColorGroup &cg,
								  SFlags flags,
								  const QStyleOption& /* opt */ ) const
{
	switch( kpe )
	{
		// Dock / Toolbar / General handles.
		// ---------------------------------

		case KPE_DockWindowHandle: {

			// Draws a nice DockWindow handle including the dock title.
			QWidget* wid = const_cast<QWidget*>(widget);
			bool horizontal = flags & Style_Horizontal;
			int x,y,w,h,x2,y2;

			if ((wid->width() <= 2) || (wid->height() <= 2)) {
				p->fillRect(r, cg.highlight());
				return;
			}

			r.rect( &x, &y, &w, &h );
			x2 = x + w - 1;
			y2 = y + h - 1;

			QFont fnt;
			fnt = QApplication::font(wid);
			fnt.setPointSize( fnt.pointSize()-2 );

			// Draw the item on an off-screen pixmap
			// to preserve Xft antialiasing for
			// vertically oriented handles.
			QPixmap pix;
			if (horizontal)
				pix.resize( h-2, w-2 );
			else
				pix.resize( w-2, h-2 );

			QString title = wid->parentWidget()->caption();
			QPainter p2;
			p2.begin(&pix);
			p2.fillRect(pix.rect(), cg.brush(QColorGroup::Highlight));
			p2.setPen(cg.highlightedText());
			p2.setFont(fnt);
			p2.drawText(pix.rect(), AlignCenter, title);
			p2.end();

			// Draw a sunken bevel
			p->setPen(cg.dark());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y2, x2, y2);
			p->drawLine(x2, y+1, x2, y2);

			if (horizontal) {
				QWMatrix m;
				m.rotate(-90.0);
				QPixmap vpix = pix.xForm(m);
				bitBlt(wid, r.x()+1, r.y()+1, &vpix);
			} else
				bitBlt(wid, r.x()+1, r.y()+1, &pix);

			break;
		}

		// Reimplement the other primitives in your styles.
		// The current implementation just paints something visibly different.
		case KPE_ToolBarHandle:
		case KPE_GeneralHandle:
		case KPE_SliderHandle:
			p->fillRect(r, cg.light());	
			break;

		case KPE_SliderGroove:
			p->fillRect(r, cg.dark());
			break;

		default:
			p->fillRect(r, Qt::yellow);	// Something really bad happened - highlight.
			break;
	}
}

// -----------------------------------------------------------------------------

void KStyle::drawPrimitive( PrimitiveElement pe,
							QPainter* p,
							const QRect &r,
							const QColorGroup &cg,
							SFlags flags,
							const QStyleOption& opt ) const
{
	// TOOLBAR/DOCK WINDOW HANDLE
	// ------------------------------------------------------------------------
	if (pe == PE_DockWindowHandle) 
	{
		// Wild workarounds are here. Beware.
		QWidget *widget, *parent;

		if (p && p->device()->devType() == QInternal::Widget) {
			widget = dynamic_cast<QWidget*>(p->device());
			parent = widget->parentWidget();
		} else
			return;		// Don't paint on non-widgets

		// Check if we are a normal toolbar or a hidden dockwidget.
		if ( parent &&
			(parent->inherits("QToolBar") ||		// Normal toolbar
			(parent->inherits("QMainWindow")) ))	// Collapsed dock

			// Draw a toolbar handle
			drawKStylePrimitive( KPE_ToolBarHandle, p, widget, r, cg, flags, opt );

		else if ( widget->inherits("QDockWindowHandle") )

			// Draw a dock window handle
			drawKStylePrimitive( KPE_DockWindowHandle, p, widget, r, cg, flags, opt );

		else 
			// General handle, probably a kicker applet handle.
			drawKStylePrimitive( KPE_GeneralHandle, p, widget, r, cg, flags, opt );

	} else
		QCommonStyle::drawPrimitive( pe, p, r, cg, flags, opt );
}



void KStyle::drawControl( ControlElement element,
						  QPainter* p,
						  const QWidget* widget,
						  const QRect &r,
						  const QColorGroup &cg,
						  SFlags flags,
						  const QStyleOption &opt ) const
{
	switch (element)
	{
		// TABS
		// ------------------------------------------------------------------------
		case CE_TabBarTab: {
			const QTabBar* tb  = (const QTabBar*) widget;
			QTabBar::Shape tbs = tb->shape();
			bool selected      = flags & Style_Selected;
			int x = r.x(), y=r.y(), bottom=r.bottom(), right=r.right();

			switch (tbs) {
				
				case QTabBar::RoundedAbove: {
					if (!selected) 
						p->translate(0,1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, y+4, x, bottom);
					p->drawLine(x, y+4, x+4, y);
					p->drawLine(x+4, y, right-1, y);
					if (selected) 
						p->setPen(cg.shadow());
					p->drawLine(right, y+1, right, bottom);

					p->setPen(cg.midlight());
					p->drawLine(x+1, y+4, x+1, bottom);
					p->drawLine(x+1, y+4, x+4, y+1);
					p->drawLine(x+5, y+1, right-2, y+1);

					if (selected) {
						p->setPen(cg.mid());
						p->drawLine(right-1, y+1, right-1, bottom);
					} else {
						p->setPen(cg.mid());
						p->drawPoint(right-1, y+1);
						p->drawLine(x+4, y+2, right-1, y+2);
						p->drawLine(x+3, y+3, right-1, y+3);
						p->fillRect(x+2, y+4, r.width()-3, r.height()-6, cg.mid());
		
						p->setPen(cg.light());
						p->drawLine(x, bottom-1, right, bottom-1);
						p->translate(0,-1);
					}
					break;
				}
				
				case QTabBar::RoundedBelow: {
					if (!selected) 
						p->translate(0,-1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, bottom-4, x, y);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(x, bottom-4, x+4, bottom);
					if (selected)
						p->setPen(cg.shadow());
					p->drawLine(x+4, bottom, right-1, bottom);
					p->drawLine(right, bottom-1, right, y);

					p->setPen(cg.midlight());
					p->drawLine(x+1, bottom-4, x+1, y);
					p->drawLine(x+1, bottom-4, x+4, bottom-1);
					p->drawLine(x+5, bottom-1, right-2, bottom-1);

					if (selected) {
						p->setPen(cg.mid());
						p->drawLine(right-1, y, right-1, bottom-1);
					} else {
						p->setPen(cg.mid());
						p->drawPoint(right-1, bottom-1);
						p->drawLine(x+4, bottom-2, right-1, bottom-2);
						p->drawLine(x+3, bottom-3, right-1, bottom-3);
						p->fillRect(x+2, y+2, r.width()-3, r.height()-6, cg.mid());
						p->translate(0,1);
						p->setPen(cg.dark());
						p->drawLine(x, y, right, y);
					}
					break;
				}
				
				case QTabBar::TriangularAbove: {
					if (!selected) 
						p->translate(0,1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, bottom, x, y+6);
					p->drawLine(x, y+6, x+6, y);
					p->drawLine(x+6, y, right-6, y);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(right-5, y+1, right-1, y+5); 
					p->setPen(cg.shadow());
					p->drawLine(right, y+6, right, bottom);	

					p->setPen(cg.midlight());
					p->drawLine(x+1, bottom, x+1, y+6);
					p->drawLine(x+1, y+6, x+6, y+1);
					p->drawLine(x+6, y+1, right-6, y+1);
					p->drawLine(right-5, y+2, right-2, y+5);
					p->setPen(cg.mid());
					p->drawLine(right-1, y+6, right-1, bottom);

					QPointArray a(6);
					a.setPoint(0, x+2, bottom);
					a.setPoint(1, x+2, y+7);
					a.setPoint(2, x+7, y+2);
					a.setPoint(3, right-7, y+2);
					a.setPoint(4, right-2, y+7);
					a.setPoint(5, right-2, bottom);
					p->setPen  (selected ? cg.background() : cg.mid());
					p->setBrush(selected ? cg.background() : cg.mid());
					p->drawPolygon(a);
					p->setBrush(NoBrush);
					if (!selected) {
						p->translate(0,-1);
						p->setPen(cg.light());
						p->drawLine(x, bottom, right, bottom);
					}
					break;
				}

				default: { // QTabBar::TriangularBelow
					if (!selected) 
						p->translate(0,-1);
					p->setPen(selected ? cg.light() : cg.shadow());
					p->drawLine(x, y, x, bottom-6);
					if (selected)
						p->setPen(cg.mid());
					p->drawLine(x, bottom-6, x+6, bottom);
					if (selected)
						p->setPen(cg.shadow());
					p->drawLine(x+6, bottom, right-6, bottom);
					p->drawLine(right-5, bottom-1, right-1, bottom-5); 
					if (!selected)
						p->setPen(cg.shadow());
					p->drawLine(right, bottom-6, right, y);	

					p->setPen(cg.midlight());
					p->drawLine(x+1, y, x+1, bottom-6);
					p->drawLine(x+1, bottom-6, x+6, bottom-1);
					p->drawLine(x+6, bottom-1, right-6, bottom-1);
					p->drawLine(right-5, bottom-2, right-2, bottom-5);
					p->setPen(cg.mid());
					p->drawLine(right-1, bottom-6, right-1, y);

					QPointArray a(6);
					a.setPoint(0, x+2, y);
					a.setPoint(1, x+2, bottom-7);
					a.setPoint(2, x+7, bottom-2);
					a.setPoint(3, right-7, bottom-2);
					a.setPoint(4, right-2, bottom-7);
					a.setPoint(5, right-2, y);
					p->setPen  (selected ? cg.background() : cg.mid());
					p->setBrush(selected ? cg.background() : cg.mid());
					p->drawPolygon(a);
					p->setBrush(NoBrush);
					if (!selected) {
						p->translate(0,1);
						p->setPen(cg.dark());
						p->drawLine(x, y, right, y);
					}
					break;
				}
			};

			break;
		}


		// PROGRESSBAR
		// ------------------------------------------------------------------------
		case CE_ProgressBarGroove: {
			QRect fr = subRect(SR_ProgressBarGroove, widget);
			drawPrimitive(PE_Panel, p, fr, cg, Style_Sunken, QStyleOption::Default);
			break;
		}

		case CE_ProgressBarContents: {
			// ### Take into account totalSteps() for busy indicator
			const QProgressBar* pb = (const QProgressBar*)widget;
			QRect cr = subRect(SR_ProgressBarContents, widget);
			double progress = pb->progress();
			bool reverse = QApplication::reverseLayout();

			if (!cr.isValid())
				return;

			// Draw progress bar
			if (progress > 0) {
				int steps = pb->totalSteps();
				double pg = progress / steps;
				int width = QMIN(cr.width(), (int)(pg * cr.width()));
	
				// Do fancy gradient for highcolor displays
				if (d->highcolor) {
					QColor c(cg.highlight());
					KPixmap pix;
					pix.resize(cr.width(), cr.height());
					KPixmapEffect::gradient(pix, reverse ? c.light(150) : c.dark(150), 
											reverse ? c.dark(150) : c.light(150),
											KPixmapEffect::HorizontalGradient);
					if (reverse)
						p->drawPixmap(cr.x()+(cr.width()-width), cr.y(), pix, 
									  cr.width()-width, 0, width, cr.height());
					else
						p->drawPixmap(cr.x(), cr.y(), pix, 0, 0, width, cr.height());
				} else 
					if (reverse)
						p->fillRect(cr.x()+(cr.width()-width), cr.y(), width, cr.height(),
									cg.brush(QColorGroup::Highlight));
					else
						p->fillRect(cr.x(), cr.y(), width, cr.height(),
									cg.brush(QColorGroup::Highlight));
			}
			break;
		}

		case CE_ProgressBarLabel: {
			const QProgressBar* pb = (const QProgressBar*)widget;
			QRect cr = subRect(SR_ProgressBarContents, widget);
			double progress = pb->progress();
			bool reverse = QApplication::reverseLayout();

			if (!cr.isValid())
				return;

			QFont font = p->font();
			font.setBold(true);
			p->setFont(font);

			// Draw label
			if (progress > 0) {
				int steps = pb->totalSteps();
				double pg = progress / steps;
				int width = QMIN(cr.width(), (int)(pg * cr.width()));
				QRect crect;
				if (reverse)
					crect.setRect(cr.x()+(cr.width()-width), cr.y(), cr.width(), cr.height());
				else
					crect.setRect(cr.x()+width, cr.y(), cr.width(), cr.height());

				p->save();
				p->setPen(pb->isEnabled() ? (reverse ? cg.text() : cg.highlightedText()) : cg.text());
				p->drawText(r, AlignCenter, pb->progressString());
				p->setClipRect(crect);
				p->setPen(reverse ? cg.highlightedText() : cg.text());
				p->drawText(r, AlignCenter, pb->progressString());
				p->restore();

			} else {
				p->setPen(cg.text());
				p->drawText(r, AlignCenter, pb->progressString());
			}

			break;
		}

		default:
			QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}


QRect KStyle::subRect(SubRect r, const QWidget* widget) const
{
	switch(r)
	{
		// KDE2 look smooth progress bar
		// ------------------------------------------------------------------------
		case SR_ProgressBarGroove:
			return widget->rect();

		case SR_ProgressBarContents:
		case SR_ProgressBarLabel: {
			// ### take into account indicatorFollowsStyle()
			QRect rt = widget->rect();
			return QRect(rt.x()+2, rt.y()+2, rt.width()-4, rt.height()-4);
		}

		default:
			return QCommonStyle::subRect(r, widget);
	}
}


int KStyle::pixelMetric(PixelMetric m, const QWidget* widget) const
{
	switch(m)
	{
		// BUTTONS
		// ------------------------------------------------------------------------
		case PM_ButtonShiftHorizontal:		// Offset by 1
		case PM_ButtonShiftVertical:		// ### Make configurable
			return 1;

		// TABS
		// ------------------------------------------------------------------------
		case PM_TabBarTabHSpace:
			return 24;

		case PM_TabBarTabVSpace: {
			const QTabBar * tb = (const QTabBar *) widget;
			if ( tb->shape() == QTabBar::RoundedAbove ||
				 tb->shape() == QTabBar::RoundedBelow )
				return 10;
			else
				return 4;
		}

		case PM_TabBarTabOverlap: {
			const QTabBar* tb = (const QTabBar*)widget;
			QTabBar::Shape tbs = tb->shape();
			
			if ( (tbs == QTabBar::RoundedAbove) || 
				 (tbs == QTabBar::RoundedBelow) )
				return 0;
			else
				return 2;
		}
		
		// SLIDER
		// ------------------------------------------------------------------------
		case PM_SliderLength:
			return 18;

		case PM_SliderThickness:
			return 24;

		// Determines how much space to leave for the actual non-tickmark 
		// portion of the slider.
		case PM_SliderControlThickness: {
			const QSlider* slider   = (const QSlider*)widget;
			QSlider::TickSetting ts = slider->tickmarks();
			int thickness = (slider->orientation() == Horizontal) ? 
							 slider->height() : slider->width();
			switch (ts) {
				case QSlider::NoMarks:				// Use total area.
					break;
				case QSlider::Both:
					thickness = (thickness/2) + 3;	// Use approx. 1/2 of area.
					break;
				default:							// Use approx. 2/3 of area
					thickness = ((thickness*2)/3) + 3;
					break;
			};
			return thickness;
		}

		// SPLITTER
		// ------------------------------------------------------------------------
		case PM_SplitterWidth:
			if (widget && widget->inherits("QDockWindowResizeHandle"))
				return 8;	// ### why do we need 2pix extra?
			else
				return 6;
		
		// FRAMES
		// ------------------------------------------------------------------------
		case PM_MenuBarFrameWidth:
			return 1;

		case PM_DockWindowFrameWidth:
			return 1;
	
		// PROGRESSBAR
		// ------------------------------------------------------------------------
		case PM_ProgressBarChunkWidth:
			return 1;

		// GENERAL
		// ------------------------------------------------------------------------
		case PM_MaximumDragDistance:
			return -1;
			
		default:
			return QCommonStyle::pixelMetric( m, widget );
	}
}


void KStyle::drawComplexControl( ComplexControl control,
								 QPainter* p,
								 const QWidget* widget,
								 const QRect &r,
								 const QColorGroup &cg,
								 SFlags flags,
								 SCFlags controls,
								 SCFlags active,
								 const QStyleOption &opt ) const
{
	switch(control)
	{
		// 3 BUTTON SCROLLBAR
		// ------------------------------------------------------------------------
		case CC_ScrollBar: {
			// Many thanks to Brad Hughes for contributing this code.
			bool useThreeButtonScrollBar = (d->scrollbarType & ThreeButtonScrollBar);

			const QScrollBar *sb = (const QScrollBar*)widget;
			bool   maxedOut   = (sb->minValue()    == sb->maxValue());
			bool   horizontal = (sb->orientation() == Qt::Horizontal);
			SFlags sflags     = ((horizontal ? Style_Horizontal : Style_Default) |
								 (maxedOut   ? Style_Default : Style_Enabled));

			QRect  addline, subline, subline2, addpage, subpage, slider, first, last;
			subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
			addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
			subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
			addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
			slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
			first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
			last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);
			subline2 = addline;

			if ( useThreeButtonScrollBar )
				if (horizontal)
					subline2.moveBy(-addline.width(), 0);
				else
					subline2.moveBy(0, -addline.height());

			// Draw the up/left button set
			if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
				drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));

				if (useThreeButtonScrollBar && subline2.isValid())
					drawPrimitive(PE_ScrollBarSubLine, p, subline2, cg,
							sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));
			}

			if ((controls & SC_ScrollBarAddLine) && addline.isValid())
				drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
							sflags | ((active == SC_ScrollBarAddLine) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
				drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
							sflags | ((active == SC_ScrollBarSubPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
				drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
							sflags | ((active == SC_ScrollBarAddPage) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarFirst) && first.isValid())
				drawPrimitive(PE_ScrollBarFirst, p, first, cg,
							sflags | ((active == SC_ScrollBarFirst) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarLast) && last.isValid())
				drawPrimitive(PE_ScrollBarLast, p, last, cg,
							sflags | ((active == SC_ScrollBarLast) ?
										Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
				drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
							sflags | ((active == SC_ScrollBarSlider) ?
										Style_Down : Style_Default));
				// Draw focus rect
				if (sb->hasFocus()) {
					QRect fr(slider.x() + 2, slider.y() + 2,
							 slider.width() - 5, slider.height() - 5);
					drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
				}
			}
			break;
		}


		// SLIDER
		// -------------------------------------------------------------------
		case CC_Slider: {
			const QSlider* slider = (const QSlider*)widget;
			QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove, opt);
			QRect handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle, opt);

			// Double-buffer slider for no flicker
			QPixmap pix(widget->size());
			QPainter p2;
			p2.begin(&pix);

			if ( slider->parentWidget() &&
				 slider->parentWidget()->backgroundPixmap() &&
				 !slider->parentWidget()->backgroundPixmap()->isNull() ) {
				QPixmap pixmap = *(slider->parentWidget()->backgroundPixmap());
				p2.drawTiledPixmap(r, pixmap, slider->pos());
			} else
				pix.fill(cg.background());

			// Draw slider groove
			if ((controls & SC_SliderGroove) && groove.isValid()) {
				drawKStylePrimitive( KPE_SliderGroove, &p2, widget, groove, cg, flags, opt );

				// Draw the focus rect around the groove
				if (slider->hasFocus())
					drawPrimitive(PE_FocusRect, &p2, groove, cg);
			}

			// Draw the tickmarks
			if (controls & SC_SliderTickmarks)
				QCommonStyle::drawComplexControl(control, &p2, widget, 
						r, cg, flags, SC_SliderTickmarks, active, opt);

			// Draw the slider handle
			if ((controls & SC_SliderHandle) && handle.isValid()) {
				drawKStylePrimitive( KPE_SliderHandle, &p2, widget, handle, cg, flags, opt );
			}

			p2.end();
			bitBlt((QWidget*)widget, r.x(), r.y(), &pix);
			break;
		}


		default:
			// ### Only needed for CC_ListView if the style has been fully implemented.
			d->winstyle->drawComplexControl( control, p, widget, r, cg,
										  flags, controls, active, opt );
			break;
	}
}


QStyle::SubControl KStyle::querySubControl( ComplexControl control,
											const QWidget* widget,
											const QPoint &pos,
											const QStyleOption &opt ) const
{
	QStyle::SubControl ret = QCommonStyle::querySubControl(control, widget, pos, opt);

	if (d->scrollbarType == ThreeButtonScrollBar) {
		// Enable third button
		if (control == CC_ScrollBar && ret == SC_None)
			ret = SC_ScrollBarSubLine;
	}
	return ret;
}


QRect KStyle::querySubControlMetrics( ComplexControl control,
									  const QWidget* widget,
									  SubControl sc,
									  const QStyleOption &opt ) const
{
    QRect ret;

	if (control == CC_ScrollBar)
	{
		bool threeButtonScrollBar = d->scrollbarType & ThreeButtonScrollBar;
		bool platinumScrollBar    = d->scrollbarType & PlatinumStyleScrollBar;
		bool nextScrollBar        = d->scrollbarType & NextStyleScrollBar;

		const QScrollBar *sb = (const QScrollBar*)widget;
		bool horizontal = sb->orientation() == Qt::Horizontal;
		int sliderstart = sb->sliderStart();
		int sbextent    = pixelMetric(PM_ScrollBarExtent, widget);
		int maxlen      = (horizontal ? sb->width() : sb->height()) 
						  - (sbextent * (threeButtonScrollBar ? 3 : 2));
		int sliderlen;

		// calculate slider length
		if (sb->maxValue() != sb->minValue())
		{
			uint range = sb->maxValue() - sb->minValue();
			sliderlen = (sb->pageStep() * maxlen) /	(range + sb->pageStep());

			int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
			if ( sliderlen < slidermin || range > INT_MAX / 2 )
				sliderlen = slidermin;
			if ( sliderlen > maxlen )
				sliderlen = maxlen;
		} else
			sliderlen = maxlen;

		// Subcontrols
		switch (sc) 
		{
			case SC_ScrollBarSubLine: {
				// top/left button
				if (platinumScrollBar) {
					if (horizontal)
						ret.setRect(sb->width() - 2 * sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, sb->height() - 2 * sbextent, sbextent, sbextent);
				} else
					ret.setRect(0, 0, sbextent, sbextent);
				break;
			}

			case SC_ScrollBarAddLine: {
				// bottom/right button
				if (nextScrollBar) {
					if (horizontal)
						ret.setRect(sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sbextent);
				} else {
					if (horizontal)
						ret.setRect(sb->width() - sbextent, 0, sbextent, sbextent);
					else
						ret.setRect(0, sb->height() - sbextent, sbextent, sbextent);
				}
				break;
			}

			case SC_ScrollBarSubPage: {
				// between top/left button and slider
				if (platinumScrollBar) {
					if (horizontal)
						ret.setRect(0, 0, sliderstart, sbextent);
					else
						ret.setRect(0, 0, sbextent, sliderstart);
				} else if (nextScrollBar) {
					if (horizontal)
						ret.setRect(sbextent*2, 0, sliderstart-2*sbextent, sbextent);
					else
						ret.setRect(0, sbextent*2, sbextent, sliderstart-2*sbextent);
				} else {
					if (horizontal)
						ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
				}
				break;
			}

			case SC_ScrollBarAddPage: {
				// between bottom/right button and slider
				int fudge;

				if (platinumScrollBar) 
					fudge = 0;
				else if (nextScrollBar)
					fudge = 2*sbextent;
				else
					fudge = sbextent;

				if (horizontal)
					ret.setRect(sliderstart + sliderlen, 0,
							maxlen - sliderstart - sliderlen + fudge, sbextent);
				else
					ret.setRect(0, sliderstart + sliderlen, sbextent,
							maxlen - sliderstart - sliderlen + fudge);
				break;
			}

			case SC_ScrollBarGroove: {
				int multi = threeButtonScrollBar ? 3 : 2;
				int fudge;

				if (platinumScrollBar)
					fudge = 0;
				else if (nextScrollBar)
					fudge = 2*sbextent;
				else
					fudge = sbextent;

				if (horizontal)
					ret.setRect(fudge, 0, sb->width() - sbextent * multi, sb->height());
				else
					ret.setRect(0, fudge, sb->width(), sb->height() - sbextent * multi);
				break;
			}

			case SC_ScrollBarSlider: {
				if (horizontal)
					ret.setRect(sliderstart, 0, sliderlen, sbextent);
				else
					ret.setRect(0, sliderstart, sbextent, sliderlen);
				break;
			}

			default:
				ret = QCommonStyle::querySubControlMetrics(control, widget, sc, opt);
				break;
		}
	} else
		ret = QCommonStyle::querySubControlMetrics(control, widget, sc, opt);

	return ret;
}


QPixmap KStyle::stylePixmap( StylePixmap stylepixmap,
						  const QWidget* widget,
						  const QStyleOption& opt) const
{
	// ### Only need new images for the others to use KStyle
	return d->winstyle->stylePixmap(stylepixmap, widget, opt);
}


int KStyle::styleHint( StyleHint sh, const QWidget* w, 
					   const QStyleOption &opt, QStyleHintReturn* shr) const
{
	switch (sh)
	{
		case SH_EtchDisabledText:
			return d->etchDisabledText ? 1 : 0;

		case SH_MenuBar_AltKeyNavigation:
			return d->menuAltKeyNavigation ? 1 : 0;

		case SH_PopupMenu_SubMenuPopupDelay:
			return d->popupMenuDelay;

		case SH_ItemView_ChangeHighlightOnFocus:
		case SH_Slider_SloppyKeyEvents:
		case SH_MainWindow_SpaceBelowMenuBar:
			return 0;

		case SH_Slider_SnapToValue:
		case SH_PrintDialog_RightAlignButtons:
		case SH_FontDialog_SelectAssociatedText:
		case SH_PopupMenu_AllowActiveAndDisabled:
		case SH_MenuBar_MouseTracking:
		case SH_PopupMenu_MouseTracking:
		case SH_ComboBox_ListMouseTracking:
			return 1;

		default:
			return QCommonStyle::styleHint(sh, w, opt, shr);
	}
}


bool KStyle::eventFilter( QObject* object, QEvent* event )
{
	if ( d->useFilledFrameWorkaround )
	{
		QMenuBar* menubar = dynamic_cast<QMenuBar*>(object);
		QToolBar* toolbar = dynamic_cast<QToolBar*>(object);

		// Make the QMenuBar/QToolBar paintEvent() cover a larger area to 
		// ensure that the filled frame contents are properly painted.
		// We essentially modify the paintEvent's rect to include the
		// panel border, which also paints the widget's interior.
		// This is nasty, but I see no other way to properly repaint 
		// filled frames in all QMenuBars and QToolBars.
		// -- Karol.
		if ( menubar || toolbar ) 
		{
			if (event->type() == QEvent::Paint) 
			{
				bool horizontal = true;
				QPaintEvent* pe = (QPaintEvent*)event;
				QFrame* frame   = (QFrame*)object;
				QRect r = pe->rect();

				if (toolbar && toolbar->orientation() == Qt::Vertical)
					horizontal = false;

				if (horizontal) {
					if ( r.height() == frame->height() )
						return false;	// Let QFrame handle the painting now.
			
					// Else, send a new paint event with an updated paint rect.
					QPaintEvent dummyPE( QRect( r.x(), 0, r.width(), frame->height()) );
					QApplication::sendEvent( frame, &dummyPE ); 
				} 
				else {	// Vertical
					if ( r.width() == frame->width() )
						return false;

					QPaintEvent dummyPE( QRect( 0, r.y(), frame->width(), r.height()) );
					QApplication::sendEvent( frame, &dummyPE ); 
				}
			
				// Discard this event as we sent a new paintEvent.
				return true;
			}
		}
	}

	return false;
}


// -----------------------------------------------------------------------------
// I N T E R N A L -  KStyle menu transparency handler
// -----------------------------------------------------------------------------

TransparencyHandler::TransparencyHandler( KStyle* style, 
	TransparencyEngine tEngine, float menuOpacity ) 
	: QObject() 
{ 
	te = tEngine;
	kstyle = style;
	opacity = menuOpacity; 
	pix.setOptimization(QPixmap::BestOptim);
};


TransparencyHandler::~TransparencyHandler() 
{
};


bool TransparencyHandler::eventFilter( QObject* object, QEvent* event )
{
	// Transparency idea was borrowed from KDE2's "MegaGradient" Style,
	// Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>
	QPopupMenu* p = (QPopupMenu*)object;
	QEvent::Type et = event->type();

	if (et == QEvent::Show) 
	{
		pix = QPixmap::grabWindow(qt_xrootwin(),
				p->x(), p->y(), p->width(), p->height());

		switch (te) {
#ifdef HAVE_XRENDER
			case XRender:
				if (qt_use_xrender) {
					XRenderBlendToPixmap(p);
					break;
				}
				// Fall through intended
#else
			case XRender:
#endif
			case SoftwareBlend:
				blendToPixmap(p->colorGroup());
				break;

			case SoftwareTint:
			default:
				blendToColor(p->colorGroup().button());
		};

		p->setErasePixmap(pix);
	} 
	else if (et == QEvent::Hide)
		p->setErasePixmap(QPixmap());

	return false;
}


// Blends a QImage to a predefined color, with a given opacity.
void TransparencyHandler::blendToColor(const QColor &col)
{
	if (opacity < 0.0 || opacity > 1.0)
		return;

	QImage img = pix.convertToImage();
	KImageEffect::blend(col, img, opacity);
	pix.convertFromImage(img);
}


void TransparencyHandler::blendToPixmap(const QColorGroup &cg)
{
	if (opacity < 0.0 || opacity > 1.0)
		return;
			
	KPixmap blendPix;
	blendPix.resize( pix.width(), pix.height() );

	if (blendPix.width()  != pix.width() ||
		blendPix.height() != pix.height())
		return;

	// Allow styles to define the blend pixmap - allows for some interesting effects.
	kstyle->renderMenuBlendPixmap( blendPix, cg );

	QImage blendImg = blendPix.convertToImage();
	QImage backImg  = pix.convertToImage();
	KImageEffect::blend(blendImg, backImg, opacity);
	pix.convertFromImage(backImg);
}


#ifdef HAVE_XRENDER
// Here we go, use XRender in all its glory.
// NOTE: This is actually a bit slower than the above routines
// on non-accelerated displays. -- Karol.
void TransparencyHandler::XRenderBlendToPixmap(const QPopupMenu* p)
{
	KPixmap renderPix;
	renderPix.resize( pix.width(), pix.height() );

	// Allow styles to define the blend pixmap - allows for some interesting effects.
	kstyle->renderMenuBlendPixmap( renderPix, p->colorGroup() );

	Display* dpy = qt_xdisplay();
	Pixmap   alphaPixmap;
	Picture  alphaPicture;
	XRenderPictFormat        Rpf;
	XRenderPictureAttributes Rpa;
	XRenderColor clr;
	clr.alpha = ((unsigned short)(255*opacity) << 8);

	Rpf.type  = PictTypeDirect;
	Rpf.depth = 8;
	Rpf.direct.alphaMask = 0xff;
	Rpa.repeat = True;	// Tile

	XRenderPictFormat* xformat = XRenderFindFormat(dpy,
		PictFormatType | PictFormatDepth | PictFormatAlphaMask, &Rpf, 0);

	alphaPixmap = XCreatePixmap(dpy, p->handle(), 1, 1, 8);
	alphaPicture = XRenderCreatePicture(dpy, alphaPixmap, xformat, CPRepeat, &Rpa);

	XRenderFillRectangle(dpy, PictOpSrc, alphaPicture, &clr, 0, 0, 1, 1);

	XRenderComposite(dpy, PictOpOver, 
			renderPix.x11RenderHandle(), alphaPicture, pix.x11RenderHandle(), // src, mask, dst
			0, 0, 	// srcx,  srcy
			0, 0,	// maskx, masky
			0, 0,	// dstx,  dsty
			pix.width(), pix.height());

	XRenderFreePicture(dpy, alphaPicture);
	XFreePixmap(dpy, alphaPixmap);
}
#endif

// vim: set noet ts=4 sw=4:

#include "kstyle.moc"
