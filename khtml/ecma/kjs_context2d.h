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
#include "html/html_canvasimpl.h"

#include <QtGui/QPainterPath>

namespace DOM {
   class HTMLElementImpl;
}

namespace KJS {
  ////////////////////// Conversion helpers //////////////////////
  template<typename Wrapper>
  ValueImp* getWrapper(ExecState *exec, typename Wrapper::wrappedType* g)
  {
      DOMObject *ret = 0;
      if (!g)
          return jsNull();

      ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->dynamicInterpreter());
      if ((ret = interp->getDOMObject(g)))
          return ret;

      ret = new Wrapper(exec, g);
      interp->putDOMObject(g, ret);
      return ret;
  }

  template<typename Wrapped>
  class DOMWrapperObject : public DOMObject
  {
  public:
    typedef Wrapped wrappedType;
    typedef DOMWrapperObject<Wrapped> WrapperBase;

    DOMWrapperObject(JSObject* proto, Wrapped* wrapee):
      DOMObject(proto), m_impl(wrapee)
    {}

    virtual ~DOMWrapperObject() {
      ScriptInterpreter::forgetDOMObject(m_impl.get());
    }

    virtual bool toBoolean(ExecState *) const { return true; }

    Wrapped* impl() { return m_impl.get(); }
    const Wrapped* impl() const { return m_impl.get(); }
  private:
    SharedPtr<Wrapped> m_impl;
  };

  ////////////////////// Context2D Object ////////////////////////
  DEFINE_PSEUDO_CONSTRUCTOR(Context2DPseudoCtor)

  class Context2D : public DOMWrapperObject<DOM::CanvasContext2DImpl> {
  friend class Context2DFunction;
  public:
    Context2D(ExecState* exec, DOM::CanvasContext2DImpl *ctx);
    
    virtual bool getOwnPropertySlot(ExecState *, const Identifier&, PropertySlot&);
    ValueImp *getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/);

    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    enum {
        Canvas,
        Save, Restore, // state
        Scale, Rotate, Translate, Transform, SetTransform, // transformations
        GlobalAlpha, GlobalCompositeOperation,             // compositing
        StrokeStyle, FillStyle, CreateLinearGradient, CreateRadialGradient, CreatePattern, // colors and styles.
        LineWidth, LineCap, LineJoin, MiterLimit, // line properties
        ShadowOffsetX, ShadowOffsetY, ShadowBlur, ShadowColor, // shadow properties
        ClearRect, FillRect, StrokeRect,          // rectangle ops
        BeginPath, ClosePath, MoveTo, LineTo, QuadraticCurveTo, BezierCurveTo, ArcTo, Rect, Arc,
        Fill, Stroke, Clip, IsPointInPath,        // paths
        DrawImage,  // do we want backwards compat for drawImageFromRect?
        GetImageData, PutImageData // pixel ops. ewww.
    };
  };

  class CanvasGradient : public DOMWrapperObject<DOM::CanvasGradientImpl> {
  public:
    CanvasGradient(ExecState* exec, DOM::CanvasGradientImpl* impl);

    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    enum {
        AddColorStop
    };
  };

  class CanvasPattern : public DOMWrapperObject<DOM::CanvasPatternImpl> {
  public:
    CanvasPattern(ExecState* exec, DOM::CanvasPatternImpl *i);
    
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };
} // namespace

#endif
