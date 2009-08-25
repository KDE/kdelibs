/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
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

#ifndef SVGRenderStyle_h
#define SVGRenderStyle_h

#include "css/css_svgvalueimpl.h"
#include "platform/graphics/GraphicsTypes.h"
#include "platform/graphics/Path.h"
#include "rendering/DataRef.h"
#include "SVGRenderStyleDefs.h"

#include <stdint.h>

using DOM::DOMString;

namespace khtml
{
    class SVGRenderStyle : public RefCounted<SVGRenderStyle> {    
    public:
        static PassRefPtr<SVGRenderStyle> create() { return adoptRef(new SVGRenderStyle); }
        PassRefPtr<SVGRenderStyle> copy() const { return adoptRef(new SVGRenderStyle(*this));}
        ~SVGRenderStyle();

        bool inheritedNotEqual(const SVGRenderStyle*) const;
        void inheritFrom(const SVGRenderStyle*);
        
        bool operator==(const SVGRenderStyle&) const;
        bool operator!=(const SVGRenderStyle& o) const { return !(*this == o); }

        // SVG CSS Properties
        SVG_RS_DEFINE_ATTRIBUTE(EAlignmentBaseline, AlignmentBaseline, alignmentBaseline, AB_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE(EDominantBaseline, DominantBaseline, dominantBaseline, DB_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE(EBaselineShift, BaselineShift, baselineShift, BS_BASELINE)

        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(LineCap, CapStyle, capStyle, ButtCap)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(WindRule, ClipRule, clipRule, RULE_NONZERO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EColorInterpolation, ColorInterpolation, colorInterpolation, CI_SRGB)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EColorInterpolation, ColorInterpolationFilters, colorInterpolationFilters, CI_LINEARRGB)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EColorRendering, ColorRendering, colorRendering, CR_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(WindRule, FillRule, fillRule, RULE_NONZERO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EImageRendering, ImageRendering, imageRendering, IR_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(LineJoin, JoinStyle, joinStyle, MiterJoin)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EPointerEvents, PointerEvents, pointerEvents, PE_VISIBLE_PAINTED)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EShapeRendering, ShapeRendering, shapeRendering, SR_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(ETextAnchor, TextAnchor, textAnchor, TA_START)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(ETextRendering, TextRendering, textRendering, TR_AUTO)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EWritingMode, WritingMode, writingMode, WM_LRTB)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EGlyphOrientation, GlyphOrientationHorizontal, glyphOrientationHorizontal, GO_0DEG)
        SVG_RS_DEFINE_ATTRIBUTE_INHERITED(EGlyphOrientation, GlyphOrientationVertical, glyphOrientationVertical, GO_AUTO)

