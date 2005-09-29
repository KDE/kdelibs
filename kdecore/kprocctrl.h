/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Czezakte (e9025461@student.tuwien.ac.at)

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

#ifndef __KPROCCTRL_H__
#define __KPROCCTRL_H__

#include <qvaluelist.h>

#include "kprocess.h"

class QSocketNotifier;

/**
 * @short Used internally by KProcess
 * @internal
 * @author Christian Czezatke <e9025461@student.tuwien.ac.at>
 *
 *  A class for internal use by KProcess only. -- Exactly one instance
 *  of this class is created by KApplication.
 *
 * This class takes care of the actual (UN*X) signal handling.
 */
class KDECORE_EXPORT KProcessController : public QObject
{
  Q_OBJECT

public:
  /**
   * Create an instance if none exists yet.
   * Called by KApplication::KApplication()
   */
  static void ref();

  /**
   * Destroy the instance if one exists and it is not referenced any more.
   * Called by KApplication::~KApplication()
   */
  static void deref();

  /**
   * Only a single instance of this class is allowed at a time,
   * and this static variable is used to track the one instance.
   */
  static KProcessController *theKProcessController; // kde4: rename: instance

  /**
   * Automatically called upon SIGCHLD. Never call it directly.
   * If your application (or some library it uses) redirects SIGCHLD,
   * the new signal handler (and only it) should call the old handler
   * returned by sigaction().
   * @internal
   */
  static void theSigCHLDHandler(int signal); // KDE4: private

  /**
   * Wait for any process to exit and handle their exit without
   * starting an event loop.
   * This function may cause KProcess to emit any of its signals.
   *
   * @param timeout the timeout in seconds. -1 means no timeout.
   * @return true if a process exited, false
   *         if no process exited within @p timeout seconds.
   * @since 3.1
   */
  bool waitForProcessExit(int timeout);

  /**
   * Call this function to defer processing of the data that became available
   * on notifierFd().
   * @since 3.2
   */
  void unscheduleCheck();

  /**
   * This function @em must be called at some point after calling
   * unscheduleCheck().
   * @since 3.2
   */
  void rescheduleCheck();

  /*
   * Obtain the file descriptor KProcessController uses to get notified
   * about process exits. select() or poll() on it if you create a custom
   * event loop that needs to act upon SIGCHLD.
   * @return the file descriptor of the reading end of the notification pipe
   * @since 3.2
   */
  int notifierFd() const;

  /**
   * @internal
   */
  void addKProcess( KProcess* );
  /**
   * @internal
   */
  void removeKProcess( KProcess* );
  /**
   * @internal
   */
  void addProcess( int pid );

private slots:
  void slotDoHousekeeping();

private:
  friend class I_just_love_gcc;

  int fd[2];
  bool needcheck;
  QSocketNotifier *notifier;
  QValueList<KProcess*> kProcessList;
  QValueList<int> unixProcessList;

  static void setupHandlers();
  static void resetHandlers();
  static struct sigaction oldChildHandlerData;
  static bool handlerSet;

  static int refCount;

  // Disallow instantiation
  KProcessController();
  ~KProcessController();

  // Disallow assignment and copy-construction
  KProcessController( const KProcessController& );
  KProcessController& operator= ( const KProcessController& );
};



#endif

