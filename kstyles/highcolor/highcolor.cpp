/*
 * $Id$
 *
 * KDE3 HighColor Style (version 1.0)
 * Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *           (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org> 
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller          <mueller@kde.org>
 *           (C) 2001 Martijn Klingens      <klingens@kde.org>
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

#include <qdrawutil.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstyleplugin.h>

#include <qcombobox.h>
#include <qheader.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabbar.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>

#include <kdrawutil.h>
#include <kpixmapeffect.h>

#include "highcolor.h"
#include "highcolor.moc"
#include "bitmaps.h"


// -- Style Plugin Interface -------------------------
class HighColorStylePlugin : public QStylePlugin
{
	public:
		HighColorStylePlugin() {}
		~HighColorStylePlugin() {}

		QStringList keys() const
		{
			return QStringList() << "HighColor" << "Default" << "B3";
		}

		QStyle* create( const QString& key )
		{
			if ( key == "highcolor" )
				return new HighColorStyle( HighColorStyle::HighColor );
			
			else if ( key == "default" )
				return new HighColorStyle( HighColorStyle::Default );

			else if ( key == "b3" )
				return new HighColorStyle( HighColorStyle::B3 );
			
			return 0;
		}
};

Q_EXPORT_PLUGIN( HighColorStylePlugin )
// ---------------------------------------------------


// ### Remove globals
QBitmap lightBmp;
QBitmap grayBmp;
QBitmap dgrayBmp;
QBitmap centerBmp;
QBitmap maskBmp;
QBitmap xBmp;
QIntDict<GradientSet> gDict;

static const int itemFrame       = 1;
static const int itemHMargin     = 3;
static const int itemVMargin     = 0;
static const int arrowHMargin    = 6;
static const int rightBorder     = 12;
static const char* kdeToolbarWidget = "kde toolbar widget";

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
			break;
		}

		case VMed: {
			gradients[VMed] = new KPixmap;
			gradients[VMed]->resize(18, 34);
			KPixmapEffect::gradient(*gradients[VMed], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case VLarge: {
			gradients[VLarge] = new KPixmap;
			gradients[VLarge]->resize(18, 64);
			KPixmapEffect::gradient(*gradients[VLarge], c.light(110), c.dark(110),
											KPixmapEffect::VerticalGradient);
			break;
		}

		case HMed: {
			gradients[HMed] = new KPixmap;
			gradients[HMed]->resize(34, 18);
			KPixmapEffect::gradient(*gradients[HMed], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		case HLarge: {
			gradients[HLarge] = new KPixmap;
			gradients[HLarge]->resize(52, 18);
			KPixmapEffect::gradient(*gradients[HLarge], c.light(110), c.dark(110),
											KPixmapEffect::HorizontalGradient);
			break;
		}

		default:
			break;
	}
	return(gradients[type]);
}



// ---------------------------------------------------------------------------

HighColorStyle::HighColorStyle( StyleType styleType ) 
	: KStyle( AllowMenuTransparency | FilledFrameWorkaround, ThreeButtonScrollBar )
{
	type = styleType;
	highcolor = (type == HighColor && QPixmap::defaultDepth() > 8);
	gDict.setAutoDelete(true);
	hoverWidget = 0L;
}


HighColorStyle::~HighColorStyle()
{
}


void HighColorStyle::polish(QWidget* widget)
{
	// Put in order of highest occurance to maximise hit rate
	if (widget->inherits("QPushButton")) {
		widget->installEventFilter(this);
	} else if (widget->inherits("QMenuBar") || widget->inherits("QPopupMenu")) {
		widget->setBackgroundMode(QWidget::NoBackground);
	} else if (type == HighColor && widget->inherits("QToolBarExtensionWidget")) {
		widget->installEventFilter(this);
	} else if (type == HighColor && !qstrcmp( widget->name(), kdeToolbarWidget) ) {
		widget->setBackgroundMode( NoBackground );	// We paint the whole background.
		widget->installEventFilter(this);
	}

	KStyle::polish( widget );
}


void HighColorStyle::unPolish(QWidget* widget)
{
	if (widget->inherits("QPushButton")) {
		widget->removeEventFilter(this);
	}
	else if (widget->inherits("QMenuBar") || widget->inherits("QPopupMenu")) {
		widget->setBackgroundMode(QWidget::PaletteBackground);
	} else if (type == HighColor && widget->inherits("QToolBarExtensionWidget")) {
		widget->removeEventFilter(this);
	} else if (type == HighColor && !qstrcmp( widget->name(), kdeToolbarWidget) ) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode( PaletteBackground );
	}

	KStyle::unPolish( widget );
}


/* reimp. */
void HighColorStyle::renderMenuBlendPixmap( KPixmap& pix, const QColorGroup &cg,
		const QPopupMenu* /* popup */ ) const
{
	QColor col = cg.button();

#ifdef Q_WS_X11 // Only draw menu gradients on TrueColor, X11 visuals
	if ( QPaintDevice::x11AppDepth() >= 24 )
		KPixmapEffect::gradient( pix, col.light(120), col.dark(115),
				KPixmapEffect::HorizontalGradient );
	else
#endif
	pix.fill( col );
}


