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
    void testMimeTypeParent();
    void testMimeTypeTraderForTextPlain();
    void testMimeTypeTraderForDerivedMimeType();
    void testHasServiceType1();
    void testHasServiceType2();
};


#endif
