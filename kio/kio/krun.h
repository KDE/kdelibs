/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __k_run_h__
#define __k_run_h__

#include <sys/stat.h>
#include <sys/types.h>

#include <qobject.h>
#include <qtimer.h>
#include <qstring.h>
#include <kurl.h>
#include <kstartupinfo.h>

class KProcess;
class KService;
namespace KIO {
   class Job;
   class StatJob;
};

/**
 * To open files with their associated applications in KDE, use KRun.
 *
 * It can execute any desktop entry, as well as any file, using
 * the default application or another application "bound" to the file type.
 *
 * In that example, the mimetype of the file is not known by the application,
 * so a KRun instance must be created. It will determine the mimetype by itself.
 * If the mimetype is known, or if you even know the service (application) to
 * use for this file, use one of the static methods.
 *
 * @short Opens files with their associated applications in KDE
 */
class KRun : public QObject
{
  Q_OBJECT
public:
  /**
   * Create a KRun object to run the preferred application for a file/URL.
   * KRun will first determine the type of the file, and will then
   * run the associated application.
   *
   * @param url the URL of the file or directory to 'run'
   *
   * @param mode The @p st_mode field of <tt>struct stat</tt>. If
   *        you don't know this set it to 0.
   *
   * @param isLocalFile
   *        If this parameter is set to @p false then @p url is
   *        examined to find out whether it is a local URL or
   *        not. This flag is just used to improve speed, since the
   *        function @ref KURL::isLocalFile is a bit slow.
   *
   * @param showProgressInfo
   *        Whether to show progress information when determining the
   *        type of the file (i.e. when using KIO::stat and KIO::mimetype)
   *        Before you set this to false to avoid a dialog box, think about
   *        a very slow FTP server...
   *        It is always better to provide progress info in such cases.
   */
  KRun( const KURL& url, mode_t mode = 0,
	bool isLocalFile = false, bool showProgressInfo = true );

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

  bool hasError() const { return m_bFault; }
  bool hasFinished() const { return m_bFinished; }

  /**
   * By default auto deletion is on.
   */
  bool autoDelete() const { return m_bAutoDelete; }
  void setAutoDelete(bool b) { m_bAutoDelete = b; }

  /**
   * Set the preferred service for opening this URL, after
   * its mimetype will have been found by KRun. IMPORTANT: the service is
   * only used if its configuration says it can handle this mimetype.
   * This is used for instance for the X-KDE-LastOpenedWith key, for
   * the recent documents list.
   * @param desktopEntryName the desktopEntryName of the service, e.g. "kate".
   */
  void setPreferredService( const QString& desktopEntryName );

  /**
   * Open a list of URLs with a certain service (application).
   *
   * @param _service the service to run
   * @param _urls the list of URLs, can be empty (app launched
   *        without argument)
   * @param tempFiles if true and _urls are local files, they will be deleted
   *        when the application exits.
   * @return the process id, or 0 on error
   */
  static pid_t run( const KService& _service, const KURL::List& _urls, bool tempFiles );
  // BIC merge with method above, using tempFiles=false
  static pid_t run( const KService& _service, const KURL::List& _urls );

  /**
   * Open a list of URLs with.
   *
   * @param _exec The name of the executable, for example
   *        "/usr/bin/netscape".
   * @param _name The logical name of the application, for example
   *        "Netscape 4.06".
   * @param _icon The icon which should be used by the application.
   * @param _obsolete1 Do not use!
   * @param _obsolete2 Do not use!
   */
  static pid_t run( const QString& _exec, const KURL::List& _urls,
		   const QString& _name = QString::null,
		   const QString& _icon = QString::null,
		   const QString& _obsolete1 = QString::null,
		   const QString& _obsolete2 = QString::null );

  /**
   * Open the given URL.
   *
   * This function is used after the mime type
   * is found out. It will search for all services which can handle
   * the mime type and call @ref run() afterwards.
   * @param _url the URL to open
   * @param _mimetype the mime type of the resource
   * @param tempFile if true and _url is a local file, it will be deleted
   *        when the launched application exits.
   * @return the process id, or 0 on error
   */
  static pid_t runURL( const KURL& _url, const QString& _mimetype, bool tempFile );
  // BIC merge with method above, using tempFiles=false
  static pid_t runURL( const KURL& _url, const QString& _mimetype );

  /**
   * Run the given shell command and notifies kicker of the starting
   * of the application. If the program to be called doesn't exist,
   * an error box will be displayed.
   *
   * @p _cmd must be a shell command. You must not append "&"
   * to it, since the function will do that for you.
   *
   * @return PID of running command, 0 if it could not be started, 0 - (PID
   * of running command) if command was unsafe for map notification.
   *
   * Use only when you know the full command line. Otherwise use the other
   * static methods, or KRun's constructor.
   */
  static pid_t runCommand( QString cmd );

