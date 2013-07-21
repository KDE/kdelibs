/* This file is part of the KDE libraries
    Copyright (C) 2013 Dawit Alemayehu <adawit@kde.org>

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

#include "clipboardupdater_p.h"
#include "jobclasses.h"
#include "copyjob.h"

#include <QApplication>
#include <QMimeSource>
#include <QClipboard>

using namespace KIO;

ClipboardUpdater::ClipboardUpdater(Job* job, UpdateMode mode)
    :QObject(job),
     m_updateMode(mode)
{
    Q_ASSERT(job);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
}

static void overwriteClipboardContent(KJob* job)
{
    QApplication::clipboard()->clear();
    KUrl::List newUrls;

    CopyJob* copyJob = qobject_cast<CopyJob*>(job);
    FileCopyJob* fileCopyJob = qobject_cast<FileCopyJob*>(job);
    if (copyJob) {
        Q_FOREACH(const KUrl& url, copyJob->srcUrls()) {
            KUrl dUrl = copyJob->destUrl();
            dUrl.addPath(url.fileName());
            newUrls.append(dUrl);
        }
    } else if (fileCopyJob) {
        newUrls << fileCopyJob->destUrl();
    }

    QMimeData* mime = new QMimeData();
    newUrls.populateMimeData(mime);
    QApplication::clipboard()->setMimeData(mime);
}

static void updateClipboardContent(KJob* job)
{
    QClipboard* clipboard = QApplication::clipboard();
    KUrl::List clipboardUrls = KUrl::List::fromMimeData( clipboard->mimeData());
    bool update = false;

    CopyJob* copyJob = qobject_cast<CopyJob*>(job);
    FileCopyJob* fileCopyJob = qobject_cast<FileCopyJob*>(job);
    if (copyJob) {
        Q_FOREACH(const KUrl& url, copyJob->srcUrls()) {
            const int index = clipboardUrls.indexOf(url);
            if (index > 0) {
                KUrl dUrl = copyJob->destUrl();
                dUrl.addPath(url.fileName());
                clipboardUrls.replace(index, dUrl);
                update = true;
            }
        }
    } else if (fileCopyJob) {
        const int index = clipboardUrls.indexOf(fileCopyJob->srcUrl());
        if (index > 0) {
            clipboardUrls.replace(index, fileCopyJob->destUrl());
            update = true;
        }
    }

    if (update) {
        QMimeData* mime = new QMimeData();
        clipboardUrls.populateMimeData(mime);
        QApplication::clipboard()->setMimeData(mime);
    }
}

void ClipboardUpdater::slotResult(KJob* job)
{
    if (job->error()) {
        return;
    }

    switch (m_updateMode) {
    case UpdateContent:
        updateClipboardContent(job);
        break;
    case OverwriteContent:
        overwriteClipboardContent(job);
        break;
    }
}
