// -*- c-basic-offset: 4 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2004 Apple Computer, Inc.
 *  Copyright (C) 2005 Zack Rusin <zack@kde.org>
 *  Copyright (C) 2007 Maksim Orlovich <maksim@kde.org>
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

#include "kjs_context2d.h"

#include "kjs_html.h"

#include <config.h>

#include <misc/loader.h>
#include <dom/dom_exception.h>
#include <xml/dom2_eventsimpl.h>
#include <xml/dom_textimpl.h>
#include <html/html_baseimpl.h>
#include <html/html_blockimpl.h>
#include <html/html_canvasimpl.h>
#include <html/html_documentimpl.h>
#include <html/html_formimpl.h>
#include <html/html_headimpl.h>
#include <html/html_imageimpl.h>
#include <html/html_inlineimpl.h>
#include <html/html_listimpl.h>
#include <html/html_objectimpl.h>
#include <html/html_tableimpl.h>

#include <khtml_part.h>
#include <khtmlview.h>

#include "kjs_css.h"
#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_proxy.h"
#include <kjs/operations.h>

#include <rendering/render_canvasimage.h>
#include <rendering/render_object.h>
#include <rendering/render_layer.h>

#include <kdebug.h>

#include <css/cssparser.h>
#include <css/css_stylesheetimpl.h>
#include <css/css_ruleimpl.h>


using namespace DOM;

#include "kjs_context2d.lut.h"

namespace KJS {

////////////////////// Context2D Object ////////////////////////

KJS_DEFINE_PROTOTYPE(Context2DProto)
KJS_IMPLEMENT_PROTOFUNC(Context2DFunction)
KJS_IMPLEMENT_PROTOTYPE("CanvasRenderingContext2DProto", Context2DProto, Context2DFunction)

/*
   @begin Context2DProtoTable 48
   #
   # state ops
   save                     Context2D::Save                        DontDelete|Function 0
   restore                  Context2D::Restore                     DontDelete|Function 0
   #
   # transformations
   scale                    Context2D::Scale                       DontDelete|Function 2
   rotate                   Context2D::Rotate                      DontDelete|Function 2
   translate                Context2D::Translate                   DontDelete|Function 1
   transform                Context2D::Transform                   DontDelete|Function 6
   setTransform             Context2D::SetTransform                DontDelete|Function 6
   #
   # colors and styles
   createLinearGradient     Context2D::CreateLinearGradient        DontDelete|Function 4
   createRadialGradient     Context2D::CreateRadialGradient        DontDelete|Function 6
   createPattern            Context2D::CreatePattern               DontDelete|Function 2
   #
   # rectangle ops
   clearRect                Context2D::ClearRect                   DontDelete|Function 4
   fillRect                 Context2D::FillRect                    DontDelete|Function 4
   strokeRect               Context2D::StrokeRect                  DontDelete|Function 4
   #
   # paths
   beginPath                Context2D::BeginPath                   DontDelete|Function 0
   closePath                Context2D::ClosePath                   DontDelete|Function 0
   moveTo                   Context2D::MoveTo                      DontDelete|Function 2
   lineTo                   Context2D::LineTo                      DontDelete|Function 2
   quadraticCurveTo         Context2D::QuadraticCurveTo            DontDelete|Function 4
   bezierCurveTo            Context2D::BezierCurveTo               DontDelete|Function 6
   arcTo                    Context2D::ArcTo                       DontDelete|Function 5
   rect                     Context2D::Rect                        DontDelete|Function 4
   arc                      Context2D::Arc                         DontDelete|Function 6
   fill                     Context2D::Fill                        DontDelete|Function 0
   isPointInPath            Context2D::IsPointInPath               DontDelete|Function 2
   stroke                   Context2D::Stroke                      DontDelete|Function 0
   clip                     Context2D::Clip                        DontDelete|Function 0
   #
   # images. Lots of overloading here!
   drawImage                Context2D::DrawImage                   DontDelete|Function 3
   #
   # pixel ops.
   getImageData             Context2D::GetImageData                DontDelete|Function 4
   putImageData             Context2D::PutImageData                DontDelete|Function 3
   @end
*/

IMPLEMENT_PSEUDO_CONSTRUCTOR(Context2DPseudoCtor, "CanvasRenderingContext2D", Context2DProto)

Context2D::Context2D(ExecState* exec, DOM::CanvasContext2DImpl *ctx):
    WrapperBase(Context2DProto::self(exec), ctx)
{}

// Checks count and sets an exception if needed
static bool enoughArguments(ExecState* exec, const List& args, int limit)
{
    if (args.size() < limit) {
        setDOMException(exec, DOMException::NOT_SUPPORTED_ERR);
        return false;
    }
    return true;
}

// Verifies that a float value is not NaN or a plus/minus infinity (unless infOK)
static bool valFloatOK(ExecState* exec, const JSValue* v, bool infOK)
{
    float val = v->toFloat(exec);
    if (KJS::isNaN(val) || (!infOK && KJS::isInf(val))) {
        setDOMException(exec, DOMException::NOT_SUPPORTED_ERR);
        return false;
    }
    return true;
}

// Verifies that float arguments are not NaN or a plus/minus infinity (unless infOK)
static bool argFloatsOK(ExecState* exec, const List& args, int minArg, int maxArg, bool infOK)
{
    for (int c = minArg; c <= maxArg; ++c) {
        if (!valFloatOK(exec, args[c], infOK))
            return false;
    }
    return true;
}

// HTML5-style checking
#define KJS_REQUIRE_ARGS(n) do { if (!enoughArguments(exec, args,n)) return jsUndefined(); } while(0);
#define KJS_CHECK_FLOAT_ARGS(min,max) do { if (!argFloatsOK(exec, args, min, max, false )) return jsUndefined(); } while(0);
#define KJS_CHECK_FLOAT_OR_INF_ARGS(min,max) do { if (!argFloatsOK(exec, args, min, max, true)) return jsUndefined(); } while(0);
#define KJS_CHECK_FLOAT_VAL(v) if (!valFloatOK(exec, v, false)) return;

ValueImp *KJS::Context2DFunction::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
    KJS_CHECK_THIS(Context2D, thisObj);
    
#ifdef KJS_VERBOSE
    kDebug(6070) << "KJS::Context2DFunction::callAsFunction " << functionName().qstring();
#endif

