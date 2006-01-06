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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include "kjs_context2d.h"
#include "kjs_html.h"

#include "misc/loader.h"
#include "dom/dom_exception.h"
#include "xml/dom2_eventsimpl.h"

#include "xml/dom_textimpl.h"
#include "html/html_baseimpl.h"
#include "html/html_blockimpl.h"
#include "html/html_canvasimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_formimpl.h"
#include "html/html_headimpl.h"
#include "html/html_imageimpl.h"
#include "html/html_inlineimpl.h"
#include "html/html_listimpl.h"
#include "html/html_objectimpl.h"
#include "html/html_tableimpl.h"

#include "khtml_part.h"
#include "khtmlview.h"

#include "kjs_css.h"
#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_proxy.h"

#include "rendering/render_canvasimage.h"
#include "rendering/render_object.h"
#include "rendering/render_layer.h"

#include <kdebug.h>

#include "css/cssparser.h"
#include "css/css_stylesheetimpl.h"
#include "css/css_ruleimpl.h"

#include <QtAlgorithms>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QPixmap>
#include <QBrush>
#include <QGradient>
#include <QColor>
#include <QtDebug>

using namespace DOM;
using khtml::RenderCanvasImage;

#include "kjs_context2d.lut.h"

namespace KJS {

////////////////////// Context2D Object ////////////////////////

IMPLEMENT_PROTOFUNC(Context2DFunction)

static bool isGradient(ValueImp *value)
{
    return value->isObject(&Gradient::info);
}

static bool isImagePattern(ValueImp *value)
{
    return value->isObject(&ImagePattern::info);
}

static QPainter::CompositionMode compositeOperatorFromString(const QString &compositeOperator)
{
    if ( compositeOperator == "source-over" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "source-out" ) {
        return QPainter::CompositionMode_SourceOut;
    } else if ( compositeOperator == "source-in" ) {
        return QPainter::CompositionMode_SourceIn;
    } else if ( compositeOperator == "source-atop" ) {
        return QPainter::CompositionMode_SourceAtop;
    } else if ( compositeOperator == "destination-atop" ) {
        return QPainter::CompositionMode_DestinationAtop;
    } else if ( compositeOperator == "destination-in" ) {
        return QPainter::CompositionMode_DestinationIn;
    } else if ( compositeOperator == "destination-out" ) {
        return QPainter::CompositionMode_DestinationOut;
    } else if ( compositeOperator == "destination-over" ) {
        return QPainter::CompositionMode_DestinationOver;
    } else if ( compositeOperator == "darker" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "lighter" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "copy" ) {
        return QPainter::CompositionMode_Source;
    } else if ( compositeOperator == "xor" ) {
        return QPainter::CompositionMode_Xor;
    }

    return QPainter::CompositionMode_SourceOver;
}

#define DEGREES(t) ((t) * 180.0 / M_PI)

ValueImp *KJS::Context2DFunction::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
    if (!thisObj->inherits(&Context2D::info))
        return throwError(exec, TypeError);

    Context2D *contextObject = static_cast<KJS::Context2D *>(thisObj);
    khtml::RenderCanvasImage *renderer = static_cast<khtml::RenderCanvasImage*>(contextObject->_element->renderer());
    if (!renderer)
        return Undefined();

    QPainter *drawingContext = renderer->drawingContext();
    if (!drawingContext)
        return Undefined();

