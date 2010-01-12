// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#ifndef KRUN_H
#define KRUN_H

#include <kio/kio_export.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <sys/types.h>
#include <kurl.h>

class KService;
class KStartupInfo;
class KJob;
namespace KIO
{
class Job;
}

/**
 * To open files with their associated applications in KDE, use KRun.
 *
 * It can execute any desktop entry, as well as any file, using
 * the default application or another application "bound" to the file type
 * (or URL protocol).
 *
 * In that example, the mimetype of the file is not known by the application,
 * so a KRun instance must be created. It will determine the mimetype by itself.
 * If the mimetype is known, or if you even know the service (application) to
 * use for this file, use one of the static methods.
 *
 * By default KRun uses auto deletion. It causes the KRun instance to delete
 * itself when the it finished its task. If you allocate the KRun
 * object on the stack you must disable auto deletion, otherwise it will crash.
 *
 * @short Opens files with their associated applications in KDE
 */
class KIO_EXPORT KRun : public QObject
{
    Q_OBJECT
public:
    /**
     * @param url the URL of the file or directory to 'run'
     *
     * @param window
     *        The top-level widget of the app that invoked this object.
     *        It is used to make sure private information like passwords
     *        are properly handled per application.
     *
     * @param mode The @p st_mode field of <tt>struct stat</tt>. If
     *        you don't know this set it to 0.
     *
     * @param isLocalFile
     *        If this parameter is set to @p false then @p url is
     *        examined to find out whether it is a local URL or
     *        not. This flag is just used to improve speed, since the
     *        function KUrl::isLocalFile is a bit slow.
     *
     * @param showProgressInfo
     *        Whether to show progress information when determining the
     *        type of the file (i.e. when using KIO::stat and KIO::mimetype)
     *        Before you set this to false to avoid a dialog box, think about
     *        a very slow FTP server...
     *        It is always better to provide progress info in such cases.
     * @param asn
     *        Application startup notification id, if available (otherwise "").
     */
    KRun(const KUrl& url, QWidget* window, mode_t mode = 0,
         bool isLocalFile = false, bool showProgressInfo = true,
         const QByteArray& asn = QByteArray());

    /**
     * Destructor. Don't call it yourself, since a KRun object auto-deletes
     * itself.
     */
    virtual ~KRun();

    /**
     * Abort this KRun. This kills any jobs launched by it,
     * and leads to deletion if auto-deletion is on.
     * This is much safer than deleting the KRun (in case it's
     * currently showing an error dialog box, for instance)
     */
    void abort();

    /**
     * Returns true if the KRun instance has an error.
     * @return true when an error occurred
     * @see error()
     */
    bool hasError() const;

    /**
     * Returns true if the KRun instance has finished.
     * @return true if the KRun instance has finished
     * @see finished()
     */
    bool hasFinished() const;

    /**
     * Checks whether auto delete is activated.
     * Auto-deletion causes the KRun instance to delete itself
     * when it finished its task.
     * By default auto deletion is on.
     * @return true if auto deletion is on, false otherwise
     */
    bool autoDelete() const;

    /**
     * Enables or disabled auto deletion.
     * Auto deletion causes the KRun instance to delete itself
     * when it finished its task. If you allocate the KRun
     * object on the stack you must disable auto deletion.
     * By default auto deletion is on.
     * @param b true to enable auto deletion, false to disable
     */
    void setAutoDelete(bool b);

    /**
     * Set the preferred service for opening this URL, after
     * its mimetype will have been found by KRun. IMPORTANT: the service is
     * only used if its configuration says it can handle this mimetype.
     * This is used for instance for the X-KDE-LastOpenedWith key, for
     * the recent documents list.
     * @param desktopEntryName the desktopEntryName of the service, e.g. "kate".
     */
    void setPreferredService(const QString& desktopEntryName);

    /**
     * Sets whether executables, .desktop files or shell scripts should
     * be run by KRun. This is enabled by default.
     * @param b whether to run executable files or not.
     * @see isExecutable()
     */
    void setRunExecutables(bool b);

    /**
     * Sets whether the external webbrowser setting should be honoured.
     * This is enabled by default.
     * This should only be disabled in webbrowser applications.
     * @param b whether to enable the external browser or not.
     */
    void setEnableExternalBrowser(bool b);

    /**
     * Sets the file name to use in the case of downloading the file to a tempfile
     * in order to give to a non-url-aware application. Some apps rely on the extension
     * to determine the mimetype of the file. Usually the file name comes from the URL,
     * but in the case of the HTTP Content-Disposition header, we need to override the
     * file name.
     */
    void setSuggestedFileName(const QString& fileName);

    /**
     * Suggested file name given by the server (e.g. HTTP content-disposition)
     */
    QString suggestedFileName() const;


