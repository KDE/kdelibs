/*
 * $Id$
 *
 * KDE3 HighColor Style (version 0.0.3a)
 * Copyright (C) 2001, Karol Szwed <gallium@kde.org>
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley  <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller     <mueller@kde.org>
 *           (C) 2001 Martijn Klingens <mklingens@yahoo.com>
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

#include <qstyleplugin.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qvariant.h>
#include <kpixmapeffect.h>
#include <kdrawutil.h>

#include "highcolor.h"
#include "bitmaps.h"


// -- Style Plugin Interface -------------------------
class HighColorStylePlugin : public QStylePlugin
{
	public:
		HighColorStylePlugin() {}
		~HighColorStylePlugin() {}

		QStringList keys() const 
		{ 
			return QStringList() << "HighColor"; 
		}

		QStyle* create( const QString& key ) 
		{ 
			if ( key == "highcolor" ) 
				return new HighColorStyle;
			return 0;
		}
};

Q_EXPORT_PLUGIN( HighColorStylePlugin )
// ---------------------------------------------------


// ### Remove globals
QBitmap lightBmp;
QBitmap grayBmp;
QBitmap dgrayBmp;
QBitmap maskBmp;
QBitmap xBmp;
QIntDict<GradientSet> gDict;


// ---------------------------------------------------------------------------

GradientSet::GradientSet(const QColor &baseColor)
{
	c = baseColor;
	for(int i=0; i < GradientCount; i++)
		gradients[i] = NULL;
}


GradientSet::~GradientSet()
{
	for(int i=0; i < GradientCount; i++)
		if(gradients[i])
			delete gradients[i];
}

					   
KPixmap* GradientSet::gradient(GradientType type)
{
	if (gradients[type]) 
		return gradients[type];
						    
	switch(type)
	{
		case VSmall: {
				gradients[VSmall] = new KPixmap;
				gradients[VSmall]->resize(18, 24);
				KPixmapEffect::gradient(*gradients[VSmall], c.light(110), c.dark(110),
												KPixmapEffect::VerticalGradient);
			}
			break;
	
		case VMed: {
				gradients[VMed] = new KPixmap;
				gradients[VMed]->resize(18, 34);
				KPixmapEffect::gradient(*gradients[VMed], c.light(110), c.dark(110),
												KPixmapEffect::VerticalGradient);
			}
			break;

		case VLarge: {
				gradients[VLarge] = new KPixmap;
				gradients[VLarge]->resize(18, 64);
            KPixmapEffect::gradient(*gradients[VLarge], c.light(110), c.dark(110),
												KPixmapEffect::VerticalGradient);
			}
			break;

		case HMed: {
				gradients[HMed] = new KPixmap;
				gradients[HMed]->resize(34, 18);
				KPixmapEffect::gradient(*gradients[HMed], c.light(110), c.dark(110),
												KPixmapEffect::HorizontalGradient);
			}
			break;

		case HLarge: {
				gradients[HLarge] = new KPixmap;
				gradients[HLarge]->resize(52, 18);
				KPixmapEffect::gradient(*gradients[HLarge], c.light(110), c.dark(110),
												KPixmapEffect::HorizontalGradient);
			}
			break;
		
		default:
			break;
	}
	return(gradients[type]);
}



// ---------------------------------------------------------------------------

HighColorStyle::HighColorStyle() : QWindowsStyle()
{
	highcolor = QPixmap::defaultDepth() > 8;
	gDict.setAutoDelete(true);
}


HighColorStyle::~HighColorStyle()
{
}


void HighColorStyle::polish(QWidget* widget)
{
	QWindowsStyle::polish(widget);

	if (widget->inherits("QMenuBar")) {
		widget->setBackgroundMode(QWidget::NoBackground);
	} 

}


void HighColorStyle::unPolish(QWidget* widget)
{
	if (widget->inherits("QMenuBar")) {
		widget->setBackgroundMode(QWidget::PaletteBackground);
	} 

	QWindowsStyle::unPolish(widget);
}


// This function draws primitive elements as well as their masks.
void HighColorStyle::drawPrimitive( PrimitiveElement pe,
									QPainter *p,
									const QRect &r,
									const QColorGroup &cg,
									SFlags flags = Style_Default,
									const QStyleOption& opt ) const
{
	bool down = flags & Style_Down;
	bool on   = flags & Style_On;

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
			break;		// Override Windows style dark box default indicator

		case PE_ButtonDropDown:
		case PE_ButtonTool: {
			bool sunken = on || down;
			int  x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame (round style)
			p->setPen(cg.shadow());
			p->drawLine(x+1,y,x2-1,y);
			p->drawLine(x,y+1,x,y2-1);
			p->drawLine(x+1,y2,x2-1,y2);
			p->drawLine(x2,y+1,x2,y2-1);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			p->fillRect(x+2, y+2, w-4, h-4, cg.button());

			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand: {
			bool sunken = on || down;
			int  x, y, w, h;
			r.rect(&x, &y, &w, &h);

			if (sunken)
				kDrawBeButton(p, x, y, w, h, cg, true,
							  &cg.brush(QColorGroup::Mid));
			else if (!(flags & (Style_Raised | Style_Sunken)))
				p->fillRect(r, cg.button());
			else if(highcolor) 
			{
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				if(!sunken) {
					p->setPen(cg.light());
					p->drawLine(x+2, y+2, x2-1, y+2);
					p->drawLine(x+2, y+3, x2-2, y+3);
					p->drawLine(x+2, y+4, x+2, y2-1);
					p->drawLine(x+3, y+4, x+3, y2-2);
				} else {
					p->setPen(cg.mid());
					p->drawLine(x+2, y+2, x2-1, y+2);
					p->drawLine(x+2, y+3, x2-2, y+3);
					p->drawLine(x+2, y+4, x+2, y2-1);
					p->drawLine(x+3, y+4, x+3, y2-2);
				}

				p->setPen(sunken? cg.light() : cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);

				p->setPen(cg.mid());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-1);
				p->drawLine(x2-2, y+3, x2-2, y2-2);

				renderGradient(p, QRect(x+4, y+4, w-6, h-6),
							   cg.button(), false);
            } else
				kDrawBeButton(p, x, y, w, h, cg, false,
						  		&cg.brush(QColorGroup::Mid));
			break;
		}

		
		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel:	
		case PE_HeaderSection: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;

			// Outer frame
			p->setPen(cg.shadow());
			p->drawRect(r);

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);

			if (sunken)
				p->fillRect(x+2, y+2, w-4, h-4, cg.background());
			else
				renderGradient( p, QRect(x+2, y+2, w-4, h-4),
							    cg.background(), flags & Style_Horizontal );
			break;
		}


		// SCROLLBAR
		// -------------------------------------------------------------------
		case PE_ScrollBarSlider: {
			// ### Small hack to ensure scrollbar gradients are drawn the right way.
			flags ^= Style_Horizontal;

			drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Enabled | Style_Raised);

			// Draw a scrollbar riffle (note direction after above changes)
			if (flags & Style_Horizontal) {
				if (r.height() >= 15) {
					int x = r.x()+3;
					int y = r.y() + (r.height()-7)/2;
					int x2 = r.right()-3;
					p->setPen(cg.light());
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y+3, x2, y+3);
					p->drawLine(x, y+6, x2, y+6);

					p->setPen(cg.mid());
					p->drawLine(x, y+1, x2, y+1);
					p->drawLine(x, y+4, x2, y+4);
					p->drawLine(x, y+7, x2, y+7);
				}
			} else {
				if (r.width() >= 15) {
					int y = r.y()+3;
					int x = r.x() + (r.width()-7)/2;
					int y2 = r.bottom()-3;
					p->setPen(cg.light());
					p->drawLine(x, y, x, y2);
					p->drawLine(x+3, y, x+3, y2);
					p->drawLine(x+6, y, x+6, y2);

					p->setPen(cg.mid());
					p->drawLine(x+1, y, x+1, y2);
					p->drawLine(x+4, y, x+4, y2);
					p->drawLine(x+7, y, x+7, y2);
				}
			}
			break;
		}

		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			p->setPen(cg.shadow());
			int x, y, w, h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			if (flags & Style_Horizontal) {
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y2, x2, y2);
				renderGradient(p, QRect(x, y+1, w, h-2), 
							   cg.mid(), false);
			} else {
				p->drawLine(x, y, x, y2);
				p->drawLine(x2, y, x2, y2);
				renderGradient(p, QRect(x+1, y, w-2, h),
							   cg.mid(), true);
			}
			break;
		}


		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator: {

			if (xBmp.isNull()) {
				xBmp = QBitmap(7, 7, x_bits, true);
				xBmp.setMask(xBmp);
			}

			int x,y,w,h;
			x=r.x(); y=r.y(); w=r.width(); h=r.height();
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.mid());
			p->drawLine(x, y, x2, y);
			p->drawLine(x, y, x, y2);

			p->setPen(cg.light());
			p->drawLine(x2, y+1, x2, y2);
			p->drawLine(x+1, y2, x2, y2);

			p->setPen(cg.shadow());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1, y2-1);

			p->setPen(cg.midlight());
			p->drawLine(x2-1, y+2, x2-1, y2-1);
			p->drawLine(x+2, y2-1, x2-1, y2-1);

			p->fillRect(x+2, y+2, w-4, h-4, cg.base());

			if (!(flags & Style_Off)) {
				if(on) {
					p->setPen(Qt::black);
					p->drawPixmap(3, 3, xBmp);
				}
				else {
					p->setPen(cg.shadow());
					p->drawRect(x+2, y+2, w-4, h-4);
					p->setPen(Qt::black);
					p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
					p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
					p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
				}
			}
			break;
		}


		// RADIOBUTTON (exclusive indicator)
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicator: {
			if (lightBmp.isNull()) {
				lightBmp = QBitmap(13, 13, radiooff_light_bits, true);
				lightBmp.setMask(lightBmp);
				grayBmp  = QBitmap(13, 13, radiooff_gray_bits,  true);
				dgrayBmp = QBitmap(13, 13, radiooff_dgray_bits, true);
			}

			p->fillRect(r, cg.brush(QColorGroup::Background));
			kColorBitmaps(p, cg, r.x(), r.y(), NULL , &grayBmp,
						  NULL, &dgrayBmp);

			// Draw the lightbits with the base color
			p->setPen(cg.base());
			p->drawPixmap(r.x(), r.y(), lightBmp);

			if(on || down) {
				p->setPen(Qt::black);
				p->drawLine(5, 4, 7, 4);
				p->drawLine(4, 5, 4, 7);
				p->drawLine(5, 8, 7, 8);
				p->drawLine(8, 5, 8, 7);
				p->fillRect(5, 5, 3, 3, Qt::black);
			}

			break;
		}


		// Draw the exclusive indicator (radio button) mask.
		case PE_ExclusiveIndicatorMask: {
			// ### Fixme - the mask isn't exactly the right shape
//			if (maskBmp.isNull())
//				maskBmp = QBitmap(13, 13, radiomask_bits, true);
//			p->setPen(Qt::color1);
//			p->drawPixmap(r.x(), r.y(), maskBmp);

			p->setPen(color1);
			p->setBrush(color1);
			p->drawEllipse(r);
			break;
		}


		// PROGRESSBAR
		// -------------------------------------------------------------------
//		case PE_ProgressBarChunk: {
			// ### KDE 3.0 - Make KDE2-style smooth gradient slider
//			break;
//		}

		// SPLITTER PANEL
		// -------------------------------------------------------------------
		case PE_Splitter: {
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect(x, y, w, h);
			p->setPen(cg.background());
			p->drawPoint(x, y);
			p->drawPoint(x2, y);
			p->drawPoint(x, y2);
			p->drawPoint(x2, y2);
			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x+1, y2-1);
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->setPen(cg.mid());
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->fillRect(x+3, y+3, w-5, h-5, cg.brush(QColorGroup::Background));	
			break;
		}


		// GENERAL PANELS
		// -------------------------------------------------------------------
		case PE_Panel:
		case PE_PanelPopup: {
			bool sunken = flags & Style_Sunken;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
										: opt.lineWidth();
			if (lw == 2 && sunken) 
			{
				QPen oldPen = p->pen();
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.light());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				p->setPen(cg.mid());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->setPen(cg.midlight());
				p->drawLine(x+1, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+1, x2-1, y2-1);
				p->setPen(cg.dark());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2-1);
				p->setPen(oldPen);
			} else
				QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);

			break;
		}


		// MENU / TOOLBAR PANEL
		// -------------------------------------------------------------------
		case PE_PanelMenuBar: 			// Menu
		case PE_PanelDockWindow: {		// Toolbar
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;

			p->setPen(cg.light());
			p->drawLine(r.x(), r.y(), x2-1,  r.y());
			p->drawLine(r.x(), r.y(), r.x(), y2-1);
			p->setPen(cg.dark());
			p->drawLine(r.x(), y2, x2, y2);
			p->drawLine(x2, r.y(), x2, y2);

			// ### Qt should specify Style_Horizontal where appropriate
			renderGradient( p, QRect(r.x()+1, r.y()+1, x2-1, y2-1),
				cg.background(), (r.width() < r.height()) && 
								 (pe != PE_PanelMenuBar) );

			break;
		}


		// TOOLBAR/DOCK WINDOW HANDLE
		// -------------------------------------------------------------------
		case PE_DockWindowHandle: {

			// wild hacks are here. beware.
			QWidget* widget;
			if (p->device()->devType() == QInternal::Widget)
				widget = dynamic_cast<QWidget*>(p->device());
			else
				return;		// Don't paint on non-widgets */
					
			// Check if we are a normal toolbar or a hidden dockwidget.
			if ( widget->parent() && 
			    (widget->parent()->inherits("QToolBar") ||		// Normal toolbar
				(widget->parent()->inherits("QMainWindow")) ))	// Collapsed dock
			{
				// Draw a toolbar handle
				int x = r.x(); int y = r.y();
				int x2 = r.x() + r.width()-1;
				int y2 = r.y() + r.height()-1;

				if (!(flags & Style_Horizontal)) {
					renderGradient( p, r, cg.background(), true);

					p->setPen(cg.light());
					p->drawLine(x+4, y+3, x2-4, y+3);
					p->drawLine(x+4, y+5, x2-4, y+5);
					p->drawLine(x+4, y+7, x2-4, y+7);

					p->setPen(cg.mid());
					p->drawLine(x+4, y+4, x2-4, y+4);
					p->drawLine(x+4, y+6, x2-4, y+6);
					p->drawLine(x+4, y+8, x2-4, y+8);

				} else {
					renderGradient( p, r, cg.background(), false);

					p->setPen(cg.light());
					p->drawLine(x+3, y+4, x+3, y2-4);
					p->drawLine(x+5, y+4, x+5, y2-4);
					p->drawLine(x+7, y+4, x+7, y2-4);

					p->setPen(cg.mid());
					p->drawLine(x+4, y+4, x+4, y2-4);
					p->drawLine(x+6, y+4, x+6, y2-4);
					p->drawLine(x+8, y+4, x+8, y2-4);
				}
			} else
				// We have a docked handle with a close button
				p->fillRect(r, cg.highlight());
				// ### Fixme - Add text caption to docked handle

			break;
		}


		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator: {
			renderGradient( p, r, cg.background(),
							!(flags & Style_Horizontal));
			if ( !(flags & Style_Horizontal) ) {
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2+1, r.width()-5, r.height()/2+1);
			} else {
				p->setPen(cg.mid());
				p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
				p->setPen(cg.light());
				p->drawLine(r.width()/2+1, 4, r.width()/2+1, r.height()-5);
			}
			break;
		}


		default:
		{
			// ARROWS
			// -------------------------------------------------------------------
			if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) 
			{
				QPointArray a;

				switch(pe) {
					case PE_ArrowUp:
						a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
						break;

					case PE_ArrowDown:
						a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
						break;

					case PE_ArrowLeft:
						a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
						break;

					default:
						a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
				}

				p->save();
				if ( flags & Style_Down )
					p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
								  pixelMetric( PM_ButtonShiftVertical ) );

				if ( flags & Style_Enabled ) {
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				} else {
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();

			} else
		        QWindowsStyle::drawPrimitive( pe, p, r, cg, flags, opt );
		}
	}
}


