/* This file is part of the KDE libraries
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#include <qdrawutil.h>
#include <qlistbox.h>
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

#include "keramik.moc"
#include "pixmaploader.h"

// -- Style Plugin Interface -------------------------
class KeramikStylePlugin : public QStylePlugin
{
	public:
		KeramikStylePlugin() {}
		~KeramikStylePlugin() {}

		QStringList keys() const
		{
			return QStringList() << "Keramik";
		}

		QStyle* create( const QString& key )
		{
			if ( key == "keramik" )
				return new KeramikStyle();
			
			return 0;
		}
};

Q_EXPORT_PLUGIN( KeramikStylePlugin )
// ---------------------------------------------------


KeramikStyle::KeramikStyle()
	: KStyle(AllowMenuTransparency | FilledFrameWorkaround, ThreeButtonScrollBar),
	  hoverWidget(0)
{
	m_loader = new Keramik::PixmapLoader;
}

KeramikStyle::~KeramikStyle()
{
	delete m_loader;
}


void KeramikStyle::polish(QWidget* widget)
{
	// Put in order of highest occurance to maximise hit rate
	if ( widget->inherits( "QPushButton" ) )
		widget->setBackgroundMode( PaletteBackground );
 	else if ( widget->parentWidget() && widget->inherits( "QListBox" ) && widget->parentWidget()->inherits( "QComboBox" ) ) {
	    QListBox* listbox = (QListBox*) widget;
	    listbox->setLineWidth( 4 );
		listbox->setBackgroundMode( NoBackground );
	    widget->installEventFilter( this );
	}

	KStyle::polish(widget);
}


void KeramikStyle::unPolish(QWidget* widget)
{
	if ( widget->inherits( "QPushButton" ) )
		widget->setBackgroundMode( PaletteBase );
	else if ( widget->inherits( "QListBox" ) )
		widget->removeEventFilter( this );
/*	if (widget->inherits("QPushButton")) {
		widget->removeEventFilter(this);
	}
	else if (widget->inherits("QMenuBar") || widget->inherits("QPopupMenu")) {
		widget->setBackgroundMode(QWidget::PaletteBackground);
	}
*/
	KStyle::unPolish(widget);
}

// This function draws primitive elements as well as their masks.
void KeramikStyle::drawPrimitive( PrimitiveElement pe,
									QPainter *p,
									const QRect &r,
									const QColorGroup &cg,
									SFlags flags,
									const QStyleOption& opt ) const
{
	bool down = flags & Style_Down;
	bool on   = flags & Style_On;
	int  x, y, w, h;
	r.rect(&x, &y, &w, &h);

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
		{
			bool sunken = on || down;

			QString name = "pushbutton-default";
			if ( sunken ) name.append( "-pressed" );

			Keramik::RectTilePainter( name ).draw(p, x, y, w, h );
			break;
		}
/*			
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
*/
		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand:
		{
			bool sunken = on || down;

			QString name = "pushbutton";
			if ( sunken ) name.append( "-pressed" );

			Keramik::RectTilePainter( name ).draw(p, x, y, w, h );

/*			if ( widget->parent() )
			{
				QPainter sp( widget->parent() );
				QPoint origin( widget->mapToParent( 
				sp.translate( widget->mapToParent( 
			}*/
/*			if ( sunken )
				kDrawBeButton( p, x, y, w, h, cg, true,
						&cg.brush(QColorGroup::Mid) );
			
			else if ( flags & Style_MouseOver ) {
				QBrush brush(cg.button().light(110));
				kDrawBeButton( p, x, y, w, h, cg, false, &brush );
			}

			// "Flat" button
			else if (!(flags & (Style_Raised | Style_Sunken)))
				p->fillRect(r, cg.button());
			
			else*/
				break;

		}
/*

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

*/
		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator: {
			
			bool enabled  = flags & Style_Enabled;
			bool nochange = flags & Style_NoChange;

			Keramik::ScaledPainter( on ? "checkbox-on" : "checkbox-off" ).draw( p, x, y, w, h );

			break;
		}


		// RADIOBUTTON (exclusive indicator)
		// -------------------------------------------------------------------
		case PE_ExclusiveIndicator: {
			
			Keramik::ScaledPainter( on ? "radiobutton-on" : "radiobutton-off" ).draw( p, x, y, w, h );

			break;
		}

/*
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
		*/
		default:
			KStyle::drawPrimitive( pe, p, r, cg, flags, opt );
	}
}