    Context2D *jsContextObject = static_cast<KJS::Context2D *>(thisObj);
    CanvasContext2DImpl* ctx   = jsContextObject->impl();
    DOMExceptionTranslator exception(exec);
    
    switch (id) {
    // State ops
    /////////////
    case Context2D::Save: {
        ctx->save();
        break;
    }
    
    case Context2D::Restore: {
        ctx->restore();
        break;
    }

    // Transform ops. These have NaN inf handled specially in the impl
    case Context2D::Scale: {
        KJS_REQUIRE_ARGS(2);
        KJS_CHECK_FLOAT_OR_INF_ARGS(0, 1);

        ctx->scale(args[0]->toFloat(exec), args[1]->toFloat(exec));
        break;
    }

    case Context2D::Rotate: {
        KJS_REQUIRE_ARGS(1);
        // Rotate actually rejects NaN/infinity as well
        KJS_CHECK_FLOAT_ARGS(0, 0);
        

        ctx->rotate(args[0]->toFloat(exec));
        break;
    }

    case Context2D::Translate: {
        KJS_REQUIRE_ARGS(2);
        KJS_CHECK_FLOAT_OR_INF_ARGS(0, 1);

        ctx->translate(args[0]->toFloat(exec), args[1]->toFloat(exec));
        break;
    }

    case Context2D::Transform: {
        KJS_REQUIRE_ARGS(6);
        KJS_CHECK_FLOAT_OR_INF_ARGS(0, 5);

        ctx->transform(args[0]->toFloat(exec), args[1]->toFloat(exec),
                       args[2]->toFloat(exec), args[3]->toFloat(exec),
                       args[4]->toFloat(exec), args[5]->toFloat(exec));

        break;
    }
    
    case Context2D::SetTransform: {
        KJS_REQUIRE_ARGS(6);
        KJS_CHECK_FLOAT_OR_INF_ARGS(0, 5);

        ctx->setTransform(args[0]->toFloat(exec), args[1]->toFloat(exec),
                          args[2]->toFloat(exec), args[3]->toFloat(exec),
                          args[4]->toFloat(exec), args[5]->toFloat(exec));
        break;
    }
    
    // Composition state is properties --- not in prototype
    
    // Color and style info..
    case Context2D::CreateLinearGradient: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);

