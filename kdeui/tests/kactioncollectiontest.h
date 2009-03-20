#ifndef KACTIONCOLLECTIONTEST_H
#define KACTIONCOLLECTIONTEST_H

#include "qtest_kde.h"
#include "kactioncollection.h"
#include <kconfig.h>
#include <kconfiggroup.h>

class tst_KActionCollection : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void clear();
    void deleted();
    void take();
    void writeSettings();
    void readSettings();
    void insertReplaces1();
    void insertReplaces2();
    void testSetShortcuts();

private:
    KConfigGroup clearConfig();

    KActionCollection *collection;
};

#endif // KACTIONCOLLECTIONTEST_H
