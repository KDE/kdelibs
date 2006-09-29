/* -*- C++ -*-

This file implements the Job class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Troll Tech, Norway. $

$Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#ifndef THREADWEAVER_JOB_P_H
#define THREADWEAVER_JOB_P_H

#include <QObject>

#include "Job.h"

using namespace ThreadWeaver;

class ThreadWeaver::JobRunHelper : public QObject
{
    Q_OBJECT
public:
    JobRunHelper();

signals:
    void started ( Job* );
    void done ( Job* );
    void failed( Job* );

public:

    void runTheJob ( Thread* th, Job* job );
};

#endif // THREADWEAVER_JOB_P_H