    /**
     * Open a list of URLs with a certain service (application).
     *
     * @param service the service to run
     * @param urls the list of URLs, can be empty (app launched
     *        without argument)
     * @param window The top-level widget of the app that invoked this object.
     * @param tempFiles if true and urls are local files, they will be deleted
     *        when the application exits.
     * @param suggestedFileName see setSuggestedFileName
     * @param asn Application startup notification id, if any (otherwise "").
     * @return @c true on success, @c false on error
     */
    static bool run(const KService& service, const KUrl::List& urls, QWidget* window,
                    bool tempFiles = false, const QString& suggestedFileName = QString(),
                    const QByteArray& asn = QByteArray());

    /**
     * Open a list of URLs with an executable.
     *
     * @param exec the name of the executable, for example
     *        "/usr/bin/netscape %u".
     *        Don't forget to include the %u if you know that the applications
     *        supports URLs. Otherwise, non-local urls will first be downloaded
     *        to a temp file (using kioexec).
     * @param urls  the list of URLs to open, can be empty (app launched without argument)
     * @param window The top-level widget of the app that invoked this object.
     * @param name the logical name of the application, for example
     *        "Netscape 4.06".
     * @param icon the icon which should be used by the application.
     * @param asn Application startup notification id, if any (otherwise "").
     * @return @c true on success, @c false on error
     */
    static bool run(const QString& exec, const KUrl::List& urls, QWidget* window,
                    const QString& name = QString(),
                    const QString& icon = QString(),
                    const QByteArray& asn = QByteArray());

    /**
     * Open the given URL.
     *
     * This function is used after the mime type
     * is found out. It will search for all services which can handle
     * the mime type and call run() afterwards.
     * @param url the URL to open
     * @param mimetype the mime type of the resource
     * @param window The top-level widget of the app that invoked this object.
     * @param tempFile if true and url is a local file, it will be deleted
     *        when the launched application exits.
     * @param runExecutables if false then local .desktop files,
     *        executables and shell scripts will not be run.
     *        See also isExecutable().
     * @param suggestedFileName see setSuggestedFileName
     * @param asn Application startup notification id, if any (otherwise "").
     * @return @c true on success, @c false on error
     */
    static bool runUrl(const KUrl& url, const QString& mimetype, QWidget* window,
                       bool tempFile = false , bool runExecutables = true,
                       const QString& suggestedFileName = QString(), const QByteArray& asn = QByteArray());

    /**
     * Run the given shell command and notifies kicker of the starting
     * of the application. If the program to be called doesn't exist,
     * an error box will be displayed.
     *
     * Use only when you know the full command line. Otherwise use the other
     * static methods, or KRun's constructor.
     *
     * @p cmd must be a shell command. You must not append "&"
     * to it, since the function will do that for you.
     * @param window The top-level widget of the app that invoked this object.
     *
     * @return @c true on success, @c false on error
     */
    static bool runCommand(const QString &cmd, QWidget* window);

    /**
     * Same as the other runCommand(), but it also takes the name of the
     * binary, to display an error message in case it couldn't find it.
     *
     * @param cmd must be a shell command. You must not append "&"
     * to it, since the function will do that for you.
     * @param execName the name of the executable
     * @param icon icon for app starting notification
     * @param window The top-level widget of the app that invoked this object.
     * @param asn Application startup notification id, if any (otherwise "").
     * @return @c true on success, @c false on error
     */
    static bool runCommand(const QString& cmd, const QString & execName,
                           const QString & icon, QWidget* window, const QByteArray& asn = QByteArray());

    /**
     * Display the Open-With dialog for those URLs, and run the chosen application.
     * @param lst the list of applications to run
     * @param window The top-level widget of the app that invoked this object.
     * @param tempFiles if true and lst are local files, they will be deleted
     *        when the application exits.
     * @param suggestedFileName see setSuggestedFileName
     * @param asn Application startup notification id, if any (otherwise "").
     * @return false if the dialog was canceled
     */
    static bool displayOpenWithDialog(const KUrl::List& lst, QWidget* window,
                                      bool tempFiles = false, const QString& suggestedFileName = QString(),
                                      const QByteArray& asn = QByteArray());

    /**
     * Quotes a string for the shell.
     * An empty string will @em not be quoted.
     *
     * @deprecated Use KShell::quoteArg() instead. @em Note that this function
     *  behaves differently for empty arguments and returns the result
     *  differently.
     *
     * @param str the string to quote. The quoted string will be written here
     */
    static KDE_DEPRECATED void shellQuote(QString &str);

    /**
     * Processes a Exec= line as found in .desktop files.
     * @param _service the service to extract information from.
     * @param _urls The urls the service should open.
     * @param tempFiles if true and urls are local files, they will be deleted
     *        when the application exits.
     * @param suggestedFileName see setSuggestedFileName
     *
     * @return a list of arguments suitable for KProcess::setProgram().
     */
    static QStringList processDesktopExec(const KService &_service, const KUrl::List &_urls,
                                          bool tempFiles = false,
                                          const QString& suggestedFileName = QString());

