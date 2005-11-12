/*
    Copyright (C) 2002, 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
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
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QMap>
#include <QImage>
#include <Q3Dict>
#include <QMatrix>
#include <QDomElement>

#include "ksvgagghelper.h"
#include "ksvgiconpainter.h"

const double deg2rad = 0.017453292519943295769; // pi/180

class KSVGIconPainterHelper
{
public:
    KSVGIconPainterHelper(int width, int height, KSVGIconPainter *painter)
    {
        m_painter = painter;

        m_fillColor = Qt::black;

        m_useFill = true;
        m_useStroke = false;

        m_useFillGradient = false;
        m_useStrokeGradient = false;

        m_worldMatrix = new QMatrix();

        // Create new 4-channel rgba buffer and
        // initialize as fully transparent buffer
        m_buffer = new unsigned char[width * height * 4];
        memset(m_buffer, 0x00, width * height * 4);

        m_image = new QImage(m_buffer, width, height, QImage::Format_ARGB32);

        m_strokeWidth = 1.0;
        m_strokeMiterLimit = 4;
        m_dashOffset = 0;
        m_dashes = "";

        m_opacity = 0xff;
        m_fillOpacity = 0xff;
        m_strokeOpacity = 0xff;

        m_fillRule = "nonzero";

        m_width = width;
        m_height = height;
    }

    ~KSVGIconPainterHelper()
    {
        delete m_image;
        delete []m_buffer;
        delete m_worldMatrix;

        for(QMap<QString, gradient_rendering_properties *>::Iterator it = m_gradientMap.begin(); it != m_gradientMap.end(); ++it)
            delete it.value();
    }

    stroke_rendering_properties generateStrokeRenderingProperties() const
    {
        stroke_rendering_properties props;
        props.width = m_strokeWidth;
        props.miterLimit = m_strokeMiterLimit;

        agg::line_cap_e capStyle = agg::butt_cap;
        agg::line_join_e joinStyle = agg::miter_join;

        if(m_joinStyle == "round")
            joinStyle = agg::round_join;
        else if(m_joinStyle == "bevel")
            joinStyle = agg::bevel_join;

        if(m_capStyle == "round")
            capStyle = agg::round_cap;
        else if(m_capStyle == "square")
            capStyle = agg::square_cap;

        props.lineCap = capStyle;
        props.lineJoin = joinStyle;

        if(m_dashes.length() > 0)
        {
            QRegExp reg("[, ]");
            QStringList dashList = m_dashes.split(reg);

            for(int i = 0; i < dashList.count(); i++)
                props.dashArray.append((float) m_painter->toPixel(dashList[i], true));

            props.dashOffset = m_dashOffset;
        }

        return props;
    }

    // Fill/Stroke - No gradient.
    void applySolid(KSVGAggData *aggData, renderer_base &rb, bool fill) const
    {
        renderer_solid ren(rb);
        agg::rasterizer_scanline_aa<> ras;

        QColor color = m_fillColor;
        if(!fill)
            color = m_strokeColor;

        agg::rgba8 aggColor(color.red(), color.green(), color.blue());
        aggColor.opacity((fill ? m_fillOpacity / 255.0 : m_strokeOpacity / 255.0));
        ren.color(aggColor);

        // TODO: Clipping
        {
            agg::scanline_p8 sl;                
            invokeRasterizer(aggData, ras, sl, ren, fill);
        }
    }

    // Fill/Stroke - Gradients supporting "linked-gradients" using xlink:href.
    void applyGradient(KSVGAggData *aggData, renderer_base &rb, bool fill) const
    {
        QString ref;

        if(fill)
        {
            m_useFillGradient = false;
            ref = m_fillGradientReference;
        }
        else
        {
            m_useStrokeGradient = false;
            ref = m_strokeGradientReference;
        }

        applyGradient(aggData, rb, fill, ref);
    }

    void applyGradient(KSVGAggData *aggData, renderer_base &rb, bool fill, const QString &ref) const
    {
        gradient_rendering_properties *props = 0;

        QMap<QString, gradient_rendering_properties *>::ConstIterator it = m_gradientMap.find(ref);
        if(it != m_gradientMap.end())
            props = it.value();

        if(props)
        {
            QDomElement element = m_gradientElementMap[props];
            if(element.isNull())
                return;
        
            bool bbox = true;
            if(element.hasAttribute("gradientUnits"))
                bbox = (element.attribute("gradientUnits") == QString::fromLatin1("objectBoundingBox"));
            
            if(!element.hasAttribute("xlink:href"))
            {
                if(props->linear)
                    applyLinearGradient(aggData, rb, fill, bbox, ref);
                else
                    applyRadialGradient(aggData, rb, fill, bbox, ref);

                return;
            }
            else
            {
                gradient_rendering_properties *props2 = 0;
                QMap<QString, gradient_rendering_properties *>::ConstIterator it2 = m_gradientMap.find(element.attribute("xlink:href").mid(1));
                if(it2 != m_gradientMap.end())
                    props2 = it2.value();
                
                QDomElement newElement = m_gradientElementMap[props2];
                if(newElement.isNull())
                    return;

                // Saved 'old' attributes
                Q3Dict<QString> refAttrs;
                refAttrs.setAutoDelete(true);

                for(unsigned int i = 0; i < newElement.attributes().length(); ++i)
                    refAttrs.insert(newElement.attributes().item(i).nodeName(), new QString(newElement.attributes().item(i).nodeValue()));

                // Copy attributes
                QDomNamedNodeMap attr = element.attributes();
                for(unsigned int i = 0; i < attr.length(); i++)
                {
                    QString name = attr.item(i).nodeName();
                    if(name != "xlink:href" && name != "id")
                        newElement.setAttribute(name, attr.item(i).nodeValue());
                }

                // Special case: if our element doesn't have gradientUnits set (aka. bounding box mode)
                // then we need to take care as maybe the referenced gradients has set it to 'userSpaceOnUse'!
                if(!attr.contains("gradientUnits") && newElement.attributes().contains("gradientUnits"))
                    newElement.setAttribute("gradientUnits", "objectBoundingBox");

                // Evil! SVG allows linear->radial gradient references
                if(element.tagName() != newElement.tagName())
                {    
                    if(newElement.tagName() == "linearGradient")
                        props2->linear = false;
                    else
                        props2->linear = true;
                }

                applyGradient(aggData, rb, fill, element.attribute("xlink:href").mid(1));

                // Restore attributes
                Q3DictIterator<QString> itr(refAttrs);
                for(; itr.current(); ++itr)
                    newElement.setAttribute(itr.currentKey(), *(itr.current()));
            }
        }
    }

    // Fill/Stroke - Linear Gradient.
    void applyLinearGradient(KSVGAggData *aggData, renderer_base &rb, bool fill, bool bbox, const QString &ref) const
    {
        gradient_rendering_properties *linear = m_gradientMap[ref];
        if(linear && linear->linear)
        {
            QDomElement element = m_gradientElementMap[linear];

            double x1 = 0, y1 = 0, x2 = 1, y2 = 0;

            if(element.hasAttribute("x1"))
                x1 = m_painter->toPixel(element.attribute("x1"), true);

            if(element.hasAttribute("y1"))
                y1 = m_painter->toPixel(element.attribute("y1"), false);

            if(element.hasAttribute("x2"))
                x2 = m_painter->toPixel(element.attribute("x2"), true);
            
            if(element.hasAttribute("y2"))
                y2 = m_painter->toPixel(element.attribute("y2"), false);

            if(bbox)
            {
                double bx1, by1, bx2, by2;
                if(fill)
                    agg::bounding_rect(aggData->curvedPathTrans, theGetIdDummy, 0, 1, &bx1, &by1, &bx2, &by2);
                else
                {
                    // Setup stroke painter, after we got valid stroke rendering properties...
                    stroke_rendering_properties props = generateStrokeRenderingProperties();
                    aggData->curvedPathStroked.setup(props);
                    
                    agg::bounding_rect(aggData->curvedPathStrokedTrans, theGetIdDummy, 0, 1, &bx1, &by1, &bx2, &by2);
                }
                
                x1 = bx1 + (x1 * (bx2 - bx1));
                y1 = by1 + (y1 * (by2 - by1));
                x2 = bx1 + (x2 * (bx2 - bx1));
                y2 = by1 + (y2 * (by2 - by1));
            }            
            
            // Adjust to gradient transform
            QMatrix m = m_painter->parseTransform(element.attribute("gradientTransform"));
            m.map(x1, y1, &x1, &y1);
            m.map(x2, y2, &x2, &y2);
            
            double dx = x2 - x1;
            double dy = y2 - y1;
            double len = sqrt(dx * dx + dy * dy);
            double angle = atan2(dy, dx);
        
            agg::trans_affine gradientMatrix;
            gradientMatrix *= agg::trans_affine_rotation(angle);
            gradientMatrix *= agg::trans_affine_translation(x1, y1);
            gradientMatrix *= aggData->transform;
            gradientMatrix.invert();

            gradient_polymorphic_wrapper_base *gradientPtr = &gradientLinearPad;
            if(linear->spreadMethod == SPREADMETHOD_REPEAT)
                gradientPtr = &gradientLinearRepeat;
            else if(linear->spreadMethod == SPREADMETHOD_REFLECT)
                gradientPtr = &gradientLinearReflect;

            gradient_span_alloc span_alloc;
            interpolator_type inter(gradientMatrix);
            color_function_profile color_array;

            gradient_span_gen span_gen(span_alloc, inter, *gradientPtr, color_array, 0, len);
            linear->gradientStops.fill_color_array(color_array.m_colors, (fill ? m_fillOpacity / 255.0 : m_strokeOpacity / 255.0));

            renderer_gradient_type ren_gradient(rb, span_gen);
            agg::rasterizer_scanline_aa<> ras;

            // TODO: Clipping
            {
                agg::scanline_u8 sl;                
                invokeRasterizer(aggData, ras, sl, ren_gradient, fill);
            }
        }
    }

    // Fill/Stroke - Radial Gradient.
    void applyRadialGradient(KSVGAggData *aggData, renderer_base &rb, bool fill, bool bbox, const QString &ref) const
    {
        gradient_rendering_properties *radial = m_gradientMap[ref];
        if(radial && !radial->linear)
        {
            QDomElement element = m_gradientElementMap[radial];

            double cx = 50, cy = 50, r = 50, fx = 50 /* cx */, fy = 50 /* cy */;
            if(element.hasAttribute("cx"))
                cx = m_painter->toPixel(element.attribute("cx"), true);

            if(element.hasAttribute("cy"))
                cy = m_painter->toPixel(element.attribute("cy"), false);

            if(element.hasAttribute("r"))
                r = m_painter->toPixel(element.attribute("r"), true);

            if(element.hasAttribute("fx"))
                fx = m_painter->toPixel(element.attribute("fx"), false);
            else if(element.hasAttribute("cx"))
                fx = cx;
                
            if(element.hasAttribute("fy"))
                fy = m_painter->toPixel(element.attribute("fy"), false);
            else if(element.hasAttribute("cy"))
                fy = cy;

            if(bbox)
            {
                double bx1, by1, bx2, by2;
                if(fill)
                    agg::bounding_rect(aggData->curvedPathTrans, theGetIdDummy, 0, 1, &bx1, &by1, &bx2, &by2);
                else
                {
                    // Setup stroke painter, after we got valid stroke rendering properties...
                    stroke_rendering_properties props = generateStrokeRenderingProperties();
                    aggData->curvedPathStroked.setup(props);
                    
                    agg::bounding_rect(aggData->curvedPathStrokedTrans, theGetIdDummy, 0, 1, &bx1, &by1, &bx2, &by2);
                }

                cx = bx1 + (cx * (bx2 - bx1));
                cy = by1 + (cy * (by2 - by1));
                fx = bx1 + (fx * (bx2 - bx1)) - cx;
                fy = by1 + (fy * (by2 - by1)) - cy;
            }
            else
            {
                fx -= cx;
                fy -= cy;
            }

            // Adjust to gradient transform
            QMatrix m = m_painter->parseTransform(element.attribute("gradientTransform"));
            agg::trans_affine gradientTransformMatrix(m.m11(), m.m12(), m.m21(), m.m22(), m.dx(), m.dy());

            agg::trans_affine gradientMatrix;
            gradientMatrix *= gradientTransformMatrix;
            gradientMatrix *= agg::trans_affine_translation(cx, cy);
            gradientMatrix *= aggData->transform;
            gradientMatrix.invert();

            if(sqrt(fx * fx + fy * fy) > r)
            {
                // Spec: If (fx, fy) lies outside the circle defined by (cx, cy) and r, set (fx, fy)
                // to the point of intersection of the line through (fx, fy) and the circle.
                double angle = atan2(fy, fx);
                fx = int(cos(angle) * r) - 1;
                fy = int(sin(angle) * r) - 1;
            }

            gradient_polymorphic_wrapper_base *gradientPtr = &gradientRadialPad;
            if(radial->spreadMethod == SPREADMETHOD_REPEAT)
                gradientPtr = &gradientRadialRepeat;
            else if(radial->spreadMethod == SPREADMETHOD_REFLECT)
                gradientPtr = &gradientRadialReflect;

            // The hairy part :)
            gradient_radial *gradientRadialPtr = static_cast<gradient_radial *>(gradientPtr);
            gradientRadialPtr->init(r, fx, fy);

            gradient_span_alloc span_alloc;
            interpolator_type inter(gradientMatrix);
            color_function_profile color_array;

            gradient_span_gen span_gen(span_alloc, inter, *gradientPtr, color_array, 0, r);
            radial->gradientStops.fill_color_array(color_array.m_colors, (fill ? m_fillOpacity / 255.0 : m_strokeOpacity / 255.0));

            renderer_gradient_type ren_gradient(rb, span_gen);
            agg::rasterizer_scanline_aa<> ras;

            // TODO: Clipping
            {
                agg::scanline_u8 sl;
                invokeRasterizer(aggData, ras, sl, ren_gradient, fill);
            }
        }
    }

    // Used by applySolid & applyLinearGradient & applyRadialGradient
    template<class ScanLine, class Renderer>
    void invokeRasterizer(KSVGAggData *aggData, agg::rasterizer_scanline_aa<> &ras, ScanLine &sl, Renderer &ren, bool fill) const
    {
        if(fill)
            ras.add_path(aggData->curvedPathTrans);
        else
        {
            // Setup stroke painter, after we got valid stroke rendering properties...
            stroke_rendering_properties props = generateStrokeRenderingProperties();
            aggData->curvedPathStroked.setup(props);

            ras.add_path(aggData->curvedPathStrokedTrans);
        }

        agg::render_scanlines(ras, sl, ren);
    }

    void renderPath(KSVGAggData *aggData) const
    {
        Q_ASSERT(aggData != 0);

        // Update internal agg matrix
        aggData->transform = agg::trans_affine(m_worldMatrix->m11(), m_worldMatrix->m12(),
                                               m_worldMatrix->m21(), m_worldMatrix->m22(),
                                               m_worldMatrix->dx(), m_worldMatrix->dy());

        // Viewport Clipping
        agg::rect cb(m_clipRect.x(), m_clipRect.y(), m_clipRect.width(), m_clipRect.height());

        aggData->curvedPathTransClipped.clip_box(cb.x1, cb.y1, cb.x2 + 1, cb.y2 + 1);
        aggData->curvedPathStrokedTransClipped.clip_box(cb.x1, cb.y1, cb.x2 + 1, cb.y2 + 1);
        aggData->curvedPath.approximation_scale(pow(aggData->transform.scale(), 0.75));

        // Rendering preperations
        agg::rendering_buffer buf;
        buf.attach(m_buffer, m_width, m_height, m_width * 4);

        pixfmt pixf(buf);
        // TODO: m_mode handling!

        renderer_base rb(pixf);

        if(m_useFillGradient)
            applyGradient(aggData, rb, true);
        else if(m_useFill)
            applySolid(aggData, rb, true);

        if(m_useStrokeGradient)
            applyGradient(aggData, rb, false);
        else if(m_useStroke)
            applySolid(aggData, rb, false);
    }

    // Helper function to calculate arcs. Used by drawPath()
    void calculateArc(KSVGAggData *aggData, bool relative, double &curx, double &cury, double angle, double x, double y, double r1, double r2, bool largeArcFlag, bool sweepFlag)
    {
        double sin_th, cos_th;
        double a00, a01, a10, a11;
        double x0, y0, x1, y1, xc, yc;
        double d, sfactor, sfactor_sq;
        double th0, th1, th_arc;
        int i, n_segs;

        sin_th = sin(angle * (M_PI / 180.0));
        cos_th = cos(angle * (M_PI / 180.0));

        double dx;

        if(!relative)
            dx = (curx - x) / 2.0;
        else
            dx = -x / 2.0;

        double dy;

        if(!relative)
            dy = (cury - y) / 2.0;
        else
            dy = -y / 2.0;

        double _x1 =  cos_th * dx + sin_th * dy;
        double _y1 = -sin_th * dx + cos_th * dy;
        double Pr1 = r1 * r1;
        double Pr2 = r2 * r2;
        double Px = _x1 * _x1;
        double Py = _y1 * _y1;

        // Spec: check if radii are large enough
        double check = Px / Pr1 + Py / Pr2;
        if(check > 1)
        {
            r1 = r1 * sqrt(check);
            r2 = r2 * sqrt(check);
        }

        a00 = cos_th / r1;
        a01 = sin_th / r1;
        a10 = -sin_th / r2;
        a11 = cos_th / r2;

        x0 = a00 * curx + a01 * cury;
        y0 = a10 * curx + a11 * cury;

        if(!relative)
            x1 = a00 * x + a01 * y;
        else
            x1 = a00 * (curx + x) + a01 * (cury + y);

        if(!relative)
            y1 = a10 * x + a11 * y;
        else
            y1 = a10 * (curx + x) + a11 * (cury + y);

        // (x0, y0) is current point in transformed coordinate space.
        // (x1, y1) is new point in transformed coordinate space.

        // The arc fits a unit-radius circle in this space.
        d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);

        sfactor_sq = 1.0 / d - 0.25;

        if(sfactor_sq < 0)
            sfactor_sq = 0;

        sfactor = sqrt(sfactor_sq);

        if(sweepFlag == largeArcFlag)
            sfactor = -sfactor;

        xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
        yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);

        // (xc, yc) is center of the circle.
        th0 = atan2(y0 - yc, x0 - xc);
        th1 = atan2(y1 - yc, x1 - xc);

        th_arc = th1 - th0;
        if(th_arc < 0 && sweepFlag)
            th_arc += 2 * M_PI;
        else if(th_arc > 0 && !sweepFlag)
            th_arc -= 2 * M_PI;

        n_segs = (int) ceil(fabs(th_arc / (M_PI * 0.5 + 0.001)));

        for(i = 0; i < n_segs; i++)
        {
            {
                double a00, a01, a10, a11;
                double x1, y1, x2, y2, x3, y3;
                double t;
                double th_half;

                double _th0 = th0 + i * th_arc / n_segs;
                double _th1 = th0 + (i + 1) * th_arc / n_segs;

                // inverse transform compared with rsvg_path_arc
                a00 = cos_th * r1;
                a01 = -sin_th * r2;
                a10 = sin_th * r1;
                a11 = cos_th * r2;

                th_half = 0.5 * (_th1 - _th0);
                t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
                x1 = xc + cos(_th0) - t * sin(_th0);
                y1 = yc + sin(_th0) + t * cos(_th0);
                x3 = xc + cos(_th1);
                y3 = yc + sin(_th1);
                x2 = x3 + t * sin(_th1);
                y2 = y3 - t * cos(_th1);

                aggData->storage.curve4(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                                        a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                                        a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
            }
        }

        if(!relative)
            curx = x;
        else
            curx += x;

        if(!relative)
            cury = y;
        else
            cury += y;
    }