    switch (id) {
    case Context2D::Save: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);
        drawingContext->save();
        contextObject->save();
        break;
    }
    case Context2D::Restore: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);
        drawingContext->restore();
        contextObject->restore();

        break;
    }
    case Context2D::BeginPath: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);
        contextObject->setPath( QPainterPath() );
        break;
    }
    case Context2D::ClosePath: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);
        contextObject->path().closeSubpath();
        break;
    }
    case Context2D::SetStrokeColor: {
        // string arg = named color
        // string arg, number arg = named color, alpha
        // number arg = gray color
        // number arg, number arg = gray color, alpha
        // 4 args (string or number) = r, g, b, a
        // 5 args (string or number) = c, m, y, k, a
        QPen pen = drawingContext->pen();
        int numArgs = args.size();
        switch (numArgs) {
        case 1: {
            if (args[0]->isString()) {
                QRgb color;
                DOM::CSSParser::parseColor(args[0]->toString(exec).qstring(), color);
                QColor qc(color);
                pen.setColor( qc );

            }
            else {
                float g = (float)args[0]->toNumber(exec);
                QColor qc;
                qc.setRgbF( g, g, g );
                pen.setColor( qc );
            }
        }
            break;
        case 2: {
            float a = args[1]->toNumber(exec);
            if (args[0]->isString()) {
                QRgb color;
                DOM::CSSParser::parseColor(args[0]->toString(exec).qstring(), color);
                QColor qc(color);
                qc.setAlphaF( a );
                pen.setColor( qc );
            }
            else {
                float g = (float)args[0]->toNumber(exec);
                QColor qc;
                qc.setRgbF( g, g, g, a );
                pen.setColor( qc );
            }
        }
            break;
        case 4: {
            float r = (float)args[0]->toNumber(exec);
            float g = (float)args[1]->toNumber(exec);
            float b = (float)args[2]->toNumber(exec);
            float a = (float)args[3]->toNumber(exec);
            QColor qc;
            qc.setRgbF( r, g, b, a );
            pen.setColor( qc );
        }
            break;
        case 5: {
            float c = (float)args[0]->toNumber(exec);
            float m = (float)args[1]->toNumber(exec);
            float y = (float)args[2]->toNumber(exec);
            float k = (float)args[3]->toNumber(exec);
            float a = (float)args[4]->toNumber(exec);
            QColor qc;
            qc.setCmykF( c, m, y, k, a );
            pen.setColor( qc );
        }
            break;
        default:
            return throwError(exec, SyntaxError);
        }
        drawingContext->setPen( pen );
        break;
    }
    case Context2D::SetFillColor: {
        // string arg = named color
        // string arg, number arg = named color, alpha
        // number arg = gray color
        // number arg, number arg = gray color, alpha
        // 4 args (string or number) = r, g, b, a
        // 5 args (string or number) = c, m, y, k, a
        int numArgs = args.size();
        QBrush brush = drawingContext->brush();
        switch (numArgs) {
        case 1: {
            if (args[0]->isString()) {
                QRgb color;
                DOM::CSSParser::parseColor(args[0]->toString(exec).qstring(), color);
                QColor qc(color);
                brush.setColor( qc );
                drawingContext->setBrush( brush );
            }
            else {
                float g = (float)args[0]->toNumber(exec);
                QColor qc;
                qc.setRgbF( g, g, g );
                brush.setColor( qc );
                drawingContext->setBrush( brush );
            }
        }
            break;
        case 2: {
            float a = args[1]->toNumber(exec);
            if (args[0]->isString()) {
                QRgb color;
                DOM::CSSParser::parseColor(args[0]->toString(exec).qstring(), color);
                QColor qc(color);
                brush.setColor( qc );
                drawingContext->setBrush( brush );
            }
            else {
                float g = (float)args[0]->toNumber(exec);
                QColor qc;
                qc.setRgbF( g, g, g, a );
                brush.setColor( qc );
                drawingContext->setBrush( brush );
            }
        }
            break;
        case 4: {
            float r = (float)args[0]->toNumber(exec);
            float g = (float)args[1]->toNumber(exec);
            float b = (float)args[2]->toNumber(exec);
            float a = (float)args[3]->toNumber(exec);
            QColor qc;
            qc.setRgbF( r, g, b, a );
            brush.setColor( qc );
            drawingContext->setBrush( brush );
        }
            break;
        case 5: {
            float c = (float)args[0]->toNumber(exec);
            float m = (float)args[1]->toNumber(exec);
            float y = (float)args[2]->toNumber(exec);
            float k = (float)args[3]->toNumber(exec);
            float a = (float)args[4]->toNumber(exec);
            QColor qc;
            qc.setCmykF( c, m, y, k, a );
            brush.setColor( qc );
            drawingContext->setBrush( brush );
        }
            break;
        default:
            return throwError(exec, SyntaxError);
        }
        break;
    }
    case Context2D::SetLineWidth: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        float w = (float)args[0]->toNumber(exec);
        QPen pen = drawingContext->pen();
        pen.setWidthF( w );
        drawingContext->setPen( pen );
        break;
    }
    case Context2D::SetLineCap: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        QPen pen = drawingContext->pen();
        DOMString capString = args[0]->toString(exec).domString().lower();
        if (capString == "round")
            pen.setCapStyle( Qt::RoundCap );
        else if (capString == "square")
            pen.setCapStyle( Qt::SquareCap );
        else
            pen.setCapStyle( Qt::FlatCap );
        drawingContext->setPen( pen );
        break;
    }
    case Context2D::SetLineJoin: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        DOMString joinString = args[0]->toString(exec).domString().lower();
        QPen pen = drawingContext->pen();
        if (joinString == "round")
            pen.setJoinStyle(Qt::RoundJoin);
        else if (joinString == "bevel")
            pen.setJoinStyle(Qt::BevelJoin);
        else
            pen.setJoinStyle(Qt::MiterJoin);
        drawingContext->setPen( pen );
        break;
    }
    case Context2D::SetMiterLimit: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        float l = (float)args[0]->toNumber(exec);
        QPen pen = drawingContext->pen();
        pen.setMiterLimit( l );
        drawingContext->setPen( pen );
        break;
    }
    case Context2D::Fill: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);

        drawingContext->fillPath( contextObject->path(), contextObject->constructBrush( exec ) );

        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::Stroke: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);

        drawingContext->strokePath( contextObject->path(), contextObject->constructPen( exec ) );

        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::Scale: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);
        float sx = (float)args[0]->toNumber(exec);
        float sy = (float)args[1]->toNumber(exec);
        drawingContext->scale( sx, sy );
        contextObject->_needsFlushRasterCache = true;
        break;
    }
    case Context2D::Rotate: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        float angle = (float)args[0]->toNumber(exec);
        drawingContext->rotate( angle );
        contextObject->_needsFlushRasterCache = true;
        break;
    }
    case Context2D::Translate: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);
        float tx = (float)args[0]->toNumber(exec);
        float ty = (float)args[1]->toNumber(exec);
        drawingContext->translate( tx, ty );
        break;
    }
    case Context2D::MoveTo: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        contextObject->path().moveTo( x, y );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::LineTo: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        contextObject->path().lineTo( x, y );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::QuadraticCurveTo: {
        if (args.size() != 4)
            return throwError(exec, SyntaxError);
        float cpx = (float)args[0]->toNumber(exec);
        float cpy = (float)args[1]->toNumber(exec);
        float x = (float)args[2]->toNumber(exec);
        float y = (float)args[3]->toNumber(exec);
        contextObject->path().quadTo( cpx, cpy, x, y );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::BezierCurveTo: {
        if (args.size() != 6)
            return throwError(exec, SyntaxError);
        float cp1x = (float)args[0]->toNumber(exec);
        float cp1y = (float)args[1]->toNumber(exec);
        float cp2x = (float)args[2]->toNumber(exec);
        float cp2y = (float)args[3]->toNumber(exec);
        float x = (float)args[4]->toNumber(exec);
        float y = (float)args[5]->toNumber(exec);
        contextObject->path().cubicTo( cp1x, cp1y, cp2x, cp2y, x, y );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::ArcTo: {
        if (args.size() != 5)
            return throwError(exec, SyntaxError);
        float x1 = (float)args[0]->toNumber(exec);
        float y1 = (float)args[1]->toNumber(exec);
        float x2 = (float)args[2]->toNumber(exec);
        float y2 = (float)args[3]->toNumber(exec);
        float r = (float)args[4]->toNumber(exec);
        //### FIXME busted
        //me no feely like doing math to convert this correctly, yo
        contextObject->path().arcTo(x1, y1, x2-x1, y2-y1, r, 90);
        break;
    }
    case Context2D::Arc: {
        if (args.size() != 6)
            return throwError(exec, SyntaxError);
        float xc = (float)args[0]->toNumber(exec);
        float yc = (float)args[1]->toNumber(exec);
        float radius = (float)args[2]->toNumber(exec);
        float sar = (float)args[3]->toNumber(exec);
        float ear = (float)args[4]->toNumber(exec);
        bool  anticlockwise = args[5]->toBoolean(exec);


        //### HACK
        // In Qt we don't switch the coordinate system for degrees
        // and still use the 0,0 as bottom left for degrees so we need
        // to switch
        sar = -sar;
        ear = -ear;
        anticlockwise = !anticlockwise;
        //end hack

        float sa = DEGREES( sar );
        float ea = DEGREES( ear );

        double span = 0;

        double xs     = xc - radius;
        double ys     = yc - radius;
        double width  = radius*2;
        double height = radius*2;

        if ( !anticlockwise && ( ea < sa ) ) {
            span += 360;
        } else if ( anticlockwise && ( sa < ea ) ) {
            span -= 360;
        }

        //### this is also due to switched coordinate system
        // we would end up with a 0 span instead of 360
        if ( !( qFuzzyCompare( span + ( ea - sa ), 0.0 ) &&
                qFuzzyCompare( abs( span ), 360.0 ) ) ) {
            span   += ea - sa;
        }

        contextObject->path().moveTo( QPointF( xc + radius  * cos( sar ),
                                               yc - radius  * sin( sar ) ) );
#if 0
        QPen pen = drawingContext->pen();
        pen.setColor( Qt::red );
        pen.setWidth( 10 );
        drawingContext->save();
        drawingContext->setPen( pen );
        drawingContext->drawPoint( QPointF( xc + radius  * cos( sar ),
                                            yc - radius * sin( sar ) ) );
        drawingContext->restore();
        qDebug()<<"arcTo "<<xs<<ys<<width<<height<<sa<<span;
#endif

        contextObject->path().arcTo(xs, ys, width, height, sa, span);
        break;
    }
    case Context2D::Rect: {
        if (args.size() != 4)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        float w = (float)args[2]->toNumber(exec);
        float h = (float)args[3]->toNumber(exec);
        contextObject->path().addRect( x, y, w, h );
        break;
    }
    case Context2D::Clip: {
        if (args.size() != 0)
            return throwError(exec, SyntaxError);
        drawingContext->setClipPath( contextObject->path() );
        break;
    }

    case Context2D::ClearRect: {
        if (args.size() != 4)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        float w = (float)args[2]->toNumber(exec);
        float h = (float)args[3]->toNumber(exec);
        drawingContext->eraseRect( QRectF( x, y, w, h ) );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::FillRect: {
        if (args.size() != 4)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        float w = (float)args[2]->toNumber(exec);
        float h = (float)args[3]->toNumber(exec);
        drawingContext->fillRect( QRectF( x, y, w, h ), contextObject->constructBrush( exec ) );
        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::StrokeRect: {
        int size = args.size();
        if (size < 4)
            return throwError(exec, SyntaxError);
        float x = (float)args[0]->toNumber(exec);
        float y = (float)args[1]->toNumber(exec);
        float w = (float)args[2]->toNumber(exec);
        float h = (float)args[3]->toNumber(exec);

        QPainterPath path; path.addRect( x, y, w, h );
        if (size > 4) {
            QPen pen = contextObject->constructPen( exec );
            pen.setWidthF( (float)args[4]->toNumber(exec) );
            drawingContext->strokePath( path, pen );
        }
        else {
            drawingContext->strokePath( path, contextObject->constructPen( exec ) );
        }
        renderer->setNeedsImageUpdate();
        break;
    }
        // DrawImage has three variants:
        // drawImage (img, dx, dy)
        // drawImage (img, dx, dy, dw, dh)
        // drawImage (img, sx, sy, sw, sh, dx, dy, dw, dh)
        // composite operation is specified with globalCompositeOperation
        // img parameter can be a JavaScript Image, <img>, or a <canvas>
    case Context2D::DrawImage: {
        if (args.size() < 3)
            return throwError(exec, SyntaxError);

        // Make sure first argument is an object.
        ObjectImp *o = static_cast<ObjectImp*>(args[0]);
        if (!o->isObject())
            return throwError(exec, TypeError);

        float w = 0; // quiet incorrect gcc 4.0 warning
        float h = 0; // quiet incorrect gcc 4.0 warning
        QPixmap pixmap;
        QPainter *sourceContext = 0;

        // Check for <img> or <canvas>.
        if (o->inherits(&KJS::HTMLElement::img_info)){
            NodeImpl *n = static_cast<HTMLElement *>(args[0])->impl();
            HTMLImageElementImpl *e = static_cast<HTMLImageElementImpl*>(n);
            pixmap = e->currentPixmap();
            w = pixmap.width();
            h = pixmap.height();
        }
        else if (o->inherits(&KJS::HTMLElement::canvas_info)){
            NodeImpl *n = static_cast<HTMLElement *>(args[0])->impl();
            HTMLCanvasElementImpl *e = static_cast<HTMLCanvasElementImpl*>(n);
            khtml::RenderCanvasImage *renderer = static_cast<khtml::RenderCanvasImage*>(e->renderer());
            if (!renderer) {
                // No renderer, nothing to draw.
                return Undefined();
            }

            sourceContext = renderer->drawingContext();
            w = (float)sourceContext->device()->width();
            h = (float)sourceContext->device()->height();
        }
        else
            return throwError(exec, TypeError);

        float dx, dy, dw = w, dh = h;
        float sx = 0.f, sy = 0.f, sw = w, sh = h;

        if (args.size() == 3) {
            dx = args[1]->toNumber(exec);
            dy = args[2]->toNumber(exec);
        }
        else if (args.size() == 5) {
            dx = args[1]->toNumber(exec);
            dy = args[2]->toNumber(exec);
            dw = args[3]->toNumber(exec);
            dh = args[4]->toNumber(exec);
        }
        else if (args.size() == 9) {
            sx = args[1]->toNumber(exec);
            sy = args[2]->toNumber(exec);
            sw = args[3]->toNumber(exec);
            sh = args[4]->toNumber(exec);
            dx = args[5]->toNumber(exec);
            dy = args[6]->toNumber(exec);
            dw = args[7]->toNumber(exec);
            dh = args[8]->toNumber(exec);
        }
        else
            return throwError(exec, SyntaxError);

        if (!sourceContext) {
            //### set composite op
            //QPainter::compositeOperatorFromString(contextObject->_globalComposite->toString(exec).qstring().lower())
            drawingContext->drawPixmap( QRectF( dx, dy, dw, dh ), pixmap, QRectF( sx, sy, sw, sh ) );
        }
        else {
            // Cheap, because the image is backed by copy-on-write memory, and we're
            // guaranteed to release before doing any more drawing in the source context.
            sourceContext->end();
            QImage *sourceImage = dynamic_cast<QImage*>( sourceContext->device() );
            if ( sourceImage ) {
                drawingContext->drawImage( QRectF( sx, sy, sw, sh ), *sourceImage,
                                           QRectF( dx, dy, dw, dh ) );
            }
        }

        renderer->setNeedsImageUpdate();
        break;
    }
    case Context2D::DrawImageFromRect: {
        if (args.size() != 10)
            return throwError(exec, SyntaxError);
        ObjectImp *o = static_cast<ObjectImp*>(args[0]);
        if (!o->isObject() || !o->inherits(&HTMLElement::img_info))
            return throwError(exec, TypeError);
        DOM::HTMLImageElementImpl *i = static_cast<DOM::HTMLImageElementImpl*>(static_cast<HTMLElement*>(o)->impl());
        float sx = args[1]->toNumber(exec);
        float sy = args[2]->toNumber(exec);
        float sw = args[3]->toNumber(exec);
        float sh = args[4]->toNumber(exec);
        float dx = args[5]->toNumber(exec);
        float dy = args[6]->toNumber(exec);
        float dw = args[7]->toNumber(exec);
        float dh = args[8]->toNumber(exec);
        QString compositeOperator = args[9]->toString(exec).qstring().lower();
        khtml::CachedImage *ci = i->image();
        if (ci) {
            QPixmap pixmap = ci->pixmap();
            if ( !compositeOperator.isEmpty() ) {
                QPainter::CompositionMode mode = compositeOperatorFromString(compositeOperator);
                drawingContext->setCompositionMode( mode );
            }

            drawingContext->drawPixmap(QRectF( dx, dy, dw, dh ), pixmap, QRectF( sx, sy, sw, sh ));

            renderer->setNeedsImageUpdate();
        }
        break;
    }
    case Context2D::SetAlpha: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        float a =  (float)args[0]->toNumber(exec);
        QPen     pen = drawingContext->pen();
        QBrush brush = drawingContext->brush();
        QColor qc = pen.color();
        qc.setAlphaF( a );
        pen.setColor( qc );
        qc = brush.color();
        qc.setAlphaF( a );
        brush.setColor( qc );
        drawingContext->setPen( pen );
        drawingContext->setBrush( brush );
        break;
    }
    case Context2D::SetCompositeOperation: {
        if (args.size() != 1)
            return throwError(exec, SyntaxError);
        QString compositeOperator = args[0]->toString(exec).qstring().lower();
        QPainter::CompositionMode mode = compositeOperatorFromString( compositeOperator );
        //###
        drawingContext->setCompositionMode(mode);
        break;
    }

    case Context2D::CreateLinearGradient: {
        if (args.size() != 4)
            return throwError(exec, SyntaxError);
        float x0 = args[0]->toNumber(exec);
        float y0 = args[1]->toNumber(exec);
        float x1 = args[2]->toNumber(exec);
        float y1 = args[3]->toNumber(exec);

        return new Gradient(x0, y0, x1, y1);
    }

    case Context2D::CreateRadialGradient: {
        if (args.size() != 6)
            return throwError(exec, SyntaxError);
        float x0 = args[0]->toNumber(exec);
        float y0 = args[1]->toNumber(exec);
        float r0 = args[2]->toNumber(exec);
        float x1 = args[3]->toNumber(exec);
        float y1 = args[4]->toNumber(exec);
        float r1 = args[5]->toNumber(exec);

        return new Gradient(x0, y0, r0, x1, y1, r1);
    }

    case Context2D::CreatePattern: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);
        ObjectImp *o = static_cast<ObjectImp*>(args[0]);
        if (!o->isObject() || !o->inherits(&HTMLElement::img_info))
            return throwError(exec, TypeError);
        int repetitionType = ImagePattern::Repeat;
        DOMString repetitionString = args[1]->toString(exec).domString().lower();
        if (repetitionString == "repeat-x")
            repetitionType = ImagePattern::RepeatX;
        else if (repetitionString == "repeat-y")
            repetitionType = ImagePattern::RepeatY;
        else if (repetitionString == "no-repeat")
            repetitionType = ImagePattern::NoRepeat;
        return new ImagePattern(static_cast<HTMLElement*>(o), repetitionType);
    }
    }

    return Undefined();
}