// This function draws primitive elements as well as their masks.
void HighColorStyle::drawPrimitive( PrimitiveElement pe,
									QPainter *p,
									const QRect &r,
									const QColorGroup &cg,
									SFlags flags,
									const QStyleOption& opt ) const
{
	bool down = flags & Style_Down;
	bool on   = flags & Style_On;

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault: {
			if ( type != HighColor ) {
				int x1, y1, x2, y2;
				r.coords( &x1, &y1, &x2, &y2 );
				
				// Button default indicator
				p->setPen( cg.shadow() );
				p->drawLine( x1+1, y1, x2-1, y1 );
				p->drawLine( x1, y1+1, x1, y2-1 );
				p->drawLine( x1+1, y2, x2-1, y2 );
				p->drawLine( x2, y1+1, x2, y2-1 );
			}
			break;
		}
			
		case PE_ButtonDropDown:
		case PE_ButtonTool: {
			bool sunken = on || down;
			int  x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;
			QPen oldPen = p->pen();

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

			p->setPen( oldPen );
			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand: {
			bool sunken = on || down;
			int  x, y, w, h;
			r.rect(&x, &y, &w, &h);

			if ( sunken )
				kDrawBeButton( p, x, y, w, h, cg, true,
						&cg.brush(QColorGroup::Mid) );
			
			else if ( flags & Style_MouseOver ) {
				QBrush brush(cg.button().light(110));
				kDrawBeButton( p, x, y, w, h, cg, false, &brush );
			}

			// "Flat" button
			else if (!(flags & (Style_Raised | Style_Sunken)))
				p->fillRect(r, cg.button());
			
			else if( highcolor )
			{
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(cg.shadow());
				p->drawLine(x+1, y, x2-1, y);
				p->drawLine(x+1, y2, x2-1, y2);
				p->drawLine(x, y+1, x, y2-1);
				p->drawLine(x2, y+1, x2, y2-1);

				p->setPen(cg.light());
				p->drawLine(x+2, y+2, x2-1, y+2);
				p->drawLine(x+2, y+3, x2-2, y+3);
				p->drawLine(x+2, y+4, x+2, y2-1);
				p->drawLine(x+3, y+4, x+3, y2-2);

				p->setPen(cg.mid());
				p->drawLine(x2-1, y+2, x2-1, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);

				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+2, x+1, y2-1);
				p->drawLine(x2-2, y+3, x2-2, y2-2);

				renderGradient(p, QRect(x+4, y+4, w-6, h-6),
								cg.button(), false);
			} else
				kDrawBeButton(p, x, y, w, h, cg, false,
							  &cg.brush(QColorGroup::Button));
			break;
		}


		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel: {
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

			if (w > 4 && h > 4) {
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					renderGradient( p, QRect(x+2, y+2, w-4, h-4),
								    cg.button(), flags & Style_Horizontal );
			}
			break;
		}


		// FOCUS RECT
		// -------------------------------------------------------------------
		case PE_FocusRect: {
			p->drawWinFocusRect( r );
			break;
		}


		// HEADER SECTION
		// -------------------------------------------------------------------
		case PE_HeaderSection: {
			// Temporary solution for the proper orientation of gradients.
			bool horizontal = true;
			if (p && p->device()->devType() == QInternal::Widget) {
				QHeader* hdr = dynamic_cast<QHeader*>(p->device());
				if (hdr)
					horizontal = hdr->orientation() == Horizontal;
			}

			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			bool sunken = on || down;
			int x2 = x+w-1;
			int y2 = y+h-1;
			QPen oldPen = p->pen();

			// Bevel
			p->setPen(sunken ? cg.mid() : cg.light());
			p->drawLine(x, y, x2-1, y);
			p->drawLine(x, y, x, y2-1);
			p->setPen(sunken ? cg.light() : cg.mid());
			p->drawLine(x+1, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+1, x2-1, y2-1);
			p->setPen(cg.shadow());
			p->drawLine(x, y2, x2, y2);
			p->drawLine(x2, y, x2, y2);

			if (sunken)
				p->fillRect(x+1, y+1, w-3, h-3, cg.button());
			else
				renderGradient( p, QRect(x+1, y+1, w-3, h-3),
							    cg.button(), !horizontal );
			p->setPen( oldPen );
			break;
		}


		// SCROLLBAR
		// -------------------------------------------------------------------
		case PE_ScrollBarSlider: {
			// Small hack to ensure scrollbar gradients are drawn the right way.
			flags ^= Style_Horizontal;

			drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Enabled | Style_Raised);

			// Draw a scrollbar riffle (note direction after above changes)
			if ( type != B3 ) {
				
				// HighColor & Default scrollbar
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
			} else {
				
				// B3 scrollbar
				if (flags & Style_Horizontal) {
					int buttons = 0;
					
					if (r.height() >= 36) buttons = 3;
					else if (r.height() >=24) buttons = 2;
					else if (r.height() >=16) buttons = 1;
					
					int x = r.x() + (r.width()-7) / 2;
					int y = r.y() + (r.height() - (buttons * 5) -
							(buttons-1)) / 2;
					int x2 = x + 7;
					
					for ( int i=0; i<buttons; i++, y+=6 )
					{
						p->setPen( cg.mid() );
						p->drawLine( x+1, y, x2-1, y );
						p->drawLine( x, y+1, x, y+3 );
						p->setPen( cg.light() );
						p->drawLine( x+1, y+1, x2-1, y+1 );
						p->drawLine( x+1, y+1, x+1, y+3 );
						p->setPen( cg.dark() );
						p->drawLine( x+1, y+4, x2-1, y+4 );
						p->drawLine( x2, y+1, x2, y+3 );
					}
				} else {
					int buttons = 0;
					
					if (r.width() >= 36) buttons = 3;
					else if (r.width() >=24) buttons = 2;
					else if (r.width() >=16) buttons = 1;
					
					int x = r.x() + (r.width() - (buttons * 5) -
							(buttons-1)) / 2;
					int y = r.y() + (r.height()-7) / 2;
					int y2 = y + 7;
					
					for ( int i=0; i<buttons; i++, x+=6 )
					{
						p->setPen( cg.mid() );
						p->drawLine( x+1, y, x+3, y );
						p->drawLine( x, y+1, x, y2-1 );
						p->setPen( cg.light() );
						p->drawLine( x+1, y+1, x+3, y+1 );
						p->drawLine( x+1, y+1, x+1, y2-1 );
						p->setPen( cg.dark() );
						p->drawLine( x+1, y2, x+3, y2 );
						p->drawLine( x+4, y+1, x+4, y2-1 );
					}
				}
			}
			break;
		}


		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage: {
			int x, y, w, h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			if ( type != B3 ) {
				// HighColor & Default scrollbar
				
				p->setPen(cg.shadow());
				
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
			} else {
				// B3 scrollbar
				
				p->setPen( cg.mid() );
				
				if (flags & Style_Horizontal) {
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y2, x2, y2);
					p->fillRect( QRect(x, y+1, w, h-2), 
							flags & Style_Down ? cg.button() : cg.midlight() );
				} else {
					p->drawLine(x, y, x, y2);
					p->drawLine(x2, y, x2, y2);
					p->fillRect( QRect(x+1, y, w-2, h), 
							flags & Style_Down ? cg.button() : cg.midlight() );
				}
			}
			break;
		}


		case PE_ScrollBarAddLine: {
			drawPrimitive( PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
					p, r, cg, flags );
			break;
		}


		case PE_ScrollBarSubLine: {
			drawPrimitive( PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
					((flags & Style_Down) ? Style_Down : Style_Raised) );

			drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
					p, r, cg, flags );
			break;
		}


		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator: {
			
			bool enabled  = flags & Style_Enabled;
			bool nochange = flags & Style_NoChange;
			
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

			if ( enabled )
				p->fillRect(x+2, y+2, w-4, h-4, 
						down ? cg.button(): cg.base());
			else
				p->fillRect(x+2, y+2, w-4, h-4, cg.background());
			
			if (!(flags & Style_Off)) {
				if (on) {
					p->setPen(nochange ? cg.dark() : cg.text());
					p->drawPixmap(x+3, y+3, xBmp);
				}
				else {
					p->setPen(cg.shadow());
					p->drawRect(x+2, y+2, w-4, h-4);
					p->setPen(nochange ? cg.text() : cg.dark());
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
				lightBmp  = QBitmap(13, 13, radiooff_light_bits,  true);
				grayBmp   = QBitmap(13, 13, radiooff_gray_bits,   true);
				dgrayBmp  = QBitmap(13, 13, radiooff_dgray_bits,  true);
				centerBmp = QBitmap(13, 13, radiooff_center_bits, true);
				centerBmp.setMask( centerBmp );
			}

			// Bevel
			kColorBitmaps(p, cg, r.x(), r.y(), &lightBmp , &grayBmp,
						  NULL, &dgrayBmp);
			
			// The center fill of the indicator (grayed out when disabled)
			if ( flags & Style_Enabled )
				p->setPen( down ? cg.button() : cg.base() );
			else
				p->setPen( cg.background() );
			p->drawPixmap( r.x(), r.y(), centerBmp );

			// Indicator "dot"
			if ( on ) {
				QColor color = flags & Style_NoChange ?
					cg.dark() : cg.text();
				
				p->setPen(color);
				p->drawLine(5, 4, 7, 4);
				p->drawLine(4, 5, 4, 7);
				p->drawLine(5, 8, 7, 8);
				p->drawLine(8, 5, 8, 7);
				p->fillRect(5, 5, 3, 3, color);
			}

			break;
		}


		// RADIOBUTTON (exclusive indicator) mask
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicatorMask: {
			if (maskBmp.isNull()) {
				maskBmp = QBitmap(13, 13, radiomask_bits, true);
				maskBmp.setMask(maskBmp);
			}
				
			p->setPen(Qt::color1);
			p->drawPixmap(r.x(), r.y(), maskBmp);
			break;
		}


		// SPLITTER/DOCKWINDOW HANDLES
		// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
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
		case PE_PanelPopup:
		case PE_WindowFrame:
		case PE_PanelLineEdit: {
			bool sunken  = flags & Style_Sunken;
			int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
										: opt.lineWidth();
			if (lw == 2)
			{
				QPen oldPen = p->pen();
				int x,y,w,h;
				r.rect(&x, &y, &w, &h);
				int x2 = x+w-1;
				int y2 = y+h-1;
				p->setPen(sunken ? cg.light() : cg.dark());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);
				p->setPen(sunken ? cg.mid() : cg.light());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
				p->setPen(sunken ? cg.midlight() : cg.mid());
				p->drawLine(x+1, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+1, x2-1, y2-1);
				p->setPen(sunken ? cg.dark() : cg.midlight());
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2-1);
				p->setPen(oldPen);
			} else
				KStyle::drawPrimitive(pe, p, r, cg, flags, opt);

			break;
		}


		// MENU / TOOLBAR PANEL
		// -------------------------------------------------------------------
		case PE_PanelMenuBar: 			// Menu
		case PE_PanelDockWindow: {		// Toolbar
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;
			
			if (opt.lineWidth())
			{
				p->setPen(cg.light());
				p->drawLine(r.x(), r.y(), x2-1,  r.y());
				p->drawLine(r.x(), r.y(), r.x(), y2-1);
				p->setPen(cg.dark());
				p->drawLine(r.x(), y2, x2, y2);
				p->drawLine(x2, r.y(), x2, y2);

				// ### Qt should specify Style_Horizontal where appropriate
				renderGradient( p, QRect(r.x()+1, r.y()+1, x2-1, y2-1),
					cg.button(), (r.width() < r.height()) &&
								 (pe != PE_PanelMenuBar) );
			}
			else
			{
				renderGradient( p, QRect(r.x(), r.y(), x2, y2),
					cg.button(), (r.width() < r.height()) &&
								 (pe != PE_PanelMenuBar) );
			}

			break;
		}



		// TOOLBAR SEPARATOR
		// -------------------------------------------------------------------
		case PE_DockWindowSeparator: {
			renderGradient( p, r, cg.button(),
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
				
				if ( type != B3 ) {
					// HighColor & Default arrows
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
				} else {
					// B3 arrows
					switch(pe) {
						case PE_ArrowUp:
							a.setPoints(QCOORDARRLEN(B3::u_arrow), B3::u_arrow);
							break;

						case PE_ArrowDown:
							a.setPoints(QCOORDARRLEN(B3::d_arrow), B3::d_arrow);
							break;

						case PE_ArrowLeft:
							a.setPoints(QCOORDARRLEN(B3::l_arrow), B3::l_arrow);
							break;

						default:
							a.setPoints(QCOORDARRLEN(B3::r_arrow), B3::r_arrow);
					}
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
				KStyle::drawPrimitive( pe, p, r, cg, flags, opt );
		}
	}
}


