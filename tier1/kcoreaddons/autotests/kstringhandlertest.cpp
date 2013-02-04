
#include "kstringhandlertest.h"

#include <QTest>

QTEST_MAIN(KStringHandlerTest)

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

    QCOMPARE(KStringHandler::naturalCompare("A", "a", Qt::CaseSensitive), QString::localeAwareCompare("A", "a"));
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

    QCOMPARE(KStringHandler::naturalCompare("1a", "2a", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1b", "1a", Qt::CaseSensitive), +1);

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

    QCOMPARE(KStringHandler::naturalCompare("A-123.txt", "A-a.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A-012.txt", "A-a.txt", Qt::CaseSensitive), -1);

    // bug 191865
    QCOMPARE(KStringHandler::naturalCompare("E & G", "E & J", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("E & J", "E & S", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("E & S", "Em & M", Qt::CaseSensitive), -1);

    // bug 181211
    QCOMPARE(KStringHandler::naturalCompare("queen__radio_ga_ga.mp3", "queen__somebody_to_love_live.mp3", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("queen__somebody_to_love_live.mp3", "queens_of_the_stone_age__no_one_knows.mp3", Qt::CaseSensitive), -1);

    // bug 201101
    QCOMPARE(KStringHandler::naturalCompare("text", "text.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("text.txt", "text1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("text1", "text1.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A B", "A.B", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A.B", "A B", Qt::CaseSensitive), +1);

    // bug 231445
    QCOMPARE(KStringHandler::naturalCompare("1", "a", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a", "v01 1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("v01 1", "v01 a", Qt::CaseSensitive), -1);

    // bug 237551
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txt", "sysvinit-2.86-i486-6.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz", "sysvinit-2.86-i486-6.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz.asc", "sysvinit-functions-8.53-i486-2.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txt", "sysvinit-functions-8.53-i486-2.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txz", "sysvinit-functions-8.53-i486-2.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txz.asc", "sysvinit-scripts-1.2-noarch-31.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-scripts-1.2-noarch-31.txt", "sysvinit-scripts-1.2-noarch-31.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-scripts-1.2-noarch-31.txz", "sysvinit-scripts-1.2-noarch-31.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz.asc", "sysvinit-scripts-1.2-noarch-31.txz", Qt::CaseSensitive), -1);

    // bug 237541
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2008-10","Car and Driver Buyer's Guide 2010", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2008-10", "Car and Driver 2009-11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2009-11", "Car and Driver 2010-05", Qt::CaseSensitive), -1);

    // bug 237788
    QCOMPARE(KStringHandler::naturalCompare(" ", ".", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare(".", "_", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare(" ", "_", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc.jpg", "abc1.jpg", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc1.jpg", "abc_a.jpg", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc.jpg", "abc_a.jpg", Qt::CaseInsensitive), -1);
}

void KStringHandlerTest::obscure()
{
  // See bug 167900, obscure() produced chars that could not properly be converted to and from
  // UTF8. The result was that storing passwords with '!' in them did not work.
  QString test = "!TEST!";
  QString obscured = KStringHandler::obscure( test );
  QByteArray obscuredBytes = obscured.toUtf8();
  QCOMPARE( KStringHandler::obscure( QString::fromUtf8( obscuredBytes.constData() ) ), test );
}

void KStringHandlerTest::preProcessWrap_data()
{
    const QChar zwsp(0x200b);

    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("expected");

    // Should result in no additional breaks
    QTest::newRow("spaces") << "foo bar baz" << "foo bar baz";

    // Should insert a ZWSP after each '_'
    QTest::newRow("underscores") << "foo_bar_baz"
               << QString("foo_" + zwsp + "bar_" + zwsp + "baz");

    // Should insert a ZWSP after each '-'
    QTest::newRow("hyphens") << "foo-bar-baz"
                << QString("foo-" + zwsp + "bar-" + zwsp + "baz");

    // Should insert a ZWSP after each '.'
    QTest::newRow("periods") << "foo.bar.baz"
                << QString("foo." + zwsp + "bar." + zwsp + "baz");

    // Should insert a ZWSP after each ','
    QTest::newRow("commas") << "foo,bar,baz"
                << QString("foo," + zwsp + "bar," + zwsp + "baz");

    // Should result in no additional breaks since the '_'s are followed by spaces
    QTest::newRow("mixed underscores and spaces")
                << "foo_ bar_ baz" << "foo_ bar_ baz";

    // Should result in no additional breaks since the '_' is the last char
    QTest::newRow("ends with underscore") << "foo_" << "foo_";

    // Should insert a ZWSP before '(' and after ')'
    QTest::newRow("parens") << "foo(bar)baz"
                << QString("foo" + zwsp + "(bar)" + zwsp + "baz");

    // Should insert a ZWSP before '[' and after ']'
    QTest::newRow("brackets") << "foo[bar]baz"
                << QString("foo" + zwsp + "[bar]" + zwsp + "baz");

    // Should insert a ZWSP before '{' and after '}'
    QTest::newRow("curly braces") << "foo{bar}baz"
                << QString("foo" + zwsp + "{bar}" + zwsp + "baz");

    // Should insert a ZWSP before '(' but not after ')' since it's the last char
    QTest::newRow("ends with ')'") << "foo(bar)"
                << QString("foo" + zwsp + "(bar)");

    // Should insert a single ZWSP between the '_' and the '('
    QTest::newRow("'_' followed by '('") << "foo_(bar)"
                << QString("foo_" + zwsp + "(bar)");

    // Should insert ZWSP's between the '_' and the '[', between the double
    // '['s and the double ']'s, but not before and after 'bar'
    QTest::newRow("'_' before double brackets") << "foo_[[bar]]"
                << QString("foo_" + zwsp + '[' + zwsp + "[bar]" + zwsp + ']');

    // Should only insert ZWSP's between the double '['s and the double ']'s
    QTest::newRow("space before double brackets") << "foo [[bar]]"
                << QString("foo [" + zwsp + "[bar]" + zwsp + "]");

    // Shouldn't result in any additional breaks since the '(' is preceded
    // by a space, and the ')' is followed by a space.
    QTest::newRow("parens with spaces") << "foo (bar) baz" << "foo (bar) baz";

    // Should insert a WJ (Word Joiner) before a single quote
    const QChar wj(0x2060);
    QTest::newRow("single quote") << "foo'bar"<< QString("foo"+ wj +"'bar");
}

static QString replaceZwsp(const QString &string)
{
    const QChar zwsp(0x200b);

    QString result;
    for (int i = 0; i < string.length(); i++)
        if (string[i] == zwsp)
            result += "<zwsp>";
        else
            result += string[i];

    return result;
}

void KStringHandlerTest::preProcessWrap()
{
    QFETCH(QString, string);
    QFETCH(QString, expected);

    QCOMPARE(replaceZwsp(KStringHandler::preProcessWrap(string)),
             replaceZwsp(expected));
}

