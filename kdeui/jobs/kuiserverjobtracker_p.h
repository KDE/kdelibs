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

#ifndef KUISERVERJOBTRACKER_P_H
#define KUISERVERJOBTRACKER_P_H

#include <QMap>

#include <kglobal.h>

#include "uiserverinterface.h"

class KJob;

class KSharedUiServerProxy : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.UiServerCallbacks")
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

    KSharedUiServerProxy();
    ~KSharedUiServerProxy();

    org::kde::UiServer &uiserver();

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
     * @param errorCode  the error code of the job
     */
    void jobFinished(int progressId, int errorCode);

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
    org::kde::UiServer m_uiserver;
    QMap<int, KJob*> m_jobs;
};

#endif
