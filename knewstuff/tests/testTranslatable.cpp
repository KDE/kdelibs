// knewstufftranslatabletest.cpp

// unit test for ktranslatable

#include <QtTest>
#include <QtCore>

#include <qtest_kde.h>

#include "../knewstuff2/core/ktranslatable.h"

class testTranslatable: public QObject
{
    Q_OBJECT
private slots:
    void testAddString();
    void testRepresentation();
};

void testTranslatable::testRepresentation()
{
    KNS::KTranslatable translatable("testRepresentation");
    QCOMPARE(translatable.representation(), QString("testRepresentation"));
}

void testTranslatable::testAddString()
{
    KNS::KTranslatable translatable;
    translatable.addString("", "testRepresentation");
    translatable.addString("1", "1");
    QCOMPARE(translatable.representation(), QString("testRepresentation"));
    QCOMPARE(translatable.translated("1"), QString("1"));
}

QTEST_KDEMAIN_CORE(testTranslatable)
#include "testTranslatable.moc"
