/*
    Copyright (C) 2005 Apple Computer, Inc.
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
    Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
    Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>

    Based on khtml css code by:
    Copyright (C) 1999-2003 Lars Knoll <knoll@kde.org>
    Copyright (C) 2003 Apple Computer, Inc.
    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>
    Copyright (C) 2004 Germain Garand <germain@ebooksfrance.org>

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

#include "cssstyleselector.h"
#include "css_valueimpl.h"
#include "css_svgvalueimpl.h"
#include "cssvalues.h"

#include "SVGNames.h"
#include "SVGRenderStyle.h"
#include "SVGRenderStyleDefs.h"
#include "SVGStyledElement.h"

#include <wtf/MathExtras.h>

#define HANDLE_INHERIT(prop, Prop) \
if (isInherit) \
{\
    svgstyle->set##Prop(parentStyle->svgStyle()->prop());\
    return;\
}

#define HANDLE_INHERIT_AND_INITIAL(prop, Prop) \
HANDLE_INHERIT(prop, Prop) \
else if (isInitial) \
    svgstyle->set##Prop(SVGRenderStyle::initial##Prop());

#define HANDLE_INHERIT_COND(propID, prop, Prop) \
if (id == propID) \
{\
    svgstyle->set##Prop(parentStyle->svgStyle()->prop());\
    return;\
}

#define HANDLE_INITIAL_COND(propID, Prop) \
if (id == propID) \
{\
    svgstyle->set##Prop(SVGRenderStyle::initial##Prop());\
    return;\
}

#define HANDLE_INITIAL_COND_WITH_VALUE(propID, Prop, Value) \
if (id == propID) { \
    svgstyle->set##Prop(SVGRenderStyle::initial##Value()); \
    return; \
}

namespace khtml {

using namespace DOM;
using namespace WebCore;

static float roundToNearestGlyphOrientationAngle(float angle)
{
    angle = fabsf(fmodf(angle, 360.0f));

    if (angle <= 45.0f || angle > 315.0f)
        return 0.0f;
    else if (angle > 45.0f && angle <= 135.0f)
        return 90.0f;
    else if (angle > 135.0f && angle <= 225.0f)
        return 180.0f;

    return 270.0f;
}

/*static int angleToGlyphOrientation(float angle)
{
    angle = roundToNearestGlyphOrientationAngle(angle);

    if (angle == 0.0f)
        return GO_0DEG;
    else if (angle == 90.0f)
        return GO_90DEG;
    else if (angle == 180.0f)
        return GO_180DEG;
    else if (angle == 270.0f)
        return GO_270DEG;

    return -1;
}*/