void KeramikStyle::drawKStylePrimitive( KStylePrimitive kpe,
										  QPainter* p,
										  const QWidget* widget,
										  const QRect &r,
										  const QColorGroup &cg,
										  SFlags flags,
										  const QStyleOption &opt ) const
{
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );
	int x2 = x + w - 1, y2 = y + h - 1;

	switch ( kpe )
	{
		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
/*		case KPE_ToolBarHandle: {
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

*/
		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove:
		{
			const QSlider* slider = static_cast< const QSlider* >( widget );
			bool horizontal = slider->orientation() == Horizontal;

//			int gcenter = (horizontal ? r.height() : r.width()) / 2;

			if ( horizontal )
				Keramik::RectTilePainter( "slider-hgroove" ).draw(p, x, y + 4, w, h - 8 );
			else
				Keramik::RectTilePainter( "slider-vgroove" ).draw( p, x + 4, y, w - 8, h );

			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle:
		{
			const QSlider* slider = static_cast< const QSlider* >( widget );
			bool horizontal = slider->orientation() == Horizontal;

			Keramik::ScaledPainter( "slider" ).draw( p, x, y, w, h );

			break;
		}

		default:
			KStyle::drawKStylePrimitive( kpe, p, widget, r, cg, flags, opt);
	}
}


void KeramikStyle::drawControl( ControlElement element,
								  QPainter *p,
								  const QWidget *widget,
								  const QRect &r,
								  const QColorGroup &cg,
								  SFlags flags,
								  const QStyleOption& opt ) const
{
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch (element)
	{
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton:
/*			if ( widget == hoverWidget )
				flags |= Style_MouseOver;*/
		
			if ( static_cast< const QPushButton* >( widget )->isDefault( ) )
				drawPrimitive( PE_ButtonDefault, p, r, cg, flags );
			else
				drawPrimitive( PE_ButtonCommand, p, r, cg, flags );
			
			break;

							   
		// PUSHBUTTON LABEL
		// -------------------------------------------------------------------
		case CE_PushButtonLabel:
		{
			const QPushButton* button = static_cast<const QPushButton *>( widget );
			bool active = button->isOn() || button->isDown();

			// Shift button contents if pushed.
			if ( active )
			{
				x += pixelMetric(PM_ButtonShiftHorizontal, widget); 
				y += pixelMetric(PM_ButtonShiftVertical, widget);
				flags |= Style_Sunken;
			}

			// Does the button have a popup menu?
			if ( button->isMenuButton() )
			{
				int dx = pixelMetric( PM_MenuButtonIndicator, widget );
				drawPrimitive( PE_ArrowDown, p, QRect(x + w - dx - 2, y + 2, dx, h - 4),
							   cg, flags, opt );
				w -= dx;
			}

			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() )
			{
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
			if ( button->isDefault() )
			{
				// Draw "fake" bold text  - this enables the font metrics to remain
				// the same as computed in QPushButton::sizeHint(), but gives
				// a reasonable bold effect.
				int i;

				// Text shadow
				if (button->isEnabled()) // Don't draw double-shadow when disabled
						drawItem( p, QRect(x+2, y+1, w, h), AlignCenter | ShowPrefix, 
								button->colorGroup(), button->isEnabled(), button->pixmap(),
								button->text(), -1,	
								&button->colorGroup().mid() );

				// Normal Text
					drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, 
							button->colorGroup(), button->isEnabled(), button->pixmap(),
							button->text(), -1,
							&button->colorGroup().buttonText() );
			} else
				drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						&button->colorGroup().buttonText() );

			// Draw a focus rect if the button has focus
/*			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p,
						QStyle::visualRect(subRect(SR_PushButtonFocusRect, widget), widget),
						cg, flags );*/
			break;
		}

		case CE_TabBarTab:
		{
			const QTabBar* tabBar = static_cast< const QTabBar* >( widget );

			QString name;
			if ( tabBar->shape() == QTabBar::RoundedAbove ||
			     tabBar->shape() == QTabBar::TriangularAbove ) name = "tab-top-";
			else name = "tab-bottom-";

			if ( flags & Style_Selected )
				Keramik::RectTilePainter( name + "active" ).draw( p, x, y, w, h );
			else
			{
				Keramik::TabPainter::Mode mode;
				int index = tabBar->indexOf( opt.tab()->identifier() );
				if ( index == 0 ) mode = Keramik::TabPainter::First;
				else if ( index == tabBar->count() - 1 ) mode = Keramik::TabPainter::Last;
				else mode = Keramik::TabPainter::Middle;
				Keramik::TabPainter( name + "inactive", mode ).draw( p, x, y + 4, w, h - 4 );
			}

			break;
		}

