/*-
 * B3Style is 99.99% based on B2Style
 * B2Style (C)2000 Daniel M. Duley <mosfet@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef B3STYLE_H
#define B3STYLE_H

#include <kdestyle.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <limits.h>

class B3Style : public KDEStyle
{
public:
    B3Style();
    virtual ~B3Style();
    void drawScrollBarControls(QPainter*,  const QScrollBar*, int sliderStart,
                               uint controls, uint activeControl );
    void scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                          int &sliderMax, int &sliderLength,
                          int &buttonDim);

    void drawArrow(QPainter *p, Qt::ArrowType type, bool down,
                   int x, int y, int w, int h, const QColorGroup &g,
                   bool enabled=true, const QBrush *fill = 0);
protected:
    void drawSBDeco(QPainter *p, const QRect &r, const QColorGroup &g,
                   bool horiz);
    void drawSBButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool down=false);
    void drawSBDecoButton(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g);
    void drawSBArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                     int y, int w, int h, const QColorGroup &g,
                     bool enabled = true, const QBrush *fill = 0L);
};

#endif