  /**
   * Same as the other runCommand, but it also takes the name of the
   * binary, to display an error message in case it couldn't find it.
   * @param icon icon for app starting notification
   */
  static pid_t runCommand( const QString& cmd, const QString & execName, const QString & icon );

  /**
   * Display the Open-With dialog for those URLs, and run the chosen application.
   * @param lst the list of applications to run
   * @param tempFiles if true and lst are local files, they will be deleted
   *        when the application exits.
   * @return false if the dialog was canceled
   */
  static bool displayOpenWithDialog( const KURL::List& lst, bool tempFiles );
  // BIC merge with method above, using tempFiles=false
  static bool displayOpenWithDialog( const KURL::List& lst );

  /**
   * Quotes a string for the shell
   */
  static void shellQuote( QString &_str );

  /**
   * Processes a Exec= line as found in .desktop files.
   * @param _service the service to extract information from.
   * @param _urls The urls the service should open.
   * @param has_shell If true, the arguments are going to be fed into a
   *        shell e.g by using system().
   *        If false, the arguments are going to be fed into a exec() kind
   *        call.
   *        If the arguments are intended for an exec() kind of call and
   *        the Exec line contains shell commands then "/bin/sh -c" is added.
   * @param tempFiles if true and _urls are local files, they will be deleted
   *        when the application exits.
   * @return a list of arguments suitable for either system() or exec().
   */
  static QStringList processDesktopExec(const KService &_service, const KURL::List &_urls, bool has_shell, bool tempFiles);
  // BIC merge with method above, using tempFiles=false
  static QStringList processDesktopExec(const KService &_service, const KURL::List &_urls, bool has_shell);

  /**
   * Given a full command line (e.g. the Exec= line from a .desktop file),
   * extract the name of the binary being run.
   * @param execLine the full command line
   * @param removePath if true, remove a (relative or absolute) path. E.g. /usr/bin/ls becomes ls.
   * @since 3.1
   */
  static QString binaryName( const QString & execLine, bool removePath );

signals:
  void finished();
  void error();

protected slots:
  void slotTimeout();
  void slotScanFinished( KIO::Job * );
  void slotScanMimeType( KIO::Job *, const QString &type );
  virtual void slotStatResult( KIO::Job * );

protected:
  virtual void init();

  virtual void scanFile();

  /**
   * Called if the mimetype has been detected. The function checks
   * whether the document and appends the gzip protocol to the
   * URL. Otherwise @ref #runURL is called to finish the job.
   */
  virtual void foundMimeType( const QString& _type );

  virtual void killJob();

  KURL m_strURL;
  bool m_bFault;
  bool m_bAutoDelete;
  bool m_bProgressInfo;
  bool m_bFinished;
  KIO::Job * m_job;
  QTimer m_timer;

  /**
   * Used to indicate that the next action is to scan the file.
   * This action is invoked from @ref #slotTimeout.
   */
  bool m_bScanFile;
  bool m_bIsDirectory;

  /**
   * USed to indicate that the next action is to initialize.
   * This action is invoked from @ref #slotTimeout
   */
  bool m_bInit;

  bool m_bIsLocalFile;
  mode_t m_mode;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KRunPrivate;
  KRunPrivate *d;
};

#ifndef KDE_NO_COMPAT
/**
 * @deprecated. Kept for source compatibility, does nothing nowadays.
 * Do not use in new source.
 * KRun can open the openwith dialog directly now.
 * Use KRun::displayOpenWithDialog() if you were using KOpenWithHandler directly.
 */
class KOpenWithHandler
{
public:
  KOpenWithHandler() {}
  static bool exists() { return true; }
};
#endif

/**
 * @internal
 * This class watches a process launched by KRun.
 * It sends a notification when the process exits (for the taskbar)
 * and it will show an error message if necessary (e.g. "program not found").
 */
class KProcessRunner : public QObject
{
  Q_OBJECT

  public:

    static pid_t run(KProcess *, const QString & binName);
#ifdef Q_WS_X11 // We don't have KStartupInfo in Qt/Embedded
    static pid_t run(KProcess *, const QString & binName, const KStartupInfoId& id );
#endif

    virtual ~KProcessRunner();

    pid_t pid() const;

  protected slots:

    void slotProcessExited(KProcess *);

  private:

    KProcessRunner(KProcess *, const QString & binName);
#ifdef Q_WS_X11 // We don't have KStartupInfo in Qt/Embedded
    KProcessRunner(KProcess *, const QString & binName, const KStartupInfoId& id );
#endif
    KProcessRunner();

    KProcess * process_;
    QString binName;
#ifdef Q_WS_X11 // We don't have KStartupInfo in Qt/Embedded
    KStartupInfoId id_;
#endif
};

#endif
