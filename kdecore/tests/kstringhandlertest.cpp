#include "qtest_kde.h"
#include "kstringhandlertest.h"
#include "kstringhandlertest.moc"

QTEST_KDEMAIN(KStringHandlerTest, NoGUI)

#include "kstringhandler.h"
#include <QString>

QString KStringHandlerTest::test = "The quick brown fox jumped over the lazy bridge. ";

void KStringHandlerTest::word()
{
    QCOMPARE(KStringHandler::word(test, "2"),
            QString("brown"));
    QCOMPARE(KStringHandler::word(test, "2:4"),
            QString("brown fox jumped"));
}

void KStringHandlerTest::insword()
{
    QCOMPARE(KStringHandler::insword(test, "very", 1),
            QString("The very quick brown fox jumped over the lazy bridge. "));
}

void KStringHandlerTest::setword()
{
    QCOMPARE(KStringHandler::setword(test, "very", 1),
            QString("The very brown fox jumped over the lazy bridge. "));
}

void KStringHandlerTest::remrange()
{
    QCOMPARE(KStringHandler::remrange(test, "4:6"),
            QString("The quick brown fox lazy bridge. " ));
    QCOMPARE(KStringHandler::remrange(test, "4:8"),
            QString("The quick brown fox "));
}

void KStringHandlerTest::remword()
{
    QCOMPARE(KStringHandler::remword(test, 4),
            QString("The quick brown fox over the lazy bridge. "));
    QCOMPARE(KStringHandler::remword(test, "lazy"),
            QString("The quick brown fox jumped over the bridge. "));
}

void KStringHandlerTest::capwords()
{
    QCOMPARE(KStringHandler::capwords(test),
            QString("The Quick Brown Fox Jumped Over The Lazy Bridge. "));
}

void KStringHandlerTest::reverse()
{
    QCOMPARE(KStringHandler::reverse(test),
            QString(" bridge. lazy the over jumped fox brown quick The"));
}

void KStringHandlerTest::center()
{
    QString result = KStringHandler::center(test, 70);
    QCOMPARE(result.length(), 70);
    QCOMPARE(result,
            QString("           The quick brown fox jumped over the lazy bridge.           "));
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

