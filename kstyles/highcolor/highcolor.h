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

#ifndef __HIGHCOLOR_H
#define __HIGHCOLOR_H

#include <qstyle.h>
#include <qcommonstyle.h>
#include <qbitmap.h>
#include <kdrawutil.h>
#include <kpixmap.h>
#include <qintdict.h>


enum GradientType{ VSmall=0, VMed, VLarge, HMed, HLarge, GradientCount };
 
class GradientSet
{
	public:
		GradientSet(const QColor &baseColor);
		~GradientSet();

		KPixmap* gradient(GradientType type);
		QColor* color() { return(&c); }
	private:
		KPixmap *gradients[5];
		QColor c;
};


class HighColorStyle : public QCommonStyle
{
	Q_OBJECT

	public:
		HighColorStyle( bool hc );
		virtual ~HighColorStyle();

		void polish( QWidget* widget );
		void unPolish( QWidget* widget );

		virtual void polishPopupMenu( QPopupMenu* );
		
		void drawPrimitive( PrimitiveElement pe,
					QPainter *p,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;
		
		void drawControl( ControlElement element,
					QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;

		void drawControlMask( ControlElement element,
					QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QStyleOption& = QStyleOption::Default ) const;
		
		void drawComplexControl( ComplexControl control,
					QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					SCFlags controls = SC_All,
					SCFlags active = SC_None,
					const QStyleOption& = QStyleOption::Default ) const;

		void drawComplexControlMask( ComplexControl control,
					QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QStyleOption& = QStyleOption::Default ) const;
		
		SubControl querySubControl( ComplexControl control,
					const QWidget *widget,
					const QPoint &pos,
					const QStyleOption& = QStyleOption::Default ) const;

		QRect querySubControlMetrics( ComplexControl control,
					const QWidget *widget,
					SubControl sc,
					const QStyleOption& = QStyleOption::Default ) const;
		
		int pixelMetric( PixelMetric m, 
					const QWidget *widget = 0 ) const;
		
		QSize sizeFromContents( ContentsType contents,
					const QWidget *widget,
					const QSize &contentSize,
					const QStyleOption& opt ) const;

		QRect subRect( SubRect r, 
					const QWidget *widget ) const;

		// Fix Qt3's wacky image positions
		QPixmap stylePixmap( StylePixmap stylepixmap,
					const QWidget *widget = 0,
					const QStyleOption& = QStyleOption::Default ) const;

		int styleHint( StyleHint sh, 
					const QWidget *w = 0,
					const QStyleOption &opt = QStyleOption::Default,
					QStyleHintReturn* shr = 0 ) const;

	protected:
		bool eventFilter( QObject *object, QEvent *event );
		
		void renderGradient( QPainter* p, 
					const QRect& r, 
					QColor clr,
					bool horizontal,
					int px=0, 
					int py=0,
					int pwidth=-1,
					int pheight=-1 ) const;

		QWidget *hoverWidget;
		bool     highcolor;

	private:
		// Disable copy constructor and = operator
		HighColorStyle( const HighColorStyle & );
		HighColorStyle& operator=( const HighColorStyle & );
		QStyle *winstyle;
};

// vim: set noet ts=4 sw=4:

#endif
