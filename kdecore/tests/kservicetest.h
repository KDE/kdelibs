#ifndef KSERVICETEST_H
#define KSERVICETEST_H

#include <QObject>

class KServiceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testByName();
    void testAllServiceTypes();
    void testAllServices();
    void testAllInitServices();
    void testServiceTypeTraderForReadOnlyPart();
    void testHasServiceType1();
    void testHasServiceType2();
};

#endif
