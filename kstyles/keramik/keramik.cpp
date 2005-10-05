/* Keramik Style
   Copyright (c) 2002       Malte Starostik <malte@kde.org>
             (c) 2002-2005  Maksim Orlovich <maksim@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "kstyle.h"
#include <qdrawutil.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include "colorutil.h"
#include "pixmaploader.h"
#include "gradients.h"
#include "keramikimage.h"
#include <qpolygon.h>
#include <QStyleOptionTab>

#define loader Keramik::PixmapLoader::the()

static const int keramik_up_arrow[] =
	{
		0, -4, 0, -4,
		-1, -3, 1, -3,
		-2, -2, 2, -2,
		-3, -1, 3, -1,
		-4, 0, 4, 0,
		-2, 1, 2, 1,
		-2, 2, 2, 2,
		-2, 3, 2, 3,
		-2, 4, 2, 4
	};
	
static const int keramik_down_arrow[] =
	{
		0, 4, 0, 4,
		-1, 3, 1, 3,
		-2, 2, 2, 2,
		-3, 1, 3, 1,
		-4, 0, 4, 0,
		-2, -1, 2, -1,
		-2, -2, 2, -2,
		-2, -3, 2, -3,
		-2, -4, 2, -4
	};


	static const int keramik_right_arrow[] =
	{
		4, 0, 4, 0,
		3, -1, 3, 1,
		2, -2, 2, 2,
		1, -3, 1, 3,
		0, -4, 0, 4,
		-1, -2, -1, 2,
		-2, -2, -2, 2,
		-3, -2, -3, 2,
		-4, -2, -4, 2
	};
	
	static const int keramik_left_arrow[] =
	{
		-4, 0, -4, 0,
		-3, -1, -3, 1,
		-2, -2, -2, 2,
		-1, -3, -1, 3,
		0, -4, 0, 4,
		1, -2, 1, 2,
		2, -2, 2, 2,
		3, -2, 3, 2,
		4, -2, 4, 2
	};

	
		
#define POLY_LEN(x) sizeof(x)/(sizeof(int)*2)



class KeramikStyle: public KStyle
{
public:
	KeramikStyle()
	{
		//Just for the heck of it, make the focus rect's RHS nearly flush
		setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin, 3);
		setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Right, 2);
		setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Top, 2);
		setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Bot, 2);

		setWidgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing,   10);

		setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin + Left,  2);
		setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin + Right, 2);

		setWidgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight,
						loader.size( keramik_scrollbar_vbar + KeramikSlider1 ).height() +
						loader.size( keramik_scrollbar_vbar + KeramikSlider3 ).height());


		setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ArrowColor,
							ColorMode(ColorMode::BWAutoContrastMode, QPalette::Button));

		setWidgetLayoutProp(WT_ScrollBar, ScrollBar::ActiveArrowColor,
							ColorMode(ColorMode::BWAutoContrastMode, QPalette::ButtonText));
		
		//### HACK,for now.
		//setWidgetLayoutProp(WT_DockWidgetTitle, DockWidgetTitle::Margin, 8);

		//setWidgetLayoutProp(WT_ProgressBar, ProgressBar::SideText, 1);

		setWidgetLayoutProp(WT_ScrollBar, ScrollBar::SingleButtonHeight,
								loader.size(keramik_scrollbar_vbar_arrow1).height());

		setWidgetLayoutProp(WT_Slider, Slider::HandleLength, 12);
		setWidgetLayoutProp(WT_Slider, Slider::HandleThickness, loader.size(keramik_slider).height());
	}
	
	void drawKStylePrimitive(WidgetType widgetType, int primitive, 
				 const QStyleOption* opt,
				 QRect r, QPalette pal, State flags,
				 QPainter* p, 
				 const QWidget* widget,
				 KStyle::Option* kOpt) const
	{
		bool disabled = !(flags & State_Enabled);
		switch (widgetType)
		{
			case WT_PushButton:
			{
				switch (primitive)
				{
					case Generic::Bevel:
					{
						const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(opt);
						if (bOpt &&  //### helper function in KStyle?
							(bOpt->features & QStyleOptionButton::DefaultButton))
							return; //The default indicator does the job for us.
						
						bool sunken   = (flags & State_On) || (flags & State_Sunken);
						bool disabled = !(flags & State_Enabled);

						int  name;

						if (sunken)
							name = keramik_pushbutton_pressed;
						else
							name =  keramik_pushbutton;

						if (flags & State_MouseOver && name == keramik_pushbutton )
							name = keramik_pushbutton_hov;

						Keramik::RectTilePainter( name, false ).draw(p, r, pal.button().color(), pal.background().color(), disabled);
						//, pmode() );

						return;
					}
					
					case PushButton::DefaultButtonBevel:
					{
						bool sunken = (flags & State_On) || (flags & State_Sunken);

						int id;
						if ( sunken ) id  = keramik_pushbutton_default_pressed;
						else id = keramik_pushbutton_default;

						if (flags & State_MouseOver && id == keramik_pushbutton_default )
							id = keramik_pushbutton_default_hov;


						Keramik::RectTilePainter( id, false ).draw(p, r, pal.button().color(), pal.background().color(), !(flags & State_Enabled));
						//,  pmode() );
						break;

						return;
					}
				}
			}
			break;
			
			case WT_Splitter:
			{
				if (primitive == Generic::Bevel)
				{
					int x,y,w,h;
					r.getRect(&x, &y, &w, &h);
					int x2 = x+w-1;
					int y2 = y+h-1;
			
					//### should probably use color() directly to avoid creating a brush
					p->setPen(pal.dark().color());
					p->drawRect( r );
					p->setPen(pal.background().color());
					p->drawPoint(x, y);
					p->drawPoint(x2, y);
					p->drawPoint(x, y2);
					p->drawPoint(x2, y2);
					p->setPen(pal.light().color());
					p->drawLine(x+1, y+1, x+1, y2-1);
					p->drawLine(x+1, y+1, x2-1, y+1);
					p->setPen(pal.midlight().color());
					p->drawLine(x+2, y+2, x+2, y2-2);
					p->drawLine(x+2, y+2, x2-2, y+2);
					p->setPen(pal.mid().color());
					p->drawLine(x2-1, y+1, x2-1, y2-1);
					p->drawLine(x+1, y2-1, x2-1, y2-1);
					p->fillRect(x+3, y+3, w-5, h-5, pal.brush(QPalette::Background));
					
					return;
				}
				break;
			}
			
			case WT_CheckBox:
			{
				switch (primitive)
				{
					case CheckBox::CheckOn:
						Keramik::CenteredPainter(keramik_checkbox_on).draw(p, r, pal.button().color(), 
										pal.background().color(), disabled);//, pmode() );
						return;
					case CheckBox::CheckOff:
						Keramik::CenteredPainter(keramik_checkbox_off).draw(p, r, pal.button().color(), 
										pal.background().color(), disabled);//, pmode() );
						return;
					case CheckBox::CheckTriState:
						Keramik::CenteredPainter(keramik_checkbox_tri).draw(p, r, pal.button().color(), 
						pal.background().color(), disabled);//, pmode() );
						return;
				}
				break;
			}
			
			case WT_RadioButton:
			{
				switch (primitive)
				{
					case RadioButton::RadioOn:
						Keramik::CenteredPainter(keramik_radiobutton_on).draw(p, r, pal.button().color(), 
												pal.background().color(), disabled);//, pmode() );
						return;
					case RadioButton::RadioOff:
						Keramik::CenteredPainter(keramik_radiobutton_off).draw(p, r, pal.button().color(), 
						pal.background().color(), disabled);//, pmode() );
						return;
				}
				break;
			}

			case WT_ProgressBar:
			{
				switch (primitive)
				{
					case ProgressBar::BusyIndicator:
						Keramik::RowPainter(keramik_progressbar).draw(p, r, 
										 pal.highlight().color(), pal.background().color());
						return;
					case ProgressBar::Indicator:
						Keramik::ProgressBarPainter(keramik_progressbar, opt->direction == Qt::RightToLeft).draw(p, r,
							pal.highlight().color(), pal.background().color());
						return;
				}
			}
			break;

			case WT_MenuBar:
			{
				switch (primitive)
				{
					case Generic::Bevel:
						Keramik::GradientPainter::renderGradient( p, r, pal.button().color(), true, true);
						return;
				}
			}
			break;

			case WT_MenuBarItem:
			{
				switch (primitive)
				{
					case Generic::Bevel:
					{
						const QStyleOptionMenuItem* miOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(opt);
						bool active  = flags & State_Selected;
						bool focused = flags & State_HasFocus;

						if ( active && focused )
							qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
											pal, true, 1, &pal.brush(QPalette::Midlight));
						else
							Keramik::GradientPainter::renderGradient( p, miOpt->menuRect, pal.button().color(), true, true);
						return;
					}
				}
			}
			break;

			case WT_Menu:
			{
				switch (primitive)
				{
					case Generic::Frame:
					{
						qDrawShadePanel(p, r, pal);
						return;
					}

					case Menu::Background:
					{
						p->fillRect( r, pal.background().color().light( 105 ) );
						return;
					}

					case Menu::TearOff:
					{
						//### hmm,may be KStyle should do a default for this.
						//### FIXME
						return;
					}

					//TODO:scrollr
				}
			}
			break;

			case WT_MenuItem:
			{
				switch (primitive)
				{
					case MenuItem::Separator:
					{
						r = centerRect(r, r.width(), 2); //Center...
						p->setPen( pal.mid().color() );
						p->drawLine(r.x()+5, r.y(), r.right()-4, r.y());
						p->setPen( pal.light().color() );
						p->drawLine(r.x()+5, r.y() + 1, r.right()-4, r.y() + 1);
						return;
					}

					case MenuItem::ItemIndicator:
					{
						if (flags & State_Enabled)
							Keramik::RowPainter( keramik_menuitem ).draw(p, r, pal.highlight().color(), pal.background().color());
						else
							drawKStylePrimitive(WT_Generic, Generic::FocusIndicator, opt, r, pal, flags, p, widget, kOpt);
						return;
					}
				}
			}
			break;

			case WT_ScrollBar:
			{
				bool down = (flags & State_Sunken);
				switch (primitive)
				{
					case ScrollBar::SliderVert:
					case ScrollBar::SliderHor:
					{
						bool horizontal = (primitive == ScrollBar::SliderHor);
						
						bool active     = (flags & State_Sunken);
						int name = KeramikSlider1;
						unsigned int count = 3;

						int w = r.width();
						int h = r.height();
						
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
			
						QColor col = pal.highlight().color();

						//### what to do with the customScrollMode stuff?
						//I suppose KStyle can take care of it, somehow?
						//if (customScrollMode || !highlightScrollBar)
						//	col = pal.button();
			
						if (!active)
							Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, col, pal.background().color(), false);
						else
							Keramik::ScrollBarPainter( name, count, horizontal ).draw( p, r, 												Keramik::ColorUtil::lighten(col ,110),
													pal.background().color(), false );
						return;
					}

					case ScrollBar::DoubleButtonHor:
					{
						const DoubleButtonOption* bOpt = extractOption<const DoubleButtonOption*>(kOpt);

						//Draw the entire, unselected bevel.
						Keramik::CenteredPainter painter(keramik_scrollbar_hbar_arrow2);
						painter.draw(p, r, pal.button().color(), pal.background().color(), !(flags & State_Enabled));

						p->setPen(pal.buttonText());
						p->drawLine(r.x()+r.width()/2 - 1, r.y() + r.height()/2 - 3,
									r.x()+r.width()/2 - 1, r.y() + r.height()/2 + 3);

 						//Check whether we need to draw any of the buttons
 						if (bOpt->activeButton != DoubleButtonOption::None)
 						{
							if (bOpt->activeButton == DoubleButtonOption::Right)
							{
								p->setClipRect(r.x() + r.width()/2, r.y(), r.width()/2, r.height());
								painter.draw(p, r, pal.buttonText().color(), pal.background().color(),
												!(flags & State_Enabled));
							}
							else
							{ //Left
								p->setClipRect(r.x(), r.y(), r.width()/2, r.height());
								painter.draw(p, r, pal.buttonText().color(), pal.background().color(),
										!(flags & State_Enabled));
							}

							p->setClipping(false);
 						}

						return;
					}

					case ScrollBar::DoubleButtonVert:
					{
						const DoubleButtonOption* bOpt = extractOption<const DoubleButtonOption*>(kOpt);

						//Draw the entire, unselected bevel.
						Keramik::CenteredPainter painter(keramik_scrollbar_vbar_arrow2);
						painter.draw(p, r, pal.button().color(), pal.background().color(), !(flags & State_Enabled));

						p->setPen(pal.buttonText());
						p->drawLine(r.x()+r.width()/2 - 4, r.y()+r.height()/2,
									r.x()+r.width()/2 + 2, r.y()+r.height()/2);


 						//Check whether any of the buttons is down
 						if (bOpt->activeButton != DoubleButtonOption::None)
 						{
							if (bOpt->activeButton == DoubleButtonOption::Bottom)
							{	//down
								p->setClipRect(r.x(), r.y() + r.height()/2, r.width(), r.height()/2);
								painter.draw(p, r, pal.buttonText().color(), pal.background().color(),
											!(flags & State_Enabled));
							}
							else
							{	//up.
								p->setClipRect(r.x(), r.y(), r.width(), r.height()/2);
								painter.draw(p, r, pal.buttonText().color(), pal.background().color(),
											!(flags & State_Enabled));
							}

							p->setClipping(false);
						}

						return;
					}

					case ScrollBar::SingleButtonHor:
					{
						Keramik::CenteredPainter painter(keramik_scrollbar_hbar_arrow1 );
						painter.draw( p, r, down? pal.buttonText().color() : pal.button().color(),
												pal.background().color(), disabled);
						return;
					}

					case ScrollBar::SingleButtonVert:
					{
						Keramik::CenteredPainter painter(keramik_scrollbar_vbar_arrow1 );
						painter.draw( p, r, down? pal.buttonText().color() : pal.button().color(),
												pal.background().color(), disabled);
						return;
					}

					case ScrollBar::GrooveAreaVert:
						Keramik::ScrollBarPainter(KeramikGroove1, 2, false).draw(
									p, r, pal.button().color(), pal.background().color(), disabled);
						return;

					case ScrollBar::GrooveAreaHor:
						Keramik::ScrollBarPainter(KeramikGroove1, 2, true).draw(
									p, r, pal.button().color(), pal.background().color(), disabled);
						return;
				}
				
				if (primitive >= Generic::ArrowUp && primitive <= Generic::ArrowLeft)
				{
					KStyle::ColorOption* colorOpt   = extractOption<KStyle::ColorOption*>(kOpt);
					QColor               arrowColor = colorOpt->color.color(pal);
					
					QPolygon poly;
					switch (primitive)
					{
						case Generic::ArrowUp:
							poly.setPoints(POLY_LEN(keramik_up_arrow), keramik_up_arrow);
							break;
		
						case Generic::ArrowDown:
							poly.setPoints(POLY_LEN(keramik_down_arrow), keramik_down_arrow);
							break;
		
						case Generic::ArrowLeft:
							poly.setPoints(POLY_LEN(keramik_left_arrow), keramik_left_arrow);
							break;
		
						default:
							poly.setPoints(POLY_LEN(keramik_right_arrow), keramik_right_arrow);
					}
		
		
					if ( flags & State_Enabled )
					{
						//CHECKME: Why is the -1 needed?
						poly.translate(r.x() + r.width()/2 - 1, r.y() + r.height()/2);
		
						p->setPen(arrowColor);
						p->drawPolygon(poly);
					}
					else
					{
						//Disabled ones ignore color parameter
						poly.translate(r.x() + r.width()/2, r.y() + r.height()/2 + 1);
						p->setPen(pal.light().color());
						p->drawPolygon(poly);
						
						poly.translate(-1,-1);
						p->setPen(pal.mid().color());
						p->drawPolygon(poly);
					}
		
					return;
				}

			}

			case WT_TabBar:
			{
				const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(opt);
				switch (primitive)
				{
					case TabBar::NorthTab:
					case TabBar::SouthTab:
					{
						QStyleOptionTab::TabPosition pos = tabOpt->position;
						//Adjust for RTL
						if (tabOpt->direction == Qt::RightToLeft)
						{
							if (pos == QStyleOptionTab::End)
								pos = QStyleOptionTab::Beginning;
							else if (pos == QStyleOptionTab::Beginning)
								pos = QStyleOptionTab::End;
						}
					
						if ( flags & State_Selected )
						{
							QRect tabRect = r;
							//If not the right-most tab, readjust the painting to be one pixel wider
							//to avoid a doubled line
							if (pos != QStyleOptionTab::End)
									tabRect.setWidth( tabRect.width() + 1);
	
							Keramik::ActiveTabPainter(primitive == TabBar::SouthTab).draw(p, tabRect,
									pal.button().color().light(110), pal.background().color(),
									disabled);
						}
						else
						{
							int x, y, w, h;
							r.getRect(&x, &y, &w, &h);
							if (primitive == TabBar::SouthTab)
							{
								Keramik::InactiveTabPainter(pos, true).draw(
									p, x, y, w, h - 3, pal.button().color(), pal.background().color(),
									disabled);
								p->setPen  (pal.dark());
								p->drawLine(x, y, x + w, y);
							}
							else
							{
								Keramik::InactiveTabPainter(pos, false).draw(
									p, x, y + 3, w, h - 3, pal.button().color(), pal.background().color(), disabled);
								p->setPen  (pal.light());
								p->drawLine(x, y + h - 1, x + w, y + h - 1);
							}
						}
					}
					return;
				//### TODO: Handle east, west tabs
				};
			} //WT_Tab
			break;

			case WT_Slider:
			{
				switch (primitive)
				{
					case Slider::GrooveVert:
						Keramik::RectTilePainter(keramik_slider_vgroove, true, false).draw(
							p, r, pal.button().color(), pal.background().color(), disabled);
						return;
					case Slider::GrooveHor:
						Keramik::RectTilePainter(keramik_slider_hgroove, false).draw(
							p, r, pal.button().color(), pal.background().color(), disabled);
						return;
					case Slider::HandleVert:
					case Slider::HandleHor:
					{
						if (primitive == Slider::HandleHor)
							r.setY(r.y() + 2);
						else
							r.setX(r.x() + 2);

						QColor hl = pal.highlight().color();
						if (!disabled && flags & State_Active)
							hl = Keramik::ColorUtil::lighten(pal.highlight().color() ,110);

						int primCode = (primitive == Slider::HandleVert) ?
											keramik_vslider : keramik_slider;
						
						Keramik::ScaledPainter(primCode).draw(
							p, r, disabled ? pal.button().color() : hl, Qt::black,  disabled, Keramik::TilePainter::PaintFullBlend);
						return;
					}
				}
			} //WT_Slider
			break;
		}

		//Handle default fallbacks
		
		KStyle::drawKStylePrimitive(widgetType, primitive, opt,
					    r, pal, flags, p, widget, kOpt);
	}

};

K_EXPORT_STYLE("Keramik-KStyle4", KeramikStyle)

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
