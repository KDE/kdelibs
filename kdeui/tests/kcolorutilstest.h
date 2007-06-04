#ifndef KCOLORUTILSTEST_H
#define KCOLORUTILSTEST_H

#include <QtTest/QtTest>

class tst_KColorUtils : public QObject
{
    Q_OBJECT
private slots:
    void testOverlay();
    void testMix();
};

#endif // KCOLORUTILSTEST_H
