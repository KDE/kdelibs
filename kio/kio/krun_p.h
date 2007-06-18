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

#ifndef KRUN_P_H
#define KRUN_P_H

#include <QtCore/QObject>
#include "kprocess.h"

class KProcess;
class QString;
class KStartupInfoId;
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

    static qint64 run(KProcess *, const QString & binName);
    static qint64 run(KProcess *, const QString & binName, const KStartupInfoId& id );

    virtual ~KProcessRunner();

    qint64 pid() const;

  protected Q_SLOTS:

    void slotProcessExited(int, QProcess::ExitStatus);

  private:
    KProcessRunner(KProcess *, const QString & binName);
    KProcessRunner(KProcess *, const QString & binName, const KStartupInfoId& id );
    KProcessRunner();

    class KProcessRunnerPrivate;
    KProcessRunnerPrivate *const d;

    Q_DISABLE_COPY(KProcessRunner)
};

#endif  // KRUN_P_H
