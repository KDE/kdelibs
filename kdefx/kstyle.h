/*
 * $Id$
 * 
 * KStyle
 * Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 * 
 * QWindowsStyle CC_ListView and style images were kindly donated by TrollTech,
 * Copyright (C) 1998-2000 TrollTech AS.
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

#ifndef __KSTYLE_H
#define __KSTYLE_H

// W A R N I N G
// -------------
// This API is still subject to change.
// I will remove this warning when I feel the API is sufficiently flexible.

#include <qcommonstyle.h>

class TransparencyHandler;
class KPixmap;

class KStylePrivate;
/** 
 * Simplifies and extends the QStyle API to make style coding easier.
 *  
 * The KStyle class provides a simple internal menu transparency engine
 * which attempts to use XRender for accelerated blending where requested,
 * or falls back to fast internal software tinting/blending routines.
 * It also simplifies more complex portions of the QStyle API, such as
 * the PopupMenuItems, ScrollBars and Sliders by providing extra "primitive
 * elements" which are simple to implement by the style writer.
 *
 * @author Karol Szwed (gallium@kde.org)
 * @see QStyle::QStyle
 * @see QCommonStyle::QCommonStyle
 * @version $Id$
 */
class KStyle: public QCommonStyle
{
	Q_OBJECT

	public:

		/**
		 * KStyle Flags:
		 * 
		 * @li Default - Default style setting, where menu transparency
		 * and the FilledFrameWorkaround are disabled.
		 * 
		 * @li AllowMenuTransparency - Enable this flag to use KStyle's 
		 * internal menu transparency engine.
		 * 
		 * @li FilledFrameWorkaround - Enable this flag to facilitate 
		 * proper repaints of QMenuBars and QToolBars when the style chooses 
		 * to paint the interior of a QFrame. The style primitives in question 
		 * are PE_PanelMenuBar and PE_PanelDockWindow. The HighColor style uses
		 * this workaround to enable painting of gradients in menubars and 
		 * toolbars.
		 */
		typedef uint KStyleFlags;
		enum KStyleOption {
			Default 			  =		0x00000000,		// All options disabled.
			AllowMenuTransparency =		0x00000001,
			FilledFrameWorkaround = 	0x00000002
		};

		/**
		 * KStyle ScrollBarType:
		 *
		 * Allows the style writer to easily select what type of scrollbar
		 * should be used without having to duplicate large amounts of source
		 * code by implementing the complex control CC_ScrollBar.
		 *
		 * @li WindowsStyleScrollBar - Two button scrollbar with the previous
		 * button at the top/left, and the next button at the bottom/right.
		 *
		 * @li PlatinumStyleSrollBar - Two button scrollbar with both the 
		 * previous and next buttons at the bottom/right.
		 *
		 * @li ThreeButtonScrollBar - KDE style three button scrollbar with
		 * two previous buttons, and one next button. The next button is always
		 * at the bottom/right, whilst the two previous buttons are on either 
		 * end of the scrollbar.
		 *
		 * @li NextStyleScrollBar - Similar to the PlatinumStyle scroll bar, but
		 * with the buttons grouped on the opposite end of the scrollbar.
		 */
		enum KStyleScrollBarType {
			WindowsStyleScrollBar  = 	0x00000000,
			PlatinumStyleScrollBar = 	0x00000001,
			ThreeButtonScrollBar   = 	0x00000002,
			NextStyleScrollBar     = 	0x00000004
		};

		/** 
		 * Constructs a KStyle object.
		 *
		 * Select the appropriate KStyle flags and scrollbar type
		 * for your style. The user's style preferences selected in KControl
		 * are read by using QSettings and are automatically applied to the style.
		 * As a fallback, KStyle paints progressbars and tabbars. It inherits from
		 * QCommonStyle for speed, so don't expect much to be implemented. 
		 *
		 * It is advisable to use a currently implemented style such as the HighColor
		 * style as a foundation for any new KStyle, so the limited number of
		 * drawing fallbacks should not prove problematic.
		 *
		 * @see KStyle::KStyleFlags
		 * @see KStyle::KStyleScrollBarType
		 * @author Karol Szwed (gallium@kde.org)
		 */
		KStyle( KStyleFlags flags = KStyle::Default, 
				KStyleScrollBarType sbtype = KStyle::WindowsStyleScrollBar );

		/** 
		 * Destructs the KStyle object.
		 */
		~KStyle();

		/**
		 * Modifies the scrollbar type used by the style.
		 * 
		 * This function is only provided for convenience. It allows
		 * you to make a late decision about what scrollbar type to use for the
		 * style after performing some processing in your style's constructor.
		 * In most situations however, setting the scrollbar type via the KStyle
		 * constructor should suffice.
		 */
		void setScrollBarType(KStyleScrollBarType sbtype);

		/**
		 * Returns the KStyle flags used to initialise the style.
		 *
		 * This is used solely for the kcmstyle module, and hence is internal.
		 */
		KStyleFlags styleFlags() const;

		// ---------------------------------------------------------------------------