const ClassInfo Context2D::info = { "Context2D", 0, &Context2DTable, 0 };

/* Source for Context2DTable. Use "make hashtables" to regenerate.
   @begin Context2DTable 48
   strokeStyle              Context2D::StrokeStyle                 DontDelete
   fillStyle                Context2D::FillStyle                   DontDelete
   lineWidth                Context2D::LineWidth                   DontDelete
   lineCap                  Context2D::LineCap                     DontDelete
   lineJoin                 Context2D::LineJoin                    DontDelete
   miterLimit               Context2D::MiterLimit                  DontDelete
   globalAlpha              Context2D::GlobalAlpha                 DontDelete
   globalCompositeOperation Context2D::GlobalCompositeOperation    DontDelete
   save                     Context2D::Save                        DontDelete|Function 0
   restore                  Context2D::Restore                     DontDelete|Function 0
   scale                    Context2D::Scale                       DontDelete|Function 2
   rotate                   Context2D::Rotate                      DontDelete|Function 2
   translate                Context2D::Translate                   DontDelete|Function 1
   beginPath                Context2D::BeginPath                   DontDelete|Function 0
   closePath                Context2D::ClosePath                   DontDelete|Function 0
   setStrokeColor           Context2D::SetStrokeColor              DontDelete|Function 1
   setFillColor             Context2D::SetFillColor                DontDelete|Function 1
   setLineWidth             Context2D::SetLineWidth                DontDelete|Function 1
   setLineCap               Context2D::SetLineCap                  DontDelete|Function 1
   setLineJoin              Context2D::SetLineJoin                 DontDelete|Function 1
   setMiterLimit            Context2D::SetMiterLimit               DontDelete|Function 1
   fill                     Context2D::Fill                        DontDelete|Function 0
   stroke                   Context2D::Stroke                      DontDelete|Function 0
   moveTo                   Context2D::MoveTo                      DontDelete|Function 2
   lineTo                   Context2D::LineTo                      DontDelete|Function 2
   quadraticCurveTo         Context2D::QuadraticCurveTo            DontDelete|Function 4
   bezierCurveTo            Context2D::BezierCurveTo               DontDelete|Function 6
   arcTo                    Context2D::ArcTo                       DontDelete|Function 5
   arc                      Context2D::Arc                         DontDelete|Function 6
   rect                     Context2D::Rect                        DontDelete|Function 4
   clip                     Context2D::Clip                        DontDelete|Function 0
   clearRect                Context2D::ClearRect                   DontDelete|Function 4
   fillRect                 Context2D::FillRect                    DontDelete|Function 4
   strokeRect               Context2D::StrokeRect                  DontDelete|Function 4
   drawImage                Context2D::DrawImage                   DontDelete|Function 6
   drawImageFromRect        Context2D::DrawImageFromRect           DontDelete|Function 10
   setAlpha                 Context2D::SetAlpha                    DontDelete|Function 1
   setCompositeOperation    Context2D::SetCompositeOperation       DontDelete|Function 1
   createLinearGradient     Context2D::CreateLinearGradient        DontDelete|Function 4
   createRadialGradient     Context2D::CreateRadialGradient        DontDelete|Function 6
   createPattern            Context2D::CreatePattern               DontDelete|Function 2
   @end
*/

