/*
 * Copyright (C) 2003, 2006 Apple Computer, Inc.  All rights reserved.
 *                     2006 Rob Buis <buis@kde.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef Path_h
#define Path_h

#include "dom/dom_string.h"
#include "platform/graphics/FloatPoint.h"
#include "platform/graphics/FloatRect.h"
#include <QPainterPath>

typedef QPainterPath PlatformPath;

namespace WebCore {
    class AffineTransform;
}

namespace khtml {
    using WebCore::AffineTransform;
    using WebCore::FloatPoint;
    using WebCore::FloatSize;
    using WebCore::FloatRect;
    //class String;

    enum WindRule {
        RULE_NONZERO = 0,
        RULE_EVENODD = 1
    };

    enum PathElementType {
        PathElementMoveToPoint,
        PathElementAddLineToPoint,
        PathElementAddQuadCurveToPoint,
        PathElementAddCurveToPoint,
        PathElementCloseSubpath
    };

    struct PathElement {
        PathElementType type;
        FloatPoint* points;
    };

    typedef void (*PathApplierFunction) (void* info, const PathElement*);

    class Path {
    public:
        Path();
        ~Path();

        Path(const Path&);
        Path& operator=(const Path&);

        bool contains(const FloatPoint&, WindRule rule = RULE_NONZERO) const;
        FloatRect boundingRect() const;
        
        float length();
        FloatPoint pointAtLength(float length, bool& ok);
        float normalAngleAtLength(float length, bool& ok);

        void clear();
        bool isEmpty() const;

        void moveTo(const FloatPoint&);
        void addLineTo(const FloatPoint&);
        void addQuadCurveTo(const FloatPoint& controlPoint, const FloatPoint& point);
        void addBezierCurveTo(const FloatPoint& controlPoint1, const FloatPoint& controlPoint2, const FloatPoint&);
        void addArcTo(const FloatPoint&, const FloatPoint&, float radius);
        void closeSubpath();

        void addArc(const FloatPoint&, float radius, float startAngle, float endAngle, bool anticlockwise);
        void addRect(const FloatRect&);
        void addEllipse(const FloatRect&);

        void translate(const FloatSize&);

        void setWindingRule(WindRule rule) { m_rule = rule; }
        WindRule windingRule() const { return m_rule; }

        DOM::DOMString debugString() const;

        PlatformPath* platformPath() const { return m_path; }

        static Path createRoundedRectangle(const FloatRect&, const FloatSize& roundingRadii);
        static Path createRoundedRectangle(const FloatRect&, const FloatSize& topLeftRadius, const FloatSize& topRightRadius, const FloatSize& bottomLeftRadius, const FloatSize& bottomRightRadius);
        static Path createRectangle(const FloatRect&);
        static Path createEllipse(const FloatPoint& center, float rx, float ry);
        static Path createCircle(const FloatPoint& center, float r);
        static Path createLine(const FloatPoint&, const FloatPoint&);

        void apply(void* info, PathApplierFunction) const;
        void transform(const AffineTransform&);

    private:
        PlatformPath* m_path;
        WindRule m_rule;
    };

}

#endif
