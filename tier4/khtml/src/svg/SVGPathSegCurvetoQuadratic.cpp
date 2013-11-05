/*
    Copyright (C) 2004, 2005, 2006 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006 Rob Buis <buis@kde.org>

    This file is part of the KDE project

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

#include "wtf/Platform.h"

#if ENABLE(SVG)
#include "SVGPathSegCurvetoQuadratic.h"

#include "SVGStyledElement.h"

namespace WebCore {

SVGPathSegCurvetoQuadraticAbs::SVGPathSegCurvetoQuadraticAbs(float x, float y, float x1, float y1)
    : SVGPathSeg()
    , m_x(x)
    , m_y(y)
    , m_x1(x1)
    , m_y1(y1)
{
}

SVGPathSegCurvetoQuadraticAbs::~SVGPathSegCurvetoQuadraticAbs()
{
}

void SVGPathSegCurvetoQuadraticAbs::setX(float x)
{
    m_x = x;
}

float SVGPathSegCurvetoQuadraticAbs::x() const
{
    return m_x;
}

void SVGPathSegCurvetoQuadraticAbs::setY(float y)
{
    m_y = y;
}

float SVGPathSegCurvetoQuadraticAbs::y() const
{
    return m_y;
}

void SVGPathSegCurvetoQuadraticAbs::setX1(float x1)
{
    m_x1 = x1;
}

float SVGPathSegCurvetoQuadraticAbs::x1() const
{
    return m_x1;
}

void SVGPathSegCurvetoQuadraticAbs::setY1(float y1)
{
    m_y1 = y1;
}

float SVGPathSegCurvetoQuadraticAbs::y1() const
{
    return m_y1;
}




SVGPathSegCurvetoQuadraticRel::SVGPathSegCurvetoQuadraticRel(float x, float y, float x1, float y1)
    : SVGPathSeg()
    , m_x(x)
    , m_y(y)
    , m_x1(x1)
    , m_y1(y1)
{
}

SVGPathSegCurvetoQuadraticRel::~SVGPathSegCurvetoQuadraticRel()
{
}

void SVGPathSegCurvetoQuadraticRel::setX(float x)
{
    m_x = x;
}

float SVGPathSegCurvetoQuadraticRel::x() const
{
    return m_x;
}

void SVGPathSegCurvetoQuadraticRel::setY(float y)
{
    m_y = y;
}

float SVGPathSegCurvetoQuadraticRel::y() const
{
    return m_y;
}

void SVGPathSegCurvetoQuadraticRel::setX1(float x1)
{
    m_x1 = x1;
}

float SVGPathSegCurvetoQuadraticRel::x1() const
{
    return m_x1;
}

void SVGPathSegCurvetoQuadraticRel::setY1(float y1)
{
    m_y1 = y1;
}

float SVGPathSegCurvetoQuadraticRel::y1() const
{
    return m_y1;
}

}

#endif // ENABLE(SVG)

// vim:ts=4:noet
