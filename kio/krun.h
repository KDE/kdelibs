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

class KService;
namespace KIO {
   class Job;
   class StatJob;
};

/**
 * Implements a generic runner, i.e. the 'exec' functionality of KDE
 *
 * It can execute any desktop entry, as well as any file, using
 * default binding (service) or another bound service.
 */
class KRun : public QObject
{
  Q_OBJECT
public:
  /**
   * @param _mode The @tt st_mode field of <tt>struct stat</tt>. If
   *        you dont know this set it to 0.
   *
   * @param _is_local_file
   *        If this parameter is set to @tt false then @p _url is
   *        examnined to find out whether it is a local URL or
   *        not. This flag is just used to improve speed, since the
   *        function @ref KURL::isLocalFile is a bit slow.
   */
  KRun( const KURL& _url, mode_t _mode = 0,
	bool _is_local_file = false, bool _auto_delete = true );
  virtual ~KRun();

  bool hasError() const { return m_bFault; }
  bool hasFinished() const { return m_bFinished; }

  /**
   * By default auto deletion is on.
   */
  bool autoDelete() const { return m_bAutoDelete; }
  void setAutoDelete() { m_bAutoDelete = m_bAutoDelete; }

  /**
   * Open a list of URLs with a certain service.
   *
   * @param _service
   * @param _urls the list of URLs, can be empty (app launched
   *        without argument)
   */
  static bool run( const KService& _service, const KURL::List& _urls );

  /**
   * Open a list of URLs with.
   *
   * @param _exec The name of the executable, for example
   *        "/usr/bin/netscape".
   * @param _name The logical name of the application, for example
   *        "Netscape 4.06".
   * @param _icon The icon which should be used by the application.
   * @param _miniicon The icon which should be used by the application.
   */
  static bool run( const QString& _exec, const KURL::List& _urls,
		   const QString& _name = QString::null,
		   const QString& _icon = QString::null,
		   const QString& _mini_icon = QString::null,
		   const QString& _desktop_file = QString::null );

  /**
   * Open the given URL.
   *
   * This function is used after the mime type
   * is found out. It will search for all services which can handle
   * the mime type and call @ref run() afterwards.
   */
  static bool runURL( const KURL& _url, const QString& _mimetype );

  /**
   * Run the given command and notifies kicker of the starting
   * of the application.
   *
   * Use only when you know the full command line. Otherwise use the other
   * static methods, or @ref KRun's constructor.
   * @param cmd the full command line to run, see @ref run()
   * @param execName Dhe name of the executable (usually the first 'word' of the command).
   * @param iconName The name of the (mini) icon to show in kicker.
   */
  static bool runCommand( const QString& cmd, const QString & execName, const QString & iconName );

signals:
  void finished();
  void error();

protected slots:
  void slotTimeout();
  void slotScanFinished( KIO::Job * );
  void slotStatResult( KIO::Job * );

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

  /**
   * For remote URLs to be opened with apps that don't support
   * remote URLs. Uses kfmexec.
   */
  static bool runOldApplication( const QString& _exec,
                                 const KURL::List& _urls,
				 bool _allow_multiple );

  /**
   * Runs a shell command.
   *
   * @ref _cmd must be a quoted shell command. You must not append "&"
   * to it, since the function will do that for you. An example is
   * "<tt>greet 'Hello Torben'</tt>".
   *
   * @return PID of running command, or -1 if it could not be started.
   */
  static pid_t run( const QString& _cmd );

  /**
   * Sends a DCOP signal to Kicker's taskbar to indicate that an
   * application has been started.
   *
   * @param execName Name of the app binary.
   * @param iconName Name of that app's mini icon.
   * @param resName Application's res_name, usually equivalent to execName.
   * @param pid PID of the KShellProcess that is executing the binary.
   */
  static void clientStarted(
    const QString & execName,
    const QString & iconName,
    pid_t pid);

  /**
   * Extracts binary name from Exec command line
   */
  static QString binaryName( const QString & execLine );

  /**
   * Quotes a string for the shell
   */
  static void shellQuote( QString &_str );

private:

  static QString libmapnotify();

};

/**
 * This class handles the displayOpenWithDialog call, made by KRun
 * when it has no idea what to do with a URL.
 * The default implementation is to print a huge fat warning
 * This behaviour is overriden by KFileOpenWithHandler, in libkfile,
 * which displays the real open-with dialog box.
 *
 * If you use KRun you @tt need to create an instance of KFileOpenWithHandler
 * (except if you can make sure you only use it for executables or
 *  Type=Application desktop files)
 *
 * @see KFileOpenWithHandler
 * @short Handler for KRun's displayOpenWithDialog() call
 */
class KOpenWithHandler
{
public:
  KOpenWithHandler() { pOpenWithHandler = this; }
  virtual ~KOpenWithHandler() { pOpenWithHandler = 0; }

  /**
   * Opens an open-with dialog box for @p urls
   * @returns true if the operation succeeded
   */
  virtual bool displayOpenWithDialog( const KURL::List& urls );

  /**
   * Call this to get the (only) instance of KOpenWithHandler
   */
  static KOpenWithHandler * getOpenWithHandler() {
    if (!pOpenWithHandler)
      pOpenWithHandler = new KOpenWithHandler;
    return pOpenWithHandler;
  }

  static bool exists() { return pOpenWithHandler != 0L; }

private:
  static KOpenWithHandler * pOpenWithHandler;
};

#endif