void HighColorStyle::drawKStylePrimitive( KStylePrimitive kpe,
										  QPainter* p,
										  const QWidget* widget,
										  const QRect &r,
										  const QColorGroup &cg,
										  SFlags flags,
										  const QStyleOption &opt ) const
{
	switch ( kpe )
	{
		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				renderGradient( p, r, cg.button(), false);
				p->setPen(cg.light());
				p->drawLine(x+1, y+4, x+1, y2-4);
				p->drawLine(x+3, y+4, x+3, y2-4);
				p->drawLine(x+5, y+4, x+5, y2-4);

				p->setPen(cg.mid());
				p->drawLine(x+2, y+4, x+2, y2-4);
				p->drawLine(x+4, y+4, x+4, y2-4);
				p->drawLine(x+6, y+4, x+6, y2-4);

			} else {
				
				renderGradient( p, r, cg.button(), true);
				p->setPen(cg.light());
				p->drawLine(x+4, y+1, x2-4, y+1);
				p->drawLine(x+4, y+3, x2-4, y+3);
				p->drawLine(x+4, y+5, x2-4, y+5);

				p->setPen(cg.mid());
				p->drawLine(x+4, y+2, x2-4, y+2);
				p->drawLine(x+4, y+4, x2-4, y+4);
				p->drawLine(x+4, y+6, x2-4, y+6);

			}
			break;
		}

							   
		// GENERAL/KICKER HANDLE
		// -------------------------------------------------------------------
		case KPE_GeneralHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				p->setPen(cg.light());
				p->drawLine(x+1, y, x+1, y2);
				p->drawLine(x+3, y, x+3, y2);
				p->drawLine(x+5, y, x+5, y2);

				p->setPen(cg.mid());
				p->drawLine(x+2, y, x+2, y2);
				p->drawLine(x+4, y, x+4, y2);
				p->drawLine(x+6, y, x+6, y2);
				
			} else {

				p->setPen(cg.light());
				p->drawLine(x, y+1, x2, y+1);
				p->drawLine(x, y+3, x2, y+3);
				p->drawLine(x, y+5, x2, y+5);

				p->setPen(cg.mid());
				p->drawLine(x, y+2, x2, y+2);
				p->drawLine(x, y+4, x2, y+4);
				p->drawLine(x, y+6, x2, y+6);

			}
			break;
		}


		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove: {
			const QSlider* slider = (const QSlider*)widget;
			bool horizontal = slider->orientation() == Horizontal;
			int gcenter = (horizontal ? r.height() : r.width()) / 2;

			QRect gr;
			if (horizontal)
				gr = QRect(r.x(), r.y()+gcenter-3, r.width(), 7);
			else
				gr = QRect(r.x()+gcenter-3, r.y(), 7, r.height());

			int x,y,w,h;
			gr.rect(&x, &y, &w, &h);
			int x2=x+w-1;
			int y2=y+h-1;

			// Draw the slider groove.
			p->setPen(cg.dark());
			p->drawLine(x+2, y, x2-2, y);
			p->drawLine(x, y+2, x, y2-2);
			p->fillRect(x+2,y+2,w-4, h-4, 
				slider->isEnabled() ? cg.dark() : cg.mid());
			p->setPen(cg.shadow());
			p->drawRect(x+1, y+1, w-2, h-2);
			p->setPen(cg.light());
			p->drawPoint(x+1,y2-1);
			p->drawPoint(x2-1,y2-1);
			p->drawLine(x2, y+2, x2, y2-2);
			p->drawLine(x+2, y2, x2-2, y2);
			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle: {
			const QSlider* slider = (const QSlider*)widget;
			bool horizontal = slider->orientation() == Horizontal;
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;
				
			p->setPen(cg.mid());
			p->drawLine(x+1, y, x2-1, y);
			p->drawLine(x, y+1, x, y2-1);
			p->setPen(cg.shadow());
			p->drawLine(x+1, y2, x2-1, y2);
			p->drawLine(x2, y+1, x2, y2-1);

			p->setPen(cg.light());
			p->drawLine(x+1, y+1, x2-1, y+1);
			p->drawLine(x+1, y+1, x+1,  y2-1);
			p->setPen(cg.dark());
			p->drawLine(x+2, y2-1, x2-1, y2-1);
			p->drawLine(x2-1, y+2, x2-1, y2-1);
			p->setPen(cg.midlight());
			p->drawLine(x+2, y+2, x2-2, y+2);
			p->drawLine(x+2, y+2, x+2, y2-2);
			p->setPen(cg.mid());
			p->drawLine(x+3, y2-2, x2-2, y2-2);
			p->drawLine(x2-2, y+3, x2-2, y2-2);
			renderGradient(p, QRect(x+3, y+3, w-6, h-6), 
						   cg.button(), !horizontal);

			// Paint riffles
			if (horizontal) {
				p->setPen(cg.light());
				p->drawLine(x+5, y+4, x+5, y2-4);
				p->drawLine(x+8, y+4, x+8, y2-4);
				p->drawLine(x+11,y+4, x+11, y2-4);
				p->setPen(slider->isEnabled() ? cg.shadow(): cg.mid());
				p->drawLine(x+6, y+4, x+6, y2-4);
				p->drawLine(x+9, y+4, x+9, y2-4);
				p->drawLine(x+12,y+4, x+12, y2-4);
			} else {
				p->setPen(cg.light());
				p->drawLine(x+4, y+5, x2-4, y+5);
				p->drawLine(x+4, y+8, x2-4, y+8);
				p->drawLine(x+4, y+11, x2-4, y+11);
				p->setPen(slider->isEnabled() ? cg.shadow() : cg.mid());
				p->drawLine(x+4, y+6, x2-4, y+6);
				p->drawLine(x+4, y+9, x2-4, y+9);
				p->drawLine(x+4, y+12, x2-4, y+12);
			}
			break;
		}

		default:
			KStyle::drawKStylePrimitive( kpe, p, widget, r, cg, flags, opt);
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
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton: {
			if ( widget == hoverWidget )
				flags |= Style_MouseOver;
			
			if ( type != HighColor ) {
				QPushButton *button = (QPushButton*) widget;
				QRect br = r;
				bool btnDefault = button->isDefault();
				
				if ( btnDefault || button->autoDefault() ) {
					// Compensate for default indicator
					static int di = pixelMetric( PM_ButtonDefaultIndicator );
					br.addCoords( di, di, -di, -di );
				}

				if ( btnDefault )
					drawPrimitive( PE_ButtonDefault, p, r, cg, flags );
				
				drawPrimitive( PE_ButtonCommand, p, br, cg, flags );
				
			} else
				drawPrimitive( PE_ButtonCommand, p, r, cg, flags );
			
			break;
		}


		// PUSHBUTTON LABEL
		// -------------------------------------------------------------------
		case CE_PushButtonLabel: {
			const QPushButton* button = (const QPushButton*)widget;
			bool active = button->isOn() || button->isDown();
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			// Shift button contents if pushed.
			if ( active ) {
				x += pixelMetric(PM_ButtonShiftHorizontal, widget); 
				y += pixelMetric(PM_ButtonShiftVertical, widget);
				flags |= Style_Sunken;
			}

			// Does the button have a popup menu?
			if ( button->isMenuButton() ) {
				int dx = pixelMetric( PM_MenuButtonIndicator, widget );
				drawPrimitive( PE_ArrowDown, p, QRect(x + w - dx - 2, y + 2, dx, h - 4),
							   cg, flags, opt );
				w -= dx;
			}

			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() ) {
				QIconSet::Mode  mode  = QIconSet::Disabled;
				QIconSet::State state = QIconSet::Off;

				if (button->isEnabled())
					mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = QIconSet::On;

				QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );
				p->drawPixmap( x + 4, y + h / 2 - pixmap.height() / 2, pixmap );
				int  pw = pixmap.width();
				x += pw + 4;
				w -= pw + 4;
			}

			// Make the label indicate if the button is a default button or not
			if ( active || button->isDefault() ) {
				// Draw "fake" bold text  - this enables the font metrics to remain
				// the same as computed in QPushButton::sizeHint(), but gives
				// a reasonable bold effect.
				int i;

				// Text shadow
				if (button->isEnabled()) // Don't draw double-shadow when disabled
					for(i=0; i<2; i++)
						drawItem( p, QRect(x+i+1, y+1, w, h), AlignCenter | ShowPrefix, 
								button->colorGroup(), button->isEnabled(), button->pixmap(),
								button->text(), -1,	
								active ? &button->colorGroup().dark() : &button->colorGroup().mid() );

				// Normal Text
				for(i=0; i<2; i++)
					drawItem( p, QRect(x+i, y, w, h), AlignCenter | ShowPrefix, 
							button->colorGroup(), button->isEnabled(), button->pixmap(),
							button->text(), -1,
							active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
			} else
				drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );

			// Draw a focus rect if the button has focus
			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p,
						QStyle::visualRect(subRect(SR_PushButtonFocusRect, widget), widget),
						cg, flags );
			break;
		}


