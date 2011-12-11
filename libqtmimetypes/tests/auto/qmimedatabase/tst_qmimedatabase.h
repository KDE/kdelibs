#ifndef TST_QMIMEDATABASE_H
#define TST_QMIMEDATABASE_H

#include <QObject>

class tst_qmimedatabase : public QObject
{
    Q_OBJECT

public:
    tst_qmimedatabase();
    ~tst_qmimedatabase();

private Q_SLOTS:
    void initTestCase();

    void test_mimeTypeForName();
    void test_findByName_data();
    void test_findByName();
    void test_inheritance();
    void test_aliases();
    void test_icons();
    void test_findByFileWithContent();
    void test_findByUrl();
    void test_findByContent_data();
    void test_findByContent();
    void test_findByNameAndContent_data();
    void test_findByNameAndContent();
    void test_allMimeTypes();
    void test_inheritsPerformance();
    void test_suffixes_data();
    void test_suffixes();
    void test_knownSuffix();
    void test_fromThreads();

    // shared-mime-info test suite

    void findByName_data();
    void findByName();

    void findByData_data();
    void findByData();

    void findByFile_data();
    void findByFile();
};

#endif // TST_QMIMEDATABASE_H