        CanvasGradientImpl* grad = ctx->createLinearGradient(
              args[0]->toFloat(exec), args[1]->toFloat(exec),
              args[2]->toFloat(exec), args[3]->toFloat(exec));
        return getWrapper<CanvasGradient>(exec, grad);
    }

    case Context2D::CreateRadialGradient: {
        KJS_REQUIRE_ARGS(6);
        KJS_CHECK_FLOAT_ARGS(0, 5);
        
        CanvasGradientImpl* grad = ctx->createRadialGradient(
              args[0]->toFloat(exec), args[1]->toFloat(exec),
              args[2]->toFloat(exec), args[3]->toFloat(exec),
              args[4]->toFloat(exec), args[5]->toFloat(exec),
              exception);

        return getWrapper<CanvasGradient>(exec, grad);
    }

    case Context2D::CreatePattern: {
        KJS_REQUIRE_ARGS(2);

        ElementImpl* el = toElement(args[0]);
        if (!el) {
          setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
          return jsUndefined();
        }

        CanvasPatternImpl* pat = ctx->createPattern(el, args[1]->toString(exec).domString(),
                                                    exception);

        return getWrapper<CanvasPattern>(exec, pat);
    }
    
    // Line properties are all... properties!
    
    // Rectangle ops
    case Context2D::ClearRect: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);
        
        ctx->clearRect(args[0]->toFloat(exec), args[1]->toFloat(exec),
                       args[2]->toFloat(exec), args[3]->toFloat(exec),
                       exception);

        break;
    }

    case Context2D::FillRect: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);
        
        ctx->fillRect(args[0]->toFloat(exec), args[1]->toFloat(exec),
                      args[2]->toFloat(exec), args[3]->toFloat(exec),
                      exception);

        break;
    }

    case Context2D::StrokeRect: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);
        
        ctx->strokeRect(args[0]->toFloat(exec), args[1]->toFloat(exec),
                        args[2]->toFloat(exec), args[3]->toFloat(exec),
                        exception);

        break;
    }

    // Path ops
    case Context2D::BeginPath: {
        ctx->beginPath();
        break;
    }
    
    case Context2D::ClosePath: {
        ctx->closePath();
        break;
    }

    case Context2D::MoveTo: {
        KJS_REQUIRE_ARGS(2);
        KJS_CHECK_FLOAT_ARGS(0, 1);
        
        ctx->moveTo(args[0]->toFloat(exec), args[1]->toFloat(exec));
        break;
    }

    case Context2D::LineTo: {
        KJS_REQUIRE_ARGS(2);
        KJS_CHECK_FLOAT_ARGS(0, 1);
            
        ctx->lineTo(args[0]->toFloat(exec), args[1]->toFloat(exec));
        break;
    }

    case Context2D::QuadraticCurveTo: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);
            
        ctx->quadraticCurveTo(args[0]->toFloat(exec), args[1]->toFloat(exec),
                              args[2]->toFloat(exec), args[3]->toFloat(exec));
        break;
    }

    case Context2D::BezierCurveTo: {
        KJS_REQUIRE_ARGS(6);
        KJS_CHECK_FLOAT_ARGS(0, 5);
            
        ctx->bezierCurveTo(args[0]->toFloat(exec), args[1]->toFloat(exec),
                           args[2]->toFloat(exec), args[3]->toFloat(exec),
                           args[4]->toFloat(exec), args[5]->toFloat(exec));
        break;
    }

    case Context2D::ArcTo: {
        KJS_REQUIRE_ARGS(5);
        KJS_CHECK_FLOAT_ARGS(0, 4);

        ctx->arcTo(args[0]->toFloat(exec), args[1]->toFloat(exec),
                   args[2]->toFloat(exec), args[3]->toFloat(exec),
                   args[4]->toFloat(exec), exception);
        break;
    }

    case Context2D::Rect: {
        KJS_REQUIRE_ARGS(4);
        KJS_CHECK_FLOAT_ARGS(0, 3);
            
        ctx->rect(args[0]->toFloat(exec), args[1]->toFloat(exec),
                  args[2]->toFloat(exec), args[3]->toFloat(exec),
                  exception);
        break;
    }

    case Context2D::Arc: {
        KJS_REQUIRE_ARGS(6);
        KJS_CHECK_FLOAT_ARGS(0, 5);
            
        ctx->arc(args[0]->toFloat(exec), args[1]->toFloat(exec),
                 args[2]->toFloat(exec), args[3]->toFloat(exec),
                 args[4]->toFloat(exec), args[5]->toBoolean(exec),
                 exception);
        break;
    }

    case Context2D::Fill: {
        ctx->fill();
        break;
    }

    case Context2D::Stroke: {
        ctx->stroke();
        break;
    }

    case Context2D::Clip: {
        ctx->clip();
        break;
    }
    
    case Context2D::IsPointInPath: {
        KJS_REQUIRE_ARGS(2);
        KJS_CHECK_FLOAT_ARGS(0, 1);
        return jsBoolean(ctx->isPointInPath(args[0]->toFloat(exec),
                                            args[1]->toFloat(exec)));
    }

    case Context2D::DrawImage: {
        ElementImpl* el = toElement(args[0]);
        if (!el) {
            setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
            break;
        }

        if (args.size() < 3) {
            setDOMException(exec, DOMException::NOT_SUPPORTED_ERR);
            break;
        }
        
        if (args.size() < 5) { // 3 or 4 arguments
            KJS_CHECK_FLOAT_ARGS(1, 2);
            ctx->drawImage(el,
                           args[1]->toFloat(exec),
                           args[2]->toFloat(exec),
                           exception);
        } else if (args.size() < 9) { // 5 through 9 arguments
            KJS_CHECK_FLOAT_ARGS(1, 4);
            ctx->drawImage(el,
                           args[1]->toFloat(exec),
                           args[2]->toFloat(exec),
                           args[3]->toFloat(exec),
                           args[4]->toFloat(exec),
                           exception);
        } else  { // 9 or more arguments
            KJS_CHECK_FLOAT_ARGS(1, 8);
            ctx->drawImage(el,
                           args[1]->toFloat(exec),
                           args[2]->toFloat(exec),
                           args[3]->toFloat(exec),
                           args[4]->toFloat(exec),
                           args[5]->toFloat(exec),
                           args[6]->toFloat(exec),
                           args[7]->toFloat(exec),
                           args[8]->toFloat(exec),
                           exception);
        }
        break;
    }

    //### TODO: ImageData
    }

    return jsUndefined();
}

