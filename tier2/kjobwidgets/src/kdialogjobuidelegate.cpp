/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006,2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdialogjobuidelegate.h"

#include <kjob.h>
#include <kjobwidgets.h>
#include <QWidget>

#include <config-kjobwidgets.h>
#if HAVE_X11
#include <QX11Info>
#endif

KDialogJobUiDelegate::KDialogJobUiDelegate()
    : d(0)
{
}

KDialogJobUiDelegate::~KDialogJobUiDelegate()
{
    //delete d;
}

bool KDialogJobUiDelegate::setJob(KJob *job)
{
    bool ret = KJobUiDelegate::setJob(job);
#if HAVE_X11
    if (ret) {
        unsigned long time = QX11Info::appUserTime();
        KJobWidgets::updateUserTimestamp(job, time);
    }
#endif
    return ret;
}

void KDialogJobUiDelegate::setWindow(QWidget *window)
{
    Q_ASSERT(job());
    KJobWidgets::setWindow(job(), window);
}

QWidget *KDialogJobUiDelegate::window() const
{
    Q_ASSERT(job());
    return KJobWidgets::window(job());
}

void KDialogJobUiDelegate::updateUserTimestamp( unsigned long time )
{
    KJobWidgets::updateUserTimestamp(job(), time);
}

unsigned long KDialogJobUiDelegate::userTimestamp() const
{
    return KJobWidgets::userTimestamp(job());
}

void KDialogJobUiDelegate::showErrorMessage()
{
    if ( job()->error() != KJob::KilledJobError )
    {
#pragma message("KDE5 TODO: Implement queueing here when jobs will get splitted from kdeui")
//         KMessageBox::queuedMessageBox(window(), KMessageBox::Error, job()->errorString());
    }
}

void KDialogJobUiDelegate::slotWarning(KJob* /*job*/, const QString &plain, const QString &/*rich*/)
{
    if (isAutoWarningHandlingEnabled())
    {
#pragma message("KDE5 TODO: Implement queueing here when jobs will get splitted from kdeui")
// 	KMessageBox::queuedMessageBox(window(), KMessageBox::Information, plain);
    }
}
