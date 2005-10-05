/*
 * High Contrast Style (version 1.0)
 *     Copyright (C) 2004 Olaf Schmidt <ojschmidt@kde.org>
 *
 * Derived from Axes Style
 *     Copyright (C) 2003 Maksim Orlovich <orlovich@cs.rochester.edu>
 * 
 * Axes Style based on KDE 3 HighColor Style,
 *     Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *               (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>
 * 
 * KDE 3 HighColor Style drawing routines adapted from the KDE2 HCStyle,
 *     Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *               (C) 2000 Dirk Mueller          <mueller@kde.org>
 *               (C) 2001 Martijn Klingens      <klingens@kde.org>
 *
 * Includes portions from KStyle,
 *     Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __HIGHCONTRAST_H
#define __HIGHCONTRAST_H

#include <qbitmap.h>
#include <q3intdict.h>
#include <kdrawutil.h>
#include <kpixmap.h>
#include <kstyle.h>


class Q3PopupMenu;

class HighContrastStyle : public KStyle
{
	Q_OBJECT

	public:
		HighContrastStyle();
		virtual ~HighContrastStyle();
        
		void polish( QPalette& pal );        

		void polish( QWidget* widget );
		void unPolish( QWidget* widget );

		void drawKStylePrimitive( KStylePrimitive kpe,
					QPainter* p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags = Style_Default,
					const QStyleOption& = QStyleOption::Default ) const;

		void drawPrimitive( PrimitiveElement pe,
					QPainter* p,
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

		QRect querySubControlMetrics( ComplexControl control,
					const QWidget* widget,
					SubControl subcontrol,
					const QStyleOption& opt = QStyleOption::Default ) const;


		void drawItem( QPainter *p,
					const QRect &r,
					int flags,
					const QColorGroup &cg,
					bool enabled,
					const QPixmap *pixmap,
					const QString &text,
					int len = -1,
					const QColor *penColor = 0 ) const;

		int pixelMetric( PixelMetric m,
					const QWidget *widget = 0 ) const;

		int kPixelMetric( KStylePixelMetric m,
					const QWidget *widget = 0 ) const;

		QSize sizeFromContents( ContentsType contents,
					const QWidget *widget,
					const QSize &contentSize,
					const QStyleOption& opt ) const;

		QRect subRect (SubRect subrect, const QWidget * widget) const;

	protected:
		bool eventFilter( QObject *object, QEvent *event );

		QWidget     *hoverWidget;

	private:
		void setColorsNormal (QPainter* p, const QColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsButton (QPainter* p, const QColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsText (QPainter* p, const QColorGroup& cg, int flags = Style_Enabled, int highlight = Style_Down|Style_MouseOver) const;
		void setColorsHighlight (QPainter* p, const QColorGroup& cg, int flags = Style_Enabled) const;
		void setColorsByState (QPainter* p, const QColorGroup& cg, const QColor& fg, const QColor& bg, int flags, int highlight) const;

		void drawRect (QPainter* p, QRect r, int offset = 0, bool filled = true) const;
		void drawRoundRect (QPainter* p, QRect r, int offset = 0, bool filled = true) const;
		void drawEllipse (QPainter* p, QRect r, int offset = 0, bool filled = true) const;
		void drawArrow (QPainter* p, QRect r, PrimitiveElement arrow, int offset = 0) const;

		int basicLineWidth;
		// Disable copy constructor and = operator
		HighContrastStyle( const HighContrastStyle & );
		HighContrastStyle& operator=( const HighContrastStyle & );
};

// vim: set noet ts=4 sw=4:
// kate: indent-width 4; replace-tabs off; smart-indent on; tab-width 4;

#endif
