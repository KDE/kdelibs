/*
 * $Id$
 * 
 * KStyle
 * Copyright (C) 2001 Karol Szwed <gallium@kde.org>
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


class KStyle: public QCommonStyle
{
	Q_OBJECT

	public:
		/** TODO: DOCUMENTATION
		 */

		typedef uint KStyleFlags;
		enum KStyleOption {
			Default 			  =		0x00000000,		// All options disabled.
			AllowMenuTransparency =		0x00000001,
			FilledFrameWorkaround = 	0x00000002
		};

		enum KStyleScrollBarType {
			WindowsStyleScrollBar  = 	0x00000000,		// Default
			PlatinumStyleScrollBar = 	0x00000001,
			ThreeButtonScrollBar   = 	0x00000002
		};

		KStyle( KStyleFlags flags = KStyle::Default, 
				KStyleScrollBarType sbtype = KStyle::WindowsStyleScrollBar );
		~KStyle();

		// ---------------------------------------------------------------------------

		/** TODO: DOCUMENTATION
		 */

		// This is the pixmap used to blend between the popup menu and the background
		// to create different menu transparency effects which are accelerated
		// by XRender. This virtual is never called if no XRender support is available,
		// or if Qt's Xrender support is currently not enabled.
		virtual void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup& cg );


		// KStyle primitive elements - expect more to come soon
		enum KStylePrimitive {
			// These simplify and extend PE_DockWindowHandle
			KPE_DockWindowHandle,
			KPE_ToolBarHandle,
			KPE_GeneralHandle,	// Usually the kicker handle

			// This makes creating sliders simple by breaking up CC_Slider
			// The slider is automatically double-buffered as well for no flicker.
			KPE_SliderGroove,
			KPE_SliderHandle
		};

		// Simplifies the QStyle API somewhat by breaking up the controls further
		virtual void drawKStylePrimitive( KStylePrimitive kpe,
					QPainter* p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;

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

		bool  highcolor                : 1;
		bool  allowMenuTransparency    : 1;
		bool  useMenuTransparency      : 1;
		bool  useFilledFrameWorkaround : 1;
		bool  etchDisabledText         : 1;
		bool  menuAltKeyNavigation     : 1;
		int   popupMenuDelay;
		float menuOpacity;
		
		KStyleScrollBarType  scrollbarType;
		TransparencyHandler* menuHandler;
		QStyle* winstyle;		// ### REMOVE
};


// vim: set noet ts=4 sw=4:
#endif

