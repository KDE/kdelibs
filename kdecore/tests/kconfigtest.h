#ifndef KCONFIGTEST_H
#define KCONFIGTEST_H

#include <qobject.h>

class KConfigTest : public QObject
{
    Q_OBJECT
    Q_ENUMS(Testing)
    Q_FLAGS(Flags)

public:
    enum Testing { Ones=1, Tens=10, Hundreds=100};
    enum bits { bit0=1, bit1=2, bit2=4, bit3=8 };
    Q_DECLARE_FLAGS(Flags, bits)

private Q_SLOTS:
    void testSimple();
    void testLists();
    void testComplex();
    void testEnums();
    void testInvalid();
    void initTestCase();
    void cleanupTestCase();
private:
    void revertEntries();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigTest::Flags)

#endif /* KCONFIGTEST_H */
