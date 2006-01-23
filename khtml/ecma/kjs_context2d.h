// -*- c-basic-offset: 4 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2004 Apple Computer, Inc.
 *  Copyright (C) 2005 Zack Rusin <zack@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef KJS_CONTEXT2D_H
#define KJS_CONTEXT2D_H

#include "kjs_dom.h"
#include "kjs_binding.h"
#include "kjs_html.h"
#include <kjs/object.h>

#include "misc/loader_client.h"

#include <QPainterPath>

namespace DOM {
    class HTMLCollectionImpl;
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class HTMLSelectElementImpl;
    class HTMLTableCaptionElementImpl;
    class HTMLTableSectionElementImpl;
}

class QGradient;

namespace KJS {
  ////////////////////// Context2D Object ////////////////////////

  class Context2D : public DOMObject {
  friend class Context2DFunction;
  public:
    Context2D(DOM::HTMLElementImpl *e);
    ~Context2D();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    ValueImp *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual void mark();
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    QPainterPath &path() {
        return m_path;
    }
    void setPath( const QPainterPath &p ) {
        m_path = p;
    }

    enum {
        StrokeStyle,
        FillStyle,
        LineWidth,
        LineCap,
        LineJoin,
        MiterLimit,
        GlobalAlpha,
        GlobalCompositeOperation,
        Save, Restore,
        Scale, Rotate, Translate,
        BeginPath, ClosePath,
        SetStrokeColor, SetFillColor, SetLineWidth, SetLineCap, SetLineJoin, SetMiterLimit,
        Fill, Stroke,
        MoveTo, LineTo, QuadraticCurveTo, BezierCurveTo, ArcTo, Arc, Rect, Clip,
        ClearRect, FillRect, StrokeRect,
        DrawImage, DrawImageFromRect,
        SetAlpha, SetCompositeOperation,
        CreateLinearGradient,
        CreateRadialGradient,
        CreatePattern
    };

private:
    QBrush constructBrush(ExecState* exec);
    QPen constructPen(ExecState* exec);

    void save();
    void restore();

    QPainter *drawingContext();
    bool _validStrokeImagePattern;
    void updateStrokeImagePattern();

    SharedPtr<DOM::HTMLElementImpl> _element;
    bool _needsFlushRasterCache;

    QPainterPath m_path;

    QList<List*> stateStack;

    ValueImp *_strokeStyle;
    ValueImp *_fillStyle;
    ValueImp *_lineWidth;
    ValueImp *_lineCap;
    ValueImp *_lineJoin;
    ValueImp *_miterLimit;
    ValueImp *_globalAlpha;
    ValueImp *_globalComposite;
  };

    QColor colorFromValue(ExecState *exec, ValueImp *value);

    struct ColorStop {
        float stop;
        float red;
        float green;
        float blue;
        float alpha;

        ColorStop(float s, float r, float g, float b, float a) : stop(s), red(r), green(g), blue(b), alpha(a) {};
    };

  class Gradient : public DOMObject {
  friend class Context2DFunction;
  public:
    Gradient(float x0, float y0, float x1, float y1);
    Gradient(float x0, float y0, float r0, float x1, float y1, float r1);
    ~Gradient();
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    ValueImp *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    QGradient *qgradient() const { return m_gradient; }


    enum {
        AddColorStop
    };

    enum {
        Radial, Linear
    };

    void addColorStop (float s, float r, float g, float b, float alpha);

    int lastStop;
    int nextStop;

private:
    void commonInit();
    QGradient *m_gradient;
    float _x0, _y0, _r0, _x1, _y1, _r1;

    int maxStops;
    int stopCount;
    ColorStop *stops;
    mutable int adjustedStopCount;
    mutable ColorStop *adjustedStops;
    mutable unsigned stopsNeedAdjusting:1;
  };

  class ImagePattern : public DOMObject {
  public:
    ImagePattern(HTMLElement *i, int type);
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    ValueImp *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    QPixmap pixmap() { return _pixmap; }
    QBrush createPattern();

    enum {
        Repeat, RepeatX, RepeatY, NoRepeat
    };

private:
    float _rw, _rh;
    QPixmap _pixmap;
  };
} // namespace

#endif