bool Context2D::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    // FIXME: functions should be on the prototype, not in the object itself
    return getStaticPropertySlot<Context2DFunction, Context2D, DOMObject>(exec, &Context2DTable, this, propertyName, slot);
}

ValueImp *Context2D::getValueProperty(ExecState *, int token) const
{
    switch(token) {
    case StrokeStyle: {
        return _strokeStyle;
    }

    case FillStyle: {
        return _fillStyle;
    }

    case LineWidth: {
        return _lineWidth;
    }

    case LineCap: {
        return _lineCap;
    }

    case LineJoin: {
        return _lineJoin;
    }

    case MiterLimit: {
        return _miterLimit;
    }

    case GlobalAlpha: {
        return _globalAlpha;
    }

    case GlobalCompositeOperation: {
        return _globalComposite;
    }

    default: {
    }
    }

    return Undefined();
}

void Context2D::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
    lookupPut<Context2D,DOMObject>(exec, propertyName, value, attr, &Context2DTable, this );
}

QPainter *Context2D::drawingContext()
{
    khtml::RenderCanvasImage *renderer = static_cast<khtml::RenderCanvasImage*>(_element->renderer());
    if (!renderer)
        return 0;

    QPainter *context = renderer->drawingContext();
    if (!context)
        return 0;

    return context;
}

