#ifndef KFILEITEMTEST_H
#define KFILEITEMTEST_H

#include <QObject>

class KFileItemTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testPermissionsString();
};


#endif
