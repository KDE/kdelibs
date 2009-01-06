/*
 * Copyright (C) 2008-2009 Fredrik Höglund <fredrik@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef BORDERARCSTROKER_H
#define BORDERARCSTROKER_H

#include <QPainterPath>
#include <QRect>

namespace khtml {

class BorderArcStroker
{
public:
    BorderArcStroker();
    ~BorderArcStroker();

    void setArc(const QRectF &r, qreal startAngle, qreal _sweepLength) { rect = r; angle = startAngle; sweepLength = _sweepLength; }
    void setPenWidth(qreal leftRight, qreal topBottom) { hlw = leftRight; vlw = topBottom; }
    void setDashPattern(qreal dashLength, qreal spaceLength) { pattern[0] = dashLength; pattern[1] = spaceLength; }
    void setDashOffset(qreal offset) { patternOffset = offset; }

    QPainterPath createStroke(qreal *nextOffset = 0) const;

private:
    QRectF rect;
    qreal angle;
    qreal hlw;
    qreal vlw;
    qreal sweepLength;
    qreal patternOffset;
    qreal pattern[2];
};

} // namespace khtml

#endif