QColor colorFromValue(ExecState *exec, ValueImp *value)
{
    QRgb color;
    DOM::CSSParser::parseColor(value->toString(exec).qstring(), color);
    return QColor(color);
}

void Context2D::updateStrokeImagePattern()
{
    QPainter *context = drawingContext();

    if (!_validStrokeImagePattern) {
        ImagePattern *imagePattern = static_cast<ImagePattern *>(_fillStyle);
        QBrush pattern = imagePattern->createPattern();
        QPen pen = context->pen();
        pen.setBrush( pattern );
        context->setPen( pen );
        _validStrokeImagePattern = true;
    }
}

QBrush Context2D::constructBrush(ExecState* exec)
{
    //### FIXME: caching and such
    if (_fillStyle->isString()) {
        QColor qc = colorFromValue(exec, _fillStyle);
        return QBrush( qc );
    }
    else {
        ObjectImp *o = static_cast<ObjectImp*>(_fillStyle);

        if (o->inherits(&Gradient::info)) {
            Gradient *gradient = static_cast<Gradient*>(o);
            return QBrush( *gradient->qgradient() );
        } else {
            //Must be an image pattern
            ImagePattern *imagePattern = static_cast<ImagePattern *>(_fillStyle);
            return imagePattern->createPattern();
        }
    }
}


