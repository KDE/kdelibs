#include "QtTest/qttest_kde.h"
#include "kstringhandlertest.h"
#include "kstringhandlertest.moc"

QTTEST_KDEMAIN(KStringHandlerTest, NoGUI)

#include "kstringhandler.h"
#include <QString>

QString KStringHandlerTest::test = "The quick brown fox jumped over the lazy bridge. ";

void KStringHandlerTest::word()
{
    COMPARE(KStringHandler::word(test, "2"), 
            QString("brown"));
    COMPARE(KStringHandler::word(test, "2:4"), 
            QString("brown fox jumped"));   
}

void KStringHandlerTest::insword()
{
    COMPARE(KStringHandler::insword(test, "very", 1),
            QString("The very quick brown fox jumped over the lazy bridge. "));
}
    
void KStringHandlerTest::setword()
{
    COMPARE(KStringHandler::setword(test, "very", 1),
            QString("The very brown fox jumped over the lazy bridge. "));
}

void KStringHandlerTest::remrange()
{
    COMPARE(KStringHandler::remrange(test, "4:6"),
            QString("The quick brown fox lazy bridge. " ));
    COMPARE(KStringHandler::remrange(test, "4:8"),
            QString("The quick brown fox "));
}
  
void KStringHandlerTest::remword()
{
    COMPARE(KStringHandler::remword(test, 4),
            QString("The quick brown fox over the lazy bridge. "));
    COMPARE(KStringHandler::remword(test, "lazy"),
            QString("The quick brown fox jumped over the bridge. "));
}

void KStringHandlerTest::capwords()
{
    COMPARE(KStringHandler::capwords(test),
            QString("The Quick Brown Fox Jumped Over The Lazy Bridge. "));
}
  
void KStringHandlerTest::reverse()
{
    COMPARE(KStringHandler::reverse(test),
            QString(" bridge. lazy the over jumped fox brown quick The"));
}

void KStringHandlerTest::center()
{
    QString result = KStringHandler::center(test, 70);
    COMPARE(result.length(), 70);
    COMPARE(result,
            QString("           The quick brown fox jumped over the lazy bridge.           "));
}

void KStringHandlerTest::tagURLs()
{
    QString test = "Click on http://foo@bar:www.kde.org/yoyo/dyne.html#a1 for info.";
    COMPARE(KStringHandler::tagURLs(test),
	    QString("Click on <a href=\"http://foo@bar:www.kde.org/yoyo/dyne.html#a1\">http://foo@bar:www.kde.org/yoyo/dyne.html#a1</a> for info."));

    test = "http://www.foo.org/story$806";
    COMPARE(KStringHandler::tagURLs(test),
	    QString("<a href=\"http://www.foo.org/story$806\">http://www.foo.org/story$806</a>"));

#if 0
  // XFAIL - i.e. this needs to be fixed, but has never been
  test = "&lt;a href=www.foo.com&gt;";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	 "&lt;a href=<a href=\"www.foo.com\">www.foo.com</a>&gt;" );
#endif
}
