#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QObject>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private slots:
    void testByName();
    void testIcons();
    void testFindByURL();
    void testFindByPath();
};


#endif
