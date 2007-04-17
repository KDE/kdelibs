/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef VIDEOWIDGET_P_H
#define VIDEOWIDGET_P_H

#include "videowidget.h"
#include "../abstractvideooutput_p.h"
#include <QHBoxLayout>
#include <QChildEvent>
#include <QCoreApplication>
#include <QPalette>
#include <QCloseEvent>
#include <QTimer>

namespace Phonon
{

class VideoWidgetPrivate : public Phonon::AbstractVideoOutputPrivate
{
    K_DECLARE_PUBLIC(VideoWidget)
    protected:
        virtual bool aboutToDeleteBackendObject();
        virtual void createBackendObject();
        void setupBackendObject();

        void _k_cursorTimeout();

        VideoWidgetPrivate(VideoWidget *parent)
            : layout(parent),
            aspectRatio(VideoWidget::AspectRatioAuto),
            scaleMode(VideoWidget::AddBarsScaleMode)
        {
            layout.setMargin(0);
            cursorTimer.setInterval(1000); // 1s timeout until the cursor disappears
            cursorTimer.setSingleShot(true);
        }

        QHBoxLayout layout;
        VideoWidget::AspectRatio aspectRatio;
        VideoWidget::ScaleMode scaleMode;
        QTimer cursorTimer;
        bool changeFlags;

    private:
        void init();
};

} // namespace Phonon

#endif // VIDEOWIDGET_P_H
// vim: sw=4 ts=4 tw=80