		/**
		 * This virtual function defines the pixmap used to blend between the popup
		 * menu and the background to create different menu transparency effects.
		 * For example, you can fill the pixmap "pix" with a gradient based on the
		 * popup's colorGroup, a texture, or some other fancy painting routine.
		 * KStyle will then internally blend this pixmap with a snapshot of the
		 * background behind the popupMenu to create the illusion of transparency.
		 * 
		 * This virtual is never called if XRender/Software blending is disabled by
		 * the user in KDE's style control module.
		 */
		virtual void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup& cg, 
											const QPopupMenu* popup ) const;

		/**
		 * KStyle Primitive Elements:
		 *
		 * The KStyle class extends the Qt's Style API by providing certain 
		 * simplifications for parts of QStyle. To do this, the KStylePrimitive
		 * elements were defined, which are very similar to Qt's PrimitiveElement.
		 * 
		 * The first three Handle primitives simplify and extend PE_DockWindowHandle, 
		 * so do not reimplement PE_DockWindowHandle if you want the KStyle handle 
		 * simplifications to be operable. Similarly do not reimplement CC_Slider,
		 * SC_SliderGroove and SC_SliderHandle when using the KStyle slider
		 * primitives. KStyle automatically double-buffers slider painting
		 * when they are drawn via these KStyle primitives to avoid flicker.
		 *
		 * @li KPE_DockWindowHandle - This primitive is already implemented in KStyle,
		 * and paints a bevelled rect with the DockWindow caption text. Re-implement
		 * this primitive to perform other more fancy effects when drawing the dock window
		 * handle.
		 *
		 * @li KPE_ToolBarHandle - This primitive must be reimplemented. It currently
		 * only paints a filled rectangle as default behaviour. This primitive is used
		 * to render QToolBar handles.
		 *
		 * @li KPE_GeneralHandle - This primitive must be reimplemented. It is used
		 * to render general handles that are not part of a QToolBar or QDockWindow, such
		 * as the applet handles used in Kicker. The default implementation paints a filled
		 * rect of arbitrary color.
		 *
		 * @li KPE_SliderGroove - This primitive must be reimplemented. It is used to 
		 * paint the slider groove. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_SliderHandle - This primitive must be reimplemented. It is used to
		 * paint the slider handle. The default implementation paints a filled rect of
		 * arbitrary color.
		 *
		 * @li KPE_ListViewExpander - This primitive is already implemented in KStyle. It
		 * is used to draw the Expand/Collapse element in QListViews. To indicate the 
		 * expanded state, the style flags are set to Style_Off, while Style_On implies collapsed.
		 *
		 * @li KPE_ListViewBranch - This primitive is already implemented in KStyle. It is
		 * used to draw the ListView branches where necessary.
		 */
		enum KStylePrimitive {
			KPE_DockWindowHandle,
			KPE_ToolBarHandle,
			KPE_GeneralHandle,

			KPE_SliderGroove,
			KPE_SliderHandle,

			KPE_ListViewExpander,
			KPE_ListViewBranch
		};

		/**
		 * This function is identical to Qt's QStyle::drawPrimitive(), except that 
		 * it adds one further parameter, 'widget', that can be used to determine 
		 * the widget state of the KStylePrimitive in question.
		 *
		 * @see KStyle::KStylePrimitive
		 * @see QStyle::drawPrimitive
		 * @see QStyle::drawComplexControl
		 */
		virtual void drawKStylePrimitive( KStylePrimitive kpe,
					QPainter* p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;


		enum KStylePixelMetric {
			KPM_MenuItemSeparatorHeight		= 0x00000001,
			KPM_MenuItemHMargin				= 0x00000002,
			KPM_MenuItemVMargin				= 0x00000004,
			KPM_MenuItemHFrame				= 0x00000008,
			KPM_MenuItemVFrame				= 0x00000010,
			KPM_MenuItemCheckMarkHMargin	= 0x00000020,
			KPM_MenuItemArrowHMargin		= 0x00000040,
			KPM_MenuItemTabSpacing			= 0x00000080,

			KPM_ListViewBranchThickness		= 0x00000100
		};

		int kPixelMetric( KStylePixelMetric kpm, const QWidget* widget = 0 ) const;

		// ---------------------------------------------------------------------------

		void polish( QWidget* widget );
		void unPolish( QWidget* widget );
		void polishPopupMenu( QPopupMenu* );

		void drawPrimitive( PrimitiveElement pe,
					QPainter* p,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;

		void drawControl( ControlElement element,
					QPainter* p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;

		void drawComplexControl( ComplexControl control,
					QPainter *p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const QStyleOption& = QStyleOption::Default ) const;

		SubControl querySubControl( ComplexControl control,
					const QWidget* widget,
					const QPoint &pos,
					const QStyleOption& = QStyleOption::Default ) const;

		QRect querySubControlMetrics( ComplexControl control,
					const QWidget* widget,
					SubControl sc,
					const QStyleOption& = QStyleOption::Default ) const;

		int pixelMetric( PixelMetric m, 
					const QWidget* widget = 0 ) const;

		QRect subRect( SubRect r, 
					const QWidget* widget ) const;

		QPixmap stylePixmap( StylePixmap stylepixmap,
					const QWidget* widget = 0,
					const QStyleOption& = QStyleOption::Default ) const;

		int styleHint( StyleHint sh, 
					const QWidget* w = 0,
					const QStyleOption &opt = QStyleOption::Default,
					QStyleHintReturn* shr = 0 ) const;

	protected:
		bool eventFilter( QObject* object, QEvent* event );

	private:
		// Disable copy constructor and = operator
		KStyle( const KStyle & );
		KStyle& operator=( const KStyle & );

	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		KStylePrivate *d;
};


// vim: set noet ts=4 sw=4:
#endif

