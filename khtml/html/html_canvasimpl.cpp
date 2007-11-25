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
#include <imload/canvasimage.h>
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
#include <climits>

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
    defaultState.clipPath.addRect(0, 0, width, height);
    defaultState.clipPath.setFillRule(Qt::WindingFill);
    
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
        dirty = DrtAll;
    }

    PaintState& state = activeState();

    if (dirty & DrtClip) {
        // Reset the transform so the clip path is in the
        // right coordinate system
        workPainter.resetTransform();
        workPainter.setClipPath(state.clipPath);

        // Restore the transform..
        dirty |= DrtTransform;
    }
    
    if (dirty & DrtTransform) {
        if (state.infinityTransform) {
            // Make stuff disappear. Cliprect should avoid this ever showing..
            state.transform.reset();
            state.transform.translate(-canvasImage->size().width() * 2, -canvasImage->size().height() * 2);
        }

        workPainter.setTransform(state.transform);
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
    {"darker",  QPainter::CompositionMode_Darken},
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
                return qMin(pos + smallDiff, 1.0);
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
    //### how do we do repetition?
    return QBrush(img);
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
    // r0 is supposed to be the inner radius, but if r0 is larger than r1,
    // we'll use r0 as the radius and invert the positions of the color stops.
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
    p->fillRect(QRectF(x, y, w, h), p->brush());
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
    path.addRect(x, y, w, h);
    p->strokePath(path, p->pen());
}

// Path ops
//
void CanvasContext2DImpl::beginPath()
{
    path = QPainterPath();
    path.setFillRule(Qt::WindingFill);

    // Unfortunately QPainterPath always contains an intial MoveTo element to (0, 0),
    // so we have no reliable way to check if the path is empty. We'll therefor
    // insert a MoveTo to (-INT_MAX, -INT_MAX) each time the path is reset, and check
    // the current position for this value in all functions that are supposed to do
    // nothing when the path is empty.
    path.moveTo(-INT_MAX, -INT_MAX);
}

void CanvasContext2DImpl::closePath()
{
    path.closeSubpath();
}

void CanvasContext2DImpl::moveTo(float x, float y)
{
    path.moveTo(x, y);
}

void CanvasContext2DImpl::lineTo(float x, float y)
{
    if (path.currentPosition() != QPointF(-INT_MAX, -INT_MAX))
        path.lineTo(x, y);
}

void CanvasContext2DImpl::quadraticCurveTo(float cpx, float cpy, float x, float y)
{
    if (path.currentPosition() != QPointF(-INT_MAX, -INT_MAX))
        path.quadTo(cpx, cpy, x, y);
}

void CanvasContext2DImpl::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y)
{
    if (path.currentPosition() != QPointF(-INT_MAX, -INT_MAX))
        path.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
}

