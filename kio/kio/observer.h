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

    /**
      * Popup a message box. See KIO::SlaveBase
      * This doesn't use DBus anymore, it shows the dialog in the application's process
      * Otherwise, other apps would block when trying to communicate with UIServer
      *
      * @param progressId   the progress ID of the job, as returned by newJob()
      * @param type         the type of the message box
      * @param text         the text to show
      * @param caption      the window caption
      * @param buttonYes    the text of the "Yes" button
      * @param buttonNo     the text of the "No button
      */
    static int messageBox(int progressId, int type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo);

    /**
      * Popup a message box. See KIO::SlaveBase
      * This doesn't use DBus anymore, it shows the dialog in the application's process
      * Otherwise, other apps would block when trying to communicate with UIServer
      *
      * @param progressId       the progress ID of the job, as returned by newJob()
      * @param type             the type of the message box
      * @param text             the text to show
      * @param caption          the window caption
      * @param buttonYes        the text of the "Yes" button
      * @param buttonNo         the text of the "No button
      * @param dontAskAgainName A checkbox is added with which further confirmation can be turned off.
      *                         The string is used to lookup and store the setting in kioslaverc.
      */
    static int messageBox(int progressId, int type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo, const QString &dontAskAgainName);

    /**
      * @internal
      * See renamedialog.h
      */
    KIO::RenameDialog_Result open_RenameDialog(KJob * job,
                                                const QString & caption,
                                                const QString& src,
                                                const QString & dest,
                                                KIO::RenameDialog_Mode mode,
                                                QString& newDest,
                                                KIO::filesize_t sizeSrc = (KIO::filesize_t) -1,
                                                KIO::filesize_t sizeDest = (KIO::filesize_t) -1,
                                                time_t ctimeSrc = (time_t) -1,
                                                time_t ctimeDest = (time_t) -1,
                                                time_t mtimeSrc = (time_t) -1,
                                                time_t mtimeDest = (time_t) -1);

    /**
      * @internal
      * See skipdialog.h
      */
    KIO::SkipDialog_Result open_SkipDialog(KJob * job,
                                            bool multi,
                                            const QString & error_text);

// FIXME: Are these slots necessary now that we have slotActionPerformed ?
//        I don't think so (ereslibre)

public Q_SLOTS:
    /**
      * Called by the UI Server (using DBus) if the user presses cancel
      *
      * @param progressId the progress ID of the job, as returned by newJob()
      */
    Q_SCRIPTABLE void killJob(int progressId);

    /**
      * Called by the UI Server (using DBus) if the user presses "pause"
      *
      * @param progressId the progress ID of the job, as returned by newJob()
      */
    Q_SCRIPTABLE void suspend(int progressId);

    /**
      * Called by the UI Server (using DBus) if the user presses "pause"
      *
      * @param progressId the progress ID of the job, as returned by newJob()
      */
    Q_SCRIPTABLE void resume(int progressId);

    /**
      * Called by the UI Server (using DBus) to get all the metadata of the job
      *
      * @param progressId the progress ID of the job, as returned by newJob()
      */
    Q_SCRIPTABLE QVariantMap metadata(int progressId);

protected:
    static Observer *s_pObserver;
    Observer();
    ~Observer() {}

    OrgKdeKIOUIServerInterface *m_uiserver;

    QMap<int, KJob*> m_dctJobs;

public Q_SLOTS:
    // TODO: Maybe it is more cute having all these methods called setFoo, instead of slotFoo.
    //       Anyway this doesn't seem a trivial change on kdelibs and kdebase, so I will go on it
    //       when have more time. Well, if you wanna try, tell me :) (ereslibre)

    void slotTotalSize(KJob *job, qulonglong size);
    void slotTotalFiles(KJob *job, unsigned long files);
    void slotTotalDirs(KJob *job, unsigned long dirs);
    void slotProcessedSize(KJob *job, qulonglong size);
    void slotProcessedFiles(KJob *job, unsigned long files);
    void slotProcessedDirs(KJob *job, unsigned long dirs);
    void slotSpeed(KJob *job, unsigned long speed);
    void slotPercent(KJob *job, unsigned long percent);
    void slotInfoMessage(KJob *job, const QString &msg);
    void slotProgressMessage(KJob *job, const QString &msg);

    void slotCopying(KJob *job, const KUrl &src, const KUrl &dest);
    void slotMoving(KJob *job, const KUrl &src, const KUrl &dest);
    void slotDeleting(KJob *job, const KUrl &url);
    void slotTransferring(KJob *job, const KUrl &url);
    void slotCreatingDir(KJob *job, const KUrl &dir);

public:
    void stating(KJob *job, const KUrl &url);
    void mounting(KJob *job, const QString &dev, const QString &point);
    void unmounting(KJob *job, const QString &point);

private Q_SLOTS:
    void slotActionPerformed(int actionId, int jobId);

Q_SIGNALS:
    void actionPerformed(KJob *job, int actionId);
};

#endif
