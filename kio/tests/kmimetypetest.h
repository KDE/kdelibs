#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QObject>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testByName();
    void testIcons();
    void testFindByURL();
    void testFindByPath();
    void testAllMimeTypes();
    void testAllServiceTypes();
    void testAllServices();
    void testAllInitServices();
    void testMimeTypeParent();
    void testMimeTypeTraderForTextPlain();
    void testMimeTypeTraderForDerivedMimeType();
    void testServiceTypeTraderForReadOnlyPart();
};


#endif