QPen Context2D::constructPen(ExecState* exec)
{
    //### FIXME: caching and such
    QPen pen = drawingContext()->pen();
    if (_strokeStyle->isString()) {
        QColor qc = colorFromValue(exec, _strokeStyle);
        pen.setColor( qc );
        return pen;
    }
    else {
        ObjectImp *o = static_cast<ObjectImp*>(_strokeStyle);
        if (o->inherits(&Gradient::info)) {
            Gradient *gradient = static_cast<Gradient*>(o);
            pen.setBrush( QBrush( *gradient->qgradient() ) );
            return pen;
        } else {
            //Must be an image pattern
            ImagePattern *imagePattern = static_cast<ImagePattern *>(_fillStyle);
            pen.setBrush( imagePattern->createPattern() );
            return pen;
        }
    }
}

void Context2D::putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/)
{
    QPainter *context = drawingContext();
    if (!context)
        return;

    switch(token) {
    case StrokeStyle: {
        _strokeStyle = value;
        if (value->isString()) {
            QColor qc = colorFromValue(exec, value);
            QPen pen = context->pen();
            pen.setColor( qc );
        }
        else {
            // _strokeStyle is used when stroke() is called on the context.
            //### fix this
            ObjectImp *o = static_cast<ObjectImp*>(value);

            if (!o->isObject() || !(o->inherits(&Gradient::info) || o->inherits(&ImagePattern::info)))
                throwError(exec, TypeError);
        }
        break;
    }

    case FillStyle: {
        if (!value->isString()) {
            ObjectImp *o = static_cast<ObjectImp*>(value);
            if (!o->isObject() || !(o->inherits(&Gradient::info) || o->inherits(&ImagePattern::info)))
                throwError(exec, TypeError);
        }
        _fillStyle = value;
        break;
    }

    case LineWidth: {
        _lineWidth = value;
        float w = (float)value->toNumber(exec);
        QPen pen = context->pen();
        pen.setWidthF( w );
        context->setPen( pen );
        break;
    }

    case LineCap: {
        _lineCap = value;
        QPen pen = context->pen();
        DOMString capString = value->toString(exec).domString().lower();
        if (capString == "round")
            pen.setCapStyle( Qt::RoundCap );
        else if (capString == "square")
            pen.setCapStyle( Qt::SquareCap );
        else
            pen.setCapStyle( Qt::FlatCap );
        context->setPen( pen );
        break;
    }

    case LineJoin: {
        _lineJoin = value;
        DOMString joinString = value->toString(exec).domString().lower();
        QPen pen = context->pen();
        if (joinString == "round")
            pen.setJoinStyle( Qt::RoundJoin );
        else if (joinString == "bevel")
            pen.setJoinStyle( Qt::BevelJoin );
        else
            pen.setJoinStyle( Qt::MiterJoin );
        context->setPen( pen );
        break;
    }

    case MiterLimit: {
        _miterLimit = value;

        float l = (float)value->toNumber(exec);
        QPen pen = context->pen();
        pen.setMiterLimit( l );
        context->setPen( pen );
        break;
    }

    case GlobalAlpha: {
        _globalAlpha = value;
        float a =  (float)value->toNumber(exec);
        QBrush brush = context->brush();
        QPen     pen = context->pen();

        QColor qc = brush.color(); qc.setAlphaF( a ); brush.setColor( qc );
        context->setBrush( brush );

        qc = pen.color(); qc.setAlphaF( a ); pen.setColor( qc );
        context->setPen( pen );
        break;
    }

    case GlobalCompositeOperation: {
        _globalComposite = value;
        QString compositeOperator = value->toString(exec).qstring().lower();
        QPainter::CompositionMode mode = compositeOperatorFromString( compositeOperator );
        context->setCompositionMode( mode );
        break;
    }

    default: {
    }
    }
}

