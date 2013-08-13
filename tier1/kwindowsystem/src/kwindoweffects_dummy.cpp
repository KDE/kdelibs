/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kwindoweffects.h"

namespace KWindowEffects
{

bool isEffectAvailable(Effect effect)
{
    Q_UNUSED(effect)
    return false;
}

void slideWindow(WId id, SlideFromLocation location, int offset)
{
    Q_UNUSED(id)
    Q_UNUSED(location)
    Q_UNUSED(offset)
}

void slideWindow(QWidget *widget, SlideFromLocation location)
{
    Q_UNUSED(widget)
    Q_UNUSED(location)
}

QList<QSize> windowSizes(const QList<WId> &ids)
{
    QList<QSize> windowSizes;
    Q_FOREACH (WId id, ids) {
        windowSizes.append(QSize());
    }
    return windowSizes;
}

void showWindowThumbnails(WId parent, const QList<WId> &windows, const QList<QRect> &rects)
{
    Q_UNUSED(parent)
    Q_UNUSED(windows)
    Q_UNUSED(rects)
}

void presentWindows(WId controller, const QList<WId> &ids)
{
    Q_UNUSED(controller)
    Q_UNUSED(ids)
}

void presentWindows(WId controller, int desktop)
{
    Q_UNUSED(controller)
    Q_UNUSED(desktop)
}

void highlightWindows(WId controller, const QList<WId> &ids)
{
    Q_UNUSED(controller)
    Q_UNUSED(ids)
}

void enableBlurBehind(WId window, bool enable, const QRegion &region)
{
    Q_UNUSED(window)
    Q_UNUSED(enable)
    Q_UNUSED(region)
}

void markAsDashboard(WId window)
{
    Q_UNUSED(window)
}

}