#if (QT_VERSION-0 >= 0x030100)
		// MENUBAR BACKGROUND
		// -------------------------------------------------------------------
		case CE_MenuBarBackground:
		{
			renderGradient(p, r, cg.button(), false);
			break;
		}
#endif

		// MENUBAR ITEM (sunken panel on mouse over)
		// -------------------------------------------------------------------
		case CE_MenuBarItem:
		{
			QMenuBar  *mb = (QMenuBar*)widget;
			QMenuItem *mi = opt.menuItem();
			QRect      pr = mb->rect();

			bool active  = flags & Style_Active;
			bool focused = flags & Style_HasFocus;

			if ( active && focused )
				qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
								cg, true, 1, &cg.brush(QColorGroup::Midlight));
			else
				renderGradient( p, r, cg.button(), false,
								r.x(), r.y()-1, pr.width()-2, pr.height()-2);

			drawItem( p, r, AlignCenter | AlignVCenter | ShowPrefix
					| DontClip | SingleLine, cg, flags & Style_Enabled,
					mi->pixmap(), mi->text() );

			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			const QPopupMenu *popupmenu = (const QPopupMenu *) widget;

			QMenuItem *mi = opt.menuItem();
			if ( !mi ) {
				// Don't leave blank holes if we set NoBackground for the QPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! (widget->erasePixmap() && !widget->erasePixmap()->isNull()) )
					p->fillRect(r, cg.brush(QColorGroup::Button) );
				break;
			}

			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled    = mi->isEnabled();
			bool checkable  = popupmenu->isCheckable();
			bool active     = flags & Style_Active;
			bool etchtext   = styleHint( SH_EtchDisabledText );
			bool reverse    = QApplication::reverseLayout();
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );

			if ( checkable )
				checkcol = QMAX( checkcol, 20 );

			// Are we a menu item separator?
			if ( mi->isSeparator() ) {
				p->setPen( cg.dark() );
				p->drawLine( x, y, x+w, y );
				p->setPen( cg.light() );
				p->drawLine( x, y+1, x+w, y+1 );
				break;
			}

			// Draw the menu item background
			if ( active )
				qDrawShadePanel( p, x, y, w, h, cg, true, 1,
				                 &cg.brush(QColorGroup::Midlight) );
			// Draw the transparency pixmap
			else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
				p->drawPixmap( x, y, *widget->erasePixmap(), x, y, w, h );
			// Draw a solid background
			else
				p->fillRect( r, cg.button() );

			// Do we have an icon?
			if ( mi->iconSet() ) {
				QIconSet::Mode mode;
				QRect cr = visualRect( QRect(x, y, checkcol, h), r );
				
				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? QIconSet::Active : QIconSet::Disabled;
				else
					mode = enabled ? QIconSet::Normal : QIconSet::Disabled;

				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && !active && mi->isChecked() )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
									 cg, true, 1, &cg.brush(QColorGroup::Midlight) );
				// Draw the icon
				QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
				QRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() ) {
				int cx = reverse ? x+w - checkcol : x;

				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
				if ( ! active )
					qDrawShadePanel( p, cx, y, checkcol, h, cg, true, 1,
					                 &cg.brush(QColorGroup::Midlight) );

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, QRect( cx + itemFrame, y + itemFrame,
								checkcol - itemFrame*2, h - itemFrame*2), cg, cflags );
			}

			// Time to draw the menu item label...
			int xm = itemFrame + checkcol + itemHMargin; // X position margin
			
			int xp = reverse ? // X position
					x + tab + rightBorder + itemHMargin + itemFrame - 1 :
					x + xm;
			
			int offset = reverse ? -1 : 1;	// Shadow offset for etched text
			
			// Label width (minus the width of the accelerator portion)
			int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1; 

			// Set the color for enabled and disabled text 
			// (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			QColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if ( mi->custom() ) {
				int m = itemVMargin;
				// Save the painter state in case the custom
				// paint method changes it in some way
				p->save();

				// Draw etched text if we're inactive and the menu item is disabled
				if ( etchtext && !enabled && !active ) {
					p->setPen( cg.light() );
					mi->custom()->paint( p, cg, active, enabled, xp+offset, y+m+1, tw, h-2*m );
					p->setPen( discol );
				}
				mi->custom()->paint( p, cg, active, enabled, xp, y+m, tw, h-2*m );
				p->restore();
			}
			else {
				// The menu item doesn't draw it's own label
				QString s = mi->text();

				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = itemVMargin;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;
					
					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled && !active ) {
							// Draw etched text if we're inactive and the menu item is disabled
							p->setPen( cg.light() );
							p->drawText( tabx+offset, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ) );
							p->setPen( discol );
						}
						p->drawText( tabx, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label 
					// if there's no accelerator)
					if ( etchtext && !enabled && !active ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, tw, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}

					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					QPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( OpaqueMode );

					int diffw = ( ( w - pixmap->width() ) / 2 )
									+ ( ( w - pixmap->width() ) % 2 );
					p->drawPixmap( x+diffw, y+itemFrame, *pixmap );
					
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				int dim = pixelMetric(PM_MenuButtonIndicator);
				QRect vr = visualRect( QRect( x + w - arrowHMargin - 2*itemFrame - dim,
							y + h / 2 - dim / 2, dim, dim), r );

				// Draw an arrow at the far end of the menu item
				if ( active ) {
					if ( enabled )
						discol = cg.buttonText();

					QColorGroup g2( discol, cg.highlight(), white, white,
									enabled ? white : discol, discol, white );

					drawPrimitive( arrow, p, vr, g2, Style_Enabled );
				} else
					drawPrimitive( arrow, p, vr, cg,
							enabled ? Style_Enabled : Style_Default );
			}
			break;
		}

		default:
			KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}


