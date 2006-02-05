#ifndef KARCHIVETEST_H
#define KARCHIVETEST_H

#include <QObject>

class KArchiveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateTar();
    void testReadTar();
    void testTarFileData();
    void testMaxLength();
};


#endif
