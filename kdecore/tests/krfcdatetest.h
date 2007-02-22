#ifndef KRFCDATETEST_H
#define KRFCDATETEST_H

#include <QtCore/QObject>

class KRFCDateTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test();
    void testRFC2822();
};

#endif
