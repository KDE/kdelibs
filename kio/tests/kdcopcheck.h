#ifndef _BLA_H_
#define _BLA_H_

#include <kprocess.h>
#include <qstring.h>
#include <qobject.h>

class TestService : public QObject
{
    Q_OBJECT
public:
    TestService(const QString &exec);

    int exec();

public slots:
    void newApp(const QCString &appId);
    void endApp(const QCString &appId);
    void appExit();
    void stop();

protected:  
    int result;
    QString m_exec;
    KProcess proc;
};

#endif