void Context2D::save()
{
    List *list = new List();

    list->append(_strokeStyle);
    list->append(_fillStyle);
    list->append(_lineWidth);
    list->append(_lineCap);
    list->append(_lineJoin);
    list->append(_miterLimit);
    list->append(_globalAlpha);
    list->append(_globalComposite);

    stateStack.append(list);
}

void Context2D::restore()
{
    if (stateStack.count() < 1) {
        return;
    }

    List *list = stateStack.last();

    int pos = 0;
    _strokeStyle = list->at(pos++);
    _fillStyle = list->at(pos++);
    _lineWidth = list->at(pos++);
    _lineCap = list->at(pos++);
    _lineJoin = list->at(pos++);
    _miterLimit = list->at(pos++);
    _globalAlpha = list->at(pos++);
    _globalComposite = list->at(pos++);

    // This will delete list.
    stateStack.removeLast();
}

Context2D::Context2D(HTMLElementImpl *e)
    : _validStrokeImagePattern(false),
      _element(e), _needsFlushRasterCache(false),
      _strokeStyle(Undefined()),
      _fillStyle(Undefined()),
      _lineWidth(Undefined()),
      _lineCap(Undefined()),
      _lineJoin(Undefined()),
      _miterLimit(Undefined()),
      _globalAlpha(Undefined()),
      _globalComposite(Undefined())
{
    _lineWidth = Number(1);
    _strokeStyle = String("black");
    _fillStyle = String("black");

    _lineCap = String("butt");
    _lineJoin = String("miter");
    _miterLimit = Number(10);

    _globalAlpha = Number(1);
    _globalComposite = String("source-over");
}