void HighColorStyle::drawControlMask( ControlElement element,
								  	  QPainter *p,
								  	  const QWidget *widget,
								  	  const QRect &r,
								  	  const QStyleOption& opt ) const
{
	switch (element)
	{
		// PUSHBUTTON MASK
		// ----------------------------------------------------------------------
		case CE_PushButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}

		default:
			KStyle::drawControlMask(element, p, widget, r, opt);
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
				p->setPen(cg.button());
				p->drawPoint(x,y);
				p->drawPoint(x,y2);
				p->drawPoint(x2,y);
				p->drawPoint(x2,y2);

				renderGradient( p, QRect(x+2, y+2, w-4, h-4),
								cg.button(), false);

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
					p->setBackgroundColor( cg.button() );
				}

				if ( cb->hasFocus() && !cb->editable() ) {
					// Draw the contents
					p->fillRect( re.x(), re.y(), re.width(), re.height(),
								 cg.brush( QColorGroup::Highlight ) );

					QRect re = QStyle::visualRect( 
								subRect(SR_ComboBoxFocusRect, cb), widget);

					drawPrimitive( PE_FocusRect, p, re, cg,
								   Style_FocusAtBorder, QStyleOption(cg.highlight()));
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
				else if (widget->parent())
				{
					if (widget->parent()->inherits("QToolBar"))
					{
						QToolBar* parent = (QToolBar*)widget->parent();
						QRect pr = parent->rect();

						renderGradient( p, r, cg.button(),
									parent->orientation() == Qt::Vertical,
									r.x(), r.y(), pr.width()-2, pr.height()-2);
					}
					else if (widget->parent()->inherits("QToolBarExtensionWidget"))
					{
						QWidget* parent = (QWidget*)widget->parent();
						QToolBar* toolbar = (QToolBar*)parent->parent();
						QRect tr = toolbar->rect();

						if ( toolbar->orientation() == Qt::Horizontal ) {
							renderGradient( p, r, cg.button(), false, r.x(), r.y(),
									r.width(), tr.height() );
						} else {
							renderGradient( p, r, cg.button(), true, r.x(), r.y(),
									tr.width(), r.height() );
						}
					}
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


		default:
			KStyle::drawComplexControl(control, p, widget,
						r, cg, flags, controls, active, opt);
			break;
	}
}


