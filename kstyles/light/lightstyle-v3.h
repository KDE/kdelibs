/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#ifndef LIGHTSTYLE_V3_H
#define LIGHTSTYLE_V3_H


#ifndef QT_H
#include <qcommonstyle.h>
#endif // QT_H

#include "kstyle.h"

#ifdef QT_PLUGIN
#  define Q_EXPORT_STYLE_LIGHT_V3
#else
#  define Q_EXPORT_STYLE_LIGHT_V3 Q_EXPORT
#endif // QT_PLUGIN


class Q_EXPORT_STYLE_LIGHT_V3 LightStyleV3 : public KStyle
{
    Q_OBJECT
    QStyle *basestyle;

public:
    LightStyleV3();
    virtual ~LightStyleV3();

    void polishPopupMenu( QPopupMenu * );

    void drawPrimitive(PrimitiveElement, QPainter *, const QRect &, const QColorGroup &,
		       SFlags = Style_Default,
		       const QStyleOption & = QStyleOption::Default ) const;

    void drawControl(ControlElement, QPainter *, const QWidget *, const QRect &,
		     const QColorGroup &, SFlags = Style_Default,
		     const QStyleOption & = QStyleOption::Default ) const;
    void drawControlMask(ControlElement, QPainter *, const QWidget *, const QRect &,
			 const QStyleOption & = QStyleOption::Default) const;

    QRect subRect(SubRect, const QWidget *) const;

    void drawComplexControl(ComplexControl, QPainter *, const QWidget *, const QRect &,
			    const QColorGroup &, SFlags = Style_Default,
			    SCFlags = SC_All, SCFlags = SC_None,
			    const QStyleOption & = QStyleOption::Default ) const;

    QRect querySubControlMetrics(ComplexControl, const QWidget *, SubControl,
				 const QStyleOption & = QStyleOption::Default ) const;

    SubControl querySubControl(ComplexControl, const QWidget *, const QPoint &,
			       const QStyleOption &data = QStyleOption::Default ) const;

    int pixelMetric(PixelMetric, const QWidget * = 0 ) const;

    QSize sizeFromContents(ContentsType, const QWidget *, const QSize &,
			   const QStyleOption & = QStyleOption::Default ) const;

    int styleHint(StyleHint, const QWidget * = 0,
		  const QStyleOption & = QStyleOption::Default,
		  QStyleHintReturn * = 0 ) const;

    QPixmap stylePixmap( StylePixmap stylepixmap,
			 const QWidget* widget = 0,
			 const QStyleOption& = QStyleOption::Default ) const;
};


#endif // LIGHTSTYLE_V3_H