void CanvasContext2DImpl::rect(float x, float y, float w, float h, int& exceptionCode)
{
    exceptionCode = 0;
    if (w < 0 || h < 0) {
        exceptionCode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    path.addRect(x, y, w, h);
}

void CanvasContext2DImpl::fill()
{
    QPainter* p = acquirePainter();
    p->fillPath(path, p->brush());
}

void CanvasContext2DImpl::stroke()
{
    QPainter* p = acquirePainter();
    p->strokePath(path, p->pen());
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
    
    state.clipPath = state.clipPath.intersected(state.transform.map(pathCopy));
    state.clipPath.setFillRule(Qt::WindingFill);
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

    if (path.currentPosition() == QPointF(-INT_MAX, -INT_MAX))
        return;

    QLineF line1(QPointF(x1, y1), path.currentPosition());
    QLineF line2(QPointF(x1, y1), QPointF(x2, y2));

    // If the first line is a point, we'll do nothing.
    if (line1.p1() == line1.p2())
        return;

    // If the second line is a point, we'll add a line segment to (x1, y1).
    if (line2.p1() == line2.p2())
    {
        path.lineTo(x1, y1);
        return;
    }

    float angle1 = M_PI_2 - std::atan2(line1.dx(), line1.dy());
    float angle2 = M_PI_2 - std::atan2(line2.dx(), line2.dy());

    // The angle between by line1 and line2
    float span = angle2 - angle1;
    if (span < -M_PI)
        span = (2 * M_PI + span);
    else if (span > M_PI)
        span = -(2 * M_PI - span);

    // If the angle between the lines is 180 degrees, we'll just add a line
    // segment to (x1, y1).
    if (qFuzzyCompare(qAbs(span), float(M_PI)))
    {
        path.lineTo(x1, y1);
        return;
    }

    // If the angle between the lines is 0 degrees, we'll add an infinitely
    // long line segment from the current position in the direction of line2.
    // This is to match Safari behavior.
    if (qFuzzyCompare(span, float(0.0)))
    {
        // ### We'll define infinity as 100,000 coordinate space units from
        //     the current position for now.
        QLineF line(path.currentPosition(),
                    path.currentPosition() + QPointF(line2.dx(), line2.dy()));
        line.setLength(100000.0);
        path.lineTo(line.p2());
        return;
    }

    // The angle to the center of the circle
    float angle = angle1 + span / 2.0;

    // The length of the hypotenuse of the right triangle formed by the points
    // (x1, y1), the center point of the circle, and one of the tangent points.
    float h = radius / std::sin(qAbs(angle1 - angle));

    // The distance from (x1, y1) to the tangent points on line1 and line2.
    float tDist = std::cos(qAbs(angle1 -angle)) * h;

    // QLineF::length() uses sqrt() to compute the length, so we'll save the results
    // since we need them twice.
    float line1Len = line1.length();
    float line2Len = line2.length();

    // If either of the lines is too short, we can't do it
    if (line1Len < tDist || (line2Len + .01) < tDist)
    {
       path.lineTo(x1, y1);
       return;
    }

    // The center point of the circle
    QPointF centerPoint(x1 + std::cos(angle) * h, y1 + std::sin(angle) * h);

    // The tangent points on line1 and line2
    QPointF t1 = line1.pointAt(tDist / line1Len);
    QPointF t2 = line2.pointAt(tDist / line2Len);

    // The lines from the center point of the circle to the tangent points on line1 and line2.
    QLineF toT1(centerPoint, t1);
    QLineF toT2(centerPoint, t2);

    // The start and end angles of the arc
    float startAngle = M_PI_2 - std::atan2(toT1.dx(), toT1.dy());
    float endAngle   = M_PI_2 - std::atan2(toT2.dx(), toT2.dy());
    bool counterClockWise = span > 0;

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

    if (path.currentPosition() == QPointF(-INT_MAX, -INT_MAX))
    {
        QPointF initialPoint(x + std::cos(startAngle) * radius,
                             y + std::sin(startAngle) * radius);
        path.moveTo(initialPoint);
    }

    path.arcTo(rect, startAngle, sweepLength);

    // When drawing the arc, Safari will loop around the circle several times if the
    // sweep length is greater than 360 degrees, leaving the current position in
    // the path at endAngle. QPainterPath::arcTo() will stop when it reaches 360
    // degrees, leaving the current position at that point. To match Safari behavior,
    // we insert an extra MoveTo element in the path, to make sure that the current
    // position is where the script expects it to be after the arc is completed.
    if (sweepLength > 360.0 || sweepLength < -360.0)
    {
        QPointF endPoint(x + std::cos(endAngle) * radius,
                         y + std::sin(endAngle) * radius);
        if (path.currentPosition() != endPoint)
            path.moveTo(endPoint);
    }
}

// Image stuff
void CanvasContext2DImpl::drawImage(ElementImpl* image, float dx, float dy, int& exceptionCode)
{
    exceptionCode = 0;
    QImage img = extractImage(image, exceptionCode);
    if (exceptionCode)
        return;

    QPainter* p = acquirePainter();
    p->drawImage(QPointF(dx, dy), img);
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
    p->drawImage(QRectF(dx, dy, dw, dh), img);
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
    p->drawImage(QRectF(dx, dy, dw, dh), img, QRectF(sx, sy, sw, sh));
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
