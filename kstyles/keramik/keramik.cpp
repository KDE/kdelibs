/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>
             (c) 2002 Maksim Orlovich <mo002j@mail.rochester.edu>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org> 
 
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

    Progressbar code based on KStyle, Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>

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

#include <qbitmap.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include <qframe.h>
#include <qheader.h>
#include <qintdict.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qstyleplugin.h>
#include <qtabbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "keramik.moc"

#include "gradients.h"
#include "colorutil.h"
#include "keramikrc.h"
#include "keramikimage.h"

#include "bitmaps.h"
#include "pixmaploader.h"

// -- Style Plugin Interface -------------------------
class KeramikStylePlugin : public QStylePlugin
{
public:
	KeramikStylePlugin() {}
	~KeramikStylePlugin() {}

	QStringList keys() const
	{
		if (QPixmap::defaultDepth() > 8)
			return QStringList() << "Keramik";
		else
			return QStringList();
	}

	QStyle* create( const QString& key )
	{
		if ( key == "keramik" ) return new KeramikStyle;
		return 0;
	}
};

Q_EXPORT_PLUGIN( KeramikStylePlugin )
// ---------------------------------------------------


// ### Remove globals
/*
QBitmap lightBmp;
QBitmap grayBmp;
QBitmap dgrayBmp;
QBitmap centerBmp;
QBitmap maskBmp;
QBitmap xBmp;
*/
namespace
{
	const int itemFrame       = 2;
	const int itemHMargin     = 6;
	const int itemVMargin     = 0;
	const int arrowHMargin    = 6;
	const int rightBorder     = 12;
	const char* kdeToolbarWidget = "kde toolbar widget";
}
// ---------------------------------------------------------------------------

namespace
{
	void drawKeramikArrow(QPainter* p, QColorGroup cg, QRect r, QStyle::PrimitiveElement pe, bool down, bool enabled)
	{
		QPointArray a;
				
		switch(pe)
		 {
			case QStyle::PE_ArrowUp:
				a.setPoints(QCOORDARRLEN(keramik_up_arrow), keramik_up_arrow);
				break;

			case QStyle::PE_ArrowDown:
				a.setPoints(QCOORDARRLEN(keramik_down_arrow), keramik_down_arrow);
				break;

			case QStyle::PE_ArrowLeft:
				a.setPoints(QCOORDARRLEN(keramik_left_arrow), keramik_left_arrow);
				break;

			default:
				a.setPoints(QCOORDARRLEN(keramik_right_arrow), keramik_right_arrow);
		}
		
		p->save();
		/*if ( down )
			p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
						pixelMetric( PM_ButtonShiftVertical ) );
		*/

		if ( enabled ) {
			//CHECKME: Why is the -1 needed?
			a.translate( r.x() + r.width() / 2 - 1, r.y() + r.height() / 2 );
			
			if (!down)
				p->setPen( cg.buttonText() );
			else
				p->setPen ( cg.button() );
			p->drawLineSegments( a );
		} else {
			a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 + 1 );
			p->setPen( cg.light() );
			p->drawLineSegments( a );
			a.translate( -1, -1 );
			p->setPen( cg.mid() );
			p->drawLineSegments( a );
		}
		p->restore();
	}
};

// XXX
/* reimp. */
void KeramikStyle::renderMenuBlendPixmap( KPixmap& pix, const QColorGroup &cg,
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

// XXX
QRect KeramikStyle::subRect(SubRect r, const QWidget *widget) const
{
	// We want the focus rect for buttons to be adjusted from
	// the Qt3 defaults to be similar to Qt 2's defaults.
	// -------------------------------------------------------------------
	switch ( r )
	{
		case SR_PushButtonFocusRect:
		{
			const QPushButton* button = (const QPushButton*) widget;
			QRect wrect(widget->rect());
			
			if (button->isDefault() || button->autoDefault())
			{
				return QRect(wrect.x() + 6, wrect.y() + 5, wrect.width() - 12,  wrect.height() - 10);
			}
			else
			{
				return QRect(wrect.x() + 3, wrect.y() + 5, wrect.width() - 8,  wrect.height() - 10);
			}

			break;
		}

		case SR_ComboBoxFocusRect:
		{
			return querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField );
		}

		default:
			return KStyle::subRect( r, widget );
	}
}


// XXX what exactly does this do ?
// Fix Qt's wacky image alignment
QPixmap KeramikStyle::stylePixmap(StylePixmap stylepixmap,
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


#define loader Keramik::PixmapLoader::the()

KeramikStyle::KeramikStyle() 
	:KStyle( AllowMenuTransparency | FilledFrameWorkaround /*| DisableMenuBlend*/, ThreeButtonScrollBar ), maskMode(false),
		toolbarBlendWidget(0), kickerMode(false)
{
	hoverWidget = 0;
}

KeramikStyle::~KeramikStyle()
{
	Keramik::PixmapLoader::release();
	KeramikDbCleanup();
}

void KeramikStyle::polish(QApplication* app)
{
	if (QString(app->argv()[0]) == "kicker")
		kickerMode = true;
}

void KeramikStyle::polish(QWidget* widget)
{
	// Put in order of highest occurance to maximise hit rate
	if ( widget->inherits( "QPushButton" )  || widget->inherits( "QComboBox" ) )
	{
		widget->installEventFilter(this);
		if ( widget->inherits( "QComboBox" ) )
			widget->setBackgroundMode( NoBackground );
	}	 
	else if ( widget->inherits( "QMenuBar" ) || widget->inherits( "QPopupMenu" ) )
		widget->setBackgroundMode( NoBackground );

 	else if ( widget->parentWidget() &&
			  ( ( widget->inherits( "QListBox" ) && widget->parentWidget()->inherits( "QComboBox" ) ) ||
	            widget->inherits( "KCompletionBox" ) ) ) {
		QListBox* listbox = (QListBox*) widget;
		listbox->setLineWidth( 4 );
		listbox->setBackgroundMode( NoBackground );
		widget->installEventFilter( this );
		
	} else if (widget->inherits("QToolBarExtensionWidget")) {
		widget->installEventFilter(this);
		//widget->setBackgroundMode( NoBackground );
 	}
	else if (kickerMode) 
	{
		/*
		if (QCString(widget->className()) == ("FittsLawFrame") )
		{
			QFrame* f = static_cast<QFrame*>(widget);
			f->setFrameStyle(QFrame::Panel | QFrame::Raised);
			f->setLineWidth(1);
			f->setMidLineWidth(1);
			
		}*/
	} else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
		widget->setBackgroundMode( NoBackground );
		widget->installEventFilter(this);
	}

	KStyle::polish(widget);
}

