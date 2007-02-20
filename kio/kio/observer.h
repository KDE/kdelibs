/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
  * Copyright (C) 2000 Matej Koss <koss@miesto.sk>
  *                    David Faure <faure@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef __kio_observer_h__
#define __kio_observer_h__

#include <qobject.h>
#include <qmap.h>

#include <kio/global.h>
#include <kio/authinfo.h>
#include "kio/job.h"
#include "kio/skipdialog.h"
#include "kio/renamedialog.h"

class OrgKdeKIOUIServerInterface;
class ObserverAdaptor;
class KUrl;

namespace KIO {
    class Job;
}

/**
 * Observer for progress information.
 *
 * This class, of which there is always only one instance,
 * "observes" what jobs do and forwards this information
 * to the progress-info server.
 *
 * It is a DBus object so that the UI server can call the
 * proper method when on the UI server an action is performed.
 *
 * Usually jobs are automatically registered by the
 * KIO::Scheduler, so you do not have to care about that.
 *
 * @short Observer for progress information
 * @author David Faure <faure@kde.org>
 */
class KIO_EXPORT Observer
    : public QObject
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


    /**
      * Returns the unique observer object
      *
      * @return the observer object
      */
    static Observer *self()
    {
        if (!s_pObserver)
            s_pObserver = new Observer;

        return s_pObserver;
    }

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

protected:
    static Observer *s_pObserver;
    Observer();
    ~Observer() {}

    OrgKdeKIOUIServerInterface *m_uiserver;

    QMap<int, KJob*> m_dctJobs;


Q_SIGNALS:
    void actionPerformed(KJob *job, int actionId);

private:
    ObserverAdaptor *observerAdaptor;
};

#endif