private:
    friend class KSVGIconPainter;

    QRect m_clipRect;

    QImage *m_image;
    unsigned char *m_buffer;

    QMatrix *m_worldMatrix;

    QString m_fillRule;
    QString m_joinStyle;
    QString m_capStyle;

    int m_strokeMiterLimit;

    QString m_dashes;
    unsigned short m_dashOffset;

    QColor m_fillColor;
    QColor m_strokeColor;

    int m_width;
    int m_height;

    double m_opacity;
    double m_fillOpacity;
    double m_strokeOpacity;

    bool m_useFill;
    bool m_useStroke;

    mutable bool m_useFillGradient;
    mutable bool m_useStrokeGradient;

    QString m_fillGradientReference;
    QString m_strokeGradientReference;

    QMap<QString, gradient_rendering_properties *> m_gradientMap;
    QMap<gradient_rendering_properties *, QDomElement> m_gradientElementMap;

    KSVGIconPainter *m_painter;

    double m_strokeWidth;

    int m_mode;
};

struct KSVGIconPainter::Private
{
    KSVGIconPainterHelper *helper;

    int drawWidth;
    int drawHeight;
};

KSVGIconPainter::KSVGIconPainter(int width, int height) : d(new Private())
{
    d->helper = new KSVGIconPainterHelper(width, height, this);

    d->drawWidth = width;
    d->drawHeight = height;
}

