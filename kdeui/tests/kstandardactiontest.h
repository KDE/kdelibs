#ifndef KSTANDARDACTIONTEST_H
#define KSTANDARDACTIONTEST_H

#include "qtest_kde.h"

class tst_KStandardAction : public QObject
{
    Q_OBJECT

public slots:
    inline void dummy() {}

private slots:
    void implicitInsertion();
};

#endif // KSTANDARDACTIONTEST_H
