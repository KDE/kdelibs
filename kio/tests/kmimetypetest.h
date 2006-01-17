#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QObject>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testByName();
    void testIcons();
    void testFindByURL();
    void testFindByPath();
    void testAllMimeTypes();
    void testAllServices();
    void testAllInitServices();
};


#endif