        // SVG CSS Properties (using DataRef's)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(float, fill, opacity, FillOpacity, fillOpacity, 1.0f)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(SVGPaintImpl, fill, paint, FillPaint, fillPaint, SVGPaintImpl::defaultFill())

        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(float, stroke, opacity, StrokeOpacity, strokeOpacity, 1.0f)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(SVGPaintImpl, stroke, paint, StrokePaint, strokePaint, SVGPaintImpl::defaultStroke())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(CSSValueListImpl, stroke, dashArray, StrokeDashArray, strokeDashArray, 0)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(float, stroke, miterLimit, StrokeMiterLimit, strokeMiterLimit, 4.0f)
        
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(CSSValueImpl, stroke, width, StrokeWidth, strokeWidth, 0)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(CSSValueImpl, stroke, dashOffset, StrokeDashOffset, strokeDashOffset, 0);

        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(CSSValueImpl, text, kerning, Kerning, kerning, 0)

        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(float, stops, opacity, StopOpacity, stopOpacity, 1.0f)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(QColor, stops, color, StopColor, stopColor, QColor(0, 0, 0))

        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, clip, clipPath, ClipPath, clipPath, DOMString())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, mask, maskElement, MaskElement, maskElement, DOMString())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, markers, startMarker, StartMarker, startMarker, DOMString())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, markers, midMarker, MidMarker, midMarker, DOMString())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, markers, endMarker, EndMarker, endMarker, DOMString())

        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(DOMString, misc, filter, Filter, filter, DOMString())
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(float, misc, floodOpacity, FloodOpacity, floodOpacity, 1.0f)
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(QColor, misc, floodColor, FloodColor, floodColor, QColor(0, 0, 0))
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(QColor, misc, lightingColor, LightingColor, lightingColor, QColor(255, 255, 255))
        SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(CSSValueImpl, misc, baselineShiftValue, BaselineShiftValue, baselineShiftValue, 0)

        // convenience
        bool hasStroke() const { return (strokePaint()->paintType() != SVGPaintImpl::SVG_PAINTTYPE_NONE); }
        bool hasFill() const { return (fillPaint()->paintType() != SVGPaintImpl::SVG_PAINTTYPE_NONE); }

        static float cssPrimitiveToLength(const RenderObject*, CSSValueImpl*, float defaultValue = 0.0f);

    protected:
        // inherit
        struct InheritedFlags {
            bool operator==(const InheritedFlags& other) const
            {
                return (_colorRendering == other._colorRendering) &&
                       (_imageRendering == other._imageRendering) &&
                       (_shapeRendering == other._shapeRendering) &&
                       (_textRendering == other._textRendering) &&
                       (_clipRule == other._clipRule) &&
                       (_fillRule == other._fillRule) &&
                       (_capStyle == other._capStyle) &&
                       (_joinStyle == other._joinStyle) &&
                       (_textAnchor == other._textAnchor) &&
                       (_colorInterpolation == other._colorInterpolation) &&
                       (_colorInterpolationFilters == other._colorInterpolationFilters) &&
                       (_pointerEvents == other._pointerEvents) &&
                       (_writingMode == other._writingMode) &&
                       (_glyphOrientationHorizontal == other._glyphOrientationHorizontal) &&
                       (_glyphOrientationVertical == other._glyphOrientationVertical);
            }

            bool operator!=(const InheritedFlags& other) const
            {
                return !(*this == other);
            }

            unsigned _colorRendering : 2; // EColorRendering
            unsigned _imageRendering : 2; // EImageRendering 
            unsigned _shapeRendering : 2; // EShapeRendering 
            unsigned _textRendering : 2; // ETextRendering
            unsigned _clipRule : 1; // WindRule
            unsigned _fillRule : 1; // WindRule
            unsigned _capStyle : 2; // LineCap
            unsigned _joinStyle : 2; // LineJoin
            unsigned _textAnchor : 2; // ETextAnchor
            unsigned _colorInterpolation : 2; // EColorInterpolation
            unsigned _colorInterpolationFilters : 2; // EColorInterpolation
            unsigned _pointerEvents : 4; // EPointerEvents
            unsigned _writingMode : 3; // EWritingMode
            unsigned _glyphOrientationHorizontal : 3; // EGlyphOrientation
            unsigned _glyphOrientationVertical : 3; // EGlyphOrientation
        } svg_inherited_flags;

        // don't inherit
        struct NonInheritedFlags {
            // 32 bit non-inherited, don't add to the struct, or the operator will break.
            bool operator==(const NonInheritedFlags &other) const { return _niflags == other._niflags; }
            bool operator!=(const NonInheritedFlags &other) const { return _niflags != other._niflags; }

            union {
                struct {
                    unsigned _alignmentBaseline : 4; // EAlignmentBaseline 
                    unsigned _dominantBaseline : 4; // EDominantBaseline
                    unsigned _baselineShift : 2; // EBaselineShift
                    // 22 bits unused
                } f;
                uint32_t _niflags;
            };
        } svg_noninherited_flags;

        // inherited attributes
        khtml::DataRef<StyleFillData> fill;
        DataRef<StyleStrokeData> stroke;
        DataRef<StyleMarkerData> markers;
        DataRef<StyleTextData> text;

        // non-inherited attributes
        DataRef<StyleStopData> stops;
        DataRef<StyleClipData> clip;
        DataRef<StyleMaskData> mask;
        DataRef<StyleMiscData> misc;

    private:
        enum CreateDefaultType { CreateDefault };

    public:
        SVGRenderStyle();
        SVGRenderStyle(const SVGRenderStyle&);
    private:
        SVGRenderStyle(CreateDefaultType); // Used to create the default style.

        void setBitDefaults()
        {
            svg_inherited_flags._clipRule = initialClipRule();
            svg_inherited_flags._colorRendering = initialColorRendering();
            svg_inherited_flags._fillRule = initialFillRule();
            svg_inherited_flags._imageRendering = initialImageRendering();
            svg_inherited_flags._shapeRendering = initialShapeRendering();
            svg_inherited_flags._textRendering = initialTextRendering();
            svg_inherited_flags._textAnchor = initialTextAnchor();
            svg_inherited_flags._capStyle = initialCapStyle();
            svg_inherited_flags._joinStyle = initialJoinStyle();
            svg_inherited_flags._colorInterpolation = initialColorInterpolation();
            svg_inherited_flags._colorInterpolationFilters = initialColorInterpolationFilters();
            svg_inherited_flags._pointerEvents = initialPointerEvents();
            svg_inherited_flags._writingMode = initialWritingMode();
            svg_inherited_flags._glyphOrientationHorizontal = initialGlyphOrientationHorizontal();
            svg_inherited_flags._glyphOrientationVertical = initialGlyphOrientationVertical();

            svg_noninherited_flags._niflags = 0;
            svg_noninherited_flags.f._alignmentBaseline = initialAlignmentBaseline();
            svg_noninherited_flags.f._dominantBaseline = initialDominantBaseline();
            svg_noninherited_flags.f._baselineShift = initialBaselineShift();
        }
    };

} // namespace khtml

#endif // SVGRenderStyle_h

// vim:ts=4
