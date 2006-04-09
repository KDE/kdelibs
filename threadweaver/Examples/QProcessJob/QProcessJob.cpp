#include "QProcessJob.h"

using namespace ThreadWeaver;

QProcessJob::QProcessJob ( QObject * parent )
    : Job ( parent )
{
}

void QProcessJob::run()
{
    QProcess m_process;

    QStringList args;
    args.append ( "+%H:%m:%S %N" );
    m_process.start( "date", args );

    if (  ! m_process.waitForFinished ( -1 ) )
    {
        return;
    }

    m_result = m_process.readAll();

    // qDebug ( "'date' returned %s", result.constData() );
}

QString QProcessJob::result()
{
    Q_ASSERT ( isFinished () );
    return m_result;
}
