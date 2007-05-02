/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
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

#ifndef KJOBTRACKERSTEST_H
#define KJOBTRACKERSTEST_H

#include <kjob.h>

#include <QtCore/QTimer>

class KTestJob : public KJob
{
    Q_OBJECT

public:
    enum State { StatingDirs, CreatingDirs, CopyingFiles, Stopped };

    // 10 files per directory
    // 1000 bytes per files
    KTestJob(int numberOfDirs = 5);
    ~KTestJob();

    void start();

private Q_SLOTS:
    void nextStep();

protected:
    void stateNextDir();
    void createNextDir();
    void copyNextFile();
    void deleteNextFile();

    bool doSuspend();
    bool doResume();
    bool doKill();

private:
    qulonglong m_numberOfDirs;
    qulonglong m_currentSpeed;
    State m_state;
    QTimer m_timer;
};

#endif
