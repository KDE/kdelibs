/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>

   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>

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

// $Id$

#ifndef __keramik_h__
#define __keramik_h__

#include <qframe.h>
#include <kstyle.h>

#include "pixmaploader.h"

class QProgressBar;

class KeramikStyle : public KStyle
{
	Q_OBJECT

public:
	KeramikStyle();
	virtual ~KeramikStyle();

	void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup &cg, const QPopupMenu* ) const;
	QPixmap stylePixmap(StylePixmap stylepixmap, const QWidget* widget, const QStyleOption& opt) const;

	void polish( QWidget* widget );
	void unPolish( QWidget* widget );
	void polish( QPalette& );
	void polish( QApplication *app );

	void drawKStylePrimitive( KStylePrimitive kpe,
	                          QPainter* p,
	                          const QWidget* widget,
	                          const QRect& r,
	                          const QColorGroup& cg,
	                          SFlags flags = Style_Default,
	                          const QStyleOption& = QStyleOption::Default ) const;

	void drawPrimitive( PrimitiveElement pe,
	                    QPainter* p,
	                    const QRect& r,
	                    const QColorGroup& cg,
	                    SFlags flags = Style_Default,
	                    const QStyleOption& = QStyleOption::Default ) const;

	void drawControl( ControlElement element,
	                  QPainter* p,
	                  const QWidget* widget,
	                  const QRect& r,
	                  const QColorGroup& cg,
	                  SFlags flags = Style_Default,
	                  const QStyleOption& opt = QStyleOption::Default ) const;

	void drawControlMask( ControlElement element,
	                      QPainter* p,
	                      const QWidget* widget,
	                      const QRect& r,
	                      const QStyleOption& opt = QStyleOption::Default ) const;

	void drawComplexControl( ComplexControl control,
	                         QPainter* p,
	                         const QWidget* widget,
	                         const QRect& r,
	                         const QColorGroup& cg,
	                         SFlags flags = Style_Default,
	                         SCFlags controls = SC_All,
	                         SCFlags active = SC_None,
	                         const QStyleOption& = QStyleOption::Default ) const;

	void drawComplexControlMask( ComplexControl control,
	                             QPainter* p,
	                             const QWidget* widget,
	                             const QRect& r,
	                             const QStyleOption& = QStyleOption::Default ) const;

	int pixelMetric( PixelMetric m, const QWidget* widget = 0 ) const;

	QSize sizeFromContents( ContentsType contents,
	                        const QWidget* widget,
	                        const QSize& contentSize,
	                        const QStyleOption& opt ) const;

	SubControl querySubControl( ComplexControl control,
	                            const QWidget* widget,
	                            const QPoint& point,
						        const QStyleOption& opt = QStyleOption::Default ) const;

	QRect querySubControlMetrics( ComplexControl control,
	                              const QWidget* widget,
	                              SubControl subcontrol,
	                              const QStyleOption& opt = QStyleOption::Default ) const;

private slots:
	//Animation slots.
	void updateProgressPos();
	void progressBarDestroyed(QObject* bar);

private:

	bool isSizeConstrainedCombo(const QComboBox* combo) const;
	bool isFormWidget          (const QWidget*   widget) const;

	///Configuration settings
	bool animateProgressBar;
	bool highlightScrollBar;

	//Rendering flags
	mutable bool forceSmallMode;
	mutable bool maskMode;   //Ugly round trip flag to permit masking with little code;
	mutable bool formMode;   //Set when rendering form widgets

	mutable const QWidget* toolbarBlendWidget;  //Ditto for blending with toolbars

	enum TitleBarMode
	{
		None = 0,
		Regular,
		Maximized
	};

	mutable TitleBarMode titleBarMode; //Set when passing back CC_TilteBar modes to handle
	//PE_ButtonTool properly for them, as well as when handling CC_ToolButton from
	//The maximized window controls.

	mutable bool flatMode; //Set when calling PE_PushButton or PE_ButtonDefault
	// on a flat button.

	mutable bool customScrollMode; //Set when drawing scrollbars with custom colors.
	
	bool firstComboPopupRelease;

	//Animation support.
	QMap<QProgressBar*, int> progAnimWidgets;

	bool eventFilter( QObject* object, QEvent* event );

	Keramik::TilePainter::PaintMode pmode() const
	{
		if (formMode)
		{
			//If we're a form widget, we blend on painting, and consider ourselves
			//not to have a mask (so we don't get clipped to it)
			if (maskMode)
				return Keramik::TilePainter::PaintTrivialMask;
			else
				return Keramik::TilePainter::PaintFullBlend;
		}
		else
		{
			if (maskMode)
				return Keramik::TilePainter::PaintMask;
			else
				return Keramik::TilePainter::PaintNormal;
		}
	}

	Keramik::TilePainter::PaintMode pmodeFullBlend() const
	{
		return maskMode?Keramik::TilePainter::PaintMask : Keramik::TilePainter::PaintFullBlend;
	}

	QWidget* hoverWidget;


	bool kickerMode;
	
	// For progress bar animation
	QTimer *animationTimer;

	QRect subRect(SubRect r, const QWidget *widget) const;

	// Disable copy constructor and = operator
	KeramikStyle( const KeramikStyle&  );
	KeramikStyle& operator=( const KeramikStyle&  );
};

#endif

// vim: ts=4 sw=4 noet
// kate: indent-width 4; replace-tabs off; tab-width 4;
