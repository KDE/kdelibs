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
      * If you don't have a KJob, but want to show some kind of
      * progress into the UI Server, you have to call this method
      *
      * @param icon the icon to be shown on the UI Server
      * @return     the progress ID assigned by the UI Server
      */
    int newJob(const QString &icon = QString());

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

    /**
      * Adds an action to a job
      *
      * @param jobId        the identification number of the job
      * @param actionId     the action identification number that will have the new action
      * @param actionText   the text that will be shown on the button
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the slot that will be called when button is clicked
      */
    void addAction(int jobId, int actionId, const QString &actionText, QObject *receiver, const char *slotName);

    /**
      * Adds a standard action to a job
      *
      * @param jobId        the identification number of the job
      * @param action       the standard action that will be added
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the slot that will be called when button is clicked
      */
    void addStandardAction(int jobId, StandardActions action, QObject *receiver = 0, const char *slotName = 0);

    /**
      * Adds an action to a job
      *
      * @param job          the job to which the action will be added
      * @param actionId     the action identification number that will have the new action
      * @param actionText   the text that will be shown on the button
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the slot that will be called when button is clicked
      */
    void addAction(KJob *job, int actionId, const QString &actionText, QObject *receiver, const char *slotName);

    /**
      * Adds a standard action to a job
      *
      * @param job          the job to which the action will be added
      * @param action       the typical action that will be added
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the slot that will be called when button is clicked
      */
    void addStandardAction(KJob *job, StandardActions action, QObject *receiver = 0, const char *slotName = 0);

    /**
      * Edits an existing action
      *
      * @param jobId        the identification number of the job
      * @param actionId     the action that is going to be edited
      * @param actionText   the new text that is going to be shown on the button
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the new slot that will be called if the action is performed
      */
    void editAction(int jobId, int actionId, const QString &actionText, QObject *receiver, const char *slotName);

    /**
      * Edits an existing standard action
      *
      * @param jobId        the identification number of the job
      * @param action       the standard action that will be added
      * @param receiver     the QObject pointer where slot @p slotName lives
      * @param slotName     the new slot that will be called if the action is performed
      */
    void editStandardAction(int jobId, StandardActions action, QObject *receiver = 0, const char *slotName = 0);

    /**
      * Enables an existing action (the press button)
      *
      * @param actionId the action that is going to be enabled
      */
    void enableAction(int actionId);

    /**
      * Disables an existing action (the press button)
      *
      * @param actionId the action that is going to be disabled
      */
    void disableAction(int actionId);

    /**
      * Removes an existing action
      *
      * @param jobId    the identification number of the job
      * @param actionId the action that is going to be removed
      */
    void removeAction(int jobId, int actionId);

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
    struct SlotInfo
    {
        QObject *receiver;
        QByteArray slotName;
    };

    struct SlotCall
    {
        QList<SlotInfo> theSlotInfo;
    };

    static Observer *s_pObserver;
    Observer();
    ~Observer() {}

    OrgKdeKIOUIServerInterface *m_uiserver;

    QMap<int, KJob*> m_dctJobs;
    QHash<int /* jobId */, QHash<int /* actionId */, SlotCall> > m_hashActions;

public Q_SLOTS:
    void setTotalSize(int jobId, qulonglong size);
    void setTotalFiles(int jobId, unsigned long files);
    void setTotalDirs(int jobId, unsigned long dirs);
    void setProcessedSize(int jobId, qulonglong size);
    void setProcessedFiles(int jobId, unsigned long files);
    void setProcessedDirs(int jobId, unsigned long dirs);
    void setSpeed(int jobId, unsigned long speed);
    void setPercent(int jobId, unsigned long percent);
    void setInfoMessage(int jobId, const QString &msg);
    void setProgressMessage(int jobId, const QString &msg);

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
    void jobPaused(KJob *job, int actionId);
    void jobResumed(KJob *job, int actionId);
    void jobCanceled(KJob *job, int actionId);

Q_SIGNALS:
    void actionPerformed(KJob *job, int actionId);
};

#endif
