#ifndef QPROCESSJOB_H
#define QPROCESSJOB_H

#include <QProcess>

#include "Job.h"

using namespace ThreadWeaver;

class QProcessJob : public Job
{
    Q_OBJECT
public:
    QProcessJob ( QObject * parent = 0 );

protected:
    void run ();
};

#endif