/*
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
				int dim = (h-2*itemFrame) / 2;
				QRect vr = visualRect( QRect( x + w - arrowHMargin - itemFrame - dim,
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
*/
		default:
			KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}


void KeramikStyle::drawControlMask( ControlElement element,
								  	  QPainter *p,
								  	  const QWidget *widget,
								  	  const QRect &r,
								  	  const QStyleOption& opt ) const
{
	switch (element)
	{
		// PUSHBUTTON MASK
		// ----------------------------------------------------------------------
/*		case CE_PushButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}
*/
		default:
			KStyle::drawControlMask(element, p, widget, r, opt);
	}
}


void KeramikStyle::drawComplexControl( ComplexControl control,
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
				if ( active ) flags |= Style_On;

				drawPrimitive( PE_ButtonCommand, p, r, cg, flags );

				QRect ar = querySubControlMetrics(CC_ComboBox, widget, SC_ComboBoxArrow);

				QRect rr = QStyle::visualRect( QRect( ar.x(), ar.y() + 4,
				                                      m_loader->pixmap( "ripple" ).width(), ar.height() - 8 ),
				                               widget );

				ar = QStyle::visualRect( QRect( ar.x() + m_loader->pixmap( "ripple" ).width() + 4, ar.y(),
				                                m_loader->pixmap( "arrow" ).width(), ar.height() ),
				                         widget );
				Keramik::ScaledPainter( "ripple" ).draw( p, rr.x(), rr.y(), rr.width(), rr.height() );
				Keramik::CenteredPainter( "arrow" ).draw( p, ar.x(), ar.y(), ar.width(), ar.height() );

/*				// Draw the combo
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

				drawPrimitive(PE_ArrowDown, p, ar, cg, flags);*/
			}

			// Draw an edit field if required
/*			if ( controls & SC_ComboBoxEditField )
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
			}*/
			break;
		}
/*
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

					renderGradient( p, r, cg.button(),
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

*/
		default:
			KStyle::drawComplexControl(control, p, widget,
						r, cg, flags, controls, active, opt);
			break;
	}
}


void KeramikStyle::drawComplexControlMask( ComplexControl control,
											 QPainter *p,
											 const QWidget *widget,
											 const QRect &r,
											 const QStyleOption& opt ) const
{
	switch (control)
	{
		// COMBOBOX & TOOLBUTTON MASKS
		// -------------------------------------------------------------------
/*		case CC_ComboBox:
		case CC_ToolButton: {
			int x1, y1, x2, y2;
			r.coords( &x1, &y1, &x2, &y2 );
			QCOORD corners[] = { x1,y1, x2,y1, x1,y2, x2,y2 };
			p->fillRect( r, color1 );
			p->setPen( color0 );
			p->drawPoints( QPointArray(4, corners) );
			break;
		}
*/
		default:
			KStyle::drawComplexControlMask(control, p, widget, r, opt);
	}
}


int KeramikStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	switch(m)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PM_ButtonMargin:				// Space btw. frame and label
			return 4;

		case PM_ButtonDefaultIndicator:
			return 4;
		case PM_SliderLength:
			return 8;
/*
		case PM_MenuButtonIndicator: {		// Arrow width
			if ( type != B3 )
				return 8;
			else
				return 7;
		}
*/										
		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
			return m_loader->pixmap( "radiobutton-on" ).width();
		case PM_ExclusiveIndicatorHeight:
			return m_loader->pixmap( "radiobutton-on" ).height();
		case PM_IndicatorWidth:				// Checkbox size
			return m_loader->pixmap( "checkbox-on" ).width();
		case PM_IndicatorHeight:
			return m_loader->pixmap( "checkbox-on") .height();

		default:
			return KStyle::pixelMetric(m, widget);
	}
}


QSize KeramikStyle::sizeFromContents( ContentsType contents,
										const QWidget* widget,
										const QSize &contentSize,
										const QStyleOption& opt ) const
{
	switch (contents)
	{
		// PUSHBUTTON SIZE
		// ------------------------------------------------------------------
		case CT_PushButton:
			return QSize( contentSize.width() + 2 * pixelMetric( PM_ButtonMargin, widget ) + 28,
			              contentSize.height() + 2 * pixelMetric( PM_ButtonMargin, widget ) );

		case CT_ComboBox:
			return QSize( contentSize.width() + m_loader->pixmap( "arrow" ).width() + m_loader->pixmap( "ripple" ).width() + 22,
			              contentSize.height() + 4 );
/*
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

*/
		default:
			return KStyle::sizeFromContents( contents, widget, contentSize, opt );
	}
}


QRect KeramikStyle::querySubControlMetrics( ComplexControl control,
									const QWidget* widget,
	                              SubControl subcontrol,
	                              const QStyleOption& opt ) const
{
	switch ( control )
	{
		case CC_ComboBox:
			switch ( subcontrol )
			{
				case SC_ComboBoxArrow:
				{
					int w = m_loader->pixmap( "arrow" ).width() + m_loader->pixmap( "ripple" ).width();
					return QRect( widget->width() - w - 10, 0, w, widget->height() );
				}
				default: break;
			}
		default: break;
	}
	return KStyle::querySubControlMetrics( control, widget, subcontrol, opt );
}

// Fix Qt's wacky image alignment
/*QPixmap KeramikStyle::stylePixmap(StylePixmap stylepixmap,
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
}*/


bool KeramikStyle::eventFilter( QObject* object, QEvent* event )
{
	if (KStyle::eventFilter( object, event ))
		return true;

	if ( event->type() == QEvent::Paint && object->isWidgetType() && object->inherits("QListBox") )
	{
		static bool recursion = false;
		if (recursion )
			return false;
		QListBox* listbox = (QListBox*) object;
		QPaintEvent* paint = (QPaintEvent*) event;
		if ( !listbox->contentsRect().contains( paint->rect() ) )
		{
			{
				QPainter p( listbox );
				Keramik::RectTilePainter( "combobox-list" ).draw( &p, 0, 0, listbox->width(), listbox->height() );
			}
			QPaintEvent newpaint( paint->region().intersect( listbox->contentsRect() ), paint->erased() );
			recursion = true;
			object->event( &newpaint );
			recursion = false;
			return true;
		}
		return false;
	}

	// Handle push button hover effects.
	/*	QPushButton* button = dynamic_cast<QPushButton*>(object);
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
	}
*/	
	return false;
}

// vim: ts=4 sw=4 noet
