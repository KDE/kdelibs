#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QObject>
#include <kmimemagicrule.h>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testByName();
    void testIcons();
    void testFindByUrl();
    void testFindByPath();
    void testFindByNameAndContent();
    void testFindByContent();
    void testAllMimeTypes();
    void testAlias();
    void testMimeTypeParent();
    void testMimeTypeTraderForTextPlain();
    void testMimeTypeTraderForDerivedMimeType();
    void testMimeTypeTraderForAlias();
    void testHasServiceType1();
    void testHasServiceType2();
    void testPatterns();

    void testParseMagicFile();
    void testParseMagicFile_data();
private:
    QList<KMimeMagicRule> m_rules;
};


#endif