void HighColorStyle::drawComplexControlMask( ComplexControl control,
											 QPainter *p,
											 const QWidget *widget,
											 const QRect &r,
											 const QStyleOption& opt ) const
{
	switch (control)
	{
		// COMBOBOX & TOOLBUTTON MASKS
		// -------------------------------------------------------------------
		case CC_ComboBox:
		case CC_ToolButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}

		default:
			KStyle::drawComplexControlMask(control, p, widget, r, opt);
	}
}


QRect HighColorStyle::subRect(SubRect r, const QWidget *widget) const
{
	// We want the focus rect for buttons to be adjusted from
	// the Qt3 defaults to be similar to Qt 2's defaults.
	// -------------------------------------------------------------------
	if (r == SR_PushButtonFocusRect ) {
		const QPushButton* button = (const QPushButton*) widget;
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
	} else
		return KStyle::subRect(r, widget);
}


int HighColorStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_ButtonDefaultIndicator: {
			if ( type == HighColor )
				return 0;					// No indicator when highcolor
			else
				return 3;
		}

		case PM_MenuButtonIndicator: {		// Arrow width
			if ( type != B3 )
				return 8;
			else
				return 7;
		}
										
		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:				// Checkbox size
		case PM_IndicatorHeight: {
			return 13;						// 13x13
		}

		default:
			return KStyle::pixelMetric(m, widget);
	}
}


