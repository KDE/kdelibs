#ifndef KCOMPONENTFACTORYTEST_H
#define KCOMPONENTFACTORYTEST_H

#include <QObject>

class KComponentFactoryTest : public QObject
{
    Q_OBJECT
private slots:
    void testNonWorking();
    void testWorking();
    void testWrongClass();
};


#endif
