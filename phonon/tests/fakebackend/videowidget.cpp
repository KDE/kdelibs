/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "videowidget.h"
#include <QtCore/QEvent>
#include <QtGui/QPalette>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QBoxLayout>

namespace Phonon
{
namespace Fake
{

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent), overlay(0), m_brightness(0.), m_hue(0.), m_contrast(0.), m_saturation(0.)
{
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::blue);
    setPalette(p);
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setMinimumSize(100, 100);
}

Phonon::VideoWidget::AspectRatio VideoWidget::aspectRatio() const
{
    return m_aspectRatio;
}

void VideoWidget::setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio)
{
    m_aspectRatio = aspectRatio;
    m_videoSize = size();
    const int w = width();
    const int h = height();
    switch(m_aspectRatio)
    {
    case Phonon::VideoWidget::AspectRatioWidget:
        break;
    case Phonon::VideoWidget::AspectRatioAuto:
    case Phonon::VideoWidget::AspectRatio4_3:
        if (h * 4 / 3 < w)
            m_videoSize.setWidth(h * 4 / 3);
        else
            m_videoSize.setHeight(w * 3 / 4);
        break;
//X     case Phonon::VideoWidget::AspectRatioSquare:
//X         if (h < w)
//X             m_videoSize.setWidth(h);
//X         else
//X             m_videoSize.setHeight(w);
//X         break;
    case Phonon::VideoWidget::AspectRatio16_9:
        if (h * 16 / 9 < w)
            m_videoSize.setWidth(h * 16 / 9);
        else
            m_videoSize.setHeight(w * 9 / 16);
        break;
//X     case Phonon::VideoWidget::AspectRatioDvb:
//X         if (h * 211 / 100 < w)
//X             m_videoSize.setWidth(h * 211 / 100);
//X         else
//X             m_videoSize.setHeight(w * 100 / 211);
//X         break;
    default:
        Q_ASSERT(false); // this may not happen
    }
}

Phonon::VideoWidget::ScaleMode VideoWidget::scaleMode() const
{
    return m_scaleMode;
}

void VideoWidget::setScaleMode(Phonon::VideoWidget::ScaleMode scaleMode)
{
    m_scaleMode = scaleMode;
}

qreal VideoWidget::brightness() const
{
    return m_brightness;
}

void VideoWidget::setBrightness(qreal b)
{
    m_brightness = b;
}

qreal VideoWidget::contrast() const
{
    return m_contrast;
}

void VideoWidget::setContrast(qreal c)
{
    m_contrast = c;
}

qreal VideoWidget::hue() const
{
    return m_hue;
}

void VideoWidget::setHue(qreal h)
{
    m_hue = h;
}

qreal VideoWidget::saturation() const
{
    return m_saturation;
}

void VideoWidget::setSaturation(qreal s)
{
    m_saturation = s;
}

void VideoWidget::processFrame(Phonon::Experimental::VideoFrame &frame)
{
    switch(frame.colorspace)
    {
    case Phonon::Experimental::VideoFrame::Format_RGBA8:
        {
            QImage image(reinterpret_cast<uchar *>(frame.data.data()), frame.width, frame.height,
                         QImage::Format_RGB32);
            image = image.scaled(m_videoSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
            m_pixmap = QPixmap::fromImage(image);
            repaint();
        }
        break;
    default:
//        kError(604) << "video frame format not implemented" << endl;
        break;
    }
}

//X int VideoWidget::overlayCapabilities() const
//X {
//X     return Experimental::OverlayApi::OverlayFull;
//X }
//X 
//X bool VideoWidget::createOverlay(QWidget *widget, int type)
//X {
//X     if ((overlay != 0) || (type != Experimental::OverlayApi::OverlayFull)) {
//X         return false;
//X     }
//X 
//X     if (layout() == 0) {
//X         QLayout *layout = new QVBoxLayout(this);
//X         layout->setMargin(0);
//X         setLayout(layout);
//X     }
//X 
//X     layout()->addWidget(widget);
//X     overlay = widget;
//X 
//X     return true;
//X }

void VideoWidget::childEvent(QChildEvent *event)
{
    if (event->removed() && (event->child() == overlay))
        overlay = 0;
    QWidget::childEvent(event);
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_pixmap);
}

void VideoWidget::resizeEvent(QResizeEvent *ev)
{
    setAspectRatio(m_aspectRatio);
    QWidget::resizeEvent(ev);
}

}} //namespace Phonon::Fake

#include "moc_videowidget.cpp"
// vim: sw=4 ts=4
