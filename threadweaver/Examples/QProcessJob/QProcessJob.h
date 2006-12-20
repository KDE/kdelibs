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

    QString result();
private:
    void run ();

    QByteArray m_result ;
};

#endif