void HighColorStyle::drawControl( ControlElement element,
								  QPainter *p,
								  const QWidget *widget,
								  const QRect &r,
								  const QColorGroup &cg,
								  SFlags flags,
								  const QStyleOption& opt ) const
{
	switch (element)
	{
		// TABS
		// -------------------------------------------------------------------
		case CE_TabBarTab: {
			const QTabBar * tb = (const QTabBar *) widget;

			// ### Only rounded "above" tabs are implemented thus far
			if (tb->shape() != QTabBar::RoundedAbove) {
				QWindowsStyle::drawControl(element, p, widget, r, cg, flags, opt);
				return;
			}

			if (flags & Style_Selected) {

				p->setPen(cg.light());
				p->drawLine(r.x(), r.y()+4, r.x(), r.bottom());
				p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
				p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
				p->setPen(cg.shadow());
				p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());

				p->setPen(cg.midlight());
				p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
				p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
				p->drawLine(r.x()+5, r.y()+1, r.right() - 2, r.y()+1);
				p->setPen(cg.mid());
				p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom());

			} else {

				p->translate(0,1);
				p->setPen(cg.shadow());
				p->drawLine(r.x(), r.y()+4, r.x(), r.bottom()-2);
				p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
				p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
				p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-2);

				p->setPen(cg.midlight());
				p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
				p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
				p->drawLine(r.x()+5, r.y()+1, r.right()-2, r.y()+1);
				p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());

				p->setPen(cg.mid());
				p->drawPoint(r.right()-1, r.y()+1);
				p->drawLine(r.x()+4, r.y()+2, r.right()-1, r.y()+2);
				p->drawLine(r.x()+3, r.y()+3, r.right()-1, r.y()+3);

				p->fillRect(r.x()+2, r.y()+4, r.width()-3, r.height()-6,
							cg.brush(QColorGroup::Mid));

				p->setPen(cg.light());
				p->drawLine(r.x(), r.bottom()-1, r.right(), r.bottom()-1);
				p->translate(0,-1);
			}
			break;
		}

		// MENUBAR ITEM (sunken panel on mouse over)
		// -------------------------------------------------------------------
		case CE_MenuBarItem:
		{
			QMenuBar* mb = (QMenuBar*)widget;
			QRect pr = mb->rect();
			
			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;

			if ( active && focused )
				qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
								cg, true, 1, &cg.brush(QColorGroup::Midlight));
			else
				renderGradient( p, r, cg.background(), false, 
								r.x(), r.y()-1, pr.width()-2, pr.height()-2);

		    QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
			break;
		}

