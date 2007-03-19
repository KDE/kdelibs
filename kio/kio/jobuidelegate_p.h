/* This file is part of the KDE libraries
    Copyright (C) 2007-2006 Rafael Fernández López <ereslibre@gmail.com>
                            Kevin Ottens <ervin@kde.org>
    Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                       David Faure <faure@kde.org>
                       Stephan Kulow <coolo@kde.org>

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

#ifndef KIO_JOBUIDELEGATE_P_H
#define KIO_JOBUIDELEGATE_P_H

#include <QPointer>
#include <QWidget>
#include <QMap>

#include <kglobal.h>

#include "uiserveriface.h"

class KJob;
class ObserverAdaptor;

namespace KIO
{

class JobUiDelegatePrivate
{
public:
    JobUiDelegatePrivate() : showProgressInfo(true),
                             progressId(0), userTimestamp(0) { }

    bool showProgressInfo;
    int progressId;
    QPointer<QWidget> errorParentWidget;
    unsigned long userTimestamp;
};

class SharedUiDelegateProxy : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KIO.Observer")
    Q_ENUMS(JobVisibility StandardActions)

public:
    enum JobVisibility
    {
        JobHidden = 0,
        JobShown = 1
    };

    enum StandardActions
    {
        ActionPause,
        ActionResume,
        ActionCancel
    };

    SharedUiDelegateProxy();
    ~SharedUiDelegateProxy();

    org::kde::KIO::UIServer &uiserver();

    /**
      * Called by the job constructor, to signal its presence to the
      * UI Server
      *
      * @param job              the new job
      * @param visibility       whether the job is shown or not
      * @param icon             the icon to be shown, if not specified, the default icon
      *                         of the app that launched the job will be loaded
      * @return                 the progress ID assigned by the UI Server to the Job
      *
      * @warning                Will return 0 if @p job is null
      *
      * @note                   If successful will be called automatically job->setProgressId(newJobId);
      *                         where newJobId is the return value of this method
      */
    int newJob(KJob *job, JobVisibility visibility = JobShown, const QString &icon = QString());

    /**
      * Called by the job destructor, to tell the UI Server that
      * the job ended
      *
      * @param progressId the progress ID of the job, as returned by newJob()
      */
    void jobFinished(int progressId);

public Q_SLOTS:
    /**
      * Called by the kuiserver when an action was performed
      *
      * @param actionId the action identification number
      * @param jobId the job to which the action belongs to
      */
    void slotActionPerformed(int actionId, int jobId);

Q_SIGNALS:
    void actionPerformed(KJob *job, int actionId);

private:
    org::kde::KIO::UIServer m_uiserver;
    QMap<int, KJob*> m_dctJobs;
    ObserverAdaptor *observerAdaptor;
};
}

#endif
