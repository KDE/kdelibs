#include "QProcessJob.h"

using namespace ThreadWeaver;

QProcessJob::QProcessJob ( QObject * parent )
    : Job ( parent )
{
}

void QProcessJob::run()
{
    QProcess m_process;

    m_process.start( "date", QStringList() );

    if (  ! m_process.waitForFinished ( -1 ) )
    {
        return;
    }

    QByteArray result = m_process.readAll();

    qDebug ( "'date' returned %s", result.constData() );
}