// ###### TODO
/*		case CE_PopupMenuItem: {
			break;
		} */

		default:
			QWindowsStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}



void HighColorStyle::drawComplexControl( ComplexControl control,
                                         QPainter *p,
                                         const QWidget *widget,
                                         const QRect &r,
                                         const QColorGroup &cg,
                                         SFlags flags,
					   				     SCFlags controls,
									     SCFlags active,
                                         const QStyleOption& opt ) const
{
	switch(control) 
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox: {

			// Draw box and arrow
			if ( controls & SC_ComboBoxArrow ) {
				bool sunken = (active == SC_ComboBoxArrow);

				// Draw the combo
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);

				int x2 = x+w-1;
				int y2 = y+h-1;

				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				// Ensure the edge notches are properly colored
				p->setPen(cg.background());
				p->drawPoint(x,y);
				p->drawPoint(x,y2);
				p->drawPoint(x2,y);
				p->drawPoint(x2,y2);

				renderGradient( p, QRect(x+2, y+2, w-4, h-4),
								cg.background(), false);

				p->setPen(sunken ? cg.light() : cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+1, y2-1, x2-1, y2-1);

				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-2);

				// Get the button bounding box
				QRect ar = QStyle::visualRect( 
					querySubControlMetrics(CC_ComboBox, widget, SC_ComboBoxArrow), 
					widget );
				
				// Are we enabled?
				if ( widget->isEnabled() )
					flags |= Style_Enabled;

				// Are we "pushed" ?
				if ( active & Style_Sunken )
					flags |= Style_Sunken;

				drawPrimitive(PE_ArrowDown, p, ar, cg, flags);
			}

			// Draw an edit field if required
			if ( controls & SC_ComboBoxEditField ) 
			{
				const QComboBox * cb = (const QComboBox *) widget;
				QRect re = QStyle::visualRect( 
					querySubControlMetrics( CC_ComboBox, widget,
						                    SC_ComboBoxEditField), widget );

				// Draw the indent
				if (cb->editable()) {
					p->setPen( cg.dark() );
					p->drawLine( re.x(), re.y()-1, re.x()+re.width(), re.y()-1 );
					p->drawLine( re.x()-1, re.y(), re.x()-1, re.y()+re.height() );
				}

				if ( cb->hasFocus() ) {
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				} else {
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.background() );
				}

				if ( cb->hasFocus() && !cb->editable() ) {
					// Draw the contents
					p->fillRect( re.x(), re.y(), re.width(), re.height(),
								 cg.brush( QColorGroup::Highlight ) );

					QRect re =
                    QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cb), widget);

					drawPrimitive( PE_FocusRect, p, re, cg, 
								   Style_FocusAtBorder, QStyleOption(cg.highlight()));
				}
			}
			break;
		}

		// 3 BUTTON SCROLLBAR
		// -------------------------------------------------------------------
		case CC_ScrollBar: {
			const QScrollBar *sb = (const QScrollBar*)widget;

			QRect  addline, subline, subline2, addpage, subpage, slider, first, last;
			bool   maxedOut   = (sb->minValue()    == sb->maxValue());
			bool   horizontal = (sb->orientation() == Qt::Horizontal);
			SFlags sflags     = ((horizontal ? Style_Horizontal : Style_Default) |
								 (maxedOut   ? Style_Default : Style_Enabled));

			subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
			addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
			subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
			addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
			slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
			first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
			last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);
			subline2 = addline;

			if (horizontal)
                subline2.moveBy(-addline.width(), 0);
            else
                subline2.moveBy(0, -addline.height());

			// Draw the up/left button set
            if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
                drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
                              sflags | (active == SC_ScrollBarSubLine ?
									   	Style_Down : Style_Default));

				if (subline2.isValid())
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

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			const QToolButton *toolbutton = (const QToolButton *) widget;

			QRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;

			if (controls & SC_ToolButton) 
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);

				// Check whether to draw a background pixmap
				else if ( toolbutton->parentWidget() &&
						  toolbutton->parentWidget()->backgroundPixmap() &&
						  !toolbutton->parentWidget()->backgroundPixmap()->isNull() ) 
				{
					QPixmap pixmap = *(toolbutton->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
				} 
				else if (widget->parent() && widget->parent()->inherits("QToolBar"))
				{
					QToolBar* parent = (QToolBar*)widget->parent();
					QRect pr = parent->rect();

					renderGradient( p, r, cg.background(), 
									parent->orientation() == Qt::Vertical,
									r.x(), r.y(), pr.width()-2, pr.height()-2);
				}
			}

			// Draw a toolbutton menu indicator if required
			if (controls & SC_ToolButtonMenu) 
			{
				if (mflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
			}

			if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
				QRect fr = toolbutton->rect();
				fr.addCoords(3, 3, -3, -3);
				drawPrimitive(PE_FocusRect, p, fr, cg);
			}

			break;
		}

