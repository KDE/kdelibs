#ifndef KCONFIGTEST_H
#define KCONFIGTEST_H

#include <qobject.h>

class KConfigTest : public QObject
{
    Q_OBJECT
private slots:
    void testSimple();
    void testLists();
    void testComplex();
    void testInvalid();
    void initTestCase();
    void cleanupTestCase();
private:
    void revertEntries();
};

#endif /* KCONFIGTEST_H */
