#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QObject>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private slots:
    void testIcons();
    void testFindByPath();
    void testFindByURL();
};


#endif