// ###### TODO
/*		case CC_Slider: {
			break;
		} */

		default: 
			QWindowsStyle::drawComplexControl(control, p, widget, 
							r, cg, flags, controls, active, opt);
			break;
	}
}


QRect HighColorStyle::subRect(SubRect r, const QWidget *widget) const
{
	switch(r)
	{
		// We want the focus rect for buttons to be adjusted from
		// the Qt3 defaults to be similar to Qt 2's defaults.
		case SR_PushButtonFocusRect: {
			const QPushButton *button = (const QPushButton *) widget;

			QRect wrect(widget->rect());
			int dbw1 = 0, dbw2 = 0;

			if (button->isDefault() || button->autoDefault()) {
				dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
				dbw2 = dbw1 * 2;
			}

			int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
				dfw2 = dfw1 * 2;

			return QRect(wrect.x()      + dfw1 + dbw1 + 1,
						 wrect.y()      + dfw1 + dbw1 + 1,
						 wrect.width()  - dfw2 - dbw2 - 1,
						 wrect.height() - dfw2 - dbw2 - 1);
        }

		default:
			return QWindowsStyle::subRect(r, widget);
	}
}


int HighColorStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	switch(m)
	{
		// TABS
		// --------------------------------------------------------------------
		case PM_TabBarTabHSpace:
			return 24;

		case PM_TabBarTabVSpace: {
			const QTabBar * tb = (const QTabBar *) widget;
			if ( tb->shape() == QTabBar::RoundedAbove ||
				 tb->shape() == QTabBar::RoundedBelow )
				return 10;
			else
				return 0;
		}

		case PM_TabBarTabOverlap: {
			const QTabBar* tb = (const QTabBar*)widget;
			if (tb->shape() != QTabBar::RoundedAbove)
				return 3;					// Leave standard size alone
			else
				return 0;					// Change size for our tabs only
		}

		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 6;

		case PM_ButtonDefaultIndicator:
			return 0;						// No indicator

		case PM_ButtonShiftHorizontal:		// Offset by 1
		case PM_ButtonShiftVertical:		// ### Make configurable
			return 1;

		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight: {
			return 13;						// 13x13
		}
		
		// SLIDER
		// -------------------------------------------------------------------
		case PM_SliderLength:
			return 18;

		// MENU FRAME
		// -------------------------------------------------------------------
		case PM_MenuBarFrameWidth:
			return 1;

		default:
			return QWindowsStyle::pixelMetric(m, widget);
	}
}