void KeramikStyle::unPolish(QWidget* widget)
{
	if ( widget->inherits( "QPushButton" ) || widget->inherits( "QComboBox"  ) )
	{
		if ( widget->inherits( "QComboBox" ) )
			widget->setBackgroundMode( PaletteButton );
	
		widget->removeEventFilter(this);
		
	}	 
	else if ( widget->inherits( "QMenuBar" ) || widget->inherits( "QPopupMenu" ) )
		widget->setBackgroundMode( PaletteBackground );

 	else if ( widget->parentWidget() &&
			  ( ( widget->inherits( "QListBox" ) && widget->parentWidget()->inherits( "QComboBox" ) ) ||
	            widget->inherits( "KCompletionBox" ) ) ) {
		QListBox* listbox = (QListBox*) widget;
		listbox->setLineWidth( 1 );
		listbox->setBackgroundMode( PaletteBackground );
		widget->removeEventFilter( this );
		widget->clearMask();
	} else if (widget->inherits("QToolBarExtensionWidget")) {
		widget->removeEventFilter(this);
 	}
	else if (kickerMode)  
	{
/*		if (QCString(widget->className()) == ("FittsLawFrame"))
		{
			QFrame* f = static_cast<QFrame*>(widget);
			f->setFrameStyle(QFrame::Panel | QFrame::Raised);
			f->setLineWidth(2);
			f->setMidLineWidth(1);
		}*/
	} else if ( !qstrcmp( widget->name(), kdeToolbarWidget ) ) {
		widget->setBackgroundMode( PaletteBackground );
		widget->removeEventFilter(this);
	}

	KStyle::unPolish(widget);
}

