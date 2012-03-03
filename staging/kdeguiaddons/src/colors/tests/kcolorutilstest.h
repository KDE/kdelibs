#ifndef KCOLORUTILSTEST_H
#define KCOLORUTILSTEST_H

#include <QtTest/QtTest>

class tst_KColorUtils : public QObject
{
    Q_OBJECT
private slots:
    void testOverlay();
    void testMix();
    void testHCY();
    void testContrast();
    void testShading();
};

#endif // KCOLORUTILSTEST_H
