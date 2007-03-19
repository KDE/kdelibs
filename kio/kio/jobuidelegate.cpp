/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "jobuidelegate.h"

#include <kmessagebox.h>
#include <kjob.h>
#include <QPointer>
#include <QWidget>

#include "kio/scheduler.h"

#if defined Q_WS_X11
#include <QX11Info>
#include <netwm.h>
#endif

class KIO::JobUiDelegate::Private
{
public:
    Private() : showProgressInfo(true),
                progressId(0), userTimestamp(0) { }

    bool showProgressInfo;
    int progressId;
    QPointer<QWidget> errorParentWidget;
    unsigned long userTimestamp;
};

KIO::JobUiDelegate::JobUiDelegate( bool showProgressInfo )
    : d(new Private())
{
    d->showProgressInfo = showProgressInfo;
    d->errorParentWidget = 0L;
#if defined Q_WS_X11
    d->userTimestamp = QX11Info::appUserTime();
#endif
}

KIO::JobUiDelegate::~JobUiDelegate()
{
    delete d;
}

void KIO::JobUiDelegate::setWindow(QWidget *window)
{
    d->errorParentWidget = window;
    KIO::Scheduler::registerWindow(window);
}

QWidget *KIO::JobUiDelegate::window() const
{
    return d->errorParentWidget;
}

void KIO::JobUiDelegate::updateUserTimestamp( unsigned long time )
{
#if defined Q_WS_X11
  if( d->userTimestamp == 0 || NET::timestampCompare( time, d->userTimestamp ) > 0 )
      d->userTimestamp = time;
#endif
}

unsigned long KIO::JobUiDelegate::userTimestamp() const
{
    return d->userTimestamp;
}

void KIO::JobUiDelegate::connectJob( KJob *job )
{
    connect( job, SIGNAL( finished(KJob*) ),
             this, SLOT( slotFinished( KJob* ) ) );
    connect( job, SIGNAL( warning( KJob*, const QString& ) ),
             this, SLOT( slotWarning( KJob*, const QString& ) ) );
}

void KIO::JobUiDelegate::showErrorMessage()
{
    if ( job()->error() != KJob::KilledJobError )
    {
        KMessageBox::queuedMessageBox( d->errorParentWidget, KMessageBox::Error, job()->errorString() );
    }
}

void KIO::JobUiDelegate::slotFinished( KJob * /*job*/ )
{
    if ( job()->error() && isAutoErrorHandlingEnabled() )
        showErrorMessage();
}

void KIO::JobUiDelegate::slotWarning( KJob * /*job*/, const QString &errorText )
{
    if (isAutoWarningHandlingEnabled())
    {
        static uint msgBoxDisplayed = 0;
        if ( msgBoxDisplayed == 0 ) // don't bomb the user with message boxes, only one at a time
        {
            msgBoxDisplayed++;
            KMessageBox::information( d->errorParentWidget, errorText );
            msgBoxDisplayed--;
        }
        // otherwise just discard it.
    }
}

KIO::RenameDialog_Result KIO::JobUiDelegate::askFileRename(KJob * job,
                                                           const QString & caption,
                                                           const QString& src,
                                                           const QString & dest,
                                                           KIO::RenameDialog_Mode mode,
                                                           QString& newDest,
                                                           KIO::filesize_t sizeSrc,
                                                           KIO::filesize_t sizeDest,
                                                           time_t ctimeSrc,
                                                           time_t ctimeDest,
                                                           time_t mtimeSrc,
                                                           time_t mtimeDest)
{
    kDebug() << "Observer::open_RenameDialog job=" << job << endl;
    if (job)
        kDebug() << "                        progressId=" << d->progressId << endl;
    // We now do it in process => KDE4: move this code out of Observer (back to job.cpp), so that
    // opening the rename dialog doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
    RenameDialog_Result res = KIO::open_RenameDialog(caption, src, dest, mode,
                                                     newDest, sizeSrc, sizeDest,
                                                     ctimeSrc, ctimeDest, mtimeSrc,
                                                     mtimeDest);
    return res;
}

KIO::SkipDialog_Result KIO::JobUiDelegate::askSkip(KJob * job,
                                              bool multi,
                                              const QString & error_text)
{
    // We now do it in process. So this method is a useless wrapper around KIO::open_RenameDialog.
    SkipDialog_Result res = KIO::open_SkipDialog(multi, error_text);
    return res;
}

#include "jobuidelegate.moc"