KSVGIconPainter::~KSVGIconPainter()
{
    delete d->helper;
    delete d;
}

void KSVGIconPainter::setDrawWidth(int dwidth)
{
    d->drawWidth = dwidth;
}

void KSVGIconPainter::setDrawHeight(int dheight)
{
    d->drawHeight = dheight;
}

QImage *KSVGIconPainter::image() const
{
    return new QImage(d->helper->m_image->copy());
}

QMatrix *KSVGIconPainter::worldMatrix() const
{
    return d->helper->m_worldMatrix;
}

void KSVGIconPainter::setWorldMatrix(QMatrix *matrix)
{
    if(d->helper->m_worldMatrix)
        delete d->helper->m_worldMatrix;

    d->helper->m_worldMatrix = matrix;
}

void KSVGIconPainter::setStrokeWidth(double width)
{
    d->helper->m_strokeWidth = width;
}

void KSVGIconPainter::setStrokeMiterLimit(const QString &miter)
{
    d->helper->m_strokeMiterLimit = miter.toInt();
}

void KSVGIconPainter::setStrokeDashOffset(const QString &dashOffset)
{
    d->helper->m_dashOffset = dashOffset.toUInt();
}

void KSVGIconPainter::setStrokeDashArray(const QString &dashes)
{
    d->helper->m_dashes = dashes;
}