QSize HighColorStyle::sizeFromContents( ContentsType contents,
										const QWidget* widget,
										const QSize &contentSize,
										const QStyleOption& opt ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton: {
			const QPushButton* button = (const QPushButton*) widget;
			int w  = contentSize.width();
			int h  = contentSize.height();
			int bm = pixelMetric( PM_ButtonMargin, widget );
			int fw = pixelMetric( PM_DefaultFrameWidth, widget ) * 2;

			w += bm + fw + 6;	// ### Add 6 to make way for bold font.
			h += bm + fw;

			// Ensure we stick to standard width and heights.
			if ( button->isDefault() || button->autoDefault() ) {
				if ( w < 80 && !button->pixmap() )
					w = 80;

				if ( type != HighColor ) {
					// Compensate for default indicator
					int di = pixelMetric( PM_ButtonDefaultIndicator );
					w += di * 2;
					h += di * 2;
				}
			}

			if ( h < 22 )
				h = 22;

			return QSize( w, h );
		}


		// POPUPMENU ITEM SIZE
		// -----------------------------------------------------------------
		case CT_PopupMenuItem: {
			if ( ! widget || opt.isDefault() )
				return contentSize;

			const QPopupMenu *popup = (const QPopupMenu *) widget;
			bool checkable = popup->isCheckable();
			QMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = contentSize.width(), h = contentSize.height();

			if ( mi->custom() ) {
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if ( ! mi->custom()->fullSpan() )
					h += 2*itemVMargin + 2*itemFrame;
			}
			else if ( mi->widget() ) {
			} else if ( mi->isSeparator() ) {
				w = 10; // Arbitrary
				h = 2;
			}
			else {
				if ( mi->pixmap() )
					h = QMAX( h, mi->pixmap()->height() + 2*itemFrame );
				else {
					// Ensure that the minimum height for text-only menu items
					// is the same as the icon size used by KDE.
					h = QMAX( h, 16 + 2*itemFrame );
					h = QMAX( h, popup->fontMetrics().height()
							+ 2*itemVMargin + 2*itemFrame );
				}
					
				if ( mi->iconSet() )
					h = QMAX( h, mi->iconSet()->pixmap(
								QIconSet::Small, QIconSet::Normal).height() +
								2 * itemFrame );
			}

			if ( ! mi->text().isNull() && mi->text().find('\t') >= 0 )
				w += 12;
			else if ( mi->popup() )
				w += 2 * arrowHMargin;

			if ( maxpmw )
				w += maxpmw + 6;
			if ( checkable && maxpmw < 20 )
				w += 20 - maxpmw;
			if ( checkable || maxpmw > 0 )
				w += 12;

			w += rightBorder;

			return QSize( w, h );
		}


		default:
			return KStyle::sizeFromContents( contents, widget, contentSize, opt );
	}
}


