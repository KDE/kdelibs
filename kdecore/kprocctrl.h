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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KPROCCTRL_H__
#define __KPROCCTRL_H__

#include <qvaluelist.h>
#include <qtimer.h>

#include "kprocess.h"

class QSocketNotifier;

/**
 * @short Used internally by @ref KProcess
 * @internal
 * @author Christian Czezatke <e9025461@student.tuwien.ac.at>
 *
 *  A class for internal use by KProcess only. -- Exactly one instance
 *  of this class is created by KApplication.
 *
 * This class takes care of the actual (UN*X) signal handling.
 */
class KProcessController : public QObject
{
  Q_OBJECT

public:
  /**
   * Create an instance if none exists yet.
   * Called by @see KApplication::KApplication()
   */
  static void create();

  /**
   * Destroy the instance if one exists.
   * Called by @see KApplication::~KApplication()
   */
  static void destroy();

  /**
   * Only a single instance of this class is allowed at a time,
   * and this static variable is used to track the one instance.
   */
  static KProcessController *theKProcessController;

  /**
   * Automatically called upon SIGCHLD. Never call it directly.
   * If your application (or some library it uses) redirects SIGCHLD,
   * the new signal handler (and only it) should call the old handler
   * returned by sigaction().
   * @internal
   */
  static void theSigCHLDHandler(int signal); // KDE4: private
  /**
   * @internal
  */
  void addKProcess( KProcess* );
  /**
   * @internal
  */
  void removeKProcess( KProcess* );

  /**
   * Wait for any process to exit and handle their exit without
   * starting an event loop.
   * This function may cause KProcess to emit any of its signals.
   *
   * @param timeout the timeout in seconds
   * @return true if a process exited, false
   *         if no process exited within @p timeout seconds.
   * @since 3.1
   */
  bool waitForProcessExit(int timeout);

public slots:
  /**
   * @internal
   */
 void slotDoHousekeeping(int socket);

private slots:
 void delayedChildrenCleanup();

private:
  int fd[2];
  QSocketNotifier *notifier;

  static void setupHandlers();
  static void resetHandlers();
  static struct sigaction oldChildHandlerData;
  static bool handlerSet;
  QValueList<KProcess*> processList;
  QTimer delayedChildrenCleanupTimer;

protected:
  // Disallow instantiation
  KProcessController();
  ~KProcessController();

private:
  // Disallow assignment and copy-construction
  KProcessController( const KProcessController& );
  KProcessController& operator= ( const KProcessController& );
};



#endif