void KSVGIconPainter::setCapStyle(const QString &cap)
{
    d->helper->m_capStyle = cap;
}

void KSVGIconPainter::setJoinStyle(const QString &join)
{
    d->helper->m_joinStyle = join;
}

void KSVGIconPainter::setStrokeColor(const QString &stroke)
{
    if(stroke.startsWith("url"))
    {
        d->helper->m_useStroke = false;
        d->helper->m_useStrokeGradient = true;

        QString url = stroke;

        unsigned int start = url.indexOf("#") + 1;
        unsigned int end = url.lastIndexOf(")");

        d->helper->m_strokeGradientReference = url.mid(start, end - start);
    }
    else
    {
        d->helper->m_strokeColor = parseColor(stroke);

        d->helper->m_useStrokeGradient = false;
        d->helper->m_strokeGradientReference = QString::null;

        if(stroke.trimmed().toLower() != "none")
            setUseStroke(true);
        else
            setUseStroke(false);
    }
}

void KSVGIconPainter::setFillColor(const QString &fill)
{
    if(fill.startsWith("url"))
    {
        d->helper->m_useFill = false;
        d->helper->m_useFillGradient = true;

        QString url = fill;

        unsigned int start = url.indexOf("#") + 1;
        unsigned int end = url.lastIndexOf(")");

        d->helper->m_fillGradientReference = url.mid(start, end - start);
    }
    else
    {
        d->helper->m_fillColor = parseColor(fill);

        d->helper->m_useFillGradient = false;
        d->helper->m_fillGradientReference = QString::null;

        if(fill.trimmed().toLower() != "none")
            setUseFill(true);
        else
            setUseFill(false);
    }
}

void KSVGIconPainter::setFillRule(const QString &fillRule)
{
    d->helper->m_fillRule = fillRule;
}

quint32 KSVGIconPainter::parseOpacity(const QString &data)
{
    quint32 opacity = 255;

    if(!data.isEmpty())
    {
        double temp;

        if(data.contains("%"))
        {
            QString tempString = data.left(data.length() - 1);
            temp = double(255 * tempString.toDouble()) / 100.0;
        }
        else
            temp = data.toDouble();

        opacity = (quint32) floor(temp * 255 + 0.5);
    }

    return opacity;
}

void KSVGIconPainter::setFillOpacity(const QString &fillOpacity)
{
    d->helper->m_fillOpacity = parseOpacity(fillOpacity);
}

void KSVGIconPainter::setStrokeOpacity(const QString &strokeOpacity)
{
    d->helper->m_strokeOpacity = parseOpacity(strokeOpacity);
}

void KSVGIconPainter::setOpacity(const QString &opacity)
{
    d->helper->m_opacity = parseOpacity(opacity);
}

void KSVGIconPainter::setUseFill(bool fill)
{
    d->helper->m_useFill = fill;
}

void KSVGIconPainter::setUseStroke(bool stroke)
{
    d->helper->m_useStroke = stroke;
}

void KSVGIconPainter::setClippingRect(int x, int y, int w, int h)
{
    d->helper->m_clipRect = QRect(x, y, w, h);
}

void KSVGIconPainter::drawRectangle(double x, double y, double w, double h, double rx, double ry)
{
    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    if((int) rx != 0 && (int) ry != 0)
    {
        double nrx = rx, nry = ry;
        // If rx is greater than half of the width of the rectangle
        // then set rx to half of the w (required in SVG spec)
        if(nrx > w / 2)
            nrx = w / 2;

        // If ry is greater than half of the h of the rectangle
        // then set ry to half of the h (required in SVG spec)
        if(nry > h / 2)
            nry = h / 2;

        aggData->storage.move_to(x + nrx, y);
        aggData->storage.curve4(x + nrx * (1 - 0.552), y, x, y + nry * (1 - 0.552), x, y + nry);
        if(nry < h / 2)
            aggData->storage.line_to(x, y + h - nry);

        aggData->storage.curve4(x, y + h - nry * (1 - 0.552), x + nrx * (1 - 0.552), y + h, x + nrx, y + h);
        if(nrx < w / 2)
            aggData->storage.line_to(x + w - nrx, y + h);

        aggData->storage.curve4(x + w - nrx * (1 - 0.552), y + h, x + w, y + h - nry * (1 - 0.552), x + w, y + h - nry);
        if(nry < h / 2)
            aggData->storage.line_to(x + w, y + nry);

        aggData->storage.curve4(x + w, y + nry * (1 - 0.552), x + w - nrx * (1 - 0.552), y, x + w - nrx, y);

        if(nrx < w / 2)
            aggData->storage.line_to(x + nrx, y);
    }
    else
    {
        aggData->storage.move_to(x, y);
        aggData->storage.line_to(x + w, y);
        aggData->storage.line_to(x + w, y + h);
        aggData->storage.line_to(x, y + h);
    }

    aggData->storage.close_polygon();
    d->helper->renderPath(aggData);
    delete aggData;
}

void KSVGIconPainter::drawEllipse(double cx, double cy, double rx, double ry)
{
    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    // Ellipse creation - nice & clean agg2 code
    double x = cx, y = cy;

    unsigned step = 0, num = 100;
    bool running = true;
    while(running)
    {
        if(step == num)
        {
            running = false;
            break;
        }

        double angle = double(step) / double(num) * 2.0 * M_PI;
        x = cx + cos(angle) * rx;
        y = cy + sin(angle) * ry;

        step++;
        if(step == 1)
            aggData->storage.move_to(x, y);
        else
            aggData->storage.line_to(x, y);
    }

    aggData->storage.close_polygon();
    d->helper->renderPath(aggData);
    delete aggData;
}

void KSVGIconPainter::drawLine(double x1, double y1, double x2, double y2)
{
    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    aggData->storage.move_to(x1, y1);
    aggData->storage.line_to(x2, y2);

    d->helper->renderPath(aggData);
    delete aggData;
}

void KSVGIconPainter::drawPolyline(const QPolygon &polyArray, int points)
{
    if(polyArray.point(0).x() == -1 || polyArray.point(0).y() == -1)
        return;

    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    if(points == -1)
        points = polyArray.count();

    aggData->storage.move_to(polyArray.point(0).x(), polyArray.point(0).y());

    for(int index = 1; index < points; index++)
        aggData->storage.line_to(polyArray.point(index).x(), polyArray.point(index).y());

    d->helper->renderPath(aggData);
    delete aggData;
}

void KSVGIconPainter::drawPolygon(const QPolygon &polyArray)
{
    if(polyArray.point(0).x() == -1 || polyArray.point(0).y() == -1)
        return;

    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();
    aggData->storage.move_to(polyArray.point(0).x(), polyArray.point(0).y());

    int points = polyArray.count();
    for(int index = 1; index < points; index++)
        aggData->storage.line_to(polyArray.point(index).x(), polyArray.point(index).y());
    
    aggData->storage.close_polygon();
    d->helper->renderPath(aggData);
    delete aggData;
}

