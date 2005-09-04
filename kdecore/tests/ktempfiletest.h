#ifndef ktempfiletest_h
#define ktempfiletest_h

#include <qobject.h>

class KTempFileTest : public QObject
{
    Q_OBJECT
public slots:
    void testBasic();
    void testFixedExtension();
    void testHomeDir();
};

#endif
