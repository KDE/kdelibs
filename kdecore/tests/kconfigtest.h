#ifndef KCONFIGTEST_H
#define KCONFIGTEST_H

#include <qobject.h>

class KConfigTest : public QObject
{
    Q_OBJECT
private slots:
    void testAll();
private:
    void writeConfigFile();
    void revertEntries();
};

#endif /* KCONFIGTEST_H */
