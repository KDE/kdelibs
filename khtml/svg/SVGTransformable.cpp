/*
    Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
                  2007 Eric Seidel <eric@webkit.org>

    This file is part of the WebKit project

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

#include "Document.h"

#if ENABLE(SVG)
#include "SVGTransformable.h"

#include "AffineTransform.h"
#include "FloatConversion.h"
//#include "RegularExpression.h"
#include "SVGNames.h"
#include "SVGParserUtilities.h"
#include "SVGStyledElement.h"
#include "SVGTransformList.h"

namespace WebCore {

SVGTransformable::SVGTransformable() : SVGLocatable()
{
}

SVGTransformable::~SVGTransformable()
{
}

AffineTransform SVGTransformable::getCTM(const SVGElement* element) const
{
    AffineTransform ctm = SVGLocatable::getCTM(element);
    return animatedLocalTransform() * ctm;
}

AffineTransform SVGTransformable::getScreenCTM(const SVGElement* element) const
{
    AffineTransform ctm = SVGLocatable::getScreenCTM(element);
    return animatedLocalTransform() * ctm;
}

int parseTransformParamList(const UChar*& ptr, const UChar* end, float* values, int required, int optional)
{
    int optionalParams = 0, requiredParams = 0;
    
    if (!skipOptionalSpaces(ptr, end) || *ptr != '(')
        return -1;
    
    ptr++;
   
    skipOptionalSpaces(ptr, end);

    while (requiredParams < required) {
        if (ptr >= end || !parseNumber(ptr, end, values[requiredParams], false))
            return -1;
        requiredParams++;
        if (requiredParams < required)
            skipOptionalSpacesOrDelimiter(ptr, end);
    }
    if (!skipOptionalSpaces(ptr, end))
        return -1;
    
    bool delimParsed = skipOptionalSpacesOrDelimiter(ptr, end);

    if (ptr >= end)
        return -1;
    
    if (*ptr == ')') { // skip optionals
        ptr++;
        if (delimParsed)
            return -1;
    } else {
        while (optionalParams < optional) {
            if (ptr >= end || !parseNumber(ptr, end, values[requiredParams + optionalParams], false))
                return -1;
            optionalParams++;
            if (optionalParams < optional)
                skipOptionalSpacesOrDelimiter(ptr, end);
        }
        
        if (!skipOptionalSpaces(ptr, end))
            return -1;
        
        delimParsed = skipOptionalSpacesOrDelimiter(ptr, end);
        
        if (ptr >= end || *ptr != ')' || delimParsed)
            return -1;
        ptr++;
    }

    return requiredParams + optionalParams;
}

// These should be kept in sync with enum SVGTransformType
static const int requiredValuesForType[] =  {0, 6, 1, 1, 1, 1, 1};
static const int optionalValuesForType[] =  {0, 0, 1, 1, 2, 0, 0};

bool SVGTransformable::parseTransformValue(unsigned type, const UChar*& ptr, const UChar* end, SVGTransform& t)
{
    if (type == SVGTransform::SVG_TRANSFORM_UNKNOWN)
        return false;

    int valueCount = 0;
    float values[] = {0, 0, 0, 0, 0, 0};
    if ((valueCount = parseTransformParamList(ptr, end, values, requiredValuesForType[type], optionalValuesForType[type])) < 0)
        return false;

    switch (type) {
        case SVGTransform::SVG_TRANSFORM_SKEWX:
           t.setSkewX(values[0]);
            break;
        case SVGTransform::SVG_TRANSFORM_SKEWY:
               t.setSkewY(values[0]);
            break;
        case SVGTransform::SVG_TRANSFORM_SCALE:
              if (valueCount == 1) // Spec: if only one param given, assume uniform scaling
                  t.setScale(values[0], values[0]);
              else
                  t.setScale(values[0], values[1]);
            break;
        case SVGTransform::SVG_TRANSFORM_TRANSLATE:
              if (valueCount == 1) // Spec: if only one param given, assume 2nd param to be 0
                  t.setTranslate(values[0], 0);
              else
                  t.setTranslate(values[0], values[1]);
            break;
        case SVGTransform::SVG_TRANSFORM_ROTATE:
              if (valueCount == 1)
                  t.setRotate(values[0], 0, 0);
              else
                  t.setRotate(values[0], values[1], values[2]);
            break;
        case SVGTransform::SVG_TRANSFORM_MATRIX:
            t.setMatrix(AffineTransform(values[0], values[1], values[2], values[3], values[4], values[5]));
            break;
    }

    return true;
}

static const UChar skewXDesc[] =  {'s','k','e','w', 'X'};
static const UChar skewYDesc[] =  {'s','k','e','w', 'Y'};
static const UChar scaleDesc[] =  {'s','c','a','l', 'e'};
static const UChar translateDesc[] =  {'t','r','a','n', 's', 'l', 'a', 't', 'e'};
static const UChar rotateDesc[] =  {'r','o','t','a', 't', 'e'};
static const UChar matrixDesc[] =  {'m','a','t','r', 'i', 'x'};

// KHTML
/*static inline bool skipString(const UChar*& currTransform, const UChar* end, const UChar* pattern, int len)
{
    int i = len;
    const UChar* curr = currTransform;
    while (i && curr < end) {
        if (*curr++ != *pattern++)
            return false;
        --i;
    }
    if (i)
        return false;
    currTransform += len;
    return true;
}*/

