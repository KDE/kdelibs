/*
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright 2007 Casper Boemann <cbr@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "helper.h"

#include <kcolorutils.h>

#include <QtGui/QPainter>
#include <QtGui/QLinearGradient>

OxygenStyleHelper::OxygenStyleHelper(const QByteArray &componentName)
    : OxygenHelper(componentName)
{
}

TileSet *OxygenStyleHelper::slab(const QColor &surroundColor)
{
    quint64 key = (quint64(surroundColor.rgba()) << 32);
    TileSet *tileSet = m_slabCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(17, 17, QImage::Format_ARGB32);
        QGradientStops stops;
        QPainter p;

        tmpImg.fill(Qt::transparent);

        p.begin(&tmpImg);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(0.875, 1.0);
        QRadialGradient rg = QRadialGradient(8.5*1.1428, 8.5+1, 8.5, 8.5*1.1428, 8.5+3.2);//4.7);
        stops.clear();
        stops << QGradientStop( 0, QColor(0,0,0, 135) )
           << QGradientStop( 0.20, QColor(0,0,0, 75) )
          << QGradientStop( 0.65, QColor(0,0,0, 20) )
           << QGradientStop( 1, QColor(0,0,0, 0 ) );
        rg.setStops(stops);
        p.setBrush(rg);
        p.setClipRect(0,2,17,15);
        p.drawEllipse(QRectF(0,0, 17*1.1428, 17));
        p.resetTransform();

        // draw white edge
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setBrush(Qt::transparent);
        QLinearGradient lg(QPointF(0, 4.5),QPointF(0, 13));
        lg.setColorAt(0.0, KColorUtils::shade(surroundColor, 0.3));
        QColor tmpColor = surroundColor;
        tmpColor.setAlpha(0);
        lg.setColorAt(1.0, tmpColor);
        p.setPen(QPen(lg,1));
        p.drawEllipse(QRectF(4.5, 4.5, 8, 8));
        p.end();

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 8, 8, 1, 1);

        m_slabCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::slabFocused(const QColor &surroundColor, QColor glowColor)
{
    quint64 key = (quint64(surroundColor.rgba()) << 32) | quint64(glowColor.rgba());
    TileSet *tileSet = m_slabFocusedCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(17, 17, QImage::Format_ARGB32);
        QGradientStops stops;
        QPainter p;

        tmpImg.fill(Qt::transparent);

        TileSet *slabTileSet = slab(surroundColor);

        p.begin(&tmpImg);
        slabTileSet->render(QRect(0,0,17,17), &p);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        QRadialGradient rg = QRadialGradient(8.5, 8.5, 8.5, 8.5, 8.5);
        QColor tmpColor = glowColor;
        glowColor.setAlpha(0);
        rg.setColorAt(4.5/8.5 - 0.01, glowColor);
        glowColor.setAlpha(180);
        rg.setColorAt(4.5/8.5, glowColor);
        glowColor.setAlpha(0);
        rg.setColorAt(1.0, glowColor);
        p.setBrush(rg);
        p.drawEllipse(QRectF(0, 0, 17, 17));
        p.end();

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 8, 8, 1, 1);

        m_slabFocusedCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::slabSunken(const QColor &surroundColor)
{
    quint64 key = (quint64(surroundColor.rgba()) << 32);
    TileSet *tileSet = m_slabSunkenCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(17, 17, QImage::Format_ARGB32);
        QGradientStops stops;
        QPainter p;

        tmpImg.fill(Qt::transparent);

        p.begin(&tmpImg);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawEllipse(QRectF(4.5, 4.5, 8, 8));
        p.end();

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 8, 8, 1, 1);

        m_slabSunkenCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::slope(const QColor &surroundColor)
{
    quint64 key = (quint64(surroundColor.rgba()) << 32);
    TileSet *tileSet = m_slopeCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(24, 24, QImage::Format_ARGB32);
        QPainter p;

        tmpImg.fill(Qt::transparent);

        p.begin(&tmpImg);
        TileSet *slabTileSet = slab(surroundColor);

        for (int i=8;i<16;i++)
        {
            p.setOpacity((16-i)/8.0);
            slabTileSet->render(QRect(0,i,24,1), &p, TileSet::Left | TileSet::Right);
        }

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 8, 8, 8, 8);
        m_slopeCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::hole(const QColor &surroundColor)
{
    quint64 key = (quint64(surroundColor.rgba()) << 32);
    TileSet *tileSet = m_setCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(9, 9, QImage::Format_ARGB32);
        QLinearGradient lg; QGradientStops stops;
        QPainter p;

        tmpImg.fill(Qt::transparent);

        p.begin(&tmpImg);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(1.25, 1.0);
        QRadialGradient rg = QRadialGradient(4.5*0.8, 4.5, 5.0, 4.5*0.8, 4.5+1.3);
        stops.clear();
        stops << QGradientStop( 0.4, QColor(0,0,0, 0) )
           << QGradientStop( 0.58, QColor(0,0,0, 20) )
           << QGradientStop( 0.75, QColor(0,0,0, 53) )
           << QGradientStop( 0.88, QColor(0,0,0, 100) )
           << QGradientStop( 1, QColor(0,0,0, 150 ) );
        rg.setStops(stops);
        p.setBrush(rg);
        p.setClipRect(0,0,9,8);
        p.drawEllipse(QRectF(0,0, 9*0.8, 9));
        p.resetTransform();

        // draw white edge at bottom
        p.setClipRect(0,7,9,2);
        p.setBrush(Qt::NoBrush);
        p.setPen( KColorUtils::shade(surroundColor, 0.3));
        p.drawEllipse(QRectF(0.5, 0.5, 8, 8));
        p.setPen(Qt::NoPen);
        p.end();

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 4, 4, 1, 1);

        m_setCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::holeFocused(const QColor &surroundColor, QColor glowColor)
{
    // FIXME need to figure out to what extent we need to care about glowColor
    // for the key as well, might be enough just to stash the color set in the
    // key since glow color should only change when the system scheme is
    // changed by the user
    quint64 key = (quint64(surroundColor.rgba()) << 32) | quint64(glowColor.rgba());
    TileSet *tileSet = m_setCache.object(key);

    if (!tileSet)
    {
        QImage tmpImg(9, 9, QImage::Format_ARGB32);
        QLinearGradient lg; QGradientStops stops;
        QPainter p;

        tmpImg.fill(Qt::transparent);

        p.begin(&tmpImg);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(1.25, 1.0);
        QRadialGradient rg = QRadialGradient(4.5*0.8, 4.5, 5.0, 4.5*0.8, 4.5+1.3);
        stops.clear();
        stops << QGradientStop( 0.4, QColor(0,0,0, 0) )
           << QGradientStop( 0.58, QColor(0,0,0, 20) )
           << QGradientStop( 0.75, QColor(0,0,0, 53) )
           << QGradientStop( 0.88, QColor(0,0,0, 100) )
           << QGradientStop( 1, QColor(0,0,0, 150 ) );
        rg.setStops(stops);
        p.setBrush(rg);
        p.setClipRect(0,0,9,8);
        p.drawEllipse(QRectF(0,0, 9*0.8, 9));
        p.resetTransform();

        rg = QRadialGradient(4.5, 4.5, 5.0, 4.5, 4.5);
        stops.clear();
        glowColor.setAlpha(0);
        stops << QGradientStop(0, glowColor);
        glowColor.setAlpha(30);
        stops  << QGradientStop(0.40, glowColor);
        glowColor.setAlpha(110);
        stops  << QGradientStop(0.65, glowColor);
        glowColor.setAlpha(170);
        stops  << QGradientStop(0.75, glowColor);
        glowColor.setAlpha(0);
        stops  << QGradientStop(0.78, glowColor);
        rg.setStops(stops);
        p.setBrush(rg);
        p.setClipRect(0,0,9,9);
        p.drawEllipse(QRectF(0,0, 9, 9));

        // draw white edge at bottom
        p.setClipRect(0,7,9,2);
        p.setBrush(Qt::NoBrush);
        p.setPen( KColorUtils::shade(surroundColor, 0.3));
        p.drawRoundRect(QRectF(0.5, 0.5, 8, 8),90,90);
        p.setPen(Qt::NoPen);
        p.end();

        tileSet = new TileSet(QPixmap::fromImage(tmpImg), 4, 4, 1, 1);

        m_setCache.insert(key, tileSet);
    }
    return tileSet;
}

TileSet *OxygenStyleHelper::verticalScrollBar(const QColor &color, int width, int height, int offset)
{
    int offsetSize = 32; // hardcoded

    offset = offset % (offsetSize);

    if(height > 32)
        height = 64;

    quint64 key = (quint64(color.rgba()) << 32) | (width<<22) | (height<<10) | offset;
    TileSet *tileSet = m_verticalScrollBarCache.object(key);
    if (!tileSet)
    {
        QPixmap tmpPixmap(width, height);
        tmpPixmap.fill(Qt::transparent);

        QPainter p(&tmpPixmap);
        QRectF r(0, 0, width, height);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);

        QLinearGradient lg(QPointF(0, 0),QPointF(width*0.6, 0));
        lg.setColorAt(0, color.lighter(140));
        lg.setColorAt(1, color);
        p.setBrush(lg);
        p.drawRoundRect(r, 90*9/width,90*9/height);

        lg = QLinearGradient (QPointF(0, 0),QPointF(width, 0));
        QColor tmpColor(28,255,28);
        tmpColor.setAlpha(0);
        lg.setColorAt(0, tmpColor);
        tmpColor.setAlpha(128);
        lg.setColorAt(1, tmpColor);
        p.setBrush(lg);
        p.drawRoundRect(QRectF(0.48*width, 0, 0.52*width, height), int(90*9/(width*0.52)),90*9/height);

        //The rolling gradient
        // Since we want to tile every 32 pixels we need to compute a gradient that does that and
        // that also has a slope of 3/2 (the value the artist wants)
        lg = QLinearGradient(QPointF(0, -offset),QPointF(offsetSize*6.0/26, -offset+offsetSize*9.0/26));
        lg.setSpread(QGradient::ReflectSpread);
        tmpColor = color.darker(130);
        tmpColor.setAlpha(110);
        lg.setColorAt(0.0, tmpColor);
        tmpColor.setAlpha(30);
        lg.setColorAt(0.6, tmpColor);
        tmpColor.setAlpha(0);
        lg.setColorAt(1.0, tmpColor);
        p.setBrush(lg);
        p.drawRoundRect(r, 90*9/width, 90*9/height);

        //the outline needs to
        lg = QLinearGradient(QPointF(0, 0), QPointF(0,height));
        tmpColor = color.darker(130);
        tmpColor.setAlpha(255);
        lg.setColorAt(0.0, tmpColor);
        tmpColor.setAlpha(60);
        lg.setColorAt(0.15, tmpColor);
        tmpColor.setAlpha(0);
        lg.setColorAt(.25, tmpColor);
        lg.setColorAt(0.75, tmpColor);
        tmpColor.setAlpha(60);
        lg.setColorAt(0.85, tmpColor);
         tmpColor.setAlpha(255);
        lg.setColorAt(1.0, tmpColor);
        p.setBrush(lg);
        p.drawRoundRect(r, 90*9/width, 90*9/height);

        lg = QLinearGradient(QPointF(0, 0), QPointF(0,height));
        lg.setColorAt(0.0, QColor(255,255,255,180));
        lg.setColorAt(0.1, QColor(0,0,0,100));
        lg.setColorAt(0.9, QColor(0,0,0,100));
        lg.setColorAt(0.9, QColor(0,0,0,100));
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(lg,2));
        p.drawRoundRect(r.adjusted(0, 0.5, 0, -0.5), 90*9/width, 90*9/height);

        if(height < 32)
            tileSet = new TileSet(tmpPixmap, 1, 1, width-2, height-2);
        else
            tileSet = new TileSet(tmpPixmap, 1, 16, width-2, 32);

        m_verticalScrollBarCache.insert(key, tileSet);
    }
    return tileSet;
}
