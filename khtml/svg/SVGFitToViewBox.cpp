/*
    Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>

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
#include "SVGFitToViewBox.h"

#include "AffineTransform.h"
#include "FloatRect.h"
#include "SVGDocumentExtensions.h"
#include "SVGNames.h"
#include "SVGParserUtilities.h"
#include "SVGPreserveAspectRatio.h"
//#include "StringImpl.h"

namespace WebCore {

SVGFitToViewBox::SVGFitToViewBox()
    : m_viewBox()
    , m_preserveAspectRatio(SVGPreserveAspectRatio::create())
{
}

SVGFitToViewBox::~SVGFitToViewBox()
{
}

ANIMATED_PROPERTY_DEFINITIONS_WITH_CONTEXT(SVGFitToViewBox, FloatRect, Rect, rect, ViewBox, viewBox, SVGNames::viewBoxAttr, m_viewBox)
ANIMATED_PROPERTY_DEFINITIONS_WITH_CONTEXT(SVGFitToViewBox, SVGPreserveAspectRatio*, PreserveAspectRatio, preserveAspectRatio, PreserveAspectRatio, preserveAspectRatio, SVGNames::preserveAspectRatioAttr, m_preserveAspectRatio.get())

bool SVGFitToViewBox::parseViewBox(const UChar*& c, const UChar* end, float& x, float& y, float& w, float& h, bool validate)
{
    Document* doc = contextElement()->document();
    String str(c, end - c);

    skipOptionalSpaces(c, end);

    bool valid = (parseNumber(c, end, x) && parseNumber(c, end, y) &&
          parseNumber(c, end, w) && parseNumber(c, end, h, false));
    if (!validate)
        return true;
    if (!valid) {
        //FIXME vtokarev doc->accessSVGExtensions()->reportWarning("Problem parsing viewBox=\"" + str + "\"");
        return false;
    }

    if (w < 0.0) { // check that width is positive
        doc->accessSVGExtensions()->reportError("A negative value for ViewBox width is not allowed");
        return false;
    } else if (h < 0.0) { // check that height is positive
        doc->accessSVGExtensions()->reportError("A negative value for ViewBox height is not allowed");
        return false;
    } else {
        skipOptionalSpaces(c, end);
        if (c < end) { // nothing should come after the last, fourth number
            //FIXME vtokarev doc->accessSVGExtensions()->reportWarning("Problem parsing viewBox=\"" + str + "\"");
            return false;
        }
    }

    return true;
}

AffineTransform SVGFitToViewBox::viewBoxToViewTransform(float viewWidth, float viewHeight) const
{
    FloatRect viewBoxRect = viewBox();
    if (!viewBoxRect.width() || !viewBoxRect.height())
        return AffineTransform();

    return preserveAspectRatio()->getCTM(viewBoxRect.x(),
            viewBoxRect.y(), viewBoxRect.width(), viewBoxRect.height(),
            0, 0, viewWidth, viewHeight);
}

bool SVGFitToViewBox::parseMappedAttribute(MappedAttribute* attr)
{
    if (attr->name() == SVGNames::viewBoxAttr) {
        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
        const UChar* c = attr->value().characters();
        const UChar* end = c + attr->value().length();
        if (parseViewBox(c, end, x, y, w, h))
            setViewBoxBaseValue(FloatRect(x, y, w, h));
        return true;
    } else if (attr->name() == SVGNames::preserveAspectRatioAttr) {
        const UChar* c = attr->value().characters();
        const UChar* end = c + attr->value().length();
        preserveAspectRatioBaseValue()->parsePreserveAspectRatio(c, end);
        return true;
    }

    return false;
}

bool SVGFitToViewBox::isKnownAttribute(const QualifiedName& attrName)
{
    return (attrName == SVGNames::viewBoxAttr ||
            attrName == SVGNames::preserveAspectRatioAttr);
}

}

#endif // ENABLE(SVG)