static inline bool parseAndSkipType(const UChar*& currTransform, const UChar* end, unsigned short& type)
{
    if (currTransform >= end)
        return false;
    
    if (*currTransform == 's') {
        if (skipString(currTransform, end, skewXDesc, sizeof(skewXDesc) / sizeof(UChar)))
            type = SVGTransform::SVG_TRANSFORM_SKEWX;
        else if (skipString(currTransform, end, skewYDesc, sizeof(skewYDesc) / sizeof(UChar)))
            type = SVGTransform::SVG_TRANSFORM_SKEWY;
        else if (skipString(currTransform, end, scaleDesc, sizeof(scaleDesc) / sizeof(UChar)))
            type = SVGTransform::SVG_TRANSFORM_SCALE;
        else
            return false;
    } else if (skipString(currTransform, end, translateDesc, sizeof(translateDesc) / sizeof(UChar)))
        type = SVGTransform::SVG_TRANSFORM_TRANSLATE;
    else if (skipString(currTransform, end, rotateDesc, sizeof(rotateDesc) / sizeof(UChar)))
        type = SVGTransform::SVG_TRANSFORM_ROTATE;
    else if (skipString(currTransform, end, matrixDesc, sizeof(matrixDesc) / sizeof(UChar)))
        type = SVGTransform::SVG_TRANSFORM_MATRIX;
    else 
        return false;
    
    return true;
}

bool SVGTransformable::parseTransformAttribute(SVGTransformList* list, const AtomicString& transform)
{
    const UChar* start = transform.characters();
    const UChar* end = start + transform.length();
    return parseTransformAttribute(list, start, end);
}

bool SVGTransformable::parseTransformAttribute(SVGTransformList* list, const UChar*& currTransform, const UChar* end)
{
    bool delimParsed = false;
    while (currTransform < end) {
        delimParsed = false;
        unsigned short type = SVGTransform::SVG_TRANSFORM_UNKNOWN;
        skipOptionalSpaces(currTransform, end);
        
        if (!parseAndSkipType(currTransform, end, type))
            return false;

        SVGTransform t;
        if (!parseTransformValue(type, currTransform, end, t))
            return false;

        ExceptionCode ec = 0;
        list->appendItem(t, ec);
        skipOptionalSpaces(currTransform, end);
        if (currTransform < end && *currTransform == ',') {
            delimParsed = true;
            currTransform++;
        }
        skipOptionalSpaces(currTransform, end);
    }

    return !delimParsed;
}

bool SVGTransformable::isKnownAttribute(const QualifiedName& attrName)
{
    return attrName.matches(SVGNames::transformAttr);
}

}

#endif // ENABLE(SVG)