void CSSStyleSelector::applySVGRule(int id, DOM::CSSValueImpl* value)
{
    CSSPrimitiveValueImpl* primitiveValue = 0;
    if (value->isPrimitiveValue())
        primitiveValue = static_cast<CSSPrimitiveValueImpl*>(value);

    SVGRenderStyle* svgstyle = style->accessSVGStyle();
    unsigned short valueType = value->cssValueType();
    
    bool isInherit = parentNode && valueType == CSSPrimitiveValue::CSS_INHERIT;
    bool isInitial = valueType == CSSPrimitiveValue::CSS_INITIAL || (!parentNode && valueType == CSSPrimitiveValue::CSS_INHERIT);

    // What follows is a list that maps the CSS properties into their
    // corresponding front-end RenderStyle values. Shorthands(e.g. border,
    // background) occur in this list as well and are only hit when mapping
    // "inherit" or "initial" into front-end values.
    switch (id) {
		// ident only properties
#if 0

		case CSS_PROP_ALIGNMENT_BASELINE:
        {
            HANDLE_INHERIT_AND_INITIAL(alignmentBaseline, AlignmentBaseline)
            if (!primitiveValue)
                break;
            
            svgstyle->setAlignmentBaseline(*primitiveValue);
            break;
        }
        case CSS_PROP_BaselineShift:
        {
            HANDLE_INHERIT_AND_INITIAL(baselineShift, BaselineShift);
            if (!primitiveValue)
                break;

            if (primitiveValue->getIdent()) {
                switch (primitiveValue->getIdent()) {
                case CSSValueBaseline:
                    svgstyle->setBaselineShift(BS_BASELINE);
                    break;
                case CSSValueSub:
                    svgstyle->setBaselineShift(BS_SUB);
                    break;
                case CSSValueSuper:
                    svgstyle->setBaselineShift(BS_SUPER);
                    break;
                default:
                    break;
                }
            } else {
                svgstyle->setBaselineShift(BS_LENGTH);
                svgstyle->setBaselineShiftValue(primitiveValue);
            }

            break;
        }
        case CSS_PROP_Kerning:
        {
            if (isInherit) {
                HANDLE_INHERIT_COND(CSS_PROP_Kerning, kerning, Kerning)
                return;
            }
            else if (isInitial) {
                HANDLE_INITIAL_COND_WITH_VALUE(CSS_PROP_Kerning, Kerning, Kerning)
                return;
            }

            svgstyle->setKerning(primitiveValue);
            break;
        }
        case CSS_PROP_PointerEvents:
        {
            HANDLE_INHERIT_AND_INITIAL(pointerEvents, PointerEvents)
            if (!primitiveValue)
                break;
            
            svgstyle->setPointerEvents(*primitiveValue);
            break;
        }
        case CSS_PROP_DominantBaseline:
        {
            HANDLE_INHERIT_AND_INITIAL(dominantBaseline, DominantBaseline)
            if (primitiveValue)
                svgstyle->setDominantBaseline(*primitiveValue);
            break;
        }
        case CSS_PROP_ColorInterpolation:
        {
            HANDLE_INHERIT_AND_INITIAL(colorInterpolation, ColorInterpolation)
            if (primitiveValue)
                svgstyle->setColorInterpolation(*primitiveValue);
            break;
        }
        case CSS_PROP_ColorInterpolationFilters:
        {
            HANDLE_INHERIT_AND_INITIAL(colorInterpolationFilters, ColorInterpolationFilters)
            if (primitiveValue)
                svgstyle->setColorInterpolationFilters(*primitiveValue);
            break;
        }
        case CSS_PROP_ColorRendering:
        {
            HANDLE_INHERIT_AND_INITIAL(colorRendering, ColorRendering)
            if (primitiveValue)
                svgstyle->setColorRendering(*primitiveValue);
            break;
        }
#endif
        case CSS_PROP_CLIP_RULE:
        {
            HANDLE_INHERIT_AND_INITIAL(clipRule, ClipRule)
            if (primitiveValue)
                svgstyle->setClipRule((primitiveValue->getIdent() == CSS_VAL_NONZERO) ? RULE_NONZERO : RULE_EVENODD);
            break;
        }
        case CSS_PROP_FILL_RULE:
        {
            HANDLE_INHERIT_AND_INITIAL(fillRule, FillRule)
            if (primitiveValue)
                svgstyle->setFillRule((primitiveValue->getIdent() == CSS_VAL_NONZERO) ? RULE_NONZERO : RULE_EVENODD);
            break;
        }
#if 0
        case CSS_PROP_StrokeLinejoin:
        {
            HANDLE_INHERIT_AND_INITIAL(joinStyle, JoinStyle)
            if (primitiveValue)
                svgstyle->setJoinStyle(*primitiveValue);
            break;
        }
        case CSS_PROP_ImageRendering:
        {
            HANDLE_INHERIT_AND_INITIAL(imageRendering, ImageRendering)
            if (primitiveValue)
                svgstyle->setImageRendering(*primitiveValue);
            break;
        }
        case CSS_PROP_ShapeRendering:
        {
            HANDLE_INHERIT_AND_INITIAL(shapeRendering, ShapeRendering)
            if (primitiveValue)
                svgstyle->setShapeRendering(*primitiveValue);
            break;
        }
        case CSS_PROP_TextRendering:
        {
            HANDLE_INHERIT_AND_INITIAL(textRendering, TextRendering)
            if (primitiveValue)
                svgstyle->setTextRendering(*primitiveValue);
            break;
        }
#endif
        // end of ident only properties
        case CSS_PROP_FILL:
        {
            HANDLE_INHERIT_AND_INITIAL(fillPaint, FillPaint)
            if (!primitiveValue && value) {
                SVGPaintImpl *paint = static_cast<SVGPaintImpl*>(value);
                if (paint)
                    svgstyle->setFillPaint(paint);
            }
            
            break;
        }
        case CSS_PROP_STROKE:
        {
            HANDLE_INHERIT_AND_INITIAL(strokePaint, StrokePaint)
            if (!primitiveValue && value) {
                SVGPaintImpl *paint = static_cast<SVGPaintImpl*>(value);
                if (paint)
                    svgstyle->setStrokePaint(paint);
            }
            
            break;
        }
        case CSS_PROP_STROKE_WIDTH:
        {
            HANDLE_INHERIT_AND_INITIAL(strokeWidth, StrokeWidth)
            if (!primitiveValue)
                return;
        
            svgstyle->setStrokeWidth(primitiveValue);
            break;
        }
#if 0
        case CSS_PROP_StrokeDasharray:
        {
            HANDLE_INHERIT_AND_INITIAL(strokeDashArray, StrokeDashArray)
            if (!primitiveValue && value) {
                CSSValueList* dashes = static_cast<CSSValueList*>(value);
                if (dashes)
                    svgstyle->setStrokeDashArray(dashes);
            }
        
            break;
        }
        case CSS_PROP_StrokeDashoffset:
        {
            HANDLE_INHERIT_AND_INITIAL(strokeDashOffset, StrokeDashOffset)
            if (!primitiveValue)
                return;

            svgstyle->setStrokeDashOffset(primitiveValue);
            break;
        }
#endif
        case CSS_PROP_FILL_OPACITY:
        {
            HANDLE_INHERIT_AND_INITIAL(fillOpacity, FillOpacity)
            if (!primitiveValue)
                return;
        
            float f = 0.0f;    
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
                f = primitiveValue->floatValue() / 100.0f;
            else if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->floatValue();
            else
                return;

            svgstyle->setFillOpacity(f);
            break;
        }
        case CSS_PROP_STROKE_OPACITY:
        {
            HANDLE_INHERIT_AND_INITIAL(strokeOpacity, StrokeOpacity)
            if (!primitiveValue)
                return;
        
            float f = 0.0f;    
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
                f = primitiveValue->floatValue() / 100.0f;
            else if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->floatValue();
            else
                return;

            svgstyle->setStrokeOpacity(f);
            break;
        }

        case CSS_PROP_STOP_OPACITY:
        {
            HANDLE_INHERIT_AND_INITIAL(stopOpacity, StopOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
                f = primitiveValue->floatValue() / 100.0f;
            else if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->floatValue();
            else
                return;

            svgstyle->setStopOpacity(f);
            break;
        }
#if 0
        case CSS_PROP_MarkerStart:
        {
            HANDLE_INHERIT_AND_INITIAL(startMarker, StartMarker)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;

            svgstyle->setStartMarker(s);
            break;
        }
        case CSS_PROP_MarkerMid:
        {
            HANDLE_INHERIT_AND_INITIAL(midMarker, MidMarker)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;

            svgstyle->setMidMarker(s);
            break;
        }
        case CSS_PROP_MarkerEnd:
        {
            HANDLE_INHERIT_AND_INITIAL(endMarker, EndMarker)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;

            svgstyle->setEndMarker(s);
            break;
        }
        case CSS_PROP_StrokeLinecap:
        {
            HANDLE_INHERIT_AND_INITIAL(capStyle, CapStyle)
            if (primitiveValue)
                svgstyle->setCapStyle(*primitiveValue);
            break;
        }
        case CSS_PROP_StrokeMiterlimit:
        {
            HANDLE_INHERIT_AND_INITIAL(strokeMiterLimit, StrokeMiterLimit)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->getFloatValue();
            else
                return;

            svgstyle->setStrokeMiterLimit(f);
            break;
        }
        case CSS_PROP_Filter:
        {
            HANDLE_INHERIT_AND_INITIAL(filter, Filter)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;
            svgstyle->setFilter(s);
            break;
        }
        case CSS_PROP_Mask:
        {
            HANDLE_INHERIT_AND_INITIAL(maskElement, MaskElement)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;

            svgstyle->setMaskElement(s);
            break;
        }
#endif
        case CSS_PROP_CLIP_PATH:
        {
            HANDLE_INHERIT_AND_INITIAL(clipPath, ClipPath)
            if (!primitiveValue)
                return;

            String s;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_URI)
                s = primitiveValue->getStringValue();
            else
                return;

            svgstyle->setClipPath(s);
            break;
        }

        case CSS_PROP_TEXT_ANCHOR:
        {
            HANDLE_INHERIT_AND_INITIAL(textAnchor, TextAnchor)
            if (primitiveValue) {
                kDebug() << primitiveValue->getIdent();
                switch (primitiveValue->getIdent()) {
                    case CSS_VAL_START:
                        svgstyle->setTextAnchor(TA_START);
                        break;
                    case CSS_VAL_MIDDLE:
                        svgstyle->setTextAnchor(TA_MIDDLE);
                        break;
                    case CSS_VAL_END:
                        svgstyle->setTextAnchor(TA_END);
                        break;
                }
                
            }
            break;
        }
        
#if 0
        case CSS_PROP_WritingMode:
        {
            HANDLE_INHERIT_AND_INITIAL(writingMode, WritingMode)
            if (primitiveValue)
                svgstyle->setWritingMode(*primitiveValue);
            break;
        }
#endif
        case CSS_PROP_STOP_COLOR:
        {
            HANDLE_INHERIT_AND_INITIAL(stopColor, StopColor);

            SVGColorImpl* c = static_cast<SVGColorImpl*>(value);
            if (!c)
                return CSSStyleSelector::applyRule(id, value);

            QColor col;
            if (c->colorType() == SVGColorImpl::SVG_COLORTYPE_CURRENTCOLOR)
                col = style->color();
            else
                col = c->color();

            svgstyle->setStopColor(col);
            break;
        }
#if 0
        case CSS_PROP_LightingColor:
        {
            HANDLE_INHERIT_AND_INITIAL(lightingColor, LightingColor);

            SVGColorImpl* c = static_cast<SVGColorImpl*>(value);
            if (!c)
                return CSSStyleSelector::applyProperty(id, value);

            Color col;
            if (c->colorType() == SVGColorImpl::SVG_COLORTYPE_CURRENTCOLOR)
                col = m_style->color();
            else
                col = c->color();

            svgstyle->setLightingColor(col);
            break;
        }
        case CSS_PROP_FloodOpacity:
        {
            HANDLE_INHERIT_AND_INITIAL(floodOpacity, FloodOpacity)
            if (!primitiveValue)
                return;

            float f = 0.0f;
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
                f = primitiveValue->getFloatValue() / 100.0f;
            else if (type == CSSPrimitiveValue::CSS_NUMBER)
                f = primitiveValue->getFloatValue();
            else
                return;

            svgstyle->setFloodOpacity(f);
            break;
        }
        case CSS_PROP_FloodColor:
        {
            Color col;
            if (isInitial)
                col = SVGRenderStyle::initialFloodColor();
            else {
                SVGColorImpl *c = static_cast<SVGColorImpl*>(value);
                if (!c)
                    return CSSStyleSelector::applyProperty(id, value);

                if (c->colorType() == SVGColorImpl::SVG_COLORTYPE_CURRENTCOLOR)
                    col = m_style->color();
                else
                    col = c->color();
            }

            svgstyle->setFloodColor(col);
            break;
        }
        case CSS_PROP_GlyphOrientationHorizontal:
        {
            HANDLE_INHERIT_AND_INITIAL(glyphOrientationHorizontal, GlyphOrientationHorizontal)
            if (!primitiveValue)
                return;

            if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_DEG) {
                int orientation = angleToGlyphOrientation(primitiveValue->getFloatValue());
                ASSERT(orientation != -1);

                svgstyle->setGlyphOrientationHorizontal((EGlyphOrientation) orientation);
            }

            break;
        }
        case CSS_PROP_GlyphOrientationVertical:
        {
            HANDLE_INHERIT_AND_INITIAL(glyphOrientationVertical, GlyphOrientationVertical)
            if (!primitiveValue)
                return;

            if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_DEG) {
                int orientation = angleToGlyphOrientation(primitiveValue->getFloatValue());
                ASSERT(orientation != -1);

                svgstyle->setGlyphOrientationVertical((EGlyphOrientation) orientation);
            } else if (primitiveValue->getIdent() == CSSValueAuto)
                svgstyle->setGlyphOrientationVertical(GO_AUTO);

            break;
        }
        case CSS_PROP_EnableBackground:
            // Silently ignoring this property for now
            // http://bugs.webkit.org/show_bug.cgi?id=6022
            break;
#endif
        default:
            // If you crash here, it's because you added a css property and are not handling it
            // in either this switch statement or the one in CSSStyleSelector::applyProperty
            //ASSERT_WITH_MESSAGE(0, "unimplemented propertyID: %d", id);
            return;
    }
}

}