const ClassInfo Context2D::info = { "CanvasRenderingContext2D", 0, &Context2DTable, 0 };

/*
   @begin Context2DTable 11
   canvas                   Context2D::Canvas                      DontDelete|ReadOnly
   #
   # compositing
   globalAlpha              Context2D::GlobalAlpha                 DontDelete
   globalCompositeOperation Context2D::GlobalCompositeOperation    DontDelete
   #
   # colors and styles
   strokeStyle              Context2D::StrokeStyle                 DontDelete
   fillStyle                Context2D::FillStyle                   DontDelete
   #
   # line drawing properties
   lineWidth                Context2D::LineWidth                   DontDelete
   lineCap                  Context2D::LineCap                     DontDelete
   lineJoin                 Context2D::LineJoin                    DontDelete
   miterLimit               Context2D::MiterLimit                  DontDelete
   # shadow properties
   shadowOffsetX            Context2D::ShadowOffsetX               DontDelete
   shadowOffsetY            Context2D::ShadowOffsetY               DontDelete
   shadowBlur               Context2D::ShadowBlur                  DontDelete
   shadowColor              Context2D::ShadowColor                 DontDelete
   @end
*/

bool Context2D::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticOwnValueSlot<Context2D>(&Context2DTable, this, propertyName, slot);
}

static ValueImp* encodeStyle(ExecState* exec, CanvasStyleBaseImpl* style)
{
    switch (style->type()) {
    case CanvasStyleBaseImpl::Color:
        return jsString(UString(static_cast<CanvasColorImpl*>(style)->toString()));
    case CanvasStyleBaseImpl::Gradient:
        return getWrapper<CanvasGradient>(exec, static_cast<CanvasGradientImpl*>(style));
    case CanvasStyleBaseImpl::Pattern:
        return getWrapper<CanvasPattern>(exec, static_cast<CanvasPatternImpl*>(style));
    }

    return jsNull();
}


// ### TODO: test how non-string things are handled in other browsers.
static CanvasStyleBaseImpl* decodeStyle(ExecState* exec, ValueImp* v)
{
    if (v->isObject() && static_cast<ObjectImp*>(v)->inherits(&CanvasGradient::info))
        return static_cast<CanvasGradient*>(v)->impl();
    else if (v->isObject() && static_cast<ObjectImp*>(v)->inherits(&CanvasPattern::info))
        return static_cast<CanvasPattern*>(v)->impl();
    else
        return CanvasColorImpl::fromString(v->toString(exec).domString());
}

ValueImp* Context2D::getValueProperty(ExecState* exec, int token) const
{
    const CanvasContext2DImpl* ctx = impl();
    switch(token) {
    case Canvas:
        return getDOMNode(exec, ctx->canvas());

    case GlobalAlpha:
        return jsNumber(ctx->globalAlpha());

    case GlobalCompositeOperation:
        return jsString(ctx->globalCompositeOperation());

    case StrokeStyle:
        return encodeStyle(exec, ctx->strokeStyle());

    case FillStyle:
        return encodeStyle(exec, ctx->fillStyle());

    case LineWidth:
        return jsNumber(ctx->lineWidth());
    
    case LineCap:
        return jsString(ctx->lineCap());

    case LineJoin:
        return jsString(ctx->lineJoin());

    case MiterLimit:
        return jsNumber(ctx->miterLimit());

    case ShadowOffsetX:
        return jsNumber(ctx->shadowOffsetX());

    case ShadowOffsetY:
        return jsNumber(ctx->shadowOffsetY());

    case ShadowBlur:
        return jsNumber(ctx->shadowBlur());

    case ShadowColor:
        return jsString(ctx->shadowColor());

    default:
        assert(0);
        return jsUndefined();
    }
}

