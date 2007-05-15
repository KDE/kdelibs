/* -*- C++ -*-

This file implements the Job class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $

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

$Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#ifndef THREADWEAVER_JOB_P_H
#define THREADWEAVER_JOB_P_H

#include <QtCore/QObject>

#include "Job.h"

using namespace ThreadWeaver;

class ThreadWeaver::JobRunHelper : public QObject
{
    Q_OBJECT
public:
    JobRunHelper();

Q_SIGNALS:
    void started ( ThreadWeaver::Job* );
    void done ( ThreadWeaver::Job* );
    void failed( ThreadWeaver::Job* );

public:

    void runTheJob ( Thread* th, Job* job );
};

#endif // THREADWEAVER_JOB_P_H
