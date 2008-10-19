/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2007 Rob Buis <buis@kde.org>
    Copyright (C) 2005, 2006 Apple Computer, Inc.

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

#include "cssparser.h"
#include "cssproperties.h"
#include "cssvalues.h"
#include "Color.h"

/*#include "CSSInheritedValue.h"
#include "CSSInitialValue.h"
#include "CSSParser.h"
#include "CSSProperty.h"
#include "CSSPropertyNames.h"
#include "CSSQuirkPrimitiveValue.h"
#include "CSSValueKeywords.h"
#include "CSSValueList.h"*/
#include "wtf/Platform.h"
#include "SVGPaint.h"

using namespace std;
using WebCore::SVGPaint;
using WebCore::SVGColor;
using WebCore::Color;

namespace DOM
{

bool CSSParser::parseSVGValue(int propId, bool important)
{
    Value* value = valueList->current();
    if (!value)
        return false;

    int id = value->id;

    bool valid_primitive = false;
    CSSValueImpl *parsedValue = 0;

    switch (propId) {
    /* The comment to the right defines all valid value of these
     * properties as defined in SVG 1.1, Appendix N. Property index */
    /*case CSSPropertyAlignmentBaseline:
    // auto | baseline | before-edge | text-before-edge | middle |
    // central | after-edge | text-after-edge | ideographic | alphabetic |
    // hanging | mathematical | inherit
        if (id == CSSValueAuto || id == CSSValueBaseline || id == CSSValueMiddle ||
          (id >= CSSValueBeforeEdge && id <= CSSValueMathematical))
            valid_primitive = true;
        break;

    case CSSPropertyBaselineShift:
    // baseline | super | sub | <percentage> | <length> | inherit
        if (id == CSSValueBaseline || id == CSSValueSub ||
           id >= CSSValueSuper)
            valid_primitive = true;
        else
            valid_primitive = validUnit(value, FLength|FPercent, false);
        break;

    case CSSPropertyDominantBaseline:
    // auto | use-script | no-change | reset-size | ideographic |
    // alphabetic | hanging | mathematical | central | middle |
    // text-after-edge | text-before-edge | inherit
        if (id == CSSValueAuto || id == CSSValueMiddle ||
          (id >= CSSValueUseScript && id <= CSSValueResetSize) ||
          (id >= CSSValueCentral && id <= CSSValueMathematical))
            valid_primitive = true;
        break;

    case CSSPropertyEnableBackground:
    // accumulate | new [x] [y] [width] [height] | inherit
        if (id == CSSValueAccumulate) // TODO : new
            valid_primitive = true;
        break;

    case CSSPropertyMarkerStart:
    case CSSPropertyMarkerMid:
    case CSSPropertyMarkerEnd:
    case CSSPropertyMask:
        if (id == CSSValueNone)
            valid_primitive = true;
        else if (value->unit == CSSPrimitiveValue::CSS_URI) {
            parsedValue = new CSSPrimitiveValue(value->string, CSSPrimitiveValue::CSS_URI);
            if (parsedValue)
                valueList->next();
        }
        break;*/

    case CSS_PROP_CLIP_RULE:            // nonzero | evenodd | inherit
    case CSS_PROP_FILL_RULE:
        if (id == CSS_VAL_NONZERO || id == CSS_VAL_EVENODD)
            valid_primitive = true;
        break;

    /*case CSSPropertyStrokeMiterlimit:   // <miterlimit> | inherit
        valid_primitive = validUnit(value, FNumber|FNonNeg, false);
        break;

    case CSSPropertyStrokeLinejoin:   // miter | round | bevel | inherit
        if (id == CSSValueMiter || id == CSSValueRound || id == CSSValueBevel)
            valid_primitive = true;
        break;

    case CSSPropertyStrokeLinecap:    // butt | round | square | inherit
        if (id == CSSValueButt || id == CSSValueRound || id == CSSValueSquare)
            valid_primitive = true;
        break;*/

    case CSS_PROP_STROKE_OPACITY:   // <opacity-value> | inherit
    case CSS_PROP_FILL_OPACITY:
    case CSS_PROP_STOP_OPACITY:
    //case CSSPropertyFloodOpacity:
        valid_primitive = (!id && validUnit(value, FNumber|FPercent, false));
        break;

    /*case CSSPropertyShapeRendering:
    // auto | optimizeSpeed | crispEdges | geometricPrecision | inherit
        if (id == CSSValueAuto || id == CSSValueOptimizespeed ||
            id == CSSValueCrispedges || id == CSSValueGeometricprecision)
            valid_primitive = true;
        break;

    case CSSPropertyTextRendering:   // auto | optimizeSpeed | optimizeLegibility | geometricPrecision | inherit
        if (id == CSSValueAuto || id == CSSValueOptimizespeed || id == CSSValueOptimizelegibility ||
       id == CSSValueGeometricprecision)
            valid_primitive = true;
        break;

    case CSSPropertyImageRendering:  // auto | optimizeSpeed |
    case CSSPropertyColorRendering:  // optimizeQuality | inherit
        if (id == CSSValueAuto || id == CSSValueOptimizespeed ||
            id == CSSValueOptimizequality)
            valid_primitive = true;
        break;

    case CSSPropertyColorProfile: // auto | sRGB | <name> | <uri> inherit
        if (id == CSSValueAuto || id == CSSValueSrgb)
            valid_primitive = true;
        break;

    case CSSPropertyColorInterpolation:   // auto | sRGB | linearRGB | inherit
    case CSSPropertyColorInterpolationFilters:  
        if (id == CSSValueAuto || id == CSSValueSrgb || id == CSSValueLinearrgb)
            valid_primitive = true;
        break;*/

    /* Start of supported CSS properties with validation. This is needed for parseShortHand to work
     * correctly and allows optimization in applyRule(..)
     */

    /*case CSSPropertyPointerEvents:
    // none | visiblePainted | visibleFill | visibleStroke | visible |
    // painted | fill | stroke | none | all | inherit
        if (id == CSSValueVisible || id == CSSValueNone ||
          (id >= CSSValueVisiblepainted && id <= CSSValueAll))
            valid_primitive = true;
        break;

    case CSSPropertyTextAnchor:    // start | middle | end | inherit
        if (id == CSSValueStart || id == CSSValueMiddle || id == CSSValueEnd)
            valid_primitive = true;
        break;

    case CSSPropertyGlyphOrientationVertical: // auto | <angle> | inherit
        if (id == CSSValueAuto) {
            valid_primitive = true;
            break;
        }*/
    /* fallthrough intentional */
    /*case CSSPropertyGlyphOrientationHorizontal: // <angle> (restricted to _deg_ per SVG 1.1 spec) | inherit
        if (value->unit == CSSPrimitiveValue::CSS_DEG || value->unit == CSSPrimitiveValue::CSS_NUMBER) {
            parsedValue = new CSSPrimitiveValue(value->fValue, CSSPrimitiveValue::CSS_DEG);

            if (parsedValue)
                valueList->next();
        }
        break;*/

    case CSS_PROP_FILL:                 // <paint> | inherit
    case CSS_PROP_STROKE:               // <paint> | inherit
        {
            if (id == CSS_VAL_NONE)
                parsedValue = new SVGPaint(SVGPaint::SVG_PAINTTYPE_NONE);
            else /*if (id == CSSValueCurrentcolor)
                parsedValue = new SVGPaint(SVGPaint::SVG_PAINTTYPE_CURRENTCOLOR);
            else */if (value->unit == CSSPrimitiveValue::CSS_URI) {
                CSSPrimitiveValueImpl* val;
                // khtml FIXME memory leak here
                if (valueList->next() && (val = parseColorFromValue(valueList->current()/*, c, true*/))) {
                    parsedValue = new SVGPaint(domString(value->string), val->getRGBColorValue());
                } else
                    parsedValue = new SVGPaint(SVGPaint::SVG_PAINTTYPE_URI, domString(value->string));
            } else
                parsedValue = parseSVGPaint();

            if (parsedValue)
                valueList->next();
        }
        break;

    /*case CSSPropertyColor:                // <color> | inherit
        if ((id >= CSSValueAqua && id <= CSSValueWindowtext) ||
           (id >= CSSValueAliceblue && id <= CSSValueYellowgreen))
            parsedValue = new SVGColor(value->string);
        else
            parsedValue = parseSVGColor();

        if (parsedValue)
            valueList->next();
        break;*/

    case CSS_PROP_STOP_COLOR: // TODO : icccolor
    //case CSSPropertyFloodColor:
    //case CSSPropertyLightingColor:
        if ((id >= CSS_VAL_AQUA && id <= CSS_VAL_WINDOWTEXT)/* ||
           (id >= CSSValueAliceblue && id <= CSSValueYellowgreen)*/)
            parsedValue = new SVGColor(domString(value->string));
        else /*if (id == CSSValueCurrentcolor)
            parsedValue = new SVGColor(SVGColor::SVG_COLORTYPE_CURRENTCOLOR);
        else*/ // TODO : svgcolor (iccColor)
            parsedValue = parseSVGColor();

        if (parsedValue)
            valueList->next();

        break;

    /*case CSSPropertyWritingMode:
    // lr-tb | rl_tb | tb-rl | lr | rl | tb | inherit
        if (id >= CSSValueLrTb && id <= CSSValueTb)
            valid_primitive = true;
        break;*/

    case CSS_PROP_STROKE_WIDTH:         // <length> | inherit
    //case CSSPropertyStrokeDashoffset:
        valid_primitive = validUnit(value, FLength | FPercent, false);
        break;
    /*case CSSPropertyStrokeDasharray:     // none | <dasharray> | inherit
        if (id == CSSValueNone)
            valid_primitive = true;
        else
            parsedValue = parseSVGStrokeDasharray();

        break;

    case CSSPropertyKerning:              // auto | normal | <length> | inherit
        if (id == CSSValueAuto || id == CSSValueNormal)
            valid_primitive = true;
        else
            valid_primitive = validUnit(value, FLength, false);
        break;*/

    case CSS_PROP_CLIP_PATH:    // <uri> | none | inherit
    //case CSSPropertyFilter:
        if (id == CSS_VAL_NONE)
            valid_primitive = true;
        else if (value->unit == CSSPrimitiveValue::CSS_URI) {
            parsedValue = new CSSPrimitiveValueImpl(domString(value->string), (CSSPrimitiveValue::UnitTypes) value->unit);
            if (parsedValue)
                valueList->next();
        }
        break;

    /* shorthand properties */
    /*case CSSPropertyMarker:
    {
        ShorthandScope scope(this, propId);
        m_implicitShorthand = true;
        if (!parseValue(CSSPropertyMarkerStart, important))
            return false;
        if (valueList->current()) {
            rollbackLastProperties(1);
            return false;
        }
        CSSValue *value = parsedProperties[numParsedProperties - 1]->value();
        addProperty(CSSPropertyMarkerMid, value, important);
        addProperty(CSSPropertyMarkerEnd, value, important);
        m_implicitShorthand = false;
        return true;
    }*/
    default:
        // If you crash here, it's because you added a css property and are not handling it
        // in either this switch statement or the one in CSSParser::parseValue
        //ASSERT_WITH_MESSAGE(0, "unimplemented propertyID: %d", propId);
        //return false;
        break;
    }

    if (valid_primitive) {
        if (id != 0)
            parsedValue = new CSSPrimitiveValueImpl(id);
        else if (value->unit == CSSPrimitiveValue::CSS_STRING)
            parsedValue = new CSSPrimitiveValueImpl(domString(value->string), (CSSPrimitiveValue::UnitTypes)value->unit);
        else if (value->unit >= CSSPrimitiveValue::CSS_NUMBER && value->unit <= CSSPrimitiveValue::CSS_KHZ)
            parsedValue = new CSSPrimitiveValueImpl(value->fValue, (CSSPrimitiveValue::UnitTypes)value->unit);
        else if (value->unit >= Value::Q_EMS)
            parsedValue = new CSSQuirkPrimitiveValueImpl(value->fValue, CSSPrimitiveValue::CSS_EMS);
        valueList->next();
    }
    if (!parsedValue || (valueList->current() && !inShorthand())) {
        if (parsedValue)
            delete parsedValue; // khtml: we should delete here, right?
        return false;
    }

    addProperty(propId, parsedValue, important);
    return true;
}
#if 0
PassRefPtr<CSSValue> CSSParser::parseSVGStrokeDasharray()
{
    CSSValueList* ret = new CSSValueList;
    Value* value = valueList->current();
    bool valid_primitive = true;
    while (value) {
        valid_primitive = validUnit(value, FLength | FPercent |FNonNeg, false);
        if (!valid_primitive)
            break;
        if (value->id != 0)
            ret->append(new CSSPrimitiveValue(value->id));
        else if (value->unit >= CSSPrimitiveValue::CSS_NUMBER && value->unit <= CSSPrimitiveValue::CSS_KHZ)
            ret->append(new CSSPrimitiveValue(value->fValue, (CSSPrimitiveValue::UnitTypes) value->unit));
        value = valueList->next();
        if (value && value->unit == Value::Operator && value->iValue == ',')
            value = valueList->next();
    }
    if (!valid_primitive) {
        delete ret;
        ret = 0;
    }

    return ret;
}
#endif
CSSValueImpl* CSSParser::parseSVGPaint()
{
    CSSPrimitiveValueImpl* val;
    if (!(val = parseColorFromValue(valueList->current()/*, c, true*/)))
        return new SVGPaint();
    // khtml FIXME memory leak
    return new SVGPaint(QColor(val->getRGBColorValue()));
}

CSSValueImpl* CSSParser::parseSVGColor()
{
    CSSPrimitiveValueImpl* val;
    if (!(val = parseColorFromValue(valueList->current()/*, c, true*/)))
        return 0;
    // khtml FIXME memory leak
    return new SVGColor(Color(val->getRGBColorValue()));
}

}