// Fix Qt's wacky image alignment
QPixmap HighColorStyle::stylePixmap(StylePixmap stylepixmap,
									const QWidget* widget,
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

	return KStyle::stylePixmap(stylepixmap, widget, opt);
}


bool HighColorStyle::eventFilter( QObject *object, QEvent *event )
{
	if (KStyle::eventFilter( object, event ))
		return true;

	QToolBar* toolbar;

	// Handle push button hover effects.
	QPushButton* button = dynamic_cast<QPushButton*>(object);
	if ( button )
	{
		if ( (event->type() == QEvent::Enter) &&
			 (button->isEnabled()) ) {
			hoverWidget = button;
			button->repaint( false );
		} 
		else if ( (event->type() == QEvent::Leave) &&
				  (object == hoverWidget) ) {
			hoverWidget = 0L;
			button->repaint( false );
		}
	} else if ( object->parent() && !qstrcmp( object->name(), kdeToolbarWidget ) )
	{
		// Draw a gradient background for custom widgets in the toolbar
		// that have specified a "kde toolbar widget" name.

		if (event->type() == QEvent::Paint ) {
			// Find the top-level toolbar of this widget, since it may be nested in other
			// widgets that are on the toolbar.
			QWidget *widget = static_cast<QWidget*>(object);
			QWidget *parent = static_cast<QWidget*>(object->parent());
			int x_offset = widget->x(), y_offset = widget->y();
			while (parent && parent->parent() && !qstrcmp( parent->name(), kdeToolbarWidget ) )
			{
				x_offset += parent->x();
				y_offset += parent->y();
				parent = static_cast<QWidget*>(parent->parent());
			}

			QRect r  = widget->rect();
			QRect pr = parent->rect();
			bool horiz_grad = pr.width() < pr.height();

			// Check if the parent is a QToolbar, and use its orientation, else guess.
			QToolBar* tb = dynamic_cast<QToolBar*>(parent);
			if (tb) horiz_grad = tb->orientation() == Qt::Vertical;

			QPainter p( widget );
			renderGradient(&p, r, parent->colorGroup().button(), horiz_grad,
					x_offset, y_offset, pr.width(), pr.height());

			return false;	// Now draw the contents
		}
	} else if ( object->parent() &&
			(toolbar = dynamic_cast<QToolBar*>(object->parent())) )
	{
		// We need to override the paint event to draw a 
		// gradient on a QToolBarExtensionWidget.
		if ( event->type() == QEvent::Paint ) {
			QWidget *widget = static_cast<QWidget*>(object);
			QRect wr = widget->rect(), tr = toolbar->rect();
			QPainter p( widget );
			renderGradient(&p, wr, toolbar->colorGroup().button(),
					toolbar->orientation() == Qt::Vertical,
					wr.x(), wr.y(), tr.width() - 2, tr.height() - 2);
			
			p.setPen( toolbar->colorGroup().dark() );
			if ( toolbar->orientation() == Qt::Horizontal )
				p.drawLine( wr.width()-1, 0, wr.width()-1, wr.height()-1 );
			else
				p.drawLine( 0, wr.height()-1, wr.width()-1, wr.height()-1 );
			
			return true;
		}
	}
	
	return false;
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
		int width = (pwidth != -1) ? pwidth : r.width();

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
				p->fillRect(r.x()+pixmapWidth, r.y(), r.width()-pixmapWidth, 
						r.height(),	clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}

	} else {
		// Vertical gradient
		// -----------------
		int height = (pheight != -1) ? pheight : r.height();

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
				p->fillRect(r.x(), r.y()+pixmapHeight, r.width(), 
						r.height()-pixmapHeight, clr.dark(110));

			} else
				p->fillRect(r, clr.dark(110));
		}
	}
}


// vim: set noet ts=4 sw=4:

