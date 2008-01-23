/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2005 Zack Rusin <zack@kde.org>
 * Copyright (C) 2007 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Portions of this code are (c) by Apple Computer, Inc. and were licensed
 * under the following terms:
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

#include "html_canvasimpl.h"
#include "html_documentimpl.h"

#include <khtmlview.h>
#include <khtml_part.h>

#include <config.h>

#include <kdebug.h>

#include <dom/dom_exception.h>
#include <rendering/render_canvasimage.h>
#include <rendering/render_flow.h>
#include <css/cssstyleselector.h>
#include <css/cssproperties.h>
#include <css/cssparser.h>
#include <css/cssvalues.h>
#include <css/csshelper.h>
#include <xml/dom2_eventsimpl.h>
#include <html/html_imageimpl.h>
#include <misc/htmltags.h>
#include <misc/htmlattrs.h>
#include <misc/translator.h>
#include <misc/imagefilter.h>
#include <imload/canvasimage.h>
#include <imload/imagemanager.h>
#include <kjs/global.h>
#include <kjs/operations.h> //uglyyy: needs for inf/NaN tests

#include <QtAlgorithms>
#include <QtCore/QCharRef>
#include <QtCore/QPoint>
#include <QtCore/QLocale>
#include <QtGui/QRegion>
#include <QtGui/QImage>
#include <QDebug>
#include <cmath>
#include <limits>

using namespace DOM;
using namespace khtml;
using namespace std;

// -------------------------------------------------------------------------

HTMLCanvasElementImpl::HTMLCanvasElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    w = 300;
    h = 150;
}

HTMLCanvasElementImpl::~HTMLCanvasElementImpl()
{
    if (context)
        context->canvasElement = 0;
}

void HTMLCanvasElementImpl::parseAttribute(AttributeImpl* attr)
{
    bool ok = false;
    int  val;
    switch (attr->id())
    {
    // ### TODO: making them reflect w/h -- how?
    case ATTR_WIDTH:
        val = attr->val() ? attr->val()->toInt(&ok) : -1;
        if (!ok || val <= 0)
            w = 300;
        else
            w = val;

        if (context)
            context->resetContext(w, h);
        setChanged();
        break;
    case ATTR_HEIGHT:
        val = attr->val() ? attr->val()->toInt(&ok) : -1;
        if (!ok || val <= 0)
            h = 150;
        else
            h = val;

        if (context)
            context->resetContext(w, h);
        setChanged();
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

NodeImpl::Id HTMLCanvasElementImpl::id() const
{
    return ID_CANVAS;
}

void HTMLCanvasElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

    RenderStyle* _style = getDocument()->styleSelector()->styleForElement(this);
    _style->ref();
    if (parentNode()->renderer() && parentNode()->renderer()->childAllowed() &&
        _style->display() != NONE)
    {
        m_render = new (getDocument()->renderArena()) RenderCanvasImage(this);
        m_render->setStyle(_style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
    }
    _style->deref();

    NodeBaseImpl::attach();
    if (m_render)
        m_render->updateFromElement();
}

CanvasContext2DImpl* HTMLCanvasElementImpl::getContext2D()
{
    if (!context)
        context = new CanvasContext2DImpl(this, w, h);;
    return context.get();
}

khtmlImLoad::CanvasImage* HTMLCanvasElementImpl::getCanvasImage()
{
    return getContext2D()->canvasImage;
}

// -------------------------------------------------------------------------
CanvasContext2DImpl::CanvasContext2DImpl(HTMLCanvasElementImpl* element, int width, int height):
    canvasElement(element), canvasImage(0)
{
    resetContext(width, height);
}

CanvasContext2DImpl::~CanvasContext2DImpl()
{
    if (workPainter.isActive())
        workPainter.end(); // Make sure to stop it before blowing the image away!
    delete canvasImage;
}

// Basic infrastructure..
void CanvasContext2DImpl::resetContext(int width, int height)
{
    // ### FIXME FIXME: use khtmlImLoad's limit policy
    // for physical canvas and transform painter to match logical resolution
    if (workPainter.isActive())
        workPainter.end();

    if (canvasImage)
        canvasImage->resizeImage(width, height);
    else
        canvasImage = new khtmlImLoad::CanvasImage(width, height);
    canvasImage->qimage()->fill(0x00000000); // transparent black is the initial state

    stateStack.clear();

    PaintState defaultState;
    beginPath();
    defaultState.infinityTransform = false;
    defaultState.clipPath = QPainterPath();
    defaultState.clipPath.setFillRule(Qt::WindingFill);
    defaultState.clipping = false;

    defaultState.globalAlpha = 1.0f;
    defaultState.globalCompositeOperation = QPainter::CompositionMode_SourceOver;

    defaultState.strokeStyle = new CanvasColorImpl(QColor(Qt::black));
    defaultState.fillStyle   = new CanvasColorImpl(QColor(Qt::black));

    defaultState.lineWidth  = 1.0f;
    defaultState.lineCap    = Qt::FlatCap;
    defaultState.lineJoin   = Qt::MiterJoin;
    defaultState.miterLimit = 10.0f;

    defaultState.shadowOffsetX = 0.0f;
    defaultState.shadowOffsetY = 0.0f;
    defaultState.shadowBlur    = 0.0f;
    defaultState.shadowColor   = QColor(0, 0, 0, 0); // Transparent black

    stateStack.push(defaultState);

    dirty = DrtAll;
    needRendererUpdate();
}

void CanvasContext2DImpl::save()
{
    stateStack.push(stateStack.top());
}

void CanvasContext2DImpl::restore()
{
    if (stateStack.size() <= 1)
        return;

    stateStack.pop();
    dirty = DrtAll;
}

QPainter* CanvasContext2DImpl::acquirePainter()
{
    if (!workPainter.isActive()) {
        workPainter.begin(canvasImage->qimage());
        workPainter.setRenderHint(QPainter::Antialiasing);
        workPainter.setRenderHint(QPainter::SmoothPixmapTransform);
        dirty = DrtAll;
    }

    PaintState& state = activeState();

    if (dirty & DrtClip) {
        if (state.clipping)
            workPainter.setClipPath(state.clipPath);
        else
            workPainter.setClipping(false);
    }

    if (dirty & DrtAlpha)
        workPainter.setOpacity(state.globalAlpha);
    if (dirty & DrtCompOp)
        workPainter.setCompositionMode(state.globalCompositeOperation);
    if (dirty & DrtStroke) {
        QPen pen;
        pen.setWidth(state.lineWidth);
        pen.setCapStyle (state.lineCap);
        pen.setJoinStyle(state.lineJoin);
        pen.setMiterLimit(state.miterLimit);

        CanvasStyleBaseImpl* style = state.strokeStyle.get();
        if (style->type() == CanvasStyleBaseImpl::Color)
            pen.setColor(static_cast<CanvasColorImpl*>(style)->color);
        else
            pen.setBrush(style->toBrush());
        workPainter.setPen(pen); // ### should I even do this?
                                 // I have a feeling I am mixing up path and
                                 // non-path ops
    }
    if (dirty & DrtFill)
        workPainter.setBrush(state.fillStyle->toBrush());

    dirty = 0;

    needRendererUpdate();
    return &workPainter;
}

QImage CanvasContext2DImpl::extractImage(ElementImpl* el, int& exceptionCode) const
{
    QImage pic;

    exceptionCode = 0;
    if (el->id() == ID_CANVAS) {
        CanvasContext2DImpl* other = static_cast<HTMLCanvasElementImpl*>(el)->getContext2D();
        if (other->workPainter.isActive())
            other->workPainter.end();
        pic = *other->canvasImage->qimage();
    } else if (el->id() == ID_IMG) {
        HTMLImageElementImpl* img = static_cast<HTMLImageElementImpl*>(el);
        if (img->complete())
            pic = img->currentImage();
        else
            exceptionCode = DOMException::INVALID_STATE_ERR;
    } else {
        exceptionCode = DOMException::TYPE_MISMATCH_ERR;
    }

    return pic;
}

void CanvasContext2DImpl::needRendererUpdate()
{
    needsCommit = true;
    if (canvasElement)
        canvasElement->setChanged();
}

void CanvasContext2DImpl::commit()
{
    // If painter is active, end it.
    if (workPainter.isActive())
        workPainter.end();

    // Flush caches if we have changes.
    if (needsCommit) {
        canvasImage->contentUpdated();
        needsCommit = false;
    }
}

HTMLCanvasElementImpl* CanvasContext2DImpl::canvas() const
{
    return canvasElement;
}

// Transformation ops
//

static inline float degrees(float radians)
{
    return radians * 180.0 / M_PI;
}

static inline bool isInfArg(float x)
{
    return KJS::isInf(x) || KJS::isNaN(x);
}

void CanvasContext2DImpl::scale(float x, float y)
{
    dirty |= DrtTransform;

    bool& infinityTransform = activeState().infinityTransform;
    infinityTransform |= isInfArg(x) | isInfArg(y);
    if (infinityTransform) return;

    activeState().transform.scale(x, y);
}

void CanvasContext2DImpl::rotate(float angle)
{
    dirty |= DrtTransform;

    bool& infinityTransform = activeState().infinityTransform;
    infinityTransform |= isInfArg(angle);
    if (infinityTransform) return;

    activeState().transform.rotate(degrees(angle));
}

void CanvasContext2DImpl::translate(float x, float y)
{
    dirty |= DrtTransform;

    bool& infinityTransform = activeState().infinityTransform;
    infinityTransform |= isInfArg(x) | isInfArg(y);
    if (infinityTransform) return;

    activeState().transform.translate(x, y);
}

void CanvasContext2DImpl::transform(float m11, float m12, float m21, float m22, float dx, float dy)
{
    dirty |= DrtTransform;

    bool& infinityTransform = activeState().infinityTransform;
    infinityTransform |= isInfArg(m11) | isInfArg(m12) | isInfArg(m21) | isInfArg(m22) |
                         isInfArg(dx)  | isInfArg(dy);
    if (infinityTransform) return;

    activeState().transform *= QTransform(m11, m12, 0.0f, m21, m22, 0.0f, dx, dy, 1.0f);
}

void CanvasContext2DImpl::setTransform(float m11, float m12, float m21, float m22, float dx, float dy)
{
    activeState().transform.reset();
    activeState().infinityTransform = false; // As cleared the matrix..
    transform(m11, m12, m21, m22, dx, dy);
}

// Composition state setting
//

float CanvasContext2DImpl::globalAlpha() const
{
    return activeState().globalAlpha;
}

void CanvasContext2DImpl::setGlobalAlpha(float a)
{
    if (a < 0.0f || a > 1.0f)
        return;

    activeState().globalAlpha = a;
    dirty |= DrtAlpha;
}

static const IDTranslator<QString, QPainter::CompositionMode, const char*>::Info compModeTranslatorTable[] = {
    {"source-over", QPainter::CompositionMode_SourceOver},
    {"source-out",  QPainter::CompositionMode_SourceOut},
    {"source-in",   QPainter::CompositionMode_SourceIn},
    {"source-atop", QPainter::CompositionMode_SourceAtop},
    {"destination-atop", QPainter::CompositionMode_DestinationAtop},
    {"destination-in",   QPainter::CompositionMode_DestinationIn},
    {"destination-out",  QPainter::CompositionMode_DestinationOut},
    {"destination-over", QPainter::CompositionMode_DestinationOver},
    {"lighter", QPainter::CompositionMode_Lighten},
    {"copy",    QPainter::CompositionMode_Source},
    {"xor",     QPainter::CompositionMode_Xor},
    {0, (QPainter::CompositionMode)0}
};

MAKE_TRANSLATOR(compModeTranslator, QString, QPainter::CompositionMode, const char*, compModeTranslatorTable)

DOM::DOMString CanvasContext2DImpl::globalCompositeOperation() const
{
    return compModeTranslator()->toLeft(activeState().globalCompositeOperation);
}

void CanvasContext2DImpl::setGlobalCompositeOperation(const DOM::DOMString& op)
{
    QString opStr = op.string();
    if (!compModeTranslator()->hasLeft(opStr))
        return; // Ignore unknown
    activeState().globalCompositeOperation = compModeTranslator()->toRight(opStr);
    dirty |= DrtCompOp;
}

// Colors and styles.
//

static QList<qreal> parseNumbersList(QString::const_iterator &itr)
{
    QList<qreal> points;
    QLocale c(QLocale::C);
    QString temp;
    bool percent = false;
    while ((*itr).isSpace())
        ++itr;
    while ((*itr).isNumber() ||
           (*itr) == '-' || (*itr) == '+' || (*itr) == '.') {
        temp = QString();

        if ((*itr) == '-')
            temp += *itr++;
        else if ((*itr) == '+')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if ((*itr) == '.')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        while ((*itr).isSpace())
            ++itr;
        if ((*itr) == '%')
            ++itr, percent = true;
        while ((*itr).isSpace())
            ++itr;
        if ((*itr) == ',')
            ++itr;
        points.append(c.toDouble(temp) * (percent ? 2.55 : 1));
        //eat spaces
        while ((*itr).isSpace())
            ++itr;
        percent = false;
    }

    return points;
}

static QColor colorFromString(DOM::DOMString domStr)
{
    QString name = domStr.string().trimmed().toLower();
    QString::const_iterator itr = name.constBegin();
    QList<qreal> compo;
    if ( name.startsWith( "rgba(" ) || name.startsWith( "hsva(" ) ) {
        ++itr; ++itr; ++itr; ++itr; ++itr;
        compo = parseNumbersList(itr);
        if ( compo.size() != 4 ) {
            return QColor();
        }
        return name.startsWith('h') ?
            QColor::fromHsvF( compo[0]/255, compo[1]/255, compo[2]/255, compo[3] ) :
            QColor::fromRgbF( compo[0]/255, compo[1]/255, compo[2]/255, compo[3] );
    } else if ( name.startsWith( "rgb(" ) ||  name.startsWith( "hsv(" ) ) {
        ++itr; ++itr; ++itr; ++itr;
        compo = parseNumbersList(itr);
        if ( compo.size() != 3 ) {
            return QColor();
        }
        return name.startsWith('h') ?
            QColor::fromHsv( compo[0], compo[1], compo[2] ) :
            QColor::fromRgb( compo[0], compo[1], compo[2] );
    } else {
        QRgb color;
        if (DOM::CSSParser::parseColor(name, color))
            return QColor(color);
        else
            return QColor();
    }
}

static DOMString colorToString(const QColor& color)
{
    QString str;
    if (color.alpha() == 255)
        str.sprintf("#%02x%02x%02x", color.red(), color.green(), color.blue());
    else
        str.sprintf("rgba(%d, %d, %d, %f)", color.red(), color.green(), color.blue(), color.alphaF());
        // ### CHECKME: risk of locale messing up . ?
    return str;
}

//-------

DOM::DOMString CanvasColorImpl::toString() const
{
    return colorToString(color);
}

CanvasColorImpl* CanvasColorImpl::fromString(const DOM::DOMString& str)
{
    QColor cl = colorFromString(str);
    if (!cl.isValid())
        return 0;
    return new CanvasColorImpl(cl);
}

//-------

CanvasGradientImpl::CanvasGradientImpl(QGradient* newGradient, float innerRadius, bool inverse)
    : gradient(newGradient), innerRadius(innerRadius), inverse(inverse)
{}


static qreal adjustPosition( qreal pos, const QGradientStops &stops )
{
    QGradientStops::const_iterator itr = stops.constBegin();
    const qreal smallDiff = 0.00001;
    while ( itr != stops.constEnd() ) {
        const QGradientStop &stop = *itr;
        ++itr;
        bool atEnd = ( itr != stops.constEnd() );
        if ( qFuzzyCompare( pos, stop.first ) ) {
            if ( atEnd || !qFuzzyCompare( pos + smallDiff, ( *itr ).first ) ) {
                return qMin(pos + smallDiff, qreal(1.0));
            }
        }
    }
    return pos;
}

void CanvasGradientImpl::addColorStop(float offset, const DOM::DOMString& color, int& exceptionCode)
{
    // ### we may have to handle the "currentColor" KW here. ouch.

    exceptionCode = 0;
    //### fuzzy compare (also for alpha)
    if (offset < 0 || offset > 1) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    QColor qcolor = colorFromString(color);
    if (!qcolor.isValid()) {
        exceptionCode = DOMException::SYNTAX_ERR;
        return;
    }

    // Adjust the position of the stop to emulate an inner radius.
    // If the inner radius is larger than the outer, we'll reverse
    // the position of the stop.
    if (gradient->type() == QGradient::RadialGradient) {
        if (inverse)
            offset = 1.0 - offset;

        offset = innerRadius + offset * (1.0 - innerRadius);
    }

    //<canvas> says that gradient can have two stops at the same position
    //Qt doesn't handle that. We hack around that by creating a fake position
    //stop.
    offset = adjustPosition(offset, gradient->stops());

    gradient->setColorAt(offset, qcolor);
}

CanvasGradientImpl::~CanvasGradientImpl()
{
    delete gradient;
}

QBrush CanvasGradientImpl::toBrush() const
{
    return QBrush(*gradient);
}

//-------

CanvasPatternImpl::CanvasPatternImpl(const QImage& inImg, bool rx, bool ry):
        img(inImg), repeatX(rx), repeatY(ry)
{}


QBrush CanvasPatternImpl::toBrush() const
{
    return QBrush(img);
}

QRectF CanvasPatternImpl::clipForRepeat(const QPointF &origin, const QRectF &fillBounds) const
{
    if (repeatX && repeatY)
        return QRectF();

    if (!repeatX && !repeatY)
        return QRectF(origin, img.size());

    if (repeatX)
        return QRectF(fillBounds.x(), origin.y(), fillBounds.width(), img.height());

    // repeatY
    return QRectF(origin.x(), fillBounds.y(), img.width(), fillBounds.height());
}

//-------

CanvasImageDataImpl::CanvasImageDataImpl(unsigned width, unsigned height) : data(width, height, QImage::Format_ARGB32_Premultiplied)
{}

unsigned CanvasImageDataImpl::width() const
{
    return data.width();
}

unsigned CanvasImageDataImpl::height() const
{
    return data.height();
}

static inline unsigned char unpremulComponent(unsigned original, unsigned alpha)
{
    unsigned char val =  alpha ? (unsigned char)(original * 255 / alpha) : 0;
    return val;
}

QColor CanvasImageDataImpl::pixel(unsigned pixelNum) const
{
    QRgb code = data.pixel(pixelNum % data.width(), pixelNum / data.width());
    unsigned char  a = qAlpha(code);
    return QColor(unpremulComponent(qRed(code),  a), unpremulComponent(qGreen(code), a),
                  unpremulComponent(qBlue(code), a), a);
}

static inline unsigned char premulComponent(unsigned original, unsigned alpha)
{
    unsigned product = original * alpha; // this is conceptually 255 * intended value.
    return (unsigned char)((product + product/256 + 128)/256);
}

void CanvasImageDataImpl::setPixel(unsigned pixelNum, const QColor& val)
{
    unsigned char a = val.alpha();
    QRgb code = qRgba(premulComponent(val.red(), a), premulComponent(val.green(), a),
                      premulComponent(val.blue(),a), a);
    data.setPixel(pixelNum % data.width(), pixelNum / data.width(), code);
}

//-------

void CanvasContext2DImpl::setStrokeStyle(CanvasStyleBaseImpl* strokeStyle)
{
    if (!strokeStyle)
        return;
    activeState().strokeStyle = strokeStyle;
    dirty |= DrtStroke;
}

CanvasStyleBaseImpl* CanvasContext2DImpl::strokeStyle() const
{
    return activeState().strokeStyle.get();
}

void CanvasContext2DImpl::setFillStyle(CanvasStyleBaseImpl* fillStyle)
{
    if (!fillStyle)
        return;
    activeState().fillStyle = fillStyle;
    dirty |= DrtFill;
}

CanvasStyleBaseImpl* CanvasContext2DImpl::fillStyle() const
{
    return activeState().fillStyle.get();
}

CanvasGradientImpl* CanvasContext2DImpl::createLinearGradient(float x0, float y0, float x1, float y1) const
{
    QLinearGradient* grad = new QLinearGradient(x0, y0, x1, y1);
    return new CanvasGradientImpl(grad);
}

CanvasGradientImpl* CanvasContext2DImpl::createRadialGradient(float x0, float y0, float r0,
                                                              float x1, float y1, float r1,
                                                              int& exceptionCode) const
{
    exceptionCode = 0;
    //### fuzzy
    if (r0 < 0.0f || r1 < 0.0f) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }

    QPointF center, focalPoint;
    float radius, innerRadius;
    bool inverse;

    // Use the larger of the two radii as the radius in the QGradient.
    // The gradient is always supposed to move from r0 to r1, so if r0 is
    // larger than r1, we'll use r0 as the radius and reverse the direction
    // of the gradient by inverting the positions of the color stops.
    // innerRadius is a percentage of the outer radius.
    if (r1 > r0) {
        center      = QPointF(x1, y1);
        focalPoint  = QPointF(x0, y0);
        radius      = r1;
        innerRadius = (r1 > 0.0f ? r0 / r1 : 0.0f);
        inverse     = false;
    } else {
        center      = QPointF(x0, y0);
        focalPoint  = QPointF(x1, y1);
        radius      = r0;
        innerRadius = (r0 > 0.0f ? r1 / r0 : 0.0f);
        inverse     = true;
    }

    QGradient *gradient = new QRadialGradient(center, radius, focalPoint);
    return new CanvasGradientImpl(gradient, innerRadius, inverse);
}

CanvasPatternImpl* CanvasContext2DImpl::createPattern(ElementImpl* pat, const DOMString& rpt,
                                                      int& exceptionCode) const
{
    exceptionCode = 0;

    // Decode repetition..
    bool repeatX;
    bool repeatY;

    if (rpt == "repeat" || rpt.isEmpty()) {
        repeatX = true;
        repeatY = true;
    } else if (rpt == "repeat-x") {
        repeatX = true;
        repeatY = false;
    } else if (rpt == "repeat-y") {
        repeatX = false;
        repeatY = true;
    } else if (rpt == "no-repeat") {
        repeatX = false;
        repeatY = false;
    } else {
        exceptionCode = DOMException::SYNTAX_ERR;
        return 0;
    }

    QImage pic = extractImage(pat, exceptionCode);
    if (exceptionCode)
        return 0;

    return new CanvasPatternImpl(pic, repeatX, repeatY);
}

// Pen style ops
//
float CanvasContext2DImpl::lineWidth() const
{
    return activeState().lineWidth;
}

void CanvasContext2DImpl::setLineWidth(float newLW)
{
    if (newLW <= 0.0)
        return;
    activeState().lineWidth = newLW;
    dirty |= DrtStroke;
}

static const IDTranslator<QString, Qt::PenCapStyle, const char*>::Info penCapTranslatorTable[] = {
    {"round", Qt::RoundCap},
    {"square", Qt::SquareCap},
    {"butt", Qt::FlatCap},
    {0, (Qt::PenCapStyle)0}
};

MAKE_TRANSLATOR(penCapTranslator, QString, Qt::PenCapStyle, const char*, penCapTranslatorTable)

DOMString CanvasContext2DImpl::lineCap() const
{
    return penCapTranslator()->toLeft(activeState().lineCap);
}

void CanvasContext2DImpl::setLineCap(const DOM::DOMString& cap)
{
    QString capStr = cap.string();
    if (!penCapTranslator()->hasLeft(capStr))
        return;
    activeState().lineCap = penCapTranslator()->toRight(capStr);
    dirty |= DrtStroke;
}

static const IDTranslator<QString, Qt::PenJoinStyle, const char*>::Info penJoinTranslatorTable[] = {
    {"round", Qt::RoundJoin},
    {"miter", Qt::MiterJoin},
    {"bevel", Qt::BevelJoin},
    {0, (Qt::PenJoinStyle)0}
};

MAKE_TRANSLATOR(penJoinTranslator, QString, Qt::PenJoinStyle, const char*, penJoinTranslatorTable)

DOMString CanvasContext2DImpl::lineJoin() const
{
    return penJoinTranslator()->toLeft(activeState().lineJoin);
}

void CanvasContext2DImpl::setLineJoin(const DOM::DOMString& join)
{
    QString joinStr = join.string();
    if (!penJoinTranslator()->hasLeft(joinStr))
        return;
    activeState().lineJoin = penJoinTranslator()->toRight(joinStr);
    dirty |= DrtStroke;
}

float CanvasContext2DImpl::miterLimit() const
{
    return activeState().miterLimit;
}

void CanvasContext2DImpl::setMiterLimit(float newML)
{
    if (newML <= 0.0)
        return;
    activeState().miterLimit = newML;
    dirty |= DrtStroke;
}

// Shadow settings
//
float CanvasContext2DImpl::shadowOffsetX() const
{
    return activeState().shadowOffsetX;
}

void  CanvasContext2DImpl::setShadowOffsetX(float newOX)
{
    activeState().shadowOffsetX = newOX;
}

float CanvasContext2DImpl::shadowOffsetY() const
{
    return activeState().shadowOffsetY;
}

void  CanvasContext2DImpl::setShadowOffsetY(float newOY)
{
    activeState().shadowOffsetY = newOY;
}

float CanvasContext2DImpl::shadowBlur() const
{
    return activeState().shadowBlur;
}

void  CanvasContext2DImpl::setShadowBlur(float newBlur)
{
    if (newBlur < 0)
        return;

    activeState().shadowBlur = newBlur;
}

DOMString CanvasContext2DImpl::shadowColor() const
{
    return colorToString(activeState().shadowColor);
}

void CanvasContext2DImpl::setShadowColor(const DOMString& newColor)
{
    // This not specified, it seems, but I presume setting
    // and invalid color does not change the state
    QColor cl = colorFromString(newColor);
    if (cl.isValid())
        activeState().shadowColor = cl;
}

// Rectangle ops
//
void CanvasContext2DImpl::clearRect (float x, float y, float w, float h, int& exceptionCode)
{
    exceptionCode = 0;
    if (w < 0.0f || h < 0.0f) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    QPainter* p = acquirePainter();
    p->setCompositionMode(QPainter::CompositionMode_Source);
    dirty |= DrtCompOp; // We messed it up..

    p->fillRect(QRectF(x, y, w, h), Qt::transparent);
}

void CanvasContext2DImpl::fillRect (float x, float y, float w, float h, int& exceptionCode)
{
    exceptionCode = 0;
    if (w < 0.0f || h < 0.0f) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    QPainter* p = acquirePainter();

    QPainterPath path;
    path.addPolygon(QRectF(x, y, w, h) * activeState().transform);
    path.closeSubpath();

    drawPath(p, path, FillPath);
}

void CanvasContext2DImpl::strokeRect (float x, float y, float w, float h, int& exceptionCode)
{
    exceptionCode = 0;
    if (w < 0.0f || h < 0.0f) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    QPainter* p = acquirePainter();

    QPainterPath path;
    path.addPolygon(QRectF(x, y, w, h) * activeState().transform);
    path.closeSubpath();

    drawPath(p, path, StrokePath);
}

inline bool CanvasContext2DImpl::isPathEmpty() const
{
    // For an explanation of this, see the comment in beginPath()
    const QPointF pos = currentPoint;
    return KJS::isInf(pos.x()) && KJS::isInf(pos.y());
}

// Path ops
//
void CanvasContext2DImpl::beginPath()
{
    path = QPainterPath();
    path.setFillRule(Qt::WindingFill);

    // QPainterPath always contains an initial MoveTo element to (0, 0), and there is
    // no way to tell that apart from an explicitly inserted MoveTo to that position.
    // This means that we have no reliable way of checking if the path is empty.
    // To work around this, we insert a MoveTo to (infinity, infinity) each time the
    // path is reset, and check the current position for this value in all functions
    // that are supposed to do nothing when the path is empty.
    firstPoint = currentPoint = QPointF(std::numeric_limits<qreal>::infinity(),
                                        std::numeric_limits<qreal>::infinity());
    path.moveTo(currentPoint);
}

void CanvasContext2DImpl::closePath()
{
    path.closeSubpath();
    currentPoint = firstPoint;
}

void CanvasContext2DImpl::moveTo(float x, float y)
{
    firstPoint = currentPoint = QPointF(x, y);
    path.moveTo(currentPoint * activeState().transform);
}

void CanvasContext2DImpl::lineTo(float x, float y)
{
    if (isPathEmpty())
        return;

    currentPoint = QPointF(x, y);
    path.lineTo(currentPoint * activeState().transform);
}

void CanvasContext2DImpl::quadraticCurveTo(float cpx, float cpy, float x, float y)
{
    if (isPathEmpty())
        return;

    const QTransform &xform = activeState().transform;
    currentPoint = QPointF(x, y);
    path.quadTo(QPointF(cpx, cpy) * xform, currentPoint * xform);
}

void CanvasContext2DImpl::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y)
{
    if (isPathEmpty())
        return;

    const QTransform &xform = activeState().transform;
    currentPoint = QPointF(x, y);
    path.cubicTo(QPointF(cp1x, cp1y) * xform, QPointF(cp2x, cp2y) * xform, currentPoint * xform);
}

void CanvasContext2DImpl::rect(float x, float y, float w, float h, int& exceptionCode)
{
    exceptionCode = 0;
    if (w < 0 || h < 0) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    path.addPolygon(QRectF(x, y, w, h) * activeState().transform);
    path.closeSubpath();

    firstPoint = currentPoint = QPointF(x, y);
}

inline bool CanvasContext2DImpl::needsShadow() const
{
    return activeState().shadowColor.alpha() > 0;
}

QRectF CanvasContext2DImpl::clipForRepeat(QPainter *p, PathPaintOp op) const
{
    const CanvasStyleBaseImpl *style = op == FillPath ?
                activeState().fillStyle.get() : activeState().strokeStyle.get();

    if (style->type() != CanvasStyleBaseImpl::Pattern)
        return QRectF();

    const CanvasPatternImpl *pattern = static_cast<const CanvasPatternImpl*>(style);
    QRectF fillBounds = activeState().transform.inverted().mapRect(QRectF(QPointF(), canvasImage->size()));
    return pattern->clipForRepeat(p->brushOrigin(), fillBounds);
}

void CanvasContext2DImpl::drawPath(QPainter *p, const QPainterPath &path, const PathPaintOp op) const
{
    const PaintState &state = activeState();
    QPainterPathStroker stroker;
    QPainterPath fillPath;
    QBrush brush;

    if (state.infinityTransform)
        return;

    switch (op)
    {
    case StrokePath:
        brush = p->pen().brush();
        stroker.setCapStyle(state.lineCap);
        stroker.setJoinStyle(state.lineJoin);
        stroker.setMiterLimit(state.miterLimit);
        stroker.setWidth(state.lineWidth);
        if (!state.transform.isIdentity() && state.transform.isInvertible())
            fillPath = stroker.createStroke(path * state.transform.inverted()) * state.transform;
        else
            fillPath = stroker.createStroke(path);
        break;

    case FillPath:
        brush = p->brush();
        fillPath = path;
        break;
    }

    brush.setTransform(state.transform);

    p->save();
    p->setPen(Qt::NoPen);
    p->setBrush(brush);

    if (needsShadow())
        drawPathWithShadow(p, fillPath, op);
    else
    {
        QRectF repeatClip = clipForRepeat(p, op);
        if (!repeatClip.isEmpty()) {
            QPainterPath clipPath;
            clipPath.addRect(repeatClip);
            p->setClipPath(clipPath * state.transform, Qt::IntersectClip);
        }
        p->drawPath(fillPath);
    }
    p->restore();
}

void CanvasContext2DImpl::drawPathWithShadow(QPainter *p, const QPainterPath &path, PathPaintOp op, PaintFlags flags) const
{
    const PaintState &state = activeState();
    float radius = shadowBlur();

    // This seems to produce a shadow that's a fairly close approximation
    // to the shadows rendered by CoreGraphics.
    if (radius > 7)
        radius = qMin(7 + std::pow(float(radius - 7.0), float(.7)), float(127.0));

    qreal xoffset = radius * 2;
    qreal yoffset = radius * 2;

    bool honorRepeat = !(flags & NotUsingCanvasPattern);
    QRectF repeatClip = clipForRepeat(p, op);
    QRect shapeBounds;

    if (honorRepeat && !repeatClip.isEmpty()) {
        QPainterPath clipPath;
        clipPath.addRect(repeatClip);
        shapeBounds = path.intersected(clipPath * state.transform).controlPointRect().toAlignedRect();
    } else
        shapeBounds = path.controlPointRect().toAlignedRect();

    QRect clipRect;
    if (state.clipping) {
        clipRect = state.clipPath.controlPointRect().toAlignedRect();
        clipRect &= QRect(QPoint(), canvasImage->size());
    } else
        clipRect = QRect(QPoint(), canvasImage->size());

    // We need the clip rect to be large enough so that items that are partially or
    // completely outside the canvas will still cast shadows into it when they should.
    clipRect.adjust(qMin(-shadowOffsetX(), float(0)), qMin(-shadowOffsetY(), float(0)),
                    qMax(-shadowOffsetX(), float(0)), qMax(-shadowOffsetY(), float(0)));
    clipRect.adjust(-xoffset, -yoffset, xoffset, yoffset);

    QRect shapeRect  = shapeBounds & clipRect;
    QRect shadowRect = shapeRect.translated(shadowOffsetX(), shadowOffsetY());
    shadowRect.adjust(-xoffset, -yoffset, xoffset, yoffset);

    QPainter painter;

    // Create the image for the original shape
    QImage shape(shapeRect.size(), QImage::Format_ARGB32_Premultiplied);
    shape.fill(0);

    // Draw the shape
    painter.begin(&shape);
    painter.setRenderHints(p->renderHints());
    painter.setBrushOrigin(p->brushOrigin());
    painter.setBrush(p->brush());
    painter.setPen(Qt::NoPen);
    painter.translate(-shapeRect.x(), -shapeRect.y());
    if (honorRepeat && !repeatClip.isEmpty())
        painter.setClipRect(repeatClip);
    painter.drawPath(path);
    painter.end();

    // Create the shadow image and draw the original image on it
    QImage shadow(shadowRect.size(), QImage::Format_ARGB32_Premultiplied);
    shadow.fill(0);

    painter.begin(&shadow);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(xoffset, yoffset, shape);
    painter.end();

    // Blur the alpha channel
    ImageFilter::shadowBlur(shadow, radius, state.shadowColor);

    // Draw the shadow on the canvas first, then composite the original image over it.
    p->drawImage(shadowRect.topLeft(), shadow);
    p->drawImage(shapeRect.topLeft(), shape);
}

void CanvasContext2DImpl::fill()
{
    QPainter* p = acquirePainter();
    drawPath(p, path, FillPath);
}

void CanvasContext2DImpl::stroke()
{
    QPainter* p = acquirePainter();
    drawPath(p, path, StrokePath);
}

void CanvasContext2DImpl::clip()
{
    PaintState& state = activeState();
    QPainterPath pathCopy = path;
    pathCopy.closeSubpath();

    //### evil workaround for apparent Qt bug(?)
/*    QTransform t;
    t.translate(-0.0001*cnt, cnt*0.0001);
    pathCopy = t.map(pathCopy);*/

    if (state.clipping)
        state.clipPath = state.clipPath.intersected(pathCopy);
    else
        state.clipPath = pathCopy;

    state.clipPath.setFillRule(Qt::WindingFill);
    state.clipping = true;
    dirty |= DrtClip;
}

bool CanvasContext2DImpl::isPointInPath(float x, float y) const
{
    return activeState().transform.map(path).contains(QPointF(x, y));
}

void CanvasContext2DImpl::arcTo(float x1, float y1, float x2, float y2, float radius, int& exceptionCode)
{
    exceptionCode = 0;

    if (radius <= 0) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    if (isPathEmpty())
        return;

    QLineF line1(QPointF(x1, y1), currentPoint);
    QLineF line2(QPointF(x1, y1), QPointF(x2, y2));

    // If the first line is a point, we'll do nothing.
    if (line1.p1() == line1.p2())
        return;

    // If the second line is a point, we'll add a line segment to (x1, y1).
    if (line2.p1() == line2.p2()) {
        currentPoint = QPointF(x1, y1);
        path.lineTo(currentPoint * activeState().transform);
        return;
    }

    float angle1 = std::atan2(line1.dy(), line1.dx());
    float angle2 = std::atan2(line2.dy(), line2.dx());

    // The smallest angle between the lines
    float theta = angle2 - angle1;
    if (theta < -M_PI)
        theta = (2 * M_PI + theta);
    else if (theta > M_PI)
        theta = -(2 * M_PI - theta);

    // If the angle between the lines is 180 degrees, the span of the arc becomes
    // zero, causing the tangent points to converge to the same point at (x1, y1).
    if (qFuzzyCompare(qAbs(theta), float(M_PI))) {
        currentPoint = QPointF(x1, y1);
        path.lineTo(currentPoint * activeState().transform);
        return;
    }

    // The length of the hypotenuse of the right triangle formed by the points
    // (x1, y1), the center point of the circle, and either of the two tangent points.
    float h = radius / std::sin(qAbs(theta / 2.0));

    // The distance from (x1, y1) to the tangent points on line1 and line2.
    float tDist = std::cos(theta / 2.0) * h;

    // As theta approaches 0, the distance to the two tangent points approach infinity.
    // If we exceeded the limit, draw a long line toward the first tangent point.
    // This matches CoreGraphics and Postscript behavior, but violates the HTML5 spec,
    // which says we should do nothing in this case.
    if (KJS::isInf(h) || KJS::isInf(tDist)) {
        currentPoint = QPointF(line1.p2().x() + std::cos(angle1) * 1e10,
                               line1.p2().y() + std::sin(angle1) * 1e10);
        path.lineTo(currentPoint * activeState().transform);
        return;
    }

    // The center point of the circle
    float angle = angle1 + theta / 2.0;
    QPointF centerPoint(x1 + std::cos(angle) * h, y1 + std::sin(angle) * h);

    // Note that we don't check if the lines are long enough for the circle to actually
    // tangent them; like CoreGraphics and Postscript, we treat the points as points on
    // two infinitely long lines that intersect one another at (x1, y1).
    float startAngle = theta < 0 ? angle1 + M_PI_2 : angle1 - M_PI_2;
    float endAngle   = theta < 0 ? angle2 - M_PI_2 : angle2 + M_PI_2;
    bool counterClockWise = theta > 0;

    int dummy; // Exception code from arc()
    arc(centerPoint.x(), centerPoint.y(), radius, startAngle, endAngle, counterClockWise, dummy);
}

void CanvasContext2DImpl::arc(float x, float y, float radius, float startAngle, float endAngle,
                              bool counterClockWise, int& exceptionCode)
{
    exceptionCode = 0;

    if (radius <= 0) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    QPainterPath arcPath;
    bool wasEmpty = isPathEmpty();

    if (wasEmpty) {
        QPointF initialPoint(x + std::cos(startAngle) * radius,
                             y + std::sin(startAngle) * radius);
        arcPath.moveTo(initialPoint);
    }

    const QRectF rect(x - radius, y - radius, radius * 2, radius * 2);
    float sweepLength = -degrees(endAngle - startAngle);
    startAngle = -degrees(startAngle);

    if (counterClockWise && (sweepLength < 0 || sweepLength > 360))
    {
        sweepLength = 360 + std::fmod(sweepLength, float(360.0));
        if (qFuzzyCompare(sweepLength, 0))
            sweepLength = 360;
    }
    else if (!counterClockWise && (sweepLength > 0 || sweepLength < -360))
    {
        sweepLength = -(360 - std::fmod(sweepLength, float(360.0)));
        if (qFuzzyCompare(sweepLength, 0))
            sweepLength = 360;
    }

    arcPath.arcTo(rect, startAngle, sweepLength);

    // When drawing the arc, Safari will loop around the circle several times if
    // the sweep length is greater than 360 degrees, leaving the current position
    // in the path at endAngle. QPainterPath::arcTo() will stop when it reaches
    // 360 degrees, thus leaving the current position at that point. To match
    // Safari behavior, we call QPainterPath::arcTo() twice in this case, to make
    // the arc continue to the intended end point. Adding a MoveTo element will
    // not suffice, since this will not produce correct results if additional
    // elements are added to the path before it is stroked or filled.
    if (sweepLength > 360.0 || sweepLength < -360.0)
    {
        if (sweepLength < 0) {
            sweepLength += 360.0;
            startAngle -= 360.0;
        } else {
            sweepLength -= 360.0;
            startAngle += 360.0;
        }

        arcPath.arcTo(rect, startAngle, sweepLength);
    }

    currentPoint = arcPath.currentPosition();
    arcPath = arcPath * activeState().transform;

    // Add the transformed arc to the path.
    // Unfortunately we can't use QPainterPath::addPath(), since it adds the
    // path as a closed sub path, and that's not what we want.
    for (int i = 0; i < arcPath.elementCount();)
    {
        QPainterPath::Element e = arcPath.elementAt(i++);
        switch (e.type)
        {
        case QPainterPath::MoveToElement:
            if (i != 1 || wasEmpty)
                path.moveTo(e);
            break;

        case QPainterPath::LineToElement:
            path.lineTo(e); break;

        default: { // QPainterPath::CurveToElement
            QPointF c2 = arcPath.elementAt(i++);
            QPointF p  = arcPath.elementAt(i++);
            path.cubicTo(e, c2, p);
        }
        }
    }
}

void CanvasContext2DImpl::drawImage(QPainter *p, const QRectF &dstRect, const QImage &image, const QRectF &srcRect) const
{
    if (activeState().infinityTransform)
        return;

    QPainterPath path;
    path.addRect(dstRect);
    path = path * activeState().transform;

    QTransform transform;
    transform.translate(dstRect.x(), dstRect.y());
    if (dstRect.size() != srcRect.size())
    {
        float xscale = dstRect.width()  / srcRect.width();
        float yscale = dstRect.height() / srcRect.height();
        transform.scale(xscale, yscale);
    }

    QBrush brush(srcRect == image.rect() ? image : image.copy(srcRect.toRect()));
    brush.setTransform(transform * activeState().transform);

    p->save();
    p->setBrush(brush);
    p->setPen(Qt::NoPen);
    if (needsShadow())
        drawPathWithShadow(p, path, FillPath, NotUsingCanvasPattern);
    else
        p->drawPath(path);
    p->restore();
}

// Image stuff
void CanvasContext2DImpl::drawImage(ElementImpl* image, float dx, float dy, int& exceptionCode)
{
    exceptionCode = 0;
    QImage img = extractImage(image, exceptionCode);
    if (exceptionCode)
        return;

    QPainter* p = acquirePainter();
    drawImage(p, QRectF(dx, dy, img.width(), img.height()), img, img.rect());
}

void CanvasContext2DImpl::drawImage(ElementImpl* image, float dx, float dy, float dw, float dh,
                                    int& exceptionCode)
{
    //### do we need DoS protection here?
    exceptionCode = 0;
    QImage img = extractImage(image, exceptionCode);
    if (exceptionCode)
        return;

    QPainter* p = acquirePainter();
    drawImage(p, QRectF(dx, dy, dw, dh), img, img.rect());
}

void CanvasContext2DImpl::drawImage(ElementImpl* image,
                                    float sx, float sy, float sw, float sh,
                                    float dx, float dy, float dw, float dh,
                                    int& exceptionCode)
{
    //### do we need DoS protection here?
    exceptionCode = 0;
    QImage img = extractImage(image, exceptionCode);
    if (exceptionCode)
        return;

    QPainter* p = acquirePainter();
    drawImage(p, QRectF(dx, dy, dw, dh), img, QRectF(sx, sy, sw, sh));
}

// Pixel stuff.
CanvasImageDataImpl* CanvasContext2DImpl::getImageData(float sx, float sy, float sw, float sh, int& exceptionCode)
{
    int w = qRound(sw);
    int h = qRound(sh);

    if (w <= 0 || h <= 0) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }

    if (!khtmlImLoad::ImageManager::isAcceptableSize(unsigned(w), unsigned(h))) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }

    int x = qRound(sx);
    int y = qRound(sy);

    CanvasImageDataImpl* id = new CanvasImageDataImpl(w, h);
    id->data.fill(Qt::transparent);

    // Clip the source rect again the viewport.

    QRect srcRect = QRect(x, y, w, h);
    QRect clpRect = srcRect & QRect(0, 0, canvasElement->width(), canvasElement->height());
    if (!clpRect.isEmpty()) {
        QPainter p(&id->data);
        p.setCompositionMode(QPainter::CompositionMode_Source);

        // Flush our data..
        if (workPainter.isActive())
            workPainter.end();

        // Copy it over..
        QImage* backBuffer = canvasImage->qimage();
        p.drawImage(clpRect.topLeft() - srcRect.topLeft(), *backBuffer, clpRect);
        p.end();
    }

    return id;
}

void CanvasContext2DImpl::putImageData(CanvasImageDataImpl* id, float dx, float dy, int& exceptionCode)
{
    if (!id) {
        exceptionCode = DOMException::TYPE_MISMATCH_ERR;
        return;
    }

    // Flush any previous changes
    if (workPainter.isActive())
        workPainter.end();

    // We use our own painter here since we should not be affected by clipping, etc.
    // Hence we need to mark ourselves dirty, too
    needRendererUpdate();
    QPainter p(canvasImage->qimage());
    int x = qRound(dx);
    int y = qRound(dy);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(x, y, id->data);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
