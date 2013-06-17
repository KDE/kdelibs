/* This file is part of the KDE libraries
 *  Copyright (c) 2013 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kjobwidgets.h"
#include <kjob.h>
#include <QVariant>
#include <QWidget>
#include <QWindow>

void KJobWidgets::setWindow(KJob *job, QWidget *widget)
{
    job->setProperty("widget", QVariant::fromValue(widget));
    QWindow *window = widget ? widget->windowHandle() : NULL;
    job->setProperty("window", QVariant::fromValue(window));
    if (window)
        job->setProperty("window-id", QVariant::fromValue(qptrdiff(window->winId())));
}

QWidget* KJobWidgets::window(KJob *job)
{
    return job->property("widget").value<QWidget *>();
}

#if 0
void KJobWidgets::setWindow(KJob *job, QWindow *window)
{
    job->setProperty("window", QVariant::fromValue(window));
}

QWindow* KJobWidgets::window(KJob *job) // ### will have to be in a different namespace
{
    return job->property("window").value<QWindow *>();
}
#endif

// duplicated from kwindowsystem
static int timestampCompare( unsigned long time1_, unsigned long time2_ ) // like strcmp()
{
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    if (time1 == time2)
        return 0;
    return quint32(time1 - time2) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
}

void KJobWidgets::updateUserTimestamp(KJob *job, unsigned long time)
{
    unsigned long currentTimestamp = userTimestamp(job);
    if (currentTimestamp == 0 || timestampCompare(time, currentTimestamp) > 0) {
        job->setProperty("userTimestamp", qulonglong(time));
    }
}

unsigned long KJobWidgets::userTimestamp(KJob *job)
{
    return job->property("userTimestamp").toULongLong();
}

