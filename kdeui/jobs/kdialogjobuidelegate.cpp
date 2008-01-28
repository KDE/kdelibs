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

#include <kmessagebox.h>
#include <kjob.h>
#include <QPointer>
#include <QWidget>

#if defined Q_WS_X11
#include <QX11Info>
#include <netwm.h>
#endif

class KDialogJobUiDelegate::Private
{
public:
    Private() : userTimestamp(0) { }

    QPointer<QWidget> errorParentWidget;
    unsigned long userTimestamp;
};

KDialogJobUiDelegate::KDialogJobUiDelegate()
    : d(new Private())
{
    d->errorParentWidget = 0L;
#if defined Q_WS_X11
    d->userTimestamp = QX11Info::appUserTime();
#endif
}

KDialogJobUiDelegate::~KDialogJobUiDelegate()
{
    delete d;
}

void KDialogJobUiDelegate::setWindow(QWidget *window)
{
    d->errorParentWidget = window;
}

QWidget *KDialogJobUiDelegate::window() const
{
    return d->errorParentWidget;
}

void KDialogJobUiDelegate::updateUserTimestamp( unsigned long time )
{
#if defined Q_WS_X11
  if( d->userTimestamp == 0 || NET::timestampCompare( time, d->userTimestamp ) > 0 )
      d->userTimestamp = time;
#endif
}

unsigned long KDialogJobUiDelegate::userTimestamp() const
{
    return d->userTimestamp;
}

void KDialogJobUiDelegate::showErrorMessage()
{
    if ( job()->error() != KJob::KilledJobError )
    {
        KMessageBox::queuedMessageBox( d->errorParentWidget, KMessageBox::Error, job()->errorString() );
    }
}

void KDialogJobUiDelegate::slotWarning(KJob* /*job*/, const QString &plain, const QString &/*rich*/)
{
    if (isAutoWarningHandlingEnabled())
    {
        static uint msgBoxDisplayed = 0;
        if ( msgBoxDisplayed == 0 ) // don't bomb the user with message boxes, only one at a time
        {
            msgBoxDisplayed++;
            KMessageBox::information( d->errorParentWidget, plain );
            msgBoxDisplayed--;
        }
        // otherwise just discard it.
    }
}

#include "kdialogjobuidelegate.moc"