QStyle::SubControl HighColorStyle::querySubControl( ComplexControl control,
                                                    const QWidget *widget,
                                                    const QPoint &pos,
                                                    const QStyleOption& opt ) const
{
    QStyle::SubControl ret =
        QWindowsStyle::querySubControl(control, widget, pos, opt);

	// Enable third button
    if (control == CC_ScrollBar &&
        ret == SC_None)
        ret = SC_ScrollBarSubLine;

    return ret;
}


// Many thanks to Brad for contributing this code.
QRect HighColorStyle::querySubControlMetrics( ComplexControl control,
											  const QWidget *widget,
											  SubControl sc,
											  const QStyleOption& opt ) const
{
    QRect ret;

    switch (control) 
	{
	    case CC_ScrollBar: {
			const QScrollBar *sb = (const QScrollBar*)widget;
			bool horizontal = sb->orientation() == Qt::Horizontal;
			int sliderstart = sb->sliderStart();
			int sbextent    = pixelMetric(PM_ScrollBarExtent, widget);
			int maxlen      = (horizontal ? sb->width() : sb->height()) - (sbextent*3);
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
			switch (sc) {
				case SC_ScrollBarSubLine:
					// top/left button
					ret.setRect(0, 0, sbextent, sbextent);
					break;

				case SC_ScrollBarAddLine:
					// bottom/right button
					if (horizontal)
					ret.setRect(sb->width() - sbextent, 0, sbextent, sbextent);
				else
					ret.setRect(0, sb->height() - sbextent, sbextent, sbextent);
                break;

				case SC_ScrollBarSubPage:
					// between top/left button and slider
					if (horizontal)
						ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
					else
						ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
					break;
	
				case SC_ScrollBarAddPage:
					// between bottom/right button and slider
					if (horizontal)
						ret.setRect(sliderstart + sliderlen, 0,
								maxlen - sliderstart - sliderlen + sbextent, sbextent);
					else
						ret.setRect(0, sliderstart + sliderlen, sbextent, 
								maxlen - sliderstart - sliderlen + sbextent);
					break;
	
				case SC_ScrollBarGroove:
					if (horizontal)
						ret.setRect(sbextent, 0, sb->width() - sbextent * 3, sb->height());
					else
						ret.setRect(0, sbextent, sb->width(), sb->height() - sbextent * 3);
					break;
	
				case SC_ScrollBarSlider:
					if (horizontal)
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
			ret = QWindowsStyle::querySubControlMetrics(control, widget, sc, opt);

			break;
	}
	return ret;
}


// Fix Qt's wacky image alignment
QPixmap HighColorStyle::stylePixmap(StylePixmap stylepixmap,
									const QWidget *widget,
									const QStyleOption& opt) const
{
    switch (stylepixmap) {
    case SP_TitleBarMinButton:
        return QPixmap((const char **)hc_minimize_xpm);
    case SP_TitleBarCloseButton:
        return QPixmap((const char **)hc_close_xpm);
    default:
		break;
    }
	return QWindowsStyle::stylePixmap(stylepixmap, widget, opt);
}


int HighColorStyle::styleHint( StyleHint sh, const QWidget *w, QStyleHintReturn *shr) const
{
	switch (sh)
	{
		// We don't want any spacing below the menu bar thanks
		case SH_MainWindow_SpaceBelowMenuBar:	
			return 0;
			
		default:
			return QWindowsStyle::styleHint(sh, w, shr);
	}
}


void HighColorStyle::renderGradient( QPainter* p, const QRect& r,
	QColor clr, bool horizontal, int px, int py, int pwidth, int pheight) const
{
	// Make 8 bit displays happy
	if (!highcolor) {
		p->fillRect(r, clr);
		return;
	}

	// px, py specify the gradient pixmap offset relative to the top-left corner.
	// pwidth, pheight specify the width and height of the parent's pixmap.
	// We use these to draw parent-relative pixmaps for toolbar buttons
	// and menubar items.
	
	GradientSet* grSet = gDict.find( clr.rgb() );
	
	if (!grSet) {
		grSet = new GradientSet(clr);
		gDict.insert( clr.rgb(), grSet );
	}

	if (horizontal) {
		
		int width;
		
		if (pwidth != -1)
			width = pwidth; 	// Parent relative
		else
			width = r.width();

		if (width <= 34) 
			p->drawTiledPixmap(r, *grSet->gradient(HMed), QPoint(px, 0));
		else if (width <= 52)
			p->drawTiledPixmap(r, *grSet->gradient(HLarge), QPoint(px, 0));
		else {
			KPixmap *hLarge = grSet->gradient(HLarge);
			
			// Don't draw a gradient if we don't need to
			if (hLarge->width() > px)
			{
				int pixmapWidth = hLarge->width() - px;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), pixmapWidth, r.height(),
									*hLarge, px, 0 );
				// Draw the remaining fill
				p->fillRect(r.x()+pixmapWidth, r.y(), r.width()-pixmapWidth, r.height(),
							clr.dark(110));
				
			} else
				p->fillRect(r, clr.dark(110));
		}
	
	} else {
		// Vertical gradient
		// -----------------

		int height;
		
		if (pheight != -1)
			height = pheight;	// Parent relative
		else
			height = r.height();

		if (height <= 24)
			p->drawTiledPixmap(r, *grSet->gradient(VSmall), QPoint(0, py));
		else if (height <= 34)
			p->drawTiledPixmap(r, *grSet->gradient(VMed), QPoint(0, py));
		else if (height <= 64)
			p->drawTiledPixmap(r, *grSet->gradient(VLarge), QPoint(0, py));
		else {
			KPixmap *vLarge = grSet->gradient(VLarge);

			// Only draw the upper gradient if we need to.
			if (vLarge->height() > py)
			{
				int pixmapHeight = vLarge->height() - py;

				// Draw the gradient
				p->drawTiledPixmap( r.x(), r.y(), r.width(), pixmapHeight,
									*vLarge, 0, py );
				// Draw the remaining fill
				p->fillRect(r.x(), r.y()+pixmapHeight, r.width(), r.height()-pixmapHeight,
							clr.dark(110));
				
			} else
				p->fillRect(r, clr.dark(110));
		}
	}
}
   

// vim: set noet ts=4 sw=4:

