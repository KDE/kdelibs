#include "qtest_kde.h"
#include "kstringhandlertest.h"
#include "kstringhandlertest.moc"

QTEST_KDEMAIN_CORE(KStringHandlerTest)

#include "kstringhandler.h"


QString KStringHandlerTest::test = "The quick brown fox jumped over the lazy bridge. ";

void KStringHandlerTest::capwords()
{
    QCOMPARE(KStringHandler::capwords(test),
            QString("The Quick Brown Fox Jumped Over The Lazy Bridge. "));
}

void KStringHandlerTest::tagURLs()
{
    QString test = "Click on http://foo@bar:www.kde.org/yoyo/dyne.html#a1 for info.";
    QCOMPARE(KStringHandler::tagUrls(test),
	    QString("Click on <a href=\"http://foo@bar:www.kde.org/yoyo/dyne.html#a1\">http://foo@bar:www.kde.org/yoyo/dyne.html#a1</a> for info."));

    test = "http://www.foo.org/story$806";
    QCOMPARE(KStringHandler::tagUrls(test),
	    QString("<a href=\"http://www.foo.org/story$806\">http://www.foo.org/story$806</a>"));

#if 0
  // XFAIL - i.e. this needs to be fixed, but has never been
  test = "&lt;a href=www.foo.com&gt;";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	 "&lt;a href=<a href=\"www.foo.com\">www.foo.com</a>&gt;" );
#endif
}

void KStringHandlerTest::perlSplit()
{
  QStringList expected;
  expected << "some" << "string" << "for" << "you__here";
  QCOMPARE(KStringHandler::perlSplit("__","some__string__for__you__here", 4),expected);

  expected.clear();
  expected << "kparts" << "reaches" << "the parts other parts can't";
  QCOMPARE(KStringHandler::perlSplit(' ',"kparts reaches the parts other parts can't", 3),expected);

  expected.clear();
  expected << "Split" << "me" << "up ! I'm bored ! OK ?";
  QCOMPARE(KStringHandler::perlSplit(QRegExp("[! ]"), "Split me up ! I'm bored ! OK ?", 3),expected);
}

void KStringHandlerTest::naturalCompare()
{
    QCOMPARE(KStringHandler::naturalCompare("a", "b", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("b", "a", Qt::CaseSensitive), +1);
    QCOMPARE(KStringHandler::naturalCompare("a", "a", Qt::CaseSensitive), 0);

    QCOMPARE(KStringHandler::naturalCompare("A", "a", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A", "a", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("a", "A", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("aAa", "AaA", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("aaa", "AAA", Qt::CaseInsensitive), 0);

    QCOMPARE(KStringHandler::naturalCompare("1", "2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("9", "10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "12", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "100", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("a1", "a2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a9", "a10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a12", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a100", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a2a1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a1a2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a10a1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a1a10", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("Test 1.gif", "Test 2.gif", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Test 1.gif", "Test 10.gif", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Test 9.gif", "Test 10.gif", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_2.4.10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.10", "cmake_2.4.11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_2.5.6", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_3.4.6", Qt::CaseSensitive), -1);
}