void KeramikStyle::polish( QPalette& )
{
	loader.clear();
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
	bool disabled = ( flags & Style_Enabled ) == 0;
	int  x, y, w, h;
	r.rect(&x, &y, &w, &h);

	switch(pe)
	{
		// BUTTONS
		// -------------------------------------------------------------------
		case PE_ButtonDefault:
		{
			bool sunken = on || down;

			int id;
			if ( sunken ) id  = keramik_pushbutton_default_pressed;
				else id = keramik_pushbutton_default;
				
			if (flags & Style_MouseOver && id == keramik_pushbutton_default )
				id = keramik_pushbutton_default_hov;

				
			//p->fillRect( r, cg.background() );
			Keramik::RectTilePainter( id, false ).draw(p, r, cg.button(), cg.background(), disabled,  pmode() );
			break;
		}

		case PE_ButtonDropDown:
		case PE_ButtonTool:
		{
//			bool sunken = on || down;
			
			int x2 = x+w-1;
			int y2 = y+h-1;
			
			if (on)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
				p->setPen(cg.dark());
				p->drawLine(x, y, x2, y);
				p->drawLine(x, y, x, y2);
			}
			else if (down)
			{
				Keramik::RectTilePainter(keramik_toolbar_clk).draw(p, r, cg.button(), cg.background());
			}
			else {
				Keramik::GradientPainter::renderGradient( p,
					QRect(0, 0, r.width(), r.height()),
					Keramik::ColorUtil::lighten(cg.button(), 115), flags & Style_Horizontal, false );

                                p->setPen(cg.button().light(70));
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);
				p->drawLine(x+2, y2-1, x2-1, y2-1);
				p->drawLine(x2-1, y+2, x2-1, y2-2);

                                p->setPen(Keramik::ColorUtil::lighten(cg.button(), 115) );
				p->drawLine(x+1, y+1, x2-1, y+1);
				p->drawLine(x+1, y+1, x+1, y2);
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);				
			}
			
			break;
		}

		// PUSH BUTTON
		// -------------------------------------------------------------------
		case PE_ButtonCommand:
		{
			bool sunken = on || down;
			
			int  name;
			
			if ( w < 28 || h <= 20 )
			{
				if (sunken)
					name = keramik_pushbutton_small_pressed;
				else
					name =  keramik_pushbutton_small;
			}
			else
			{
				if (sunken)
					name = keramik_pushbutton_pressed;
				else
					name =  keramik_pushbutton;
			}
			
			if (flags & Style_MouseOver && name == keramik_pushbutton )
				name = keramik_pushbutton_hov;
				
				
				
			//p->fillRect( r, cg.background() );
			if (toolbarBlendWidget)
			{
				// Draw a gradient background for custom widgets in the toolbar
				// that have specified a "kde toolbar widget" name.

				// Find the top-level toolbar of this widget, since it may be nested in other
				// widgets that are on the toolbar.
				QWidget *parent = static_cast<QWidget*>(toolbarBlendWidget->parent());
				int x_offset = toolbarBlendWidget->x(), y_offset = toolbarBlendWidget->y();
				while (parent && parent->parent() && !qstrcmp( parent->name(), kdeToolbarWidget ) )
				{
					x_offset += parent->x();
					y_offset += parent->y();
					parent = static_cast<QWidget*>(parent->parent());
				}

				QRect pr = parent->rect();
				bool horiz_grad = pr.width() > pr.height();

				// Check if the parent is a QToolbar, and use its orientation, else guess.
				QToolBar* tb = dynamic_cast<QToolBar*>(parent);
				if (tb) horiz_grad = tb->orientation() == Qt::Horizontal;
                                
				Keramik::GradientPainter::renderGradient( p,
						QRect(0, 0, pr.width(), pr.height()),
						parent->colorGroup().button(), horiz_grad, false , x_offset, y_offset);
                                           
				//Draw and blend the actual bevel..
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(), 
					disabled, Keramik::TilePainter::PaintFullBlend  );
					
				
			}
			else
				Keramik::RectTilePainter( name, false ).draw(p, r, cg.button(), cg.background(), disabled, pmode()  );
			
			break;

		}

		// BEVELS
		// -------------------------------------------------------------------
		case PE_ButtonBevel:
		{
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

			if (w > 4 && h > 4)
			{
				if (sunken)
					p->fillRect(x+2, y+2, w-4, h-4, cg.button());
				else
					Keramik::GradientPainter::renderGradient( p, QRect(x+2, y+2, w-4, h-4),
							cg.button(), flags & Style_Horizontal );
			}
			break;
		}


			// FOCUS RECT
			// -------------------------------------------------------------------
		case PE_FocusRect:
			p->drawWinFocusRect( r );
			break;

			// HEADER SECTION
			// -------------------------------------------------------------------
		case PE_HeaderSection:
			if ( flags & Style_Down )
				Keramik::RectTilePainter( keramik_listview_pressed, false ).draw( p, r, cg.button(), cg.background() );
			else
				Keramik::RectTilePainter( keramik_listview, false ).draw( p, r, cg.button(), cg.background() );
			break;

		case PE_HeaderArrow:
			if ( flags & Style_Up )
				drawPrimitive( PE_ArrowUp, p, r, cg, Style_Enabled );
			else drawPrimitive( PE_ArrowDown, p, r, cg, Style_Enabled );
			break;
		

		// 	// SCROLLBAR
		// -------------------------------------------------------------------

		case PE_ScrollBarSlider:
		{
			bool horizontal = flags & Style_Horizontal;
			bool active = ( flags & Style_Active ) || ( flags & Style_Down );
			int name = KeramikSlider1;
			unsigned int count = 3;
      
      

			if ( horizontal )
			{
				if ( w > ( loader.size( keramik_scrollbar_hbar+KeramikSlider1 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider4 ).width() +
				           loader.size( keramik_scrollbar_hbar+KeramikSlider3 ).width() + 2 ) )
					count = 5;
			}
			else if ( h > ( loader.size( keramik_scrollbar_vbar+KeramikSlider1 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider4 ).height() +
			                loader.size( keramik_scrollbar_vbar+KeramikSlider3 ).height() + 2 ) )
					count = 5;
					
			QColor col = cg.highlight();
			
			if (cg.button() != QApplication::palette().active().button() )
			{
				col = cg.button();
			}

			if (!active)
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, col, cg.background(), false, pmode() );
			else
				Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, Keramik::ColorUtil::lighten(col ,110), 
																					cg.background(), false, pmode() );
			break;
		}

		case PE_ScrollBarAddLine:
		{
			bool down = flags & Style_Down;
		
			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_hbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				
				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+r.width()/2 - 1, r.y()+ 5, r.x()+r.width()/2-1, r.y()+r.height() - 5);
				
				
				drawKeramikArrow(p, cg, QRect(r.x(), r.y(), r.width()/2, r.height()), PE_ArrowLeft, down, !disabled);
				
				drawKeramikArrow(p, cg, QRect(r.x()+r.width()/2, r.y(), r.width() - r.width()/2, r.height()), 
									PE_ArrowRight, down, !disabled);
			}
			else
			{
				Keramik::CenteredPainter painter(  keramik_scrollbar_vbar_arrow2 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				
				p->setPen( cg.buttonText() );
				p->drawLine(r.x()+4, r.y()+r.height()/2, r.x()+r.width() - 6, r.y()+r.height()/2);
				
				
				drawKeramikArrow(p, cg, QRect(r.x(), r.y(), r.width(), r.height()/2), PE_ArrowUp, down, !disabled);
				
				drawKeramikArrow(p, cg, QRect(r.x(), r.y()+r.height()/2, r.width(), r.height() - r.height()/2), 
									PE_ArrowDown, down, !disabled);
				//drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}
			
			
			break;
		}

		case PE_ScrollBarSubLine:
		{
			bool down = flags & Style_Down;
			
			if ( flags & Style_Horizontal )
			{
				Keramik::CenteredPainter painter(keramik_scrollbar_hbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowLeft, down, !disabled);

			}
			else
			{
				Keramik::CenteredPainter painter( keramik_scrollbar_vbar_arrow1 );
				painter.draw( p, r, down? cg.buttonText() : cg.button(), cg.background(), disabled, pmode() );
				drawKeramikArrow(p, cg, r, PE_ArrowUp, down, !disabled);
			}
			break;
		}

		// CHECKBOX (indicator)
		// -------------------------------------------------------------------
		case PE_Indicator:
		case PE_IndicatorMask:
			if (flags & Style_On)
				Keramik::ScaledPainter( keramik_checkbox_on ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else if (flags & Style_Off) 
				Keramik::ScaledPainter( keramik_checkbox_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			else
				Keramik::ScaledPainter( keramik_checkbox_tri ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
							
			break;

			// RADIOBUTTON (exclusive indicator)
			// -------------------------------------------------------------------
		case PE_ExclusiveIndicator:
		case PE_ExclusiveIndicatorMask:
		{

			Keramik::ScaledPainter( on ? keramik_radiobutton_on : keramik_radiobutton_off ).draw( p, r, cg.button(), cg.background(), disabled, pmode() );
			break;
		}

			// line edit frame
		case PE_PanelLineEdit:
		{
			p->setPen( cg.dark() );
			p->drawLine( x, y, x + w - 1, y );
			p->drawLine( x, y, x, y + h - 1 );
			p->setPen( cg.mid() );
			p->drawLine( x + 1, y + 1, x + w - 1, y + 1 );
			p->drawLine( x + 1, y + 1, x + 1, y + h - 1 );
			p->setPen( cg.light() );
			p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
			p->drawLine( x, y + h - 1, x + w - 1, y + h - 1);
			p->setPen( cg.light().dark( 110 ) );
			p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );
			p->drawLine( x + 1, y + h - 2, x + w - 2, y + h - 2);
			break;
		}

			// SPLITTER/DOCKWINDOW HANDLES
			// -------------------------------------------------------------------
		case PE_DockWindowResizeHandle:
		case PE_Splitter:
		{
			int x,y,w,h;
			r.rect(&x, &y, &w, &h);
			int x2 = x+w-1;
			int y2 = y+h-1;

			p->setPen(cg.dark());
			p->drawRect( r );
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


		//case PE_PanelPopup:
			//p->setPen (cg.light() );
			//p->setBrush( cg.background().light( 110 ) );
			//p->drawRect( r );
			
			//p->setPen( cg.shadow() );
			//p->drawRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2);
			//p->fillRect( visualRect( QRect( x + 1, y + 1, 23, h - 2 ), r ), cg.background().dark( 105 ) );
			//break;

			// GENERAL PANELS
			// -------------------------------------------------------------------
		case PE_Panel:
		{
			if (kickerMode)
			{
				if (p->device() && p->device()->devType() == QInternal::Widget &&
											 QCString(static_cast<QWidget*>(p->device())->className()) == "FittsLawFrame" )
				{
					int x2 = x + r.width() - 1;
					int y2 = y + r.height() - 1;
					p->setPen(cg.dark());
					p->drawLine(x+1,y2,x2-1,y2);
					p->drawLine(x2,y+1,x2,y2);
					
					p->setPen( cg.light() );
					p->drawLine(x, y, x2, y);
					p->drawLine(x, y, x, y2);

					
					return;
				}
			}
			KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
			break;
		}
		case PE_WindowFrame:
		{
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
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), true, true);
			//Keramik::ScaledPainter( keramik_menubar , Keramik::ScaledPainter::Vertical).draw( p, r, cg.button(), cg.background() );
			
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;
			if (opt.lineWidth())
			{
				p->setPen(cg.dark());
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}
			
		case PE_PanelDockWindow:		// Toolbar
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(), r.width() > r.height() );
			//Keramik::ScaledPainter( keramik_toolbar , Keramik::ScaledPainter::Vertical).draw( p, r, cg.button(), cg.background() );
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;

			if (opt.lineWidth())
			{
				p->setPen(cg.dark());
				p->drawLine(x2, y, x2, y2);
			}

			break;
		}

/*		case PE_PanelDockWindow:		// Toolbar
		{
			int x2 = r.x()+r.width()-1;
			int y2 = r.y()+r.height()-1;

			if (opt.lineWidth())
			{
				p->setPen(cg.light());
				p->drawLine(x, y, x2-1, y);
				p->drawLine(x, y, x, y2-1);
				p->setPen(cg.dark());
				p->drawLine(x, y2, x2, y2);
				p->drawLine(x2, y, x2, y2);

				// ### Qt should specify Style_Horizontal where appropriate
				renderGradient( p, QRect( x + 1, y, x2 - 1, y2 - 1),
						cg.button(), (w < h) && (pe != PE_PanelMenuBar) );
			}
			else
			{
				renderGradient( p, QRect(x, y, x2, y2),
						cg.button(), (w < h) && (pe != PE_PanelMenuBar) );
			}
			break;
		}*/



			// TOOLBAR SEPARATOR
			// -------------------------------------------------------------------
		case PE_DockWindowSeparator:
		{
			Keramik::GradientPainter::renderGradient( p, r, cg.button(),
					(flags & Style_Horizontal));
			if ( !(flags & Style_Horizontal) )
			{
				p->setPen(cg.mid());
				p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
				p->setPen(cg.light());
				p->drawLine(4, r.height()/2+1, r.width()-5, r.height()/2+1);
			}
			else
			{
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

				switch(pe)
				{
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

				if ( flags & Style_Enabled )
				{
					a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
					p->setPen( cg.buttonText() );
					p->drawLineSegments( a );
				}
				else
				{
					a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
					p->setPen( cg.light() );
					p->drawLineSegments( a );
					a.translate( -1, -1 );
					p->setPen( cg.mid() );
					p->drawLineSegments( a );
				}
				p->restore();

			}
			else
				KStyle::drawPrimitive( pe, p, r, cg, flags, opt );
		}
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
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch ( kpe )
	{
		// SLIDER GROOVE
		// -------------------------------------------------------------------
		case KPE_SliderGroove:
		{
			const QSlider* slider = static_cast< const QSlider* >( widget );
			bool horizontal = slider->orientation() == Horizontal;

			if ( horizontal )
				Keramik::RectTilePainter( keramik_slider_hgroove, false ).draw(p, r, cg.button(), cg.background(), disabled);
			else
				Keramik::RectTilePainter( keramik_slider_vgroove, true, false ).draw( p, r, cg.button(), cg.background(), disabled);
				
			break;
		}

		// SLIDER HANDLE
		// -------------------------------------------------------------------
		case KPE_SliderHandle:
			{
				const QSlider* slider = static_cast< const QSlider* >( widget );		
				bool horizontal = slider->orientation() == Horizontal;
				
				QColor hl = cg.highlight();
				if (!disabled && flags & Style_Active)
					hl = Keramik::ColorUtil::lighten(cg.highlight(),110);

				if (horizontal)
					Keramik::ScaledPainter( keramik_slider ).draw( p, r, disabled ? cg.button() : hl, 
						Qt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				else
					Keramik::ScaledPainter( keramik_vslider ).draw( p, r, disabled ? cg.button() : hl, 
						Qt::black,  disabled, Keramik::TilePainter::PaintFullBlend );
				break;
			}

		// TOOLBAR HANDLE
		// -------------------------------------------------------------------
		case KPE_ToolBarHandle: {
			int x = r.x(); int y = r.y();
			int x2 = r.x() + r.width()-1;
			int y2 = r.y() + r.height()-1;

			if (flags & Style_Horizontal) {

				Keramik::GradientPainter::renderGradient( p, r, cg.button(), true);
				p->setPen(cg.light());
				p->drawLine(x+1, y+4, x+1, y2-4);
				p->drawLine(x+3, y+4, x+3, y2-4);
				p->drawLine(x+5, y+4, x+5, y2-4);

				p->setPen(cg.mid());
				p->drawLine(x+2, y+4, x+2, y2-4);
				p->drawLine(x+4, y+4, x+4, y2-4);
				p->drawLine(x+6, y+4, x+6, y2-4);

			} else {
				
				Keramik::GradientPainter::renderGradient( p, r, cg.button(), false);
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
	bool disabled = ( flags & Style_Enabled ) == 0;
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );

	switch (element)
	{
		// PUSHBUTTON
		// -------------------------------------------------------------------
		case CE_PushButton:
			if ( widget == hoverWidget )
				flags |= Style_MouseOver;

			if ( static_cast< const QPushButton* >( widget )->isDefault( ) )
				drawPrimitive( PE_ButtonDefault, p, r, cg, flags );
			else
			{
				if (widget->parent() && widget->parent()->inherits("QToolBar"))
					toolbarBlendWidget = widget;
				
				drawPrimitive( PE_ButtonCommand, p, r, cg, flags );
				
				toolbarBlendWidget = 0;
			}
			
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
				drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - dx - 8, y + 2, dx, h - 4), r ),
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
			drawItem( p, QRect(x, y, w, h), AlignCenter | ShowPrefix, button->colorGroup(),
						button->isEnabled(), button->pixmap(), button->text(), -1,
						&button->colorGroup().buttonText() );

			if ( flags & Style_HasFocus )
				drawPrimitive( PE_FocusRect, p,
				               visualRect( subRect( SR_PushButtonFocusRect, widget ), widget ),
				               cg, flags );
			break;
		}
		
		case CE_ToolButtonLabel:
		{
			const QToolButton *toolbutton = static_cast<const QToolButton * >(widget);
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( "QToolBar" );
			QRect nr = r;
			
			if (!onToolbar)
				nr.setWidth(r.width()-2); //Account for shadow

			KStyle::drawControl(element, p, widget, nr, cg, flags, opt);
			break;
		}

		case CE_TabBarTab:
		{
			const QTabBar* tabBar = static_cast< const QTabBar* >( widget );

			bool bottom = tabBar->shape() == QTabBar::RoundedBelow ||
			              tabBar->shape() == QTabBar::TriangularBelow;

			if ( flags & Style_Selected )
				Keramik::ActiveTabPainter( bottom ).draw( p, r, cg.button(), cg.background(), !tabBar->isEnabled(), pmode());
			else
			{
				Keramik::InactiveTabPainter::Mode mode;
				int index = tabBar->indexOf( opt.tab()->identifier() );
				if ( index == 0 ) mode = Keramik::InactiveTabPainter::First;
				else if ( index == tabBar->count() - 1 ) mode = Keramik::InactiveTabPainter::Last;
				else mode = Keramik::InactiveTabPainter::Middle;
				
				if ( bottom )
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y, w, h - 4, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.dark() );
					p->drawLine( x, y, x + w, y );
				}
				else
				{
					Keramik::InactiveTabPainter( mode, bottom ).draw( p, x, y + 4, w, h - 4, cg.button(), cg.background(), disabled, pmode() );
					p->setPen( cg.light() );
					p->drawLine( x, y + h - 1, x + w, y + h - 1 );
				}
			}

			break;
		}

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
				Keramik::GradientPainter::renderGradient( p, pr, cg.button(), true, true);
				
			drawItem( p, r, AlignCenter | AlignVCenter | ShowPrefix
					| DontClip | SingleLine, cg, flags & Style_Enabled,
					mi->pixmap(), mi->text() );

			break;
		}


		// POPUPMENU ITEM
		// -------------------------------------------------------------------
		case CE_PopupMenuItem: {
			const QPopupMenu *popupmenu = static_cast< const QPopupMenu * >( widget );
			QRect main = r;

			QMenuItem *mi = opt.menuItem();

			if ( !mi )
			{
				// Don't leave blank holes if we set NoBackground for the QPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ) )
					p->fillRect( r, cg.background().light( 105 ) );

				break;
			}
			int  tab        = opt.tabWidth();
			int  checkcol   = opt.maxIconWidth();
			bool enabled    = mi->isEnabled();
			bool checkable  = popupmenu->isCheckable();
			bool active     = flags & Style_Active;
			bool etchtext   = styleHint( SH_EtchDisabledText );
			bool reverse    = QApplication::reverseLayout();
			if ( checkable )
				checkcol = QMAX( checkcol, 20 );

			// Draw the menu item background
			if ( active )
			{
				if ( enabled )
					Keramik::RowPainter( keramik_menuitem ).draw( p, main, cg.highlight(), cg.background() );
				else {
					p->fillRect( main, cg.background().light( 105 ) );
					p->drawWinFocusRect( r );
				}
			}
			// Draw the transparency pixmap
			else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
				p->drawPixmap( main.topLeft(), *widget->erasePixmap(), main );
			// Draw a solid background
			else
				p->fillRect( main, cg.background().light( 105 ) );
			// Are we a menu item separator?

			if ( mi->isSeparator() )
			{
				p->setPen( cg.mid() );
				p->drawLine( main.x()+5, main.y() + 1, main.width()-10, main.y() + 1 );
				p->setPen( cg.light() );
				p->drawLine( main.x()+5, main.y() + 2, main.width()-10 , main.y() + 2 );
				break;
			}

			QRect cr = visualRect( QRect( x + 2, y + 2, checkcol - 1, h - 4 ), r );
			// Do we have an icon?
			if ( mi->iconSet() )
			{
				QIconSet::Mode mode;
				
				
				
				// Select the correct icon from the iconset
				if ( active )
					mode = enabled ? QIconSet::Active : QIconSet::Disabled;
				else
					mode = enabled ? QIconSet::Normal : QIconSet::Disabled;
					
				// Do we have an icon and are checked at the same time?
				// Then draw a "pressed" background behind the icon
				if ( checkable && /*!active &&*/ mi->isChecked() )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
									 cg, true, 1, &cg.brush(QColorGroup::Midlight) );
				// Draw the icon
				QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
				QRect pmr( 0, 0, pixmap.width(), pixmap.height() );
				pmr.moveCenter( cr.center() );
				p->drawPixmap( pmr.topLeft(), pixmap );
			}

			// Are we checked? (This time without an icon)
			else if ( checkable && mi->isChecked() )
			{
				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
			//	if ( ! active )
					qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1,
					                 &cg.brush(QColorGroup::Midlight) );

				// Draw the checkmark
				SFlags cflags = Style_Default;
				cflags |= active ? Style_Enabled : Style_On;

				drawPrimitive( PE_CheckMark, p, cr, cg, cflags );
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
					
					//QColor draw = cg.text();
					QColor draw = (active && enabled) ? cg.highlightedText () : cg.text();
					p->setPen(draw);     


					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
							x + w - tab - rightBorder - itemHMargin - itemFrame;

						// Draw the right part of the label (accelerator text)
						if ( etchtext && !enabled ) {
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
					if ( etchtext && !enabled ) {
						// Etched text again for inactive disabled menu items...
						p->setPen( cg.light() );
						p->drawText( xp+offset, y+m+1, tw, h-2*m, text_flags, s, t );
						p->setPen( discol );
					}


					p->drawText( xp, y+m, tw, h-2*m, text_flags, s, t );

					p->setPen(cg.text());

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
				int dim = pixelMetric(PM_MenuButtonIndicator) - itemFrame;
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
		case CE_ProgressBarContents: {
			// ### Take into account totalSteps() for busy indicator
			const QProgressBar* pb = (const QProgressBar*)widget;
			QRect cr = subRect(SR_ProgressBarContents, widget);
			double progress = pb->progress();
			bool reverse = QApplication::reverseLayout();
			int steps = pb->totalSteps();

			if (!cr.isValid())
				return;

			// Draw progress bar
			if (progress > 0 || steps == 0) {
				double pg = (steps == 0) ? 0.1 : progress / steps;
				int width = QMIN(cr.width(), (int)(pg * cr.width()));

				if (steps == 0) { //Busy indicator
				
					if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless
					
					int remWidth = cr.width() - width; //Never disappear completely
					if (remWidth <= 0) remWidth = 1; //Do something non-crashy when too small...                                       
					
					int pstep =  int(progress) % ( 2 *  remWidth ); 
					
					if ( pstep > remWidth ) {
						//Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...                                           
						// - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
						pstep = - (pstep - 2 * remWidth );                                                                                      
					}
					
					if (reverse)
						Keramik::RowPainter(keramik_progressbar).draw(p, cr.x() + cr.width() - width - pstep, cr.y(), width, cr.height(),
									 cg.highlight(), cg.background() );
					else
						Keramik::RowPainter(keramik_progressbar).draw(p, cr.x() + pstep, cr.y(), width, cr.height(),
									cg.highlight(), cg.background() );
									
					return;                                       
				}
				

				if (reverse)
					Keramik::ProgressBarPainter(keramik_progressbar, reverse).draw( p, cr.x()+(cr.width()-width), cr.y(), width, cr.height(), cg.highlight(), cg.background());
				else
					Keramik::ProgressBarPainter(keramik_progressbar, reverse).draw( p, cr.x(), cr.y(), width, cr.height(), cg.highlight(), cg.background());
			}
			break;
		}


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
	p->fillRect(r, color1);
	maskMode = true;
	drawControl( element, p, widget, r, QApplication::palette().active(), QStyle::Style_Default, opt);
	maskMode = false;
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
	bool disabled = ( flags & Style_Enabled ) == 0;
	switch(control)
	{
		// COMBOBOX
		// -------------------------------------------------------------------
		case CC_ComboBox:
		{
			const QComboBox* cb = static_cast< const QComboBox* >( widget );
			QPixmap buf;
			QPainter* p2 = p;
			
			QRect br = r;
			
			if (controls == SC_All)
			{
				buf.resize(r.width(), r.height() );
				br.setX(0);
				br.setY(0);
				p2 = new QPainter(&buf);
			}
                        
			
			if ( br.width() >= 28 && br.height() > 20 ) br.addCoords( 0, -2, 0, 0 );
			if ( controls & SC_ComboBoxFrame )
			{
				if (widget->parent() && 
						( widget->parent()->inherits("QToolBar")|| !qstrcmp(widget->parent()->name(), kdeToolbarWidget) ) )
				{
					toolbarBlendWidget = widget;
				}
				
				if ( widget == hoverWidget )
					flags |= Style_MouseOver;
					
				drawPrimitive( PE_ButtonCommand, p2, br, cg, flags );
				
				toolbarBlendWidget = 0;
			}
				
			// don't draw the focus rect etc. on the mask
			if ( cg.button() == color1 && cg.background() == color0 ) break;

			if ( controls & SC_ComboBoxArrow )
			{
				if ( active ) flags |= Style_On;

				QRect ar = querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxArrow );
				ar.setWidth(ar.width()-13);

				QRect rr = visualRect( QRect( ar.x(), ar.y() + 4, loader.size( 
keramik_ripple ).width(), ar.height() - 8 ), widget );

				ar = visualRect( QRect( ar.x() + loader.size( keramik_ripple ).width() + 
4, ar.y(), 11, ar.height() ), widget );

				QPointArray a;

				a.setPoints(QCOORDARRLEN(keramik_combo_arrow), keramik_combo_arrow);
				p2->save();

				a.translate( ar.x() + ar.width() / 2, ar.y() + ar.height() / 2 );
				p2->setPen( cg.buttonText() );
				p2->drawLineSegments( a );
				p2->restore();

				Keramik::ScaledPainter( keramik_ripple ).draw( p2, rr, cg.button(), Qt::black, disabled, Keramik::TilePainter::PaintFullBlend );
			}

			if ( controls & SC_ComboBoxEditField )
			{
				if ( cb->editable() )
				{
					QRect er = visualRect( querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField ), widget );
					er.addCoords( -2, -2, 2, 2 );
					p2->fillRect( er, cg.base() );
					drawPrimitive( PE_PanelLineEdit, p2, er, cg );
					Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( p2, er, cg.button(), 
						Qt::black, false, pmodeFullBlend() );
				}
				else if ( cb->hasFocus() )
				{
					QRect re = QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cb), widget);
					p2->fillRect( re, cg.brush( QColorGroup::Highlight ) );
					drawPrimitive( PE_FocusRect, p2, re, cg,
					Style_FocusAtBorder, QStyleOption( cg.highlight() ) );
				}
				// QComboBox draws the text on its own and uses the painter's current colours
				if ( cb->hasFocus() )
				{
					p->setPen( cg.highlightedText() );
					p->setBackgroundColor( cg.highlight() );
				}
				else
				{
					p->setPen( cg.text() );
					p->setBackgroundColor( cg.button() );
				}
			}
			
			if (p2 != p)
			{
				p2->end();
				delete p2;
				p->drawPixmap(r.x(), r.y(), buf);
			}

			break;
		}

		case CC_SpinWidget:
		{
			const QSpinWidget* sw = static_cast< const QSpinWidget* >( widget );
			QRect br = visualRect( querySubControlMetrics( CC_SpinWidget, widget, SC_SpinWidgetButtonField ), widget );
			if ( controls & SC_SpinWidgetButtonField )
			{
				Keramik::SpinBoxPainter().draw( p, br, cg.button(), cg.background(), !sw->isEnabled() );
				if ( active & SC_SpinWidgetUp )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background() );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_up ).draw( p, br.x(), br.y() + 3, br.width(), br.height() / 2, cg.button(), cg.background(), !sw->isUpEnabled() );
				if ( active & SC_SpinWidgetDown )
					Keramik::CenteredPainter( keramik_spinbox_pressed_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2 , br.width(), br.height() / 2 - 8, cg.button(), cg.background()  );
				else
					Keramik::CenteredPainter( keramik_spinbox_arrow_down ).draw( p, br.x(), br.y() + br.height() / 2, br.width(), br.height() / 2 - 8, cg.background(), cg.button(), !sw->isDownEnabled() );
			}

			if ( controls & SC_SpinWidgetFrame )
				drawPrimitive( PE_PanelLineEdit, p, r, cg );

			break;
		}
		case CC_ScrollBar:
		{
			const QScrollBar* sb = static_cast< const QScrollBar* >( widget );
			bool horizontal = sb->orientation() == Horizontal;
			QRect slider, subpage, addpage, subline, addline;
			if ( sb->minValue() == sb->maxValue() ) flags &= ~Style_Enabled;

			slider = querySubControlMetrics( control, widget, SC_ScrollBarSlider, opt );
			subpage = querySubControlMetrics( control, widget, SC_ScrollBarSubPage, opt );
			addpage = querySubControlMetrics( control, widget, SC_ScrollBarAddPage, opt );
			subline = querySubControlMetrics( control, widget, SC_ScrollBarSubLine, opt );
			addline = querySubControlMetrics( control, widget, SC_ScrollBarAddLine, opt );

			if ( controls & SC_ScrollBarSubLine )
				drawPrimitive( PE_ScrollBarSubLine, p, subline, cg,
				               flags | ( ( active & SC_ScrollBarSubLine ) ? Style_Down : 0 ) );

			QRegion clip;
			if ( controls & SC_ScrollBarSubPage ) clip |= subpage;
			if ( controls & SC_ScrollBarAddPage ) clip |= addpage;
			if ( horizontal )
				clip |= QRect( slider.x(), 0, slider.width(), sb->height() );
			else
				clip |= QRect( 0, slider.y(), sb->width(), slider.height() );
			clip ^= slider;

			p->setClipRegion( clip );
			Keramik::ScrollBarPainter( KeramikGroove1, 2, horizontal ).draw( p, slider | subpage | addpage, cg.button(), cg.background(), disabled );

			if ( controls & SC_ScrollBarSlider )
			{
				if ( horizontal )
					p->setClipRect( slider.x(), slider.y(), addpage.right() - slider.x() + 1, slider.height() );
				else
					p->setClipRect( slider.x(), slider.y(), slider.width(), addpage.bottom() - slider.y() + 1 );
				drawPrimitive( PE_ScrollBarSlider, p, slider, cg, 
					flags | ( ( active == SC_ScrollBarSlider ) ? Style_Down : 0 )  );
			}
			p->setClipping( false );

			if ( controls & ( SC_ScrollBarSubLine | SC_ScrollBarAddLine ) )
			{
				drawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags );
				if ( active & SC_ScrollBarSubLine )
				{
					if ( horizontal )
						p->setClipRect( QRect( addline.x(), addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( QRect( addline.x(), addline.y(), addline.width(), addline.height() / 2 ) );
					drawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags | Style_Down );
				}
				else if ( active & SC_ScrollBarAddLine )
				{
					if ( horizontal )
						p->setClipRect( QRect( addline.x() + addline.width() / 2, addline.y(), addline.width() / 2, addline.height() ) );
					else
						p->setClipRect( QRect( addline.x(), addline.y() + addline.height() / 2, addline.width(), addline.height() / 2 ) );
					drawPrimitive( PE_ScrollBarAddLine, p, addline, cg, flags | Style_Down );
				}
			}
            

			break;
		}

		// TOOLBUTTON
		// -------------------------------------------------------------------
		case CC_ToolButton: {
			const QToolButton *toolbutton = (const QToolButton *) widget;
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( "QToolBar" );
			bool onExtender = !onToolbar && widget->parentWidget() && widget->parentWidget()->inherits( "QToolBarExtensionWidget" );

			QRect button, menuarea;
			button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

			SFlags bflags = flags,
				   mflags = flags;

			if (active & SC_ToolButton)
				bflags |= Style_Down;
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;
				
			if (onToolbar &&  static_cast<QToolBar*>(widget->parent())->orientation() == Qt::Horizontal)
				bflags |= Style_Horizontal;

			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised))
					drawPrimitive(onToolbar ? PE_ButtonTool : PE_ButtonCommand, p, button, cg,
					 bflags, opt);

				// Check whether to draw a background pixmap
				else if ( toolbutton->parentWidget() &&
						  toolbutton->parentWidget()->backgroundPixmap() &&
						  !toolbutton->parentWidget()->backgroundPixmap()->isNull() )
				{
					QPixmap pixmap = *(toolbutton->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
				}
				else if (onToolbar)
				{
					QToolBar* parent = (QToolBar*)widget->parent();
					QRect pr = parent->rect();

					Keramik::GradientPainter::renderGradient( p, r, cg.button(), 
												parent->orientation() == Qt::Horizontal, false /*Not a menubar*/,  
												r.x(), r.y(), pr.width()-2, pr.height()-2);
				}
				else if (onExtender)
				{
					QWidget* parent = (QWidget*)widget->parent();
					QToolBar* toolbar = (QToolBar*)parent->parent();
					QRect tr = toolbar->rect();
					//QPainter p2(toolbar);
					//p2.fillRect(tr, Qt::red);
					//p2.end();
					Keramik::GradientPainter::renderGradient( p, QRect(r.x(), r.y(), tr.width(), tr.height()), cg.button(), toolbar->orientation() != Qt::Vertical );
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
			KStyle::drawComplexControl( control, p, widget,
						r, cg, flags, controls, active, opt );
	}
}