// Path parsing tool
// parses the coord into number and forwards to the next token
static const char *getCoord(const char *ptr, double &number)
{
    int integer, exponent;
    double decimal, frac;
    int sign, expsign;

    exponent = 0;
    integer = 0;
    frac = 1.0;
    decimal = 0;
    sign = 1;
    expsign = 1;

    // read the sign
    if(*ptr == '+')
        ptr++;
    else if(*ptr == '-')
    {
        ptr++;
        sign = -1;
    }
    // read the integer part
    while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        integer = (integer * 10) + *(ptr++) - '0';

    if(*ptr == '.') // read the decimals
    {
        ptr++;
        while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
            decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

    if(*ptr == 'e' || *ptr == 'E') // read the exponent part
    {
        ptr++;

        // read the sign of the exponent
        if(*ptr == '+')
            ptr++;
        else if(*ptr == '-')
        {
            ptr++;
            expsign = -1;
        }

        exponent = 0;
        while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        {
            exponent *= 10;
            exponent += *ptr - '0';
            ptr++;
        }
    }

    number = integer + decimal;
    number *= sign * pow(10.0, expsign * exponent);

    // skip the following space
    if(*ptr == ' ')
        ptr++;

    return ptr;
}

void KSVGIconPainter::drawPath(const QString &data)
{
    if(data.isEmpty())
        return;

    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    double curx = 0.0, cury = 0.0, contrlx = 0.0, contrly = 0.0, xc, yc;
    unsigned int lastCommand = 0;

    QString value = data;
    QString temp = value.replace(",", " ");
    QByteArray _d = temp.simplified().toLatin1();

    const char *ptr = _d.constData();
    const char *end = ptr + qstrlen(ptr) + 1;

    double tox, toy, x1, y1, x2, y2, rx, ry, angle;
    bool largeArc, sweep;
    char command = *(ptr++);

    while(ptr < end)
    {
        if(*ptr == ' ')
            ptr++;

        switch(command)
        {
            case 'm':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                curx += tox;
                cury += toy;

                aggData->storage.move_to(curx, cury);
                lastCommand = 'm';
                break;
            case 'M':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                curx = tox;
                cury = toy;

                aggData->storage.move_to(curx, cury);
                lastCommand = 'M';
                break;
            case 'l':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                curx += tox;
                cury += toy;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'l';
                break;
            case 'L':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                curx = tox;
                cury = toy;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'L';
                break;
            case 'h':
                ptr = getCoord(ptr, tox);

                curx += tox;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'h';
                break;
            case 'H':
                ptr = getCoord(ptr, tox);

                curx = tox;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'H';
                break;
            case 'v':
                ptr = getCoord(ptr, toy);

                cury += toy;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'v';
                break;
            case 'V':
                ptr = getCoord(ptr, toy);

                cury = toy;

                aggData->storage.line_to(curx, cury);
                lastCommand = 'V';
                break;
            case 'c':
                ptr = getCoord(ptr, x1);
                ptr = getCoord(ptr, y1);
                ptr = getCoord(ptr, x2);
                ptr = getCoord(ptr, y2);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4(curx + x1, cury + y1,
                                        curx + x2, cury + y2,
                                        curx + tox, cury + toy);

                contrlx = curx + x2;
                contrly = cury + y2;

                curx += tox;
                cury += toy;

                lastCommand = 'c';
                break;
            case 'C':
                ptr = getCoord(ptr, x1);
                ptr = getCoord(ptr, y1);
                ptr = getCoord(ptr, x2);
                ptr = getCoord(ptr, y2);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4(x1, y1, x2, y2, tox, toy);

                contrlx = x2;
                contrly = y2;

                curx = tox;
                cury = toy;

                lastCommand = 'C';
                break;
            case 's':
                ptr = getCoord(ptr, x2);
                ptr = getCoord(ptr, y2);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4(2 * curx - contrlx, 2 * cury - contrly,
                                        curx + x2, cury + y2,
                                        curx + tox, cury + toy);

                contrlx = curx + x2;
                contrly = cury + y2;

                curx += tox;
                cury += toy;

                lastCommand = 's';
                break;
            case 'S':
                ptr = getCoord(ptr, x2);
                ptr = getCoord(ptr, y2);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4(2 * curx - contrlx, 2 * cury - contrly,
                                        x2, y2,
                                        tox, toy);
                contrlx = x2;
                contrly = y2;

                curx = tox;
                cury = toy;

                lastCommand = 'S';
                break;
            case 'q':
                ptr = getCoord(ptr, x1);
                ptr = getCoord(ptr, y1);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4((curx + 2 * (x1 + curx)) * (1.0 / 3.0), (cury + 2 * (y1 + cury)) * (1.0 / 3.0), 
                                         ((curx + tox) + 2 * (x1 + curx)) * (1.0 / 3.0), ((cury + toy) + 2 * (y1 + cury)) * (1.0 / 3.0),
                                        curx + tox, cury + toy);

                contrlx = curx + x1;
                contrly = cury + y1;
                
                curx += tox;
                cury += toy;

                lastCommand = 'q';
                break;
            case 'Q':
                ptr = getCoord(ptr, x1);
                ptr = getCoord(ptr, y1);
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                aggData->storage.curve4((curx + 2 * x1) * (1.0 / 3.0), (cury + 2 * y1) * (1.0 / 3.0),
                                        (tox + 2 * x1) * (1.0 / 3.0), (toy + 2 * y1) * (1.0 / 3.0),
                                        tox, toy);

                contrlx = (tox + 2 * x1) * (1.0 / 3.0);
                contrly = (toy + 2 * y1) * (1.0 / 3.0);

                curx = tox;
                cury = toy;

                lastCommand = 'Q';
                break;
            case 't':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                xc = 2 * curx - contrlx;
                yc = 2 * cury - contrly;

                aggData->storage.curve4((curx + 2 * xc) * (1.0 / 3.0), (cury + 2 * yc) * (1.0 / 3.0),
                                        ((curx + tox) + 2 * xc) * (1.0 / 3.0), ((cury + toy) + 2 * yc) * (1.0 / 3.0),
                                        curx + tox, cury + toy);

                contrlx = xc;
                contrly = yc;

                curx += tox;
                cury += toy;

                lastCommand = 't';
                break;
            case 'T':
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                xc = 2 * curx - contrlx;
                yc = 2 * cury - contrly;

                aggData->storage.curve4((curx + 2 * xc) * (1.0 / 3.0), (cury + 2 * yc) * (1.0 / 3.0),
                                        (tox + 2 * xc) * (1.0 / 3.0), (toy + 2 * yc) * (1.0 / 3.0),
                                        tox, toy);
                contrlx = xc;
                contrly = yc;

                curx = tox;
                cury = toy;

                lastCommand = 'T';
                break;
            case 'z':
            case 'Z':
                // Should be enough with agg2 :)
                aggData->storage.end_poly(agg::path_flags_close);
                lastCommand = 'z';
                break;
            case 'a':
                ptr = getCoord(ptr, rx);
                ptr = getCoord(ptr, ry);
                ptr = getCoord(ptr, angle);
                ptr = getCoord(ptr, tox);
                largeArc = tox == 1;
                ptr = getCoord(ptr, tox);
                sweep = tox == 1;
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                // Spec: radii are nonnegative numbers
                rx = fabs(rx);
                ry = fabs(ry);

                d->helper->calculateArc(aggData, true, curx, cury, angle, tox, toy, rx, ry, largeArc, sweep);

                lastCommand = 'a';
                break;
            case 'A':
                ptr = getCoord(ptr, rx);
                ptr = getCoord(ptr, ry);
                ptr = getCoord(ptr, angle);
                ptr = getCoord(ptr, tox);
                largeArc = tox == 1;
                ptr = getCoord(ptr, tox);
                sweep = tox == 1;
                ptr = getCoord(ptr, tox);
                ptr = getCoord(ptr, toy);

                // Spec: radii are nonnegative numbers
                rx = fabs(rx);
                ry = fabs(ry);

                d->helper->calculateArc(aggData, false, curx, cury, angle, tox, toy, rx, ry, largeArc, sweep);

                lastCommand = 'A';
                break;
        }

        if(*ptr == '+' || *ptr == '-' || (*ptr >= '0' && *ptr <= '9'))
        {
            // there are still coords in this command
            if(command == 'M')
                command = 'L';
            else if(command == 'm')
                command = 'l';
        }
        else
            command = *(ptr++);

        // Detect reflection points
        if(lastCommand != 'C' && lastCommand != 'c' &&
            lastCommand != 'S' && lastCommand != 's' &&
            lastCommand != 'Q' && lastCommand != 'q' &&
            lastCommand != 'T' && lastCommand != 't')
        {
            contrlx = curx;
            contrly = cury;
        }
    }

    // A subpath consisting of a moveto and lineto to the same exact location or a subpath consisting of a moveto
    // and a closepath will be stroked only if the 'stroke-linecap' property is set to "round", producing a circle
    // centered at the given point.
    if(aggData->storage.total_vertices() == 2 && agg::is_line_to(aggData->storage.command(1)))
    {
        double x1, y1, x2, y2;
        aggData->storage.vertex(0, &x1, &y1);
        aggData->storage.vertex(1, &x2, &y2);
        
        if(x1 == x2 && y1 == y2 && d->helper->m_capStyle == "round")
            aggData->storage.modify_vertex(1, x2 + .5, y2);
    }

    /* FIXME
    bool dontRender = aggData->storage.total_vertices() == 1 && agg::is_move_to((*aggData->storage.begin()).cmd);
    if(!dontRender)
	*/
        d->helper->renderPath(aggData);
    
	delete aggData;
}

void KSVGIconPainter::drawImage(double x, double y, const QImage &image)
{
    int width = image.width(), height = image.height();

    // We don't want applySolid/applyGradient to be called.
    d->helper->m_useFill = d->helper->m_useFillGradient = false;
    d->helper->m_useStroke = d->helper->m_useStrokeGradient = false;

    KSVGAggData *aggData = new KSVGAggData();
    aggData->storage.start_new_path();

    aggData->storage.move_to(x, y);
    aggData->storage.line_to(x + width, y);
    aggData->storage.line_to(x + width, y + height);
    aggData->storage.line_to(x, y + height);

    aggData->storage.close_polygon();

    // Render the path avoiding any standard processing method...
    d->helper->renderPath(aggData);

    // ... and use our own custom agg2 code :)
    agg::rasterizer_scanline_aa<> ras;

    agg::rendering_buffer buf;
    buf.attach(d->helper->m_buffer, d->helper->m_width, d->helper->m_height, d->helper->m_width * 4);

    agg::rendering_buffer imageBuf;
    imageBuf.attach(const_cast<unsigned char *>(image.bits()), width, height, width * 4); // We need RGBA input.

    agg::trans_affine imageMatrix;
    imageMatrix *= aggData->transform;
    imageMatrix.invert();

    interpolator_type interpolator(imageMatrix);


    image_span_alloc sa;
    image_span_gen sg(sa, imageBuf, agg::rgba(1, 1, 1, 0), interpolator);

    span_conv_opacity image_opacity(/* FIXME style ? style->opacity() : */ 1.0);
    image_span_conv sc(sg, image_opacity);

    pixfmt pixf(buf);
    renderer_base rb(pixf);
    renderer_image_type ren_image(rb, sc);

    // TODO: Clipping
    {
        agg::scanline_u8 sl;
//        d->helper->invokeRasterizer(aggData, ras, sl, ren_image, true);
    }

    delete aggData;
}

QColor KSVGIconPainter::parseColor(const QString &param)
{
    if(param.trimmed().startsWith("#"))
    {
        QColor color;
        color.setNamedColor(param.trimmed());
        return color;
    }
    else if(param.trimmed().startsWith("rgb("))
    {
        QString parse = param.trimmed();
        QStringList colors = parse.split(',');
        QString r = colors[0].right((colors[0].length() - 4));
        QString g = colors[1];
        QString b = colors[2].left((colors[2].length() - 1));

        if(r.contains("%"))
        {
            r = r.left(r.length() - 1);
            r = QString::number(int((double(255 * r.toDouble()) / 100.0)));
        }

        if(g.contains("%"))
        {
            g = g.left(g.length() - 1);
            g = QString::number(int((double(255 * g.toDouble()) / 100.0)));
        }

        if(b.contains("%"))
        {
            b = b.left(b.length() - 1);
            b = QString::number(int((double(255 * b.toDouble()) / 100.0)));
        }

        return QColor(r.toInt(), g.toInt(), b.toInt());
    }
    else
    {
        QString rgbColor = param.trimmed();

        if(rgbColor == "aliceblue")
            return QColor(240, 248, 255);
        else if(rgbColor == "antiquewhite")
            return QColor(250, 235, 215);
        else if(rgbColor == "aqua")
            return QColor(0, 255, 255);
        else if(rgbColor == "aquamarine")
            return QColor(127, 255, 212);
        else if(rgbColor == "azure")
            return QColor(240, 255, 255);
        else if(rgbColor == "beige")
            return QColor(245, 245, 220);
        else if(rgbColor == "bisque")
            return QColor(255, 228, 196);
        else if(rgbColor == "black")
            return QColor(0, 0, 0);
        else if(rgbColor == "blanchedalmond")
            return QColor(255, 235, 205);
        else if(rgbColor == "blue")
            return QColor(0, 0, 255);
        else if(rgbColor == "blueviolet")
            return QColor(138, 43, 226);
        else if(rgbColor == "brown")
            return QColor(165, 42, 42);
        else if(rgbColor == "burlywood")
            return QColor(222, 184, 135);
        else if(rgbColor == "cadetblue")
            return QColor(95, 158, 160);
        else if(rgbColor == "chartreuse")
            return QColor(127, 255, 0);
        else if(rgbColor == "chocolate")
            return QColor(210, 105, 30);
        else if(rgbColor == "coral")
            return QColor(255, 127, 80);
        else if(rgbColor == "cornflowerblue")
            return QColor(100, 149, 237);
        else if(rgbColor == "cornsilk")
            return QColor(255, 248, 220);
        else if(rgbColor == "crimson")
            return QColor(220, 20, 60);
        else if(rgbColor == "cyan")
            return QColor(0, 255, 255);
        else if(rgbColor == "darkblue")
            return QColor(0, 0, 139);
        else if(rgbColor == "darkcyan")
            return QColor(0, 139, 139);
        else if(rgbColor == "darkgoldenrod")
            return QColor(184, 134, 11);
        else if(rgbColor == "darkgray")
            return QColor(169, 169, 169);
        else if(rgbColor == "darkgrey")
            return QColor(169, 169, 169);
        else if(rgbColor == "darkgreen")
            return QColor(0, 100, 0);
        else if(rgbColor == "darkkhaki")
            return QColor(189, 183, 107);
        else if(rgbColor == "darkmagenta")
            return QColor(139, 0, 139);
        else if(rgbColor == "darkolivegreen")
            return QColor(85, 107, 47);
        else if(rgbColor == "darkorange")
            return QColor(255, 140, 0);
        else if(rgbColor == "darkorchid")
            return QColor(153, 50, 204);
        else if(rgbColor == "darkred")
            return QColor(139, 0, 0);
        else if(rgbColor == "darksalmon")
            return QColor(233, 150, 122);
        else if(rgbColor == "darkseagreen")
            return QColor(143, 188, 143);
        else if(rgbColor == "darkslateblue")
            return QColor(72, 61, 139);
        else if(rgbColor == "darkslategray")
            return QColor(47, 79, 79);
        else if(rgbColor == "darkslategrey")
            return QColor(47, 79, 79);
        else if(rgbColor == "darkturquoise")
            return QColor(0, 206, 209);
        else if(rgbColor == "darkviolet")
            return QColor(148, 0, 211);
        else if(rgbColor == "deeppink")
            return QColor(255, 20, 147);
        else if(rgbColor == "deepskyblue")
            return QColor(0, 191, 255);
        else if(rgbColor == "dimgray")
            return QColor(105, 105, 105);
        else if(rgbColor == "dimgrey")
            return QColor(105, 105, 105);
        else if(rgbColor == "dodgerblue")
            return QColor(30, 144, 255);
        else if(rgbColor == "firebrick")
            return QColor(178, 34, 34);
        else if(rgbColor == "floralwhite")
            return QColor(255, 250, 240);
        else if(rgbColor == "forestgreen")
            return QColor(34, 139, 34);
        else if(rgbColor == "fuchsia")
            return QColor(255, 0, 255);
        else if(rgbColor == "gainsboro")
            return QColor(220, 220, 220);
        else if(rgbColor == "ghostwhite")
            return QColor(248, 248, 255);
        else if(rgbColor == "gold")
            return QColor(255, 215, 0);
        else if(rgbColor == "goldenrod")
            return QColor(218, 165, 32);
        else if(rgbColor == "gray")
            return QColor(128, 128, 128);
        else if(rgbColor == "grey")
            return QColor(128, 128, 128);
        else if(rgbColor == "green")
            return QColor(0, 128, 0);
        else if(rgbColor == "greenyellow")
            return QColor(173, 255, 47);
        else if(rgbColor == "honeydew")
            return QColor(240, 255, 240);
        else if(rgbColor == "hotpink")
            return QColor(255, 105, 180);
        else if(rgbColor == "indianred")
            return QColor(205, 92, 92);
        else if(rgbColor == "indigo")
            return QColor(75, 0, 130);
        else if(rgbColor == "ivory")
            return QColor(255, 255, 240);
        else if(rgbColor == "khaki")
            return QColor(240, 230, 140);
        else if(rgbColor == "lavender")
            return QColor(230, 230, 250);
        else if(rgbColor == "lavenderblush")
            return QColor(255, 240, 245);
        else if(rgbColor == "lawngreen")
            return QColor(124, 252, 0);
        else if(rgbColor == "lemonchiffon")
            return QColor(255, 250, 205);
        else if(rgbColor == "lightblue")
            return QColor(173, 216, 230);
        else if(rgbColor == "lightcoral")
            return QColor(240, 128, 128);
        else if(rgbColor == "lightcyan")
            return QColor(224, 255, 255);
        else if(rgbColor == "lightgoldenrodyellow")
            return QColor(250, 250, 210);
        else if(rgbColor == "lightgray")
            return QColor(211, 211, 211);
        else if(rgbColor == "lightgrey")
            return QColor(211, 211, 211);
        else if(rgbColor == "lightgreen")
            return QColor(144, 238, 144);
        else if(rgbColor == "lightpink")
            return QColor(255, 182, 193);
        else if(rgbColor == "lightsalmon")
            return QColor(255, 160, 122);
        else if(rgbColor == "lightseagreen")
            return QColor(32, 178, 170);
        else if(rgbColor == "lightskyblue")
            return QColor(135, 206, 250);
        else if(rgbColor == "lightslategray")
            return QColor(119, 136, 153);
        else if(rgbColor == "lightslategrey")
            return QColor(119, 136, 153);
        else if(rgbColor == "lightsteelblue")
            return QColor(176, 196, 222);
        else if(rgbColor == "lightyellow")
            return QColor(255, 255, 224);
        else if(rgbColor == "lime")
            return QColor(0, 255, 0);
        else if(rgbColor == "limegreen")
            return QColor(50, 205, 50);
        else if(rgbColor == "linen")
            return QColor(250, 240, 230);
        else if(rgbColor == "magenta")
            return QColor(255, 0, 255);
        else if(rgbColor == "maroon")
            return QColor(128, 0, 0);
        else if(rgbColor == "mediumaquamarine")
            return QColor(102, 205, 170);
        else if(rgbColor == "mediumblue")
            return QColor(0, 0, 205);
        else if(rgbColor == "mediumorchid")
            return QColor(186, 85, 211);
        else if(rgbColor == "mediumpurple")
            return QColor(147, 112, 219);
        else if(rgbColor == "mediumseagreen")
            return QColor(60, 179, 113);
        else if(rgbColor == "mediumslateblue")
            return QColor(123, 104, 238);
        else if(rgbColor == "mediumspringgreen")
            return QColor(0, 250, 154);
        else if(rgbColor == "mediumturquoise")
            return QColor(72, 209, 204);
        else if(rgbColor == "mediumvioletred")
            return QColor(199, 21, 133);
        else if(rgbColor == "midnightblue")
            return QColor(25, 25, 112);
        else if(rgbColor == "mintcream")
            return QColor(245, 255, 250);
        else if(rgbColor == "mistyrose")
            return QColor(255, 228, 225);
        else if(rgbColor == "moccasin")
            return QColor(255, 228, 181);
        else if(rgbColor == "navajowhite")
            return QColor(255, 222, 173);
        else if(rgbColor == "navy")
            return QColor(0, 0, 128);
        else if(rgbColor == "oldlace")
            return QColor(253, 245, 230);
        else if(rgbColor == "olive")
            return QColor(128, 128, 0);
        else if(rgbColor == "olivedrab")
            return QColor(107, 142, 35);
        else if(rgbColor == "orange")
            return QColor(255, 165, 0);
        else if(rgbColor == "orangered")
            return QColor(255, 69, 0);
        else if(rgbColor == "orchid")
            return QColor(218, 112, 214);
        else if(rgbColor == "palegoldenrod")
            return QColor(238, 232, 170);
        else if(rgbColor == "palegreen")
            return QColor(152, 251, 152);
        else if(rgbColor == "paleturquoise")
            return QColor(175, 238, 238);
        else if(rgbColor == "palevioletred")
            return QColor(219, 112, 147);
        else if(rgbColor == "papayawhip")
            return QColor(255, 239, 213);
        else if(rgbColor == "peachpuff")
            return QColor(255, 218, 185);
        else if(rgbColor == "peru")
            return QColor(205, 133, 63);
        else if(rgbColor == "pink")
            return QColor(255, 192, 203);
        else if(rgbColor == "plum")
            return QColor(221, 160, 221);
        else if(rgbColor == "powderblue")
            return QColor(176, 224, 230);
        else if(rgbColor == "purple")
            return QColor(128, 0, 128);
        else if(rgbColor == "red")
            return QColor(255, 0, 0);
        if(rgbColor == "rosybrown")
            return QColor(188, 143, 143);
        else if(rgbColor == "royalblue")
            return QColor(65, 105, 225);
        else if(rgbColor == "saddlebrown")
            return QColor(139, 69, 19);
        else if(rgbColor == "salmon")
            return QColor(250, 128, 114);
        else if(rgbColor == "sandybrown")
            return QColor(244, 164, 96);
        else if(rgbColor == "seagreen")
            return QColor(46, 139, 87);
        else if(rgbColor == "seashell")
            return QColor(255, 245, 238);
        else if(rgbColor == "sienna")
            return QColor(160, 82, 45);
        else if(rgbColor == "silver")
            return QColor(192, 192, 192);
        else if(rgbColor == "skyblue")
            return QColor(135, 206, 235);
        else if(rgbColor == "slateblue")
            return QColor(106, 90, 205);
        else if(rgbColor == "slategray")
            return QColor(112, 128, 144);
        else if(rgbColor == "slategrey")
            return QColor(112, 128, 144);
        else if(rgbColor == "snow")
            return QColor(255, 250, 250);
        else if(rgbColor == "springgreen")
            return QColor(0, 255, 127);
        else if(rgbColor == "steelblue")
            return QColor(70, 130, 180);
        else if(rgbColor == "tan")
            return QColor(210, 180, 140);
        else if(rgbColor == "teal")
            return QColor(0, 128, 128);
        else if(rgbColor == "thistle")
            return QColor(216, 191, 216);
        else if(rgbColor == "tomato")
            return QColor(255, 99, 71);
        else if(rgbColor == "turquoise")
            return QColor(64, 224, 208);
        else if(rgbColor == "violet")
            return QColor(238, 130, 238);
        else if(rgbColor == "wheat")
            return QColor(245, 222, 179);
        else if(rgbColor == "white")
            return QColor(255, 255, 255);
        else if(rgbColor == "whitesmoke")
            return QColor(245, 245, 245);
        else if(rgbColor == "yellow")
            return QColor(255, 255, 0);
        else if(rgbColor == "yellowgreen")
            return QColor(154, 205, 50);
    }

    return QColor();
}

double KSVGIconPainter::dpi() const
{
    return 90.0; // TODO: make modal?
}

double KSVGIconPainter::toPixel(const QString &s, bool hmode) const
{
    if(s.isEmpty())
        return 0.0;

    QByteArray check = s.toLatin1();

    double ret = 0.0;

    double value = 0;
    const char *start = check.constData();
    const char *end = getCoord(start, value);

    if(int(end - start) < check.length())
    {
        if(check.endsWith("px"))
            ret = value;
        else if(check.endsWith("cm"))
            ret = (value / 2.54) * dpi();
        else if(check.endsWith("pc"))
            ret = (value / 6.0) * dpi();
        else if(check.endsWith("mm"))
            ret = (value / 25.4) * dpi();
        else if(check.endsWith("in"))
            ret = value * dpi();
        else if(check.endsWith("pt"))
            ret = (value / 72.0) * dpi();
        else if(check.endsWith("%"))
        {
            ret = value / 100.0;

            if(hmode)
                ret *= d->drawWidth;
            else
                ret *= d->drawHeight;
        }
        else if(check.endsWith("em"))
        {
            ret = value * 10.0; // TODO make this depend on actual font size
        }
    }
    else
        ret = value;

    return ret;
}

void KSVGIconPainter::addGradient(const QString &id, gradient_rendering_properties *gradient)
{
    d->helper->m_gradientMap.insert(id, gradient);
}

void KSVGIconPainter::addGradientElement(gradient_rendering_properties *gradient, QDomElement element)
{
    d->helper->m_gradientElementMap.insert(gradient, element);
}

QMatrix KSVGIconPainter::parseTransform(const QString &transform)
{
    QMatrix result;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')');
    QStringList::ConstIterator it = subtransforms.begin();
    QStringList::ConstIterator end = subtransforms.end();
    for(; it != end; ++it)
    {
        QStringList subtransform = (*it).split('(');

        if(subtransform.size() == 0)
        {
            // do kdDebug on empty QStringList ?
            continue;
        }

        QRegExp reg("([-]?\\d*\\.?\\d+(?:e[-]?\\d+)?)");

        int pos = 0;
        QStringList params;

        subtransform[0] = subtransform[0].trimmed().toLower();
        if(subtransform.size() >= 2)
        {
            subtransform[1] = subtransform[1].simplified();
            while(pos >= 0)
            {
                pos = reg.indexIn(subtransform[1], pos);

                if(pos != -1)
                {
                    params += reg.cap(1);
                    pos += reg.matchedLength();
                }
            }
        }

        if(subtransform[0].startsWith(";") || subtransform[0].startsWith(","))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        if(subtransform[0] == "rotate")
        {
            switch(params.count())
            {
                case 3:
                {
                    double x = params[1].toDouble();
                    double y = params[2].toDouble();

                    result.translate(x, y);
                    result.rotate(params[0].toDouble());
                    result.translate(-x, -y);
                    break;
                }
                case 1:
                {
                    result.rotate(params[0].toDouble());
                    break;
                }
                // default:
                // TODO: report error
            }
        }
        else if(subtransform[0] == "translate")
        {
            switch(params.count())
            {
                case 2:
                {
                    result.translate(params[0].toDouble(), params[1].toDouble());
                    break;
                }
                case 1: // Spec: if only one param given, assume 2nd param to be 0
                {
                    result.translate(params[0].toDouble() , 0);
                    break;
                }
                // default:
                // TODO: report error
            }
        }
        else if(subtransform[0] == "scale")
        {
            switch(params.count())
            {
                case 2:
                {
                    result.scale(params[0].toDouble(), params[1].toDouble());
                    break;
                }
                case 1: // Spec: if only one param given, assume uniform scaling
                {
                    result.scale(params[0].toDouble(), params[0].toDouble());
                    break;
                }
                // default:
                // TODO: report error
            }
        }
        else if(subtransform[0] == "skewx")
        {
            if(params.count() != 1)
                result.shear(tan(params[0].toDouble() * deg2rad), 0.0F);
            // else
            // TODO: report error
        }
        else if(subtransform[0] == "skewy")
        {
            if(params.count() != 1)
                result.shear(0.0F, tan(params[0].toDouble() * deg2rad));
            // else
            // TODO: report error
        }
        else if(subtransform[0] == "matrix")
        {
            if(params.count() == 6)
            {
                result.setMatrix(params[0].toDouble(), params[1].toDouble(),
                                 params[2].toDouble(), params[3].toDouble(),
                                 params[4].toDouble(), params[5].toDouble());
            }
            // else
            // TODO: report error
        }
    }

    return result;
}

void KSVGIconPainter::setMode(int m)
{
    d->helper->m_mode = m;
}

// vim:ts=4:noet
