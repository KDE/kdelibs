#ifndef ktempfiletest_h
#define ktempfiletest_h

#include <kunittest/tester.h>

class KTempFileTest : public KUnitTest::SlotTester
{
    Q_OBJECT
public slots:
    void testBasic();
    void testFixedExtension();
    void testHomeDir();
};

#endif