void KeramikStyle::drawComplexControlMask( ComplexControl control,
                                         QPainter *p,
                                         const QWidget *widget,
                                         const QRect &r,
                                         const QStyleOption& opt ) const
{
	if (control == CC_ComboBox)
	{
		maskMode = true;
		drawComplexControl(CC_ComboBox, p, widget, r, 
				QApplication::palette().active(), Style_Default, 
				SC_ComboBoxFrame,SC_None, opt);
		maskMode = false;

	}
	else
		p->fillRect(r, color1);
	
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
			return 12;
		case PM_SliderControlThickness:
			return loader.size( keramik_slider ).height() - 4;
		case PM_SliderThickness:
			return loader.size( keramik_slider ).height();
			
		case PM_ButtonShiftHorizontal:		
			return 0;
		case PM_ButtonShiftVertical: // Offset by 1
			return 1;


		// CHECKBOXES / RADIO BUTTONS
		// -------------------------------------------------------------------
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
			return loader.size( keramik_radiobutton_on ).width();
		case PM_ExclusiveIndicatorHeight:
			return loader.size( keramik_radiobutton_on ).height();
		case PM_IndicatorWidth:				// Checkbox size
			return loader.size( keramik_checkbox_on ).width();
		case PM_IndicatorHeight:
			return loader.size( keramik_checkbox_on) .height();

		case PM_ScrollBarExtent:
			return loader.size( keramik_scrollbar_vbar + KeramikGroove1).width();
		case PM_ScrollBarSliderMin:
			return loader.size( keramik_scrollbar_vbar + KeramikSlider1 ).height() + 
                        loader.size( keramik_scrollbar_vbar + KeramikSlider3 ).height();

		case PM_DefaultFrameWidth:
			return 1;

		case PM_MenuButtonIndicator:
			return 8;

		case PM_TabBarTabVSpace:
			return 14;

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
		{
			const QPushButton* btn = static_cast< const QPushButton* >( widget );
			
			int w = contentSize.width() + 2 * pixelMetric( PM_ButtonMargin, widget );
			int h = contentSize.height() + 2 * pixelMetric( PM_ButtonMargin, widget );
			if ( btn->text().isEmpty() && contentSize.width() < 32 ) return QSize( w, h );
			

			//For some reason kcontrol no longer does this...
			//if ( btn->isDefault() || btn->autoDefault() )
			//            w = QMAX( w, 40 );

			return QSize( w + 30, h + 5 ); //MX: No longer blank space -- can make a bit smaller
		}
                
		case CT_ToolButton:
		{
			bool onToolbar = widget->parentWidget() && widget->parentWidget()->inherits( "QToolBar" );
			if (!onToolbar) //Behaves like a button, so scale appropriately to the border
			{
				int w = contentSize.width();
				int h = contentSize.height();
				return QSize( w + 12, h + 10 );
			}
			else
			{
				return KStyle::sizeFromContents( contents, widget, contentSize, opt );
			}
		}

		case CT_ComboBox:
			return QSize( contentSize.width() + 11 + loader.size( keramik_ripple ).width() + 36,
			              contentSize.height() + 9 );

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
				w = 30; // Arbitrary
				h = 3;
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


QStyle::SubControl KeramikStyle::querySubControl( ComplexControl control,
	                                              const QWidget* widget,
                                                  const QPoint& point,
                                                  const QStyleOption& opt ) const
{
	SubControl result = KStyle::querySubControl( control, widget, point, opt );
	if ( control == CC_ScrollBar && result == SC_ScrollBarAddLine )
	{
		QRect addline = querySubControlMetrics( control, widget, result, opt );
		if ( static_cast< const QScrollBar* >( widget )->orientation() == Horizontal )
		{
			if ( point.x() < addline.center().x() ) result = SC_ScrollBarSubLine;
		}
		else if ( point.y() < addline.center().y() ) result = SC_ScrollBarSubLine;
	}
	return result;
}

QRect KeramikStyle::querySubControlMetrics( ComplexControl control,
									const QWidget* widget,
	                              SubControl subcontrol,
	                              const QStyleOption& opt ) const
{
	switch ( control )
	{
		case CC_ComboBox:
		{
			int arrow = 11 + loader.size( keramik_ripple ).width();
			switch ( subcontrol )
			{
				case SC_ComboBoxArrow:
					return QRect( widget->width() - arrow - 14, 0, arrow+13, widget->height() );

				case SC_ComboBoxEditField:
				{
					if ( widget->width() < 36 || widget->height() < 22 )
					{

						return QRect( 4, 3, widget->width() - arrow - 20, widget->height() - 6 );
					}
//					if ( static_cast< const QComboBox* >( widget )->editable() )
//						return QRect( 10, 8, widget->width() - arrow - 30, widget->height() - 16 );
					return QRect( 8, 4, widget->width() - arrow - 28, widget->height() - 11 );
				}

				default: break;
			}
			break;
		}

		case CC_ScrollBar:
		{
			const QScrollBar* sb = static_cast< const QScrollBar* >( widget );
			bool horizontal = sb->orientation() == Horizontal;
			int addline, subline, sliderpos, sliderlen, maxlen, slidermin;
			if ( horizontal )
			{
				subline = loader.size( keramik_scrollbar_hbar_arrow1 ).width();
				addline = loader.size( keramik_scrollbar_hbar_arrow2 ).width();
				maxlen = sb->width() - subline - addline + 2;
			}
			else
			{
				subline = loader.size( keramik_scrollbar_vbar_arrow1 ).height();
				addline = loader.size( keramik_scrollbar_vbar_arrow2 ).height();
				maxlen = sb->height() - subline - addline + 2;
			}
			sliderpos = sb->sliderStart();
			if ( sb->minValue() != sb->maxValue() )
			{
				int range = sb->maxValue() - sb->minValue();
				sliderlen = ( sb->pageStep() * maxlen ) / ( range + sb->pageStep() );
				slidermin = pixelMetric( PM_ScrollBarSliderMin, sb );
				if ( sliderlen < slidermin ) sliderlen = slidermin;
				if ( sliderlen > maxlen ) sliderlen = maxlen;
			}
			else sliderlen = maxlen;

			switch ( subcontrol )
			{
				case SC_ScrollBarGroove:
					if ( horizontal ) return QRect( subline, 0, maxlen, sb->height() );
					else return QRect( 0, subline, sb->width(), maxlen );

				case SC_ScrollBarSlider:
					if (horizontal) return QRect( sliderpos, 0, sliderlen, sb->height() );
					else return QRect( 0, sliderpos, sb->width(), sliderlen );

				case SC_ScrollBarSubLine:
					if ( horizontal ) return QRect( 0, 0, subline, sb->height() );
					else return QRect( 0, 0, sb->width(), subline );

				case SC_ScrollBarAddLine:
					if ( horizontal ) return QRect( sb->width() - addline, 0, addline, sb->height() );
					else return QRect( 0, sb->height() - addline, sb->width(), addline );

				case SC_ScrollBarSubPage:
					if ( horizontal ) return QRect( subline, 0, sliderpos - subline, sb->height() );
					else return QRect( 0, subline, sb->width(), sliderpos - subline );

				case SC_ScrollBarAddPage:
					if ( horizontal ) return QRect( sliderpos + sliderlen, 0, sb->width() - addline  - (sliderpos + sliderlen) , sb->height() );
					else return QRect( 0, sliderpos + sliderlen, sb->width(), sb->height() - addline - (sliderpos + sliderlen) 
                    /*maxlen - sliderpos - sliderlen + subline - 5*/ );

				default: break;
			};
			break;
		}
		case CC_Slider:
		{
			const QSlider* sl = static_cast< const QSlider* >( widget );
			bool horizontal = sl->orientation() == Horizontal;
			QSlider::TickSetting ticks = sl->tickmarks();
			int pos = sl->sliderStart();
			int size = pixelMetric( PM_SliderControlThickness, widget );
			int handleSize = pixelMetric( PM_SliderThickness, widget );
			int len = pixelMetric( PM_SliderLength, widget );
			switch ( subcontrol )
			{
				case SC_SliderGroove:
					if ( horizontal )
					{
						if ( ticks == QSlider::Both )
							return QRect( 0, ( sl->height() - size ) / 2, sl->width(), size );
						else if ( ticks == QSlider::Above )
							return QRect( 0, sl->height() - size - ( handleSize - size ) / 2, sl->width(), size );
						return QRect( 0, ( handleSize - size ) / 2, sl->width(), size );
					}
					else
					{
						if ( ticks == QSlider::Both )
							return QRect( ( sl->width() - size ) / 2, 0, size, sl->height() );
						else if ( ticks == QSlider::Above )
							return QRect( sl->width() - size - ( handleSize - size ) / 2, 0, size, sl->height() );
						return QRect( ( handleSize - size ) / 2, 0, size, sl->height() );
					}
				case SC_SliderHandle: 
					if ( horizontal )
					{
						if ( ticks == QSlider::Both )
							return QRect( pos, ( sl->height() - handleSize ) / 2, len, handleSize );
						else if ( ticks == QSlider::Above )
							return QRect( pos, sl->height() - handleSize, len, handleSize );
						return QRect( pos, 0, len, handleSize );
					}
					else
					{
						if ( ticks == QSlider::Both )
							return QRect( ( sl->width() - handleSize ) / 2, pos, handleSize, len );
						else if ( ticks == QSlider::Above )
							return QRect( sl->width() - handleSize, pos, handleSize, len );
						return QRect( 0, pos, handleSize, len );
					}
				default: break;
			}
			break;
		}
		default: break;
	}
	return KStyle::querySubControlMetrics( control, widget, subcontrol, opt );
}


#include <config.h>

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
//Xlib headers are a mess -> include them down here (any way to ensure that we go second in enable-final order?)
#include  <X11/Xlib.h>
#include <X11/extensions/shape.h>
#endif 

bool KeramikStyle::eventFilter( QObject* object, QEvent* event )
{
	if (KStyle::eventFilter( object, event ))
		return true;

	if ( !object->isWidgetType() ) return false;

	if ( object->inherits("QPushButton") || object->inherits("QComboBox") )
	{
		if ( (event->type() == QEvent::Enter) && static_cast<QWidget*>(object)->isEnabled())
		{
			QWidget* button = static_cast<QWidget*>(object);
			hoverWidget = button;
			button->repaint( false );
		} 
		else if ( (event->type() == QEvent::Leave) && (object == hoverWidget) ) 
		{
			QWidget* button = static_cast<QWidget*>(object);
			hoverWidget = 0;
			button->repaint( false );
		}
		return false;
	}
	else if ( event->type() == QEvent::Paint && object->inherits( "QLineEdit" ) )
	{
		static bool recursion = false;
		if (recursion )
			return false;

		recursion = true;
		object->event( static_cast< QPaintEvent* >( event ) );
		QWidget* widget = static_cast< QWidget* >( object );
		QPainter p( widget );
		Keramik::RectTilePainter( keramik_frame_shadow, false, false, 2, 2 ).draw( &p, widget->rect(), 
			widget->palette().color( QPalette::Normal, QColorGroup::Button ), 
			Qt::black, false, Keramik::TilePainter::PaintFullBlend);
		recursion = false;
		return true;
	}
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H	
	else if ( event->type() == QEvent::Resize && object->inherits("QListBox") )
	{
		QListBox* listbox = static_cast<QListBox*>(object);
		QResizeEvent* resize = static_cast<QResizeEvent*>(event);
		if (resize->size().height() < 6)
			return false;

		//CHECKME: Not sure the rects are perfect..
		XRectangle rects[5] = {
			{0, 0, resize->size().width()-2, resize->size().height()-6},
			{1, resize->size().height()-6, resize->size().width()-3, 1},
			{1, resize->size().height()-5, resize->size().width()-3, 1},
			{2, resize->size().height()-4, resize->size().width()-5, 1},
			{3, resize->size().height()-3, resize->size().width()-7, 1}
		};
		
		XShapeCombineRectangles(qt_xdisplay(), listbox->handle(), ShapeBounding, 0, 0,
			  rects, 5, ShapeSet, YXSorted);
	}	
#endif	
	else if ( event->type() == QEvent::Paint && object->inherits("QListBox") )
	{
		static bool recursion = false;
		if (recursion )
			return false;
		QListBox* listbox = (QListBox*) object;
		QPaintEvent* paint = (QPaintEvent*) event;
		
		
		
		if ( !listbox->contentsRect().contains( paint->rect() ) )
		{
			QPainter p( listbox );
			Keramik::RectTilePainter( keramik_combobox_list, false ).draw( &p, 0, 0, listbox->width(), listbox->height(), 
					listbox->palette().color( QPalette::Normal, QColorGroup::Button ),
					listbox->palette().color( QPalette::Normal, QColorGroup::Background ) );
					
			QPaintEvent newpaint( paint->region().intersect( listbox->contentsRect() ), paint->erased() );
			recursion = true;
			object->event( &newpaint );
			recursion = false;
			return true;
		}
	}
	else if ( event->type() == QEvent::Show && object->inherits( "QListBox" ) )
	{
		QListBox* listbox = (QListBox*) object;
		QRect geometry = listbox->geometry();
		if ( listbox->inherits( "KCompletionBox" ) )
			geometry.addCoords( -4, 4, 10, 8 );
		else geometry.addCoords( 4, -4, -6, 4 );
		listbox->setGeometry( geometry );
	}
	else if (event->type() == QEvent::Paint && 
			 object->parent() && !qstrcmp(object->name(), kdeToolbarWidget) )
	{
		// Draw a gradient background for custom widgets in the toolbar
		// that have specified a "kde toolbar widget" name.

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

//		QRect r  = widget->rect();
		QRect pr = parent->rect();
		bool horiz_grad = pr.width() > pr.height();

		// Check if the parent is a QToolbar, and use its orientation, else guess.
		QToolBar* tb = dynamic_cast<QToolBar*>(parent);
		if (tb) horiz_grad = tb->orientation() == Qt::Horizontal;
		QPainter p( widget );

		Keramik::GradientPainter::renderGradient( &p,
				QRect(0, 0, pr.width(), pr.height()),
			   	parent->colorGroup().button(), horiz_grad, false , x_offset, y_offset);

		return false;	// Now draw the contents
	}
	else if (event->type() == QEvent::Paint  &&  object->parent() && object->parent()->inherits("QToolBar"))
	{
		// We need to override the paint event to draw a 
		// gradient on a QToolBarExtensionWidget.
		QToolBar* toolbar = static_cast<QToolBar*>(object->parent());
		QWidget* widget = static_cast<QWidget*>(object);
		QRect wr = widget->rect (), tr = toolbar->rect();
		QPainter p( widget );
		
		Keramik::GradientPainter::renderGradient( &p, QRect(0 , 0, wr.width(), wr.height()), 
			widget->colorGroup().button(), tr.width() > tr.height() );
		p.setPen( toolbar->colorGroup().dark() );
		if ( toolbar->orientation() == Qt::Horizontal )
			p.drawLine( wr.width()-1, 0, wr.width()-1, wr.height()-1 );
		else
			p.drawLine( 0, wr.height()-1, wr.width()-1, wr.height()-1 );
		return true;

	}
	return false;
}

// vim: ts=4 sw=4 noet
