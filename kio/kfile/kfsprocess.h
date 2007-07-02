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

#ifndef KFSPROC_H
#define KFSPROC_H

#include <QtCore/QList>
#include <QtCore/QObject>

#include <sys/types.h> // for pid_t

class QSocketNotifier;
class KfsProcess;

/** @internal @em Don't use outside KIO! I mean it. */
class KfsProcessController : public QObject
{
  Q_OBJECT
  friend class KfsProcess;

public:
  static void theSigCHLDHandler(int signal);

private:
  KfsProcessController();
  ~KfsProcessController();
  static void setupHandlers();
  static void resetHandlers();
  static void ref();
  static void deref();
  static KfsProcessController *instance();
  void addKProcess( KfsProcess* );
  void removeKProcess( KfsProcess* );
  void addProcess(int pid);

  int m_fd[2];
  QSocketNotifier *m_notifier;
  QList<KfsProcess*> m_kProcessList;
  QList<int> m_unixProcessList;
  static struct sigaction s_oldChildHandlerData;
  static bool s_handlerSet;
  static int s_refCount;
  static KfsProcessController* s_instance;

private Q_SLOTS:
  void slotDoHousekeeping();
};

/** @internal @em Don't use outside KIO! I mean it. */
class KfsProcess : public QObject
{
  Q_OBJECT
  friend class KfsProcessController;

public:

  explicit KfsProcess( QObject* parent=0L );
  ~KfsProcess();

  KfsProcess &operator<<(const QString& arg);
  KfsProcess &operator<<(const char * arg);
  bool start();
  void detach();

Q_SIGNALS:
  void processExited();

private:
  QList<QByteArray> arguments;

  bool runs;
  pid_t pid_;
  void processHasExited();
};

#endif

