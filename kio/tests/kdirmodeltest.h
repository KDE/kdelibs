#ifndef KDIRMODELTEST_H
#define KDIRMODELTEST_H

#include <QObject>
#include <ktempdir.h>
#include <QDateTime>
#include <kdirmodel.h>

class KDirModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRowCount();
    void testIndex();
    void testNames();
    void testItemForIndex();
    void testData();

Q_SIGNALS:
    void exitLoop();

protected Q_SLOTS: // 'more private than private slots' - i.e. not seen by qtestlib
    void slotListingCompleted();

private:
    void enterLoop();

private:
    KTempDir m_tempDir;
    KDirModel m_dirModel;
    QModelIndex m_fileIndex;
    QModelIndex m_dirIndex;
    QModelIndex m_fileInDirIndex;
    QModelIndex m_fileInSubdirIndex;
};


#endif