Context2D::~Context2D()
{
    qDeleteAll(stateStack.begin(), stateStack.end());
}

void Context2D::mark()
{
    ValueImp *v;

    v = _strokeStyle;
    if (!v->marked())
        v->mark();

    v = _fillStyle;
    if (!v->marked())
        v->mark();

    v = _lineWidth;
    if (!v->marked())
        v->mark();

    v = _lineCap;
    if (!v->marked())
        v->mark();

    v = _lineJoin;
    if (!v->marked())
        v->mark();

    v = _miterLimit;
    if (!v->marked())
        v->mark();

    v = _globalAlpha;
    if (!v->marked())
        v->mark();

    v = _globalComposite;
    if (!v->marked())
        v->mark();

    QListIterator<List*> it(stateStack);
    List *list;
    while (it.hasNext()) {
        list = it.next();
        list->mark();
    }

    DOMObject::mark();
}

const ClassInfo KJS::Gradient::info = { "Gradient", 0, &GradientTable, 0 };

/* Source for GradientTable. Use "make hashtables" to regenerate.
   @begin GradientTable 1
   addColorStop             Gradient::AddColorStop                DontDelete|Function 2
   @end
*/

IMPLEMENT_PROTOFUNC(GradientFunction)

ValueImp *GradientFunction::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
    if (!thisObj->inherits(&Gradient::info))
        return throwError(exec, TypeError);

    Gradient *gradient = static_cast<KJS::Gradient *>(thisObj);

    switch (id) {
    case Gradient::AddColorStop: {
        if (args.size() != 2)
            return throwError(exec, SyntaxError);

        QColor color = colorFromValue(exec, args[1]);
        gradient->addColorStop ((float)args[0]->toNumber(exec), color.red()/255.f, color.green()/255.f, color.blue()/255.f, color.alpha()/255.f);
    }
    }

    return Undefined();
}

void Gradient::commonInit()
{
    stops = 0;
    stopCount = 0;
    maxStops = 0;
    stopsNeedAdjusting = false;
    adjustedStopCount = 0;
    adjustedStops = 0;
}

Gradient::Gradient(float x0, float y0, float x1, float y1)
{
    m_gradient = new QLinearGradient( x0, y0, x1, y1 );

    commonInit();
}

Gradient::Gradient(float x0, float y0, float r0, float x1, float y1, float /*r1*/)
{
    //### need to add the outer redius to Qt Arthur
    m_gradient = new QRadialGradient( x0, y0, r0, x1, y1 );

    commonInit();
}

bool Gradient::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticPropertySlot<GradientFunction, Gradient, DOMObject>(exec, &GradientTable, this, propertyName, slot);
}

ValueImp *Gradient::getValueProperty(ExecState *, int token) const
{
    return Undefined();
}

void Gradient::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
    lookupPut<Gradient,DOMObject>(exec, propertyName, value, attr, &GradientTable, this );
}

void Gradient::putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/)
{
}

Gradient::~Gradient()
{
    delete m_gradient; m_gradient = 0;
}

void Gradient::addColorStop (float s, float r, float g, float b, float a)
{
    QColor qc;
    qc.setRgbF( r, g, b, a );
    m_gradient->setColorAt( s, qc );
}

const ClassInfo ImagePattern::info = { "ImagePattern", 0, &ImagePatternTable, 0 };

/* Source for ImagePatternTable. Use "make hashtables" to regenerate.
   @begin ImagePatternTable 0
   @end
*/

ImagePattern::ImagePattern(HTMLElement *i, int repetitionType)
    : _rw(0), _rh(0)
{
    khtml::CachedImage *ci = static_cast<DOM::HTMLImageElementImpl*>(i->impl())->image();
    if (ci) {
        _pixmap = ci->pixmap();
        float w = _pixmap.width();
        float h = _pixmap.height();
    }
}

QBrush ImagePattern::createPattern()
{
    if (_pixmap.isNull())
        return QBrush();
    return QBrush( _pixmap );
}

bool ImagePattern::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<ImagePattern, DOMObject>(exec, &ImagePatternTable, this, propertyName, slot);
}

ValueImp *ImagePattern::getValueProperty(ExecState *, int token) const
{
    return Undefined();
}

void ImagePattern::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
    lookupPut<ImagePattern,DOMObject>(exec, propertyName, value, attr, &ImagePatternTable, this );
}

void ImagePattern::putValueProperty(ExecState *exec, int token, ValueImp *value, int /*attr*/)
{
}

} // namespace