    /**
     * Given a full command line (e.g. the Exec= line from a .desktop file),
     * extract the name of the binary being run.
     * @param execLine the full command line
     * @param removePath if true, remove a (relative or absolute) path. E.g. /usr/bin/ls becomes ls.
     * @return the name of the binary to run
     */
    static QString binaryName(const QString & execLine, bool removePath);

    /**
     * Returns whether @p serviceType refers to an executable program instead
     * of a data file.
     */
    static bool isExecutable(const QString& serviceType);

    /**
     * Returns whether the @p url of @p mimetype is executable.
     * To be executable the file must pass the following rules:
     * -# Must reside on the local filesystem.
     * -# Must be marked as executable for the user by the filesystem.
     * -# The mime type must inherit application/x-executable or application/x-executable-script.
     * To allow a script to run when the above rules are satisfied add the entry
     * @code
     * X-KDE-IsAlso=application/x-executable-script
     * @endcode
     * to the mimetype's desktop file.
     */
    static bool isExecutableFile(const KUrl& url, const QString &mimetype);

    /**
     * @internal
     */
    static bool checkStartupNotify(const QString& binName, const KService* service, bool* silent_arg,
                                   QByteArray* wmclass_arg);

Q_SIGNALS:
    /**
     * Emitted when the operation finished.
     * @see hasFinished()
     */
    void finished();
    /**
     * Emitted when the operation had an error.
     * @see hasError()
     */
    void error();

protected Q_SLOTS:
    /**
     * All following protected slots are used by subclasses of KRun!
     */

    /**
     * This slot is called whenever the internal timer fired,
     * in order to move on to the next step.
     */
    void slotTimeout(); // KDE5: rename to slotNextStep() or something like that

    /**
     * This slot is called when the scan job is finished.
     */
    void slotScanFinished(KJob *);

    /**
     * This slot is called when the scan job has found out
     * the mime type.
     */
    void slotScanMimeType(KIO::Job *, const QString &type);

    /**
     * Call this from subclasses when you have determined the mimetype.
     * It will call foundMimeType, but also sets up protection against deletion during message boxes.
     * @since 4.0.2
     */
    void mimeTypeDetermined(const QString& mimeType);

    /**
     * This slot is called when the 'stat' job has finished.
     */
    virtual void slotStatResult(KJob *);

protected:
    /**
     * All following protected methods are used by subclasses of KRun!
     */

    /**
     * Initializes the krun object.
     */
    virtual void init();

    /**
     * Start scanning a file.
     */
    virtual void scanFile();

    /**
     * Called if the mimetype has been detected. The function runs
     * the application associated with this mimetype.
     * Reimplement this method to implement a different behavior,
     * like opening the component for displaying the URL embedded.
     */
    virtual void foundMimeType(const QString& type);

    /**
     * Kills the file scanning job.
     */
    virtual void killJob();

    /**
     * Sets the url.
     */
    void setUrl(const KUrl &url);

    /**
     * Returns the url.
     */
    KUrl url() const;

    /**
     * Sets whether an error has occurred.
     */
    void setError(bool error);

    /**
     * Sets whether progress information shall be shown.
     */
    void setProgressInfo(bool progressInfo);

    /**
     * Returns whether progress information are shown.
     */
    bool progressInfo() const;

    /**
     * Marks this 'KRun' instance as finished.
     */
    void setFinished(bool finished);

    /**
     * Sets the job.
     */
    void setJob(KIO::Job *job);

    /**
     * Returns the job.
     */
    KIO::Job* job();

    /**
     * Returns the timer object.
     */
    QTimer& timer();

    /**
     * Indicate that the next action is to scan the file.
     * @deprecated not useful in public API
     */
    KDE_DEPRECATED void setDoScanFile(bool scanFile);

    /**
     * Returns whether the file shall be scanned.
     * @deprecated not useful in public API
     */
    KDE_DEPRECATED bool doScanFile() const;

    /**
     * Sets whether it is a directory.
     * @deprecated typo in the name, and not useful as a public method
     */
    KDE_DEPRECATED void setIsDirecory(bool isDirectory);

    /**
     * Returns whether it is a directory.
     */
    bool isDirectory() const;

    /**
     * @deprecated not useful in public API
     */
    KDE_DEPRECATED void setInitializeNextAction(bool initialize);

    /**
     * @deprecated not useful in public API
     */
    KDE_DEPRECATED bool initializeNextAction() const;

    /**
     * Sets whether it is a local file.
     */
    void setIsLocalFile(bool isLocalFile);

    /**
     * Returns whether it is a local file.
     */
    bool isLocalFile() const;

    /**
     * Sets the file mode.
     */
    void setMode(mode_t mode);

    /**
     * Returns the file mode.
     */
    mode_t mode() const;

private:
    class KRunPrivate;
    KRunPrivate* const d;
};

#endif