void Context2D::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
    lookupPut<Context2D,DOMObject>(exec, propertyName, value, attr, &Context2DTable, this );
}

void Context2D::putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/)
{
    CanvasContext2DImpl* ctx = impl();
    switch(token) {
    case GlobalAlpha:
        ctx->setGlobalAlpha(value->toFloat(exec));
        break;
    case GlobalCompositeOperation:
        ctx->setGlobalCompositeOperation(value->toString(exec).domString());
        break;
    case StrokeStyle:
        ctx->setStrokeStyle(decodeStyle(exec, value));
        break;
    case FillStyle:
        ctx->setFillStyle(decodeStyle(exec, value));
        break;
    case LineWidth:
        KJS_CHECK_FLOAT_VAL(value);
        ctx->setLineWidth(value->toFloat(exec));
        break;
    case LineCap:
        ctx->setLineCap(value->toString(exec).domString());
        break;
    case LineJoin:
        ctx->setLineJoin(value->toString(exec).domString());
        break;
    case MiterLimit:
        KJS_CHECK_FLOAT_VAL(value);
        ctx->setMiterLimit(value->toFloat(exec));
        break;
    case ShadowOffsetX:
        KJS_CHECK_FLOAT_VAL(value);
        ctx->setShadowOffsetX(value->toFloat(exec));
        break;
    case ShadowOffsetY:
        KJS_CHECK_FLOAT_VAL(value);
        ctx->setShadowOffsetY(value->toFloat(exec));
        break;
    case ShadowBlur:
        KJS_CHECK_FLOAT_VAL(value);
        ctx->setShadowBlur(value->toFloat(exec));
        break;
    case ShadowColor:
        ctx->setShadowColor(value->toString(exec).domString());
        break;
    default:
        {} // huh?
    }
}



////////////////////// CanvasGradient Object ////////////////////////
const ClassInfo KJS::CanvasGradient::info = { "CanvasGradient", 0, 0, 0 };

KJS_DEFINE_PROTOTYPE(CanvasGradientProto)
KJS_IMPLEMENT_PROTOFUNC(CanvasGradientFunction)
KJS_IMPLEMENT_PROTOTYPE("CanvasGradientProto", CanvasGradientProto, CanvasGradientFunction)

/*
   @begin CanvasGradientProtoTable 1
   addColorStop             CanvasGradient::AddColorStop                DontDelete|Function 2
   @end
*/

ValueImp *CanvasGradientFunction::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
    KJS_CHECK_THIS(CanvasGradient, thisObj);

    CanvasGradientImpl* impl = static_cast<KJS::CanvasGradient*>(thisObj)->impl();

    DOMExceptionTranslator exception(exec);
    switch (id) {
    case CanvasGradient::AddColorStop:
        KJS_REQUIRE_ARGS(2);
        impl->addColorStop(args[0]->toFloat(exec), args[1]->toString(exec).domString(), exception);
        break;
    default:
        assert(0);
    }

    return jsUndefined();
}

CanvasGradient::CanvasGradient(ExecState* exec, DOM::CanvasGradientImpl* impl) :
    WrapperBase(CanvasGradientProto::self(exec), impl)
{}

////////////////////// CanvasPattern Object ////////////////////////

const ClassInfo CanvasPattern::info = { "CanvasPattern", 0, 0, 0 };

// Provide an empty prototype in case people want to hack it
KJS_DEFINE_PROTOTYPE(CanvasPatternProto)
KJS_IMPLEMENT_PROTOFUNC(CanvasPatternFunction)
KJS_IMPLEMENT_PROTOTYPE("CanvasPatternProto", CanvasPatternProto, CanvasPatternFunction)

/*
   @begin CanvasPatternProtoTable 0
   @end
*/

ValueImp *CanvasPatternFunction::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
    assert(0);
    return NULL;
}

CanvasPattern::CanvasPattern(ExecState* exec, DOM::CanvasPatternImpl* impl) :
    WrapperBase(CanvasPatternProto::self(exec), impl)
{}

} // namespace

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
