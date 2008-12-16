/* This file is part of the KDE libraries
    Copyright (c) 1999-2005 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2000-2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
// -*- mode: c++; c-basic-offset: 2 -*-

#include "qtest_kde.h"
#include "kurltest.h"
#include "kurltest.moc"

QTEST_KDEMAIN_CORE( KUrlTest )

#include <config.h>
#include <kcmdlineargs.h>

#include "kurl.h"
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kuser.h>
#include <QtCore/QTextCodec>
#include <QtCore/QDataStream>
#include <QtCore/QMap>
#include <stdio.h>
#include <stdlib.h>

// needed to build a valid path on windows
#ifdef Q_OS_WIN
#define LOCAL_FILE_PREFIX "C:"
#else
#define LOCAL_FILE_PREFIX
#endif

void KUrlTest::testEmptyURL()
{
  KUrl emptyURL;
  QVERIFY( !emptyURL.isValid() );
  QVERIFY( emptyURL.isEmpty() );
  QVERIFY( emptyURL.prettyUrl().isEmpty() );
  QCOMPARE( emptyURL.url(), QString() );
  QVERIFY( emptyURL.url().isEmpty() );
  QVERIFY( !emptyURL.url().isNull() ); // well, a null string would be correct too...
  QVERIFY(!emptyURL.isLocalFile());

  KUrl emptyStringURL("");
  QVERIFY( !emptyStringURL.isValid() );
  QVERIFY( emptyStringURL.isEmpty() );
  QVERIFY( emptyStringURL.url().isEmpty() );
  QVERIFY( !emptyStringURL.url().isNull() );
  QCOMPARE( emptyURL, emptyStringURL );

  // Roundtrip via .url()
  KUrl emptyCopy( emptyURL.url() );
  QCOMPARE( emptyURL, emptyCopy );
  KUrl emptyStringCopy = KUrl( emptyStringURL.url() );
  QCOMPARE( emptyStringURL, emptyCopy );

  KUrl fileURL( "file:/" );
  QVERIFY( !fileURL.isEmpty() );

  fileURL = "file:///";
  QVERIFY( !fileURL.isEmpty() );

  KUrl udir;
  QCOMPARE( udir.url(), QString() );
  QVERIFY( udir.isEmpty() );
  QVERIFY( !udir.isValid() );
  udir = udir.upUrl();
  QVERIFY( udir.upUrl().isEmpty() );
}

void KUrlTest::testIsValid()
{
  KUrl url1( "gg:www.kde.org" );
  QVERIFY( url1.isValid() );

  url1 = "KDE";
  QVERIFY( url1.isValid() ); // KDE3 difference: was FALSE

  url1 = "$HOME/.kde/share/config";
  QVERIFY( url1.isValid() ); // KDE3 difference: was FALSE
}

void KUrlTest::testSetQuery()
{
  KUrl url1 = KUrl( QByteArray( "http://www.kde.org/foo.cgi?foo=bar" ) );
  QCOMPARE( url1.query(), QString("?foo=bar") );
  url1.setQuery( "toto=titi&kde=rocks" );
  QCOMPARE( url1.query(), QString("?toto=titi&kde=rocks") );
  url1.setQuery( "?kde=rocks&a=b" );
  QCOMPARE( url1.query(), QString("?kde=rocks&a=b") );
  url1.setQuery( "?" );
  QCOMPARE( url1.query(), QString("?") );
  url1.setQuery( "" );
  QCOMPARE( url1.query(), QString("?") );
  url1.setQuery( QString() );
  QCOMPARE( url1.query(), QString() );
}

void KUrlTest::testSetRef()
{
  KUrl url1 = KUrl( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
  QCOMPARE( url1.ref(), QString("foo%3Dbar" ) ); // KDE3 difference: was foo=bar
#if 0// ditto (TODO)
  url1.setRef( "toto=titi&kde=rocks" );
  QCOMPARE( url1.ref(), QString("toto=titi&kde=rocks" ) );
  url1.setRef( "kde=rocks&a=b" );
  QCOMPARE( url1.ref(), QString("kde=rocks&a=b" ) );
  url1.setRef( "#" );
  QCOMPARE( url1.ref(), QString("#" ) );
#endif
  url1.setRef( "" );
  QCOMPARE( url1.ref(), QString("" ) );
  url1.setRef( QString() );
  QCOMPARE( url1.ref(), QString() );
}

void KUrlTest::testSetHTMLRef()
{
  KUrl url1 = KUrl( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
  QCOMPARE( url1.htmlRef(), QString("foo=bar") );
  url1.setHTMLRef( "toto=titi&kde=rocks" );
  QCOMPARE( url1.htmlRef(), QString("toto=titi&kde=rocks") );
  url1.setHTMLRef( "kde=rocks&a=b" );
  QCOMPARE( url1.htmlRef(), QString("kde=rocks&a=b") );
  url1.setHTMLRef( "#" );
  QCOMPARE( url1.htmlRef(), QString("#") );
  QCOMPARE( url1.ref(), QString("%23") ); // it's encoded
  url1.setHTMLRef( "" );
  QCOMPARE( url1.htmlRef(), QString("") );
  url1.setHTMLRef( QString() );
  QCOMPARE( url1.htmlRef(), QString() );
}

void KUrlTest::testQUrl()
{
  QUrl url1( "file:///home/dfaure/my#%2f" );
  QCOMPARE( url1.toString(), QString( "file:///home/dfaure/my#%2f" ) );
#ifdef Q_WS_WIN
  QUrl url2( "file:///c:/home/dfaure/my#%2f" );
  QCOMPARE( url2.toString(), QString( "file:///c:/home/dfaure/my#%2f" ) );
#endif
}


void KUrlTest::testIsLocalFile()
{
  KUrl local_file_1("file://localhost/my/file");
  QVERIFY( local_file_1.isLocalFile() );

  KUrl local_file_2("file://www.kde.org/my/file");
  QVERIFY( !local_file_2.isLocalFile() );

  KUrl local_file_3;
  local_file_3.setHost(getenv("HOSTNAME"));
  local_file_3.setPath("/my/file");
  //qDebug("URL=%s\n", qPrintable( local_file_3.url() ));
  QVERIFY( local_file_3.isLocalFile() );

  KUrl local_file_4("file:///my/file");
  QVERIFY( local_file_4.isLocalFile() );

#ifdef Q_WS_WIN
  KUrl local_file_4a("file:///c:/my/file");
  QVERIFY( local_file_4a.isLocalFile() );
#endif

  KUrl local_file_5;
  local_file_5.setPath("/foo?bar");
  QCOMPARE( local_file_5.url(), QString("file:///foo%3Fbar") );

#ifdef Q_WS_WIN
  KUrl local_file_5a;
  local_file_5a.setPath("c:/foo?bar");
  QCOMPARE( local_file_5a.url(), QString("file:///c:/foo%3Fbar") );
#endif
}

void KUrlTest::testSimpleMethods() // to test parsing, mostly
{
  KUrl kde( "http://www.kde.org" );
  QVERIFY( kde.isValid() );
  QCOMPARE( kde.port(), -1 ); // KDE3 DIFFERENCE: was 0.

  KUrl mlc( "http://mlc:80/" );
  QVERIFY( mlc.isValid() );
  QCOMPARE( mlc.port(), 80 );
  QCOMPARE( mlc.path(), QString("/") );

  KUrl ulong("https://swww.gad.de:443/servlet/CookieAccepted?MAIL=s@gad.de&VER=25901");
  QCOMPARE(ulong.host(),QString("swww.gad.de") );
  QCOMPARE(ulong.path(),QString("/servlet/CookieAccepted") );

  KUrl fileURL( "file:///home/dfaure/myfile" );
  QCOMPARE( fileURL.url(), QString("file:///home/dfaure/myfile") );
  QCOMPARE( fileURL.path(), QString("/home/dfaure/myfile") );
  QVERIFY( !fileURL.hasRef() );

  QString u1 = "file:/home/dfaure/my#myref";
  KUrl url1(u1);
  // KDE3 difference: QUrl doesn't resolve file:/ into file:///
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#myref") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/") );

#if 0
  QUrl qurl = QUrl::fromEncoded( "file:///home/dfaure/my#%23" );
  printf( "toString = %s\n", qurl.toString().toLatin1().constData() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
  qurl = QUrl::fromEncoded( "file:///home/dfaure/my#%2f" );
  printf( "toString = %s\n", qurl.toString().toLatin1().constData() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
  qurl = QUrl::fromEncoded( "file:///home/dfaure/my#/" );
  printf( "toString = %s\n", qurl.toString().toLatin1().constData() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
#endif

  u1 = "file:///home/dfaure/my#%2f";
  url1 = u1;
  // KDE3: was %2f, but this is OK too
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#/") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref().toLower(), QString("%2f") );
  QCOMPARE( url1.encodedHtmlRef().toLower(), QString("%2f") );
  QCOMPARE( url1.htmlRef(), QString("/") );

  u1 = "file:///home/dfaure/my#%23";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#%23") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref(), QString("%23") );
  QCOMPARE( url1.encodedHtmlRef(), QString("%23") );
  QCOMPARE( url1.htmlRef(), QString("#") );

#if 0 // TODO
  url1 = KUrl(url1, "#%6a");
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#%6a") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref(), QString("j") );
  QCOMPARE( url1.encodedHtmlRef().toLower(), QString("%6a") );
  QCOMPARE( url1.htmlRef(), QString("j") );
#endif

  u1 = "file:///home/dfaure/my#myref";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#myref") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/") );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref(), QString("QObject%3A%3Aconnect") );
  QCOMPARE( url1.htmlRef(), QString("QObject::connect") );
  QCOMPARE( url1.upUrl().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  u1 = "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref(), QString("QObject%3A%3Aconnect") );
  QCOMPARE( url1.htmlRef(), QString("QObject::connect") );
  QCOMPARE( url1.upUrl().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubUrl() );
  QCOMPARE( url1.ref(), QString("QObject%3Aconnect") );
  QCOMPARE( url1.htmlRef(), QString("QObject:connect") );
  QCOMPARE( url1.upUrl().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  KUrl carsten;
  carsten.setPath("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");
  QCOMPARE( carsten.path(), QString("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18") );

  KUrl longUserName("http://thisisaverylongusername@foobar.com/");
  QCOMPARE(longUserName.prettyUrl(), QString("http://thisisaverylongusername@foobar.com/"));
  QCOMPARE(KUrl(longUserName.prettyUrl()).url(), QString("http://thisisaverylongusername@foobar.com/"));
  KUrl whitespaceInUser("http://www.google.com%20%20%20%20%20@foobar.com/");
  QCOMPARE(whitespaceInUser.prettyUrl(), QString("http://www.google.com%20%20%20%20%20@foobar.com/"));
  KUrl whitespaceInPath("http://www.google.com/foo%20bar/");
  QCOMPARE(whitespaceInPath.prettyUrl(), QString("http://www.google.com/foo bar/"));
  KUrl whitespaceInPath2("http://www.google.com/foo%20%20%20%20%20%20%20bar/");
  QCOMPARE(whitespaceInPath2.prettyUrl(), QString("http://www.google.com/foo%20%20%20%20%20%20 bar/"));

  KUrl charles;
  charles.setPath( "/home/charles/foo%20moo" );
  QCOMPARE( charles.path(), QString("/home/charles/foo%20moo") );
  KUrl charles2("file:/home/charles/foo%20moo");
  QCOMPARE( charles2.path(), QString("/home/charles/foo moo") );

  //NOTE this test should be ran in UTF8 locale
  KUrl percentEncodedQuery( "http://mail.yandex.ru/message_part/%D0%9A%D1%80%D0%B8%D1%82%D0%B5%D1%80%D0%B8%D0%B8%20%D0%BE%D1%86%D0%B5%D0%BD%D0%B8%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F%20%D0%BE%D1%80%D0%BB%D0%BE%D0%B2%D0%BE%D0%B9.rar?hid=1.1&mid=391.56424458.99241672611486679803334485488&name=%D0%9A%D1%80%D0%B8%D1%82%D0%B5%D1%80%D0%B8%D0%B8%20%D0%BE%D1%86%D0%B5%D0%BD%D0%B8%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F%20%D0%BE%D1%80%D0%BB%D0%BE%D0%B2%D0%BE%D0%B9.rar" );
  QCOMPARE( percentEncodedQuery.prettyUrl(), QString::fromUtf8("http://mail.yandex.ru/message_part/Критерии оценивания орловой.rar?hid=1.1&mid=391.56424458.99241672611486679803334485488&name=Критерии оценивания орловой.rar"));

#ifdef Q_WS_WIN
#ifdef Q_CC_MSVC
#pragma message ("port KUser")
#else
#warning port KUser
#endif
#else
  KUrl tilde;
  KUser currentUser;
  const QString userName = currentUser.loginName();
  QVERIFY( !userName.isEmpty() );
  tilde.setPath( QString::fromUtf8( "~%1/Matériel" ).arg( userName ) );
  QString homeDir = currentUser.homeDir();
  QCOMPARE( tilde.url(), QString("file://%1/Mat%C3%A9riel").arg(homeDir));
#endif
}

void KUrlTest::testHostName()
{
    KUrl u1("http://www.Abc.de/FR");
    QCOMPARE(u1.host(), QString("www.abc.de")); // lowercase
    QCOMPARE(u1.path(), QString("/FR")); // lowercase
    QCOMPARE(u1.url(), QString("http://www.abc.de/FR")); // hostname is lowercase
    QCOMPARE(u1.prettyUrl(), QString("http://www.abc.de/FR")); // hostname is lowercase

    KUrl u2;
    u2.setProtocol("http");
    u2.setHost("www.Abc.de");
    QCOMPARE(u2.host(), QString("www.abc.de")); // lowercase
    QCOMPARE(u2.url(), QString("http://www.abc.de")); // lowercase

    KUrl u3("donkey://Abc/DE");
    QCOMPARE(u3.host(), QString("abc")); // lowercase
    QCOMPARE(u3.url(), QString("donkey://abc/DE")); // lowercase
}

void KUrlTest::testEmptyQueryOrRef()
{
  QUrl url = QUrl::fromEncoded( "http://www.kde.org" );
  QCOMPARE( url.toEncoded(), QByteArray( "http://www.kde.org" ) );
  QCOMPARE( url.encodedQuery(), QByteArray() );
  url = QUrl::fromEncoded( "http://www.kde.org?" );
  QCOMPARE( url.toEncoded(), QByteArray( "http://www.kde.org?" ) );
  QCOMPARE( url.encodedQuery(), QByteArray() ); // note that QByteArray() == QByteArray("")

  url = QUrl::fromEncoded( "http://www.kde.org" );
  QVERIFY( url.encodedQuery().isEmpty() );
  QVERIFY( !url.hasQuery() );
  url = QUrl::fromEncoded( "http://www.kde.org?" );
  QVERIFY( !url.encodedQuery().isNull() );
  QVERIFY( url.encodedQuery().isEmpty() );
  QVERIFY( url.hasQuery() );

  KUrl noQuery( "http://www.kde.org");
  QCOMPARE( noQuery.query(), QString() ); // query at all
  QVERIFY( !noQuery.hasQuery() );
  QVERIFY( !noQuery.hasRef());
  QVERIFY( noQuery.ref().isNull() );

  // Empty queries should be preserved!
  QUrl qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi?", QUrl::TolerantMode);
  QCOMPARE( qurl.toEncoded(), QByteArray("http://www.kde.org/cgi/test.cgi?"));

  KUrl waba1( "http://www.kde.org/cgi/test.cgi?");
  QCOMPARE( waba1.url(), QString( "http://www.kde.org/cgi/test.cgi?" ) );
  QCOMPARE( waba1.query(), QString( "?" ) ); // empty query
  QVERIFY( waba1.hasQuery() );

  // Empty references should be preserved
  waba1 = "http://www.kde.org/cgi/test.cgi#";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi#") );
  QVERIFY( waba1.hasRef() );
  QVERIFY( waba1.hasFragment() );
  QVERIFY( waba1.hasHTMLRef() );
  QCOMPARE( waba1.encodedHtmlRef(), QString() );
  //qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi#", QUrl::TolerantMode);
  //QCOMPARE( qurl.toEncoded(), QByteArray("http://www.kde.org/cgi/test.cgi#") );

  KUrl tobi1( "http://host.net/path/?#http://broken-adsfk-poij31-029mu-2890zupyc-*!*'O-+-0i" );
  QCOMPARE(tobi1.query(), QString("?")); // query is empty
  QVERIFY( tobi1.hasQuery() );

  tobi1 = "http://host.net/path/#no-query";
  QCOMPARE(tobi1.query(), QString("")); // no query
  QVERIFY( !tobi1.hasQuery() );
}

void KUrlTest::testParsingTolerance()
{
  // URLs who forgot to encode spaces in the query - the QUrl version first
  QUrl incorrectEncoded = QUrl::fromEncoded( "http://www.kde.org/cgi/qurl.cgi?hello=My Value" );
  QVERIFY( incorrectEncoded.isValid() );
  QVERIFY( !incorrectEncoded.toEncoded().isEmpty() );
  //qDebug( "%s", incorrectEncoded.toEncoded().data() );
  QCOMPARE( incorrectEncoded.toEncoded(),
           QByteArray("http://www.kde.org/cgi/qurl.cgi?hello=My%20Value") );

  // URLs who forgot to encode spaces in the query.
  KUrl waba1( "http://www.kde.org/cgi/test.cgi?hello=My Value" );
  //QVERIFY( waba1.isValid() );
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello=My%20Value") );

  // URL with ':' in query (':' should NOT be encoded!)
  waba1 = "http://www.kde.org/cgi/test.cgi?hello:My Value";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello:My%20Value") );
  QCOMPARE( waba1.upUrl().url(), QString("http://www.kde.org/cgi/test.cgi") );

  // URLs who forgot to encode spaces in the query.
  waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value+20";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello=My%20Value+20") );
}

void KUrlTest::testNewLine()
{
  QUrl qurl_newline_1 = QUrl::fromEncoded( "http://www.foo.bar/foo/bar\ngnork", QUrl::TolerantMode );
  QVERIFY( qurl_newline_1.isValid() );
  QCOMPARE( qurl_newline_1.toEncoded(), QByteArray("http://www.foo.bar/foo/bar%0Agnork") );

  KUrl url_newline_1("http://www.foo.bar/foo/bar\ngnork");
  QCOMPARE( url_newline_1.url(), QLatin1String("http://www.foo.bar/foo/bar%0Agnork") );

  KUrl url_newline_2("http://www.foo.bar/foo?bar\ngnork");
  QCOMPARE( url_newline_2.url(), QLatin1String("http://www.foo.bar/foo?bar%0Agnork") );
}

void KUrlTest::testQueryParsing()
{
  KUrl ldap( "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)" );
  QCOMPARE( ldap.host(), QString("host.com") );
  QCOMPARE( ldap.port(), 6666 );
  QCOMPARE( ldap.path(), QString("/o=University of Michigan,c=US") );
  QCOMPARE( ldap.query(), QString("??sub?(cn=Babs%20Jensen)") );
  QCOMPARE( ldap.url(), QString("ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)") );
  ldap.setQuery("??sub?(cn=Karl%20Marx)");
  QCOMPARE( ldap.query(), QString("??sub?(cn=Karl%20Marx)") );
  QCOMPARE( ldap.url(), QString("ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Karl%20Marx)") );
}

void KUrlTest::testURLsWithoutPath()
{
  // Urls without path (BR21387)
  KUrl waba1( "http://meine.db24.de?link=home_c_login_login" ); // has query
  QCOMPARE( waba1.url(), QString("http://meine.db24.de?link=home_c_login_login") );
  QCOMPARE( waba1.path(), QString("") );
  QCOMPARE( waba1.query(), QString("?link=home_c_login_login") );

  waba1 = "http://a:389?b=c"; // has port and query
  QCOMPARE( waba1.url(), QString( "http://a:389?b=c" ) );
  QCOMPARE( waba1.host(), QString( "a" ) );
  QCOMPARE( waba1.port(), 389 );
  QCOMPARE( waba1.path(), QString( "" ) );
  QCOMPARE( waba1.query(), QString( "?b=c" ) );

  // Urls without path (BR21387)
  waba1 = "http://meine.db24.de#link=home_c_login_login"; // has fragment
  QCOMPARE( waba1.url(), QString("http://meine.db24.de#link=home_c_login_login") );
  QCOMPARE( waba1.path(), QString(""));

  waba1 = "http://a:389#b=c"; // has port and fragment
  //qDebug( "%s", qPrintable( waba1.url() ) );
  QCOMPARE( waba1.scheme(), QString( "http" ) );
  QCOMPARE( waba1.url(), QString( "http://a:389#b=c" ) );
  QCOMPARE( waba1.host(), QString( "a" ) );
  QCOMPARE( waba1.port(), 389 );
  QCOMPARE( waba1.path(), QString( "" ) );
  QCOMPARE( waba1.ref(), QString( "b%3Dc" ) ); // was b=c with KDE3, but the docu says encoded, so encoding the = is ok
  QCOMPARE( waba1.htmlRef(), QString( "b=c" ) );
  QCOMPARE( waba1.query(), QString() );

  QUrl schemeOnly( "ftp:" );
  QVERIFY( schemeOnly.isValid() );
  QCOMPARE( schemeOnly.scheme(), QString( "ftp" ) );
}

void KUrlTest::testPathAndQuery()
{
#if 0
  // this KDE3 test fails, but Tobias Anton didn't say where it came from, and Andreas Hanssen (TT) says:
  // "I can't see any reason to support this; it looks like a junk artifact from older days.
  // Everything after # is the fragment. Parsing what comes after # is broken; tolerant or not."
  KUrl tobi0("http://some.host.net/path/to/file#fragmentPrecedes?theQuery");
  QCOMPARE( tobi0.ref(), QString("fragmentPrecedes") );
  QCOMPARE( tobi0.query(), QString("?theQuery") );
#endif

  KUrl tobi1( "http://host.net/path?myfirstquery#andsomeReference" );
  tobi1.setEncodedPathAndQuery("another/path/?another&query");
  QCOMPARE( tobi1.query(), QString("?another&query") );
  QCOMPARE( tobi1.path(), QString("another/path/") ); // with trailing slash
  QCOMPARE( tobi1.encodedPathAndQuery(), QString( "another/path/?another&query" ) );
  tobi1.setEncodedPathAndQuery("another/path?another&query");
  QCOMPARE( tobi1.query(), QString("?another&query") );
  QCOMPARE( tobi1.path(), QString("another/path") ); // without trailing slash
  QCOMPARE( tobi1.encodedPathAndQuery(), QString( "another/path?another&query" ) );

  tobi1 = "http://host.net/path/#no-query";
  QCOMPARE( tobi1.encodedPathAndQuery(), QString( "/path/" ) );

  KUrl kde( "http://www.kde.org" );
  QCOMPARE( kde.encodedPathAndQuery(), QString( "" ) );
  QCOMPARE( kde.encodedPathAndQuery( KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath ), QString( "/" ) );

  KUrl theKow( "http://www.google.de/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25" );
  QCOMPARE( theKow.encodedPathAndQuery(), QString( "/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25" ) );

  KUrl uloc( "file:///home/dfaure/konqtests/Mat%C3%A9riel" );
  QCOMPARE( uloc.encodedPathAndQuery(), QString( "/home/dfaure/konqtests/Mat%C3%A9riel" ) );

  KUrl urlWithAccent( "file:///home/dfaure/konqtests/Matériel" );
  QCOMPARE(urlWithAccent.encodedPathAndQuery(), QString("/home/dfaure/konqtests/Mat%C3%A9riel"));

  KUrl urlWithUnicodeChar(QString::fromUtf8("file:///home/dfaure/konqtests/Matériel"));
  QCOMPARE(urlWithUnicodeChar.encodedPathAndQuery(), QString("/home/dfaure/konqtests/Mat%C3%A9riel"));

  KUrl maelcum(QString::fromUtf8("http://a.b.c/äöu"));
  QCOMPARE(maelcum.encodedPathAndQuery(), QString("/%C3%A4%C3%B6u"));
}

void KUrlTest::testUpUrl()
{
  KUrl url1( "ftp://user%40host.com@ftp.host.com/var/www/" );
  QCOMPARE( url1.user(), QString("user@host.com" ) );
  QCOMPARE( url1.host(), QString("ftp.host.com" ) );
  KUrl up = url1.upUrl();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/var/") );
  up = up.upUrl();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/") );
  up = up.upUrl();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/") ); // unchanged

  // Going up from a relative url is not supported (#170695)
  KUrl invalid("tmp");
  QVERIFY(invalid.isValid());
  up = invalid.upUrl();
  QVERIFY(!up.isValid());
}

void KUrlTest::testSetFileName() // and addPath
{
  KUrl u2( "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README" );
  //qDebug( "* URL is %s", qPrintable( u2.url() ) );

  u2.setFileName( "myfile.txt" );
  QCOMPARE( u2.url(), QString("file:///home/dfaure/myfile.txt") );
  u2.setFileName( "myotherfile.txt" );
  QCOMPARE( u2.url(), QString("file:///home/dfaure/myotherfile.txt") );

  // more tricky, renaming a directory (kpropertiesdialog.cc)
  QString tmpurl = "file:/home/dfaure/myolddir/";
  if ( tmpurl.at(tmpurl.length() - 1) == '/')
      // It's a directory, so strip the trailing slash first
      tmpurl.truncate( tmpurl.length() - 1);
  KUrl newUrl(tmpurl);
  newUrl.setFileName( "mynewdir" );
  QCOMPARE( newUrl.url(), QString("file:///home/dfaure/mynewdir") );

  // addPath tests
  newUrl.addPath( "subdir" );
  QCOMPARE( newUrl.url(), QString("file:///home/dfaure/mynewdir/subdir") );
  newUrl.addPath( "/foo/" );
  QCOMPARE( newUrl.url(), QString("file:///home/dfaure/mynewdir/subdir/foo/") );
  u2 = "http://www.kde.org"; // no path
  u2.addPath( "subdir" );
  QCOMPARE( u2.url(), QString("http://www.kde.org/subdir") );
  u2.addPath( "" );
  QCOMPARE( u2.url(), QString("http://www.kde.org/subdir") ); // unchanged

  QUrl qurl2 = QUrl::fromEncoded( "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)", QUrl::TolerantMode );
  QCOMPARE( qurl2.path(), QString::fromLatin1("/specials/Print To File (PDF%2FAcrobat)") );
  QCOMPARE( qurl2.toEncoded(), QByteArray("print:/specials/Print%20To%20File%20(PDF%252FAcrobat)") );

  // even more tricky
  u2 = "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)";
  QCOMPARE( u2.path(), QString("/specials/Print To File (PDF%2FAcrobat)") );
  QCOMPARE( u2.fileName(), QString("Print To File (PDF%2FAcrobat)") );
  u2.setFileName( "" );
  QCOMPARE( u2.url(), QString("print:/specials/") );

  u2 = "file:/specials/Print";
  QCOMPARE( u2.path(), QString("/specials/Print") );
  QCOMPARE( u2.fileName(), QString("Print") );
  u2.setFileName( "" );
  QCOMPARE( u2.url(), QString("file:///specials/") );

  const char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  QVERIFY( !KUrl(u3).hasSubUrl() );

  KUrl::List lst = KUrl::split( KUrl(u3) );
  QCOMPARE( lst.count(), 1 );
  QCOMPARE( lst.first().url(), QString("ftp://host/dir1/dir2/myfile.txt") );

  // cdUp code
  KUrl lastUrl = lst.last();
  QString dir = lastUrl.directory();
  QCOMPARE(  dir, QString("/dir1/dir2") );
}

void KUrlTest::testDirectory()
{
  KUrl udir;
  udir.setPath("/home/dfaure/file.txt");
  //qDebug( "URL is %s", qPrintable( udir.url() ) );
  QCOMPARE( udir.path(), QString("/home/dfaure/file.txt") );
  QCOMPARE( udir.url(), QString("file:///home/dfaure/file.txt") );
  QCOMPARE( udir.directory(KUrl::AppendTrailingSlash|KUrl::ObeyTrailingSlash), QString("/home/dfaure/") );
  QCOMPARE( udir.directory(KUrl::ObeyTrailingSlash), QString("/home/dfaure") );

  KUrl u2( QByteArray("file:///home/dfaure/") );
  // not ignoring trailing slash
  QCOMPARE( u2.directory(KUrl::AppendTrailingSlash|KUrl::ObeyTrailingSlash), QString("/home/dfaure/") );
  QCOMPARE( u2.directory(KUrl::ObeyTrailingSlash), QString("/home/dfaure") );
  // ignoring trailing slash
  QCOMPARE( u2.directory(KUrl::AppendTrailingSlash), QString("/home/") );
  QCOMPARE( u2.directory(), QString("/home") );

  // cleanPath() tests (before cd() since cd uses that)
  u2.cleanPath();
  QCOMPARE( u2.url(), QString("file:///home/dfaure/") );
  u2.addPath( "/..//foo" );
  QCOMPARE( u2.url(), QString("file:///home/dfaure/..//foo") );
  u2.cleanPath(KUrl::KeepDirSeparators);
  QCOMPARE( u2.url(), QString("file:///home//foo") );
  u2.cleanPath(KUrl::SimplifyDirSeparators);
  QCOMPARE( u2.url(), QString("file:///home/foo") );

  // cd() tests
  u2.cd("..");
  QCOMPARE( u2.url(), QString("file:///home") );
  u2.cd("thomas");
  QCOMPARE( u2.url(), QString("file:///home/thomas") );
  u2.cd("../");
  QCOMPARE( u2.url(), QString("file:///home/") );
  u2.cd("/opt/kde/bin/");
  QCOMPARE( u2.url(), QString("file:///opt/kde/bin/") );
  u2 = "ftp://ftp.kde.org/";
  u2.cd("pub");
  QCOMPARE( u2.url(), QString("ftp://ftp.kde.org/pub") );
  u2 = u2.upUrl();
  QCOMPARE( u2.url(), QString("ftp://ftp.kde.org/") );
}

void KUrlTest::testPrettyURL()
{
  KUrl notPretty("http://ferret.lmh.ox.ac.uk/%7Ekdecvs/");
  QCOMPARE( notPretty.prettyUrl(), QString("http://ferret.lmh.ox.ac.uk/~kdecvs/") );
  KUrl notPretty2("file:/home/test/directory%20with%20spaces");
  QCOMPARE( notPretty2.prettyUrl(), QString("file:///home/test/directory with spaces") );

  KUrl notPretty3("fish://foo/%23README%23");
  QCOMPARE( notPretty3.prettyUrl(), QString("fish://foo/%23README%23") );
  KUrl url15581("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  QCOMPARE( url15581.prettyUrl(), QString("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html") );
  QCOMPARE( url15581.url(), QString("http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html") );

  // KDE3 test was for parsing "percentage%in%url.html", but this is not supported; too broken.
  KUrl url15581bis("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html");
  QCOMPARE( url15581bis.prettyUrl(), QString("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html") );
  QCOMPARE( url15581bis.url(), QString("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html") );

  KUrl urlWithPass("ftp://user:password@ftp.kde.org/path");
  QCOMPARE( urlWithPass.pass(), QString::fromLatin1( "password" ) );
  QCOMPARE( urlWithPass.prettyUrl(), QString::fromLatin1( "ftp://user@ftp.kde.org/path" ) );

  KUrl xmppUri("xmpp:ogoffart@kde.org");
  QCOMPARE( xmppUri.prettyUrl(), QString::fromLatin1( "xmpp:ogoffart@kde.org" ) );

  KUrl openWithUrl("kate --use %25U");
  QCOMPARE(openWithUrl.url(), QString::fromLatin1("kate%20--use%20%25U"));
  QCOMPARE(openWithUrl.prettyUrl(), QString::fromLatin1("kate --use %25U"));
  QCOMPARE(openWithUrl.path(), QString::fromLatin1("kate --use %U"));
  QCOMPARE(openWithUrl.pathOrUrl(), QString::fromLatin1("kate --use %25U")); // alas...
}

void KUrlTest::testIsRelative()
{
  QVERIFY( !KUrl::isRelativeUrl("man:mmap") );
  QVERIFY( !KUrl::isRelativeUrl("javascript:doSomething()") );
  QVERIFY( !KUrl::isRelativeUrl("file:///blah") );
  // arguable, but necessary for KUrl( baseURL, "//www1.foo.bar" );
  QVERIFY( KUrl::isRelativeUrl("/path") );
  QVERIFY( KUrl::isRelativeUrl("something") );

  KUrl something("something");
  QCOMPARE(something.url(), QString("something"));
  QCOMPARE(something.protocol(), QString());
  QVERIFY(!something.isLocalFile());
}

void KUrlTest::testRelativePath()
{
  QString basePath = "/home/bastian";

  QCOMPARE( KUrl::relativePath(basePath, "/home/bastian"), QString("./") );
  bool b;
  QCOMPARE( KUrl::relativePath(basePath, "/home/bastian/src/plugins", &b), QString("./src/plugins"));
  QVERIFY( b );
  QCOMPARE( KUrl::relativePath(basePath, "./src/plugins"), QString("./src/plugins") );
  QCOMPARE( KUrl::relativePath(basePath, "/home/waba/src/plugins", &b), QString("../waba/src/plugins") );
  QVERIFY( !b );
  QCOMPARE( KUrl::relativePath(basePath, "/"), QString("../../"));

  QCOMPARE( KUrl::relativePath("/", "/"), QString("./") );
  QCOMPARE( KUrl::relativePath("/", "/home/bastian"), QString("./home/bastian") );
  QCOMPARE( KUrl::relativePath("", "/home/bastian"), QString("/home/bastian") );
}


void KUrlTest::testRelativeURL()
{
  KUrl baseURL( "http://www.kde.org/index.html" );
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("http://www.kde.org/index.html#help") ), QString("#help") );
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("http://www.kde.org/index.html?help=true") ), QString("index.html?help=true") );
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("http://www.kde.org/contact.html") ), QString("contact.html") );
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("ftp://ftp.kde.org/pub/kde") ), QString("ftp://ftp.kde.org/pub/kde") );
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("http://www.kde.org/index.html") ), QString("./") );

  baseURL = "http://www.kde.org/info/index.html";
  QCOMPARE( KUrl::relativeUrl(baseURL, KUrl("http://www.kde.org/bugs/contact.html") ), QString( "../bugs/contact.html") );
}

void KUrlTest::testAdjustPath()
{
    KUrl url0("file:///");
    QCOMPARE( url0.url( KUrl::RemoveTrailingSlash ), QString("file:///") );
    url0.adjustPath( KUrl::RemoveTrailingSlash );
    QCOMPARE( url0.url(), QString("file:///") );

    KUrl url1("file:///home/kde/");
    url1.adjustPath( KUrl::LeaveTrailingSlash );
    QCOMPARE(  url1.path(), QString("/home/kde/" ) );
    url1.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE(  url1.path(), QString("/home/kde" ) );
    url1.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE(  url1.path(), QString("/home/kde" ) );
    url1.adjustPath(KUrl::AddTrailingSlash);
    QCOMPARE(  url1.path(), QString("/home/kde/" ) );

    KUrl url2("file:///home/kde//");
    url2.adjustPath(KUrl::LeaveTrailingSlash);
    QCOMPARE(  url2.path(), QString("/home/kde//" ) );
    url2.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE(  url2.path(), QString("/home/kde" ) );
    url2.adjustPath(KUrl::AddTrailingSlash);
    QCOMPARE(  url2.path(), QString("/home/kde/" ) );

    KUrl ftpurl1("ftp://ftp.kde.org/");
    ftpurl1.adjustPath(KUrl::LeaveTrailingSlash);
    QCOMPARE(  ftpurl1.path(), QString("/" ) );
    ftpurl1.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE(  ftpurl1.path(), QString("/" ) );

    KUrl ftpurl2("ftp://ftp.kde.org///");
    ftpurl2.adjustPath(KUrl::LeaveTrailingSlash);
    QCOMPARE(  ftpurl2.path(), QString("///" ) );
    ftpurl2.adjustPath(KUrl::RemoveTrailingSlash); // should remove all but trailing slash
    QCOMPARE(  ftpurl2.path(), QString("/" ) );
    ftpurl2.adjustPath(KUrl::AddTrailingSlash);
    QCOMPARE(  ftpurl2.path(), QString("/" ) );

    // Equivalent tests written by the KDirLister maintainer :)

    KUrl u3( QByteArray("ftp://brade@ftp.kde.org///") );
    u3.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE( u3.url(), QString("ftp://brade@ftp.kde.org/") );

    KUrl u4( QByteArray("ftp://brade@ftp.kde.org/kde///") );
    u4.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE( u4.url(), QString("ftp://brade@ftp.kde.org/kde") );

    // applying adjustPath(KUrl::RemoveTrailingSlash) twice should not yield two different urls
    // (follows from the above test)
    KUrl u5 = u4;
    u5.adjustPath(KUrl::RemoveTrailingSlash);
    QCOMPARE( u5.url(), u4.url() );

    {
    KUrl remote("remote:/");
    QCOMPARE( remote.url(KUrl::RemoveTrailingSlash ), QString("remote:/") );
    remote.adjustPath( KUrl::RemoveTrailingSlash );
    QCOMPARE( remote.url(), QString("remote:/") );
    remote.adjustPath( KUrl::RemoveTrailingSlash );
    QCOMPARE( remote.url(), QString("remote:/") );
    }

    {
    KUrl remote2("remote://");
    QCOMPARE( remote2.url(), QString("remote:") );
    QCOMPARE( remote2.url(KUrl::RemoveTrailingSlash ), QString("remote:") );
    }
}

void KUrlTest::testIPV6()
{
  // IPV6
  KUrl waba1( "http://[::FFFF:129.144.52.38]:81/index.html" );
  QCOMPARE( waba1.host(), QString("::ffff:129.144.52.38") );
  QCOMPARE( waba1.port(), 81 );

  // IPV6
  waba1 = "http://waba:pass@[::FFFF:129.144.52.38]:81/index.html";
  QCOMPARE( waba1.host(), QString("::ffff:129.144.52.38") );
  QCOMPARE( waba1.user(), QString("waba") );
  QCOMPARE( waba1.pass(), QString("pass") );
  QCOMPARE( waba1.port(), 81 );

  // IPV6
  waba1 = "http://www.kde.org/cgi/test.cgi";
  waba1.setHost("::ffff:129.144.52.38");
  QCOMPARE( waba1.url(), QString("http://[::ffff:129.144.52.38]/cgi/test.cgi") );
  waba1 = "http://[::ffff:129.144.52.38]/cgi/test.cgi";
  QVERIFY( waba1.isValid() );

  // IPV6 without path
  waba1 = "http://[::ffff:129.144.52.38]?query";
  QVERIFY( waba1.isValid() );
  QCOMPARE( waba1.url(), QString("http://[::ffff:129.144.52.38]?query") );
  QCOMPARE( waba1.query(), QString("?query") );
  waba1 = "http://[::ffff:129.144.52.38]#ref";
  QVERIFY( waba1.isValid() );
  QCOMPARE( waba1.url(), QString("http://[::ffff:129.144.52.38]#ref") );
  QCOMPARE( waba1.ref(), QString("ref") );
  // IPV6 without path but with a port
  waba1 = "http://[::ffff:129.144.52.38]:81?query";
  QVERIFY( waba1.isValid() );
  QCOMPARE( waba1.url(), QString("http://[::ffff:129.144.52.38]:81?query") );
  QCOMPARE( waba1.port(), 81 );
  QCOMPARE( waba1.query(), QString("?query") );
  waba1 = "http://[::ffff:129.144.52.38]:81#ref";
  QVERIFY( waba1.isValid() );
  QCOMPARE( waba1.url(), QString("http://[::ffff:129.144.52.38]:81#ref") );
  QCOMPARE( waba1.port(), 81 );
  QCOMPARE( waba1.ref(), QString("ref") );

  KUrl weird( "http://[::fff:1:23]/" );
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("::fff:1:23") );
}

void KUrlTest::testBaseURL() // those are tests for the KUrl(base,relative) constructor
{
  KUrl com1(KUrl("http://server.com/dir/"), ".");
  QCOMPARE( com1.url(), QString("http://server.com/dir/") );

  KUrl com2(KUrl("http://server.com/dir/blubb/") , "blah/");
  QCOMPARE( com2.url(), QString("http://server.com/dir/blubb/blah/") );

  KUrl baseURL("http://www.foo.bar:80" );
  QVERIFY( baseURL.isValid() );
  QCOMPARE( baseURL.protocol(), QString( "http" ) ); // lowercase
  QCOMPARE( baseURL.port(), 80 );

  QString relativeUrl = "//www1.foo.bar";
  QVERIFY( KUrl::isRelativeUrl( relativeUrl ) );

  // Mimick what KUrl(2 urls) does:
  QUrl qurl;
  qurl = "http://www.foo.bar:80";
  QCOMPARE( qurl.toEncoded(), QByteArray("http://www.foo.bar:80") );

  qurl.setHost( QString() );
  qurl.setPath( QString() );
  QCOMPARE( qurl.toEncoded(), QByteArray("http://:80") );
  qurl.setPort( -1 );
  QCOMPARE( qurl.toEncoded(), QByteArray("http:") ); // hmm we have no '//' anymore

  KUrl url1 ( baseURL, relativeUrl );
  QCOMPARE( url1.url(), QString("http://www1.foo.bar"));
  QCOMPARE( url1.host(), QString("www1.foo.bar"));

  baseURL = "http://www.foo.bar";
  KUrl rel_url( baseURL, "/top//test/../test1/file.html" );
  QCOMPARE( rel_url.url(), QString("http://www.foo.bar/top//test1/file.html" ));


  baseURL = "http://www.foo.bar/top//test2/file2.html";
  QCOMPARE( baseURL.url(), QString("http://www.foo.bar/top//test2/file2.html" ));

  baseURL = "file:/usr/local/src/kde2/////kdelibs/kio";
  QCOMPARE( baseURL.url(), QString("file:///usr/local/src/kde2/////kdelibs/kio" ));

  baseURL = "http://www.foo.bar";
  KUrl rel_url2( baseURL, "mailto:bastian@kde.org" );
  QCOMPARE( rel_url2.url(), QString("mailto:bastian@kde.org" ));

  baseURL = "mailto:bastian@kde.org?subject=hello";
  QCOMPARE( baseURL.url(), QString("mailto:bastian@kde.org?subject=hello" ));

  baseURL = "file:/usr/local/src/kde2/kdelibs/kio/";
  KUrl url2( baseURL, "../../////kdebase/konqueror" );
  QCOMPARE( url2.url(), QString("file:///usr/local/src/kde2/////kdebase/konqueror" ));


  // WABA: The following tests are to test the handling of relative URLs as
  //       found on web-pages.

  KUrl waba1( "http://www.website.com/directory/?hello#ref" );
  {
     KUrl waba2( waba1, "relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html") );
  }
  {
     KUrl waba2( waba1, "../relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  {
     KUrl waba2( waba1, "down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/down/relative.html") );
  }
  {
     KUrl waba2( waba1, "/down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/down/relative.html") );
  }
  {
     KUrl waba2( waba1, "//www.kde.org/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.kde.org/relative.html") );
  }
  {
     KUrl waba2( waba1, "relative.html?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?query=test&name=harry") );
     waba2.removeQueryItem("query");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?name=harry") );
     waba2.addQueryItem("age", "18");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?name=harry&age=18") );
     waba2.addQueryItem("age", "21");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?name=harry&age=18&age=21") );
     waba2.addQueryItem("fullname", "Harry Potter");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?name=harry&age=18&age=21&fullname=Harry%20Potter") );
  }
  {
     KUrl waba2( waba1, "?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?query=test&name=harry") );
  }
  {
     KUrl waba2( waba1, "relative.html#with_reference");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html#with_reference") );
  }

  {
     KUrl waba2( waba1, "#");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?hello#") );
  }
  {
     KUrl waba2( waba1, "");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?hello#ref") );
  }
  {
     KUrl base( "http://faure@www.kde.org" ); // no path
     KUrl waba2( base, "filename.html");
     QCOMPARE( waba2.url(), QString("http://faure@www.kde.org/filename.html") );
  }
  {
     KUrl base( "http://faure:pass@www.kde.org:81?query" );
     KUrl rel1( base, "http://www.kde.org/bleh/"); // same host
     QCOMPARE( rel1.url(), QString("http://faure:pass@www.kde.org/bleh/") );
     KUrl rel2( base, "http://www.yahoo.org"); // different host
     QCOMPARE( rel2.url(), QString("http://www.yahoo.org") );
  }

  waba1 = "http://www.website.com/directory/filename?bla#blub";
  {
     KUrl waba2( waba1, "relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html") );
  }
  {
     KUrl waba2( waba1, "./relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html") );
  }
  {
     KUrl waba2( waba1, "../relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  {
     KUrl waba2( waba1, "down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/down/relative.html") );
  }
  {
     KUrl waba2( waba1, "down/./relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/down/relative.html") );
  }
  {
     KUrl waba2( waba1, "/down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/down/relative.html") );
  }
  {
     KUrl waba2( waba1, "relative.html?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?query=test&name=harry") );
  }
  {
     KUrl waba2( waba1, "?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/filename?query=test&name=harry") );
  }
  {
     KUrl waba2( waba1, "relative.html#with_reference");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html#with_reference") );
  }
  {
      KUrl waba2( waba1, "http:/relative.html"); // "rfc 1606 loophole"
      QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  waba1.setUser("waldo");
  QCOMPARE( waba1.url(), QString("http://waldo@www.website.com/directory/filename?bla#blub") );
  waba1.setUser("waldo/bastian");
  QCOMPARE( waba1.url(), QString("http://waldo%2Fbastian@www.website.com/directory/filename?bla#blub") );
  waba1.setRef( QString() );
  waba1.setPass( "pass" );
  waba1.setDirectory( "/foo" );
  waba1.setProtocol( "https" );
  waba1.setHost( "web.com" );
  waba1.setPort( 881 );
  QCOMPARE( waba1.url(), QString("https://waldo%2Fbastian:pass@web.com:881/foo/?bla") );
  waba1.setDirectory( "/foo/" );
  QCOMPARE( waba1.url(), QString("https://waldo%2Fbastian:pass@web.com:881/foo/?bla") );

  QUrl sadEagleTest;
  sadEagleTest.setEncodedUrl( "http://www.calorieking.com/foo.php?P0=[2006-3-8]", QUrl::TolerantMode );
  QVERIFY( sadEagleTest.isValid() );
  KUrl sadEagleExpectedResult( "http://www.calorieking.com/personal/diary/rpc.php?C=jsrs1&F=getDiaryDay&P0=[2006-3-8]&U=1141858921458" );
  QVERIFY( sadEagleExpectedResult.isValid() );
  KUrl sadEagleBase( "http://www.calorieking.com/personal/diary/" );
  QVERIFY( sadEagleBase.isValid() );
  KUrl sadEagleCombined( sadEagleBase, "/personal/diary/rpc.php?C=jsrs1&F=getDiaryDay&P0=[2006-3-8]&U=1141858921458" );
  QCOMPARE( sadEagleCombined.url(), sadEagleExpectedResult.url() );

  KUrl dxOffEagle( KUrl("http://something/other.html"), "newpage.html?[{\"foo: bar\"}]" );
  //QEXPECT_FAIL("","Issue N183630, task ID 183874", Continue); // Fixed by _setEncodedUrl
  QVERIFY(dxOffEagle.isValid());
  //QEXPECT_FAIL("","Issue N183630, task ID 183874", Continue); // Fixed by _setEncodedUrl
  QCOMPARE(dxOffEagle.url(), QString("http://something/newpage.html?%5B%7B%22foo:%20bar%22%7D%5D") );

  // Shows up in nspluginviewer/flash
  QString flashRel = "javascript:window.location+\"__flashplugin_unique__\"";
  KUrl flashUrl(flashRel);
  QVERIFY(flashUrl.isValid());
  KUrl flashBase("http://www.youtube.com/?v=JvOSnRD5aNk");
  KUrl flashComposed(flashBase, flashRel);
  QCOMPARE(flashComposed.url(), QString("javascript:window.location+%22__flashplugin_unique__%22"));
}

void KUrlTest::testSubURL()
{
  QString u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  KUrl url1(u1);
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/%23tar:/%23myref") ); // KDE3 would say # 3 times
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.isLocalFile() );  // Not strictly local!
  QVERIFY( url1.hasSubUrl() );
  //QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/") );

  const KUrl::List splitted = KUrl::split( url1 );
  QCOMPARE( splitted.count(), 3 );
  //kDebug() << splitted.toStringList();
  QCOMPARE( splitted[0].url(), QString("file:///home/dfaure/my%20tar%20file.tgz#myref") );
  QCOMPARE( splitted[1].url(), QString("gzip:/#myref") );
  QCOMPARE( splitted[2].url(), QString("tar:/#myref") );

  QSKIP( "Multiple sub urls not supported at the moment, due to #-escaping", SkipSingle );

  // FAILS, due to the #->%23 escaping, see the comment in KUrl::join.
  // But do we really want to keep the suburl stuff?
  KUrl rejoined = KUrl::join( splitted );
  QCOMPARE( rejoined.url(), url1.url() );

  u1 = "error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi";
  url1 = u1;
  QCOMPARE( url1.url(), QString("error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi") );
  QVERIFY( url1.hasSubUrl() );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.isLocalFile() );
  QVERIFY( !url1.hasHTMLRef() );

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/%23tar:/") ); // KDE3: #tar
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/") );

  u1 = "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/%23tar:/") ); // KDE3: #tar
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/") );

#if 0
// This URL is broken, '#' should be escaped.
  u1 = "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#") );
  QVERIFY( !url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#") );
#endif

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/%23tar:/README") );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubUrl() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upUrl().url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/%23tar:/") );
}

void KUrlTest::testSetUser()
{
  // The KUrl equality test below works because in Qt4 null == empty.
  QString str1;
  QString str2 = "";
  QVERIFY( str1 == str2 );

  KUrl emptyUserTest1( "http://www.foobar.com/");
  QVERIFY( emptyUserTest1.user().isEmpty() );
  QVERIFY( emptyUserTest1.user().isNull() ); // Expected result. This was fixed in Qt-4.4
  KUrl emptyUserTest2( "http://www.foobar.com/");
  emptyUserTest2.setUser( "" );
  //QVERIFY( emptyUserTest2.user().isNull() );
  QCOMPARE( emptyUserTest1==emptyUserTest2?"TRUE":"FALSE","TRUE" );
  emptyUserTest2.setPass( "" );
  QCOMPARE( emptyUserTest1==emptyUserTest2?"TRUE":"FALSE","TRUE" );
  emptyUserTest2.setUser( "foo" );
  QCOMPARE( emptyUserTest2.user(), QString::fromLatin1( "foo" ) );
  emptyUserTest2.setUser( QString() );
  QCOMPARE( emptyUserTest1==emptyUserTest2, true );

  KUrl uga("ftp://ftp.kde.org");
  uga.setUser("foo@bar");
  QCOMPARE(uga.user(), QString::fromLatin1("foo@bar"));
  QCOMPARE(uga.url(), QString::fromLatin1("ftp://foo%40bar@ftp.kde.org"));
}

void KUrlTest::testComparisons()
{
  /// Comparisons
  QString ucmp1 = "ftp://ftp.de.kde.org/dir";
  QString ucmp2 = "ftp://ftp.de.kde.org/dir/";
  QVERIFY( !urlcmp(ucmp1,ucmp2) );

  /* QUrl version of it */
  QUrl u1( "ftp://ftp.de.kde.org/dir" );
  QUrl u2( "ftp://ftp.de.kde.org/dir/" );
  QUrl::FormattingOptions options = QUrl::None;
  options |= QUrl::StripTrailingSlash;
  QString str1 = u1.toString(options);
  QString str2 = u2.toString(options);
  QCOMPARE( str1, u1.toString() );
  QCOMPARE( str2, u1.toString() );
  bool same = str1 == str2;
  QVERIFY( same );

  QVERIFY( urlcmp(ucmp1,ucmp2,KUrl::CompareWithoutTrailingSlash) ); //only slash difference, ignore_trailing

  QString ucmp3 = "ftp://ftp.de.kde.org/dir/#";
  QVERIFY( !urlcmp(ucmp2,ucmp3) ); // (only hash difference)
  QVERIFY( urlcmp(ucmp2,ucmp3,KUrl::CompareWithoutFragment) ); // (only hash difference, ignore_ref)
  QVERIFY( urlcmp(ucmp2,ucmp3,KUrl::CompareWithoutTrailingSlash | KUrl::CompareWithoutFragment) ); // (slash and hash difference, ignore_trailing, ignore_ref)
  QVERIFY( urlcmp("","",KUrl::CompareWithoutFragment) ); // (empty, empty)
  QVERIFY( urlcmp("","") ); // (empty, empty)
  QVERIFY( !urlcmp("",ucmp1) ); // (empty, not empty)
  QVERIFY( !urlcmp("",ucmp1,KUrl::CompareWithoutFragment) ); // (empty, not empty)
  QVERIFY( !urlcmp("file",ucmp1) ); // (malformed, not empty)
  QVERIFY( !urlcmp("file",ucmp1,KUrl::CompareWithoutFragment) ); // (malformed, not empty)

  KUrl ftpUrl ( "ftp://ftp.de.kde.org" );
  QCOMPARE( ftpUrl.path(), QString());
  ftpUrl = "ftp://ftp.de.kde.org/";
  QVERIFY( ftpUrl.isParentOf( KUrl("ftp://ftp.de.kde.org/host/subdir/") ) );
  ftpUrl = "ftp://ftp/host/subdir/";
  QVERIFY( ftpUrl.isParentOf( KUrl("ftp://ftp/host/subdir/") ) );
  QVERIFY( ftpUrl.isParentOf( KUrl("ftp://ftp/host/subdir") ) );
  QVERIFY( !ftpUrl.isParentOf( KUrl("ftp://ftp/host/subdi") ) );
  QVERIFY( ftpUrl.isParentOf( KUrl("ftp://ftp/host/subdir/blah/") ) );
  QVERIFY( !ftpUrl.isParentOf( KUrl("ftp://ftp/blah/subdir") ) );
  QVERIFY( !ftpUrl.isParentOf( KUrl("file:////ftp/host/subdir/") ) );
  QVERIFY( ftpUrl.isParentOf( KUrl("ftp://ftp/host/subdir/subsub") ) );
}

void KUrlTest::testStreaming()
{
  // Streaming operators
  KUrl origURL( "http://www.website.com/directory/?#ref" );
  KUrl empty( "" );
  KUrl invalid( "ptal://mlc:usb" );
  QVERIFY( !invalid.isValid() );
  KUrl waba1( "http://[::ffff:129.144.52.38]:81?query" );
  QByteArray buffer;
  {
      QDataStream stream( &buffer, QIODevice::WriteOnly );
      stream << origURL
             << empty
             << invalid
             << waba1; // the IPv6 one
  }
  {
      QDataStream stream( buffer );
      KUrl restoredURL;
      stream >> restoredURL; // streaming valid url
      QCOMPARE( restoredURL.url(), origURL.url() );
      stream >> restoredURL; // streaming empty url
      QVERIFY( !restoredURL.isValid() );
      QVERIFY( restoredURL.isEmpty() );
      QCOMPARE( restoredURL.url(), QString("") );
      stream >> restoredURL; // streaming invalid url
      QVERIFY( !restoredURL.isValid() );
      // note that this doesn't say what url() returns, that's for testBrokenStuff
      QCOMPARE( restoredURL.url(), invalid.url() );
      stream >> restoredURL; // streaming ipv6 url with query
      QCOMPARE( restoredURL.url(), waba1.url() );
  }
}

void KUrlTest::testBrokenStuff()
{
  // Broken stuff
  KUrl waba1( "file:a" );
  QCOMPARE( waba1.path(), QString("a") );
  QCOMPARE( waba1.fileName(KUrl::ObeyTrailingSlash), QString("a") );
  QCOMPARE( waba1.fileName(), QString("a") );
  QCOMPARE( waba1.directory(KUrl::AppendTrailingSlash|KUrl::ObeyTrailingSlash), QString("") );
  QCOMPARE( waba1.directory(KUrl::ObeyTrailingSlash), QString("") );
  QCOMPARE( waba1.directory(), QString("") );

  waba1 = "file:a/";
  QCOMPARE( waba1.path(), QString("a/") );
  QCOMPARE( waba1.fileName(KUrl::ObeyTrailingSlash), QString("") );
  QCOMPARE( waba1.fileName(), QString("a") );
  QCOMPARE( waba1.directory(KUrl::ObeyTrailingSlash | KUrl::AppendTrailingSlash), QString("a/") );
  QCOMPARE( waba1.directory(KUrl::ObeyTrailingSlash), QString("a") );
  QCOMPARE( waba1.directory(), QString("") );

  waba1 = "file:";
  QVERIFY( !waba1.isEmpty() );
  QVERIFY( waba1.isValid() ); // KDE3: was invalid. Now it's qurl with scheme="file".
  QCOMPARE( waba1.path(), QString("") );
  QCOMPARE( waba1.fileName(KUrl::ObeyTrailingSlash), QString("") );
  QCOMPARE( waba1.fileName(), QString("") );
  QCOMPARE( waba1.directory(KUrl::ObeyTrailingSlash | KUrl::AppendTrailingSlash), QString("") );
  QCOMPARE( waba1.directory(KUrl::AppendTrailingSlash), QString("") );
  QCOMPARE( waba1.directory(), QString("") );
  KUrl broken;
  broken.setPath( QString() );
  QVERIFY( !broken.isEmpty() );
  // It's valid: because isValid refers to parsing, not to what happens afterwards.
  QVERIFY( broken.isValid() );
  QCOMPARE( broken.path(), QString("") );
  broken = "file://"; // just because coolo wondered
  QVERIFY( !broken.isEmpty() );
  QVERIFY( broken.isValid() ); // KDE3: was invalid; same as above
  QCOMPARE( broken.path(), QString("") );
  broken = "file";
  QVERIFY( broken.isValid() ); // KDE3: was invalid; now it's path="file"

#if 0
  // KUrl has a Q_ASSERT on this now, so we can't test it.
  broken = "/";
  QVERIFY( broken.isValid() );
  QCOMPARE( broken.path(), QString("/") );
  QCOMPARE( broken.url(), QString("/") ); // KDE3: was resolved to "file:///". QUrl supports urls without a protocol.
  QCOMPARE( broken.protocol(), QString("") ); // KDE3: was "file"
#endif

  {
      QUrl url;
      url.setEncodedUrl("LABEL=USB_STICK", QUrl::TolerantMode);
      QVERIFY( url.isValid() );
      QCOMPARE( url.path(), QString("LABEL=USB_STICK") );
      QVERIFY( !url.isEmpty() );
  }
  {
      QUrl url;
      url.setEncodedUrl("LABEL=USB_STICK", QUrl::TolerantMode);
      QVERIFY( url.isValid() );
      QVERIFY( !url.isEmpty() ); // Qt-4.4-snapshot20080213 bug, reported to TT
      QCOMPARE( url.path(), QString("LABEL=USB_STICK") );
  }

  broken = "LABEL=USB_STICK"; // 71430, can we use KUrl for this?
  QVERIFY( broken.isValid() ); // KDE3 difference: QUrl likes this one too
  QVERIFY( !broken.isEmpty() );
  QCOMPARE( broken.path(), QString("LABEL=USB_STICK") ); // was "" in KDE3
}

void KUrlTest::testMoreBrokenStuff()
{
#if 0 // BROKEN?
  // UNC like names
  KUrl unc1("FILE://localhost/home/root");
  QCOMPARE( unc1.path(), QString("/home/root") );
  QCOMPARE( unc1.url(), QString("file:///home/root") );
#endif
  KUrl unc2("file:///home/root"); // with empty host
  QCOMPARE( unc2.path(), QString("/home/root") );
  QCOMPARE( unc2.url(), QString("file:///home/root") );

  {
     KUrl unc3("FILE://remotehost/home/root");
#if 0 // BROKEN?
     QCOMPARE( unc3.path(), QString("//remotehost/home/root") );
#endif
     QCOMPARE( unc3.url(), QString("FILE://remotehost/home/root") ); // KDE3: file:// (lowercase)
     KUrl url2("file://atlas/dfaure");
     QCOMPARE( url2.host(), QString("atlas") );
     QCOMPARE( url2.path(), QString("/dfaure") );
     //QCOMPARE( url3.path(), QString("//atlas/dfaure")); // says Waba
     //KUrl url3("file:////atlas/dfaure");
     //QCOMPARE( url3.path(), QString("//atlas/dfaure")); // says Waba

     KUrl url4(url2, "//remotehost/home/root");
     QCOMPARE( url4.host(), QString("remotehost") );
     QCOMPARE( url4.path(), QString("/home/root") );
  }

  KUrl weird;
  weird = "http://strange<hostname>/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange<username>@strange<hostname>/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange<username>@ok_hostname/";
  QVERIFY( weird.isValid() ); // KDE3: was valid. Fixed by _setEncodedUrl.
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://strange;hostname/";
  QVERIFY( weird.isValid() ); // KDE3: was invalid. bah*2.

  weird = "http://strange;username@strange;hostname/";
  QVERIFY( weird.isValid() ); // KDE3: was invalid. bah*3.

  weird = "http://strange;username@ok_hostname/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://strange;username:password@strange;hostname/";
  QVERIFY( weird.isValid() ); // KDE3: was invalid

  weird = "http://strange;username:password@ok_hostname/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://[strange;hostname]/";
  QVERIFY( !weird.isValid() );

  weird = "ssh://user@machine?cmd='echo $HOSTNAME'";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("machine") );
  //qDebug("%s",qPrintable( weird.query() ) );
  QCOMPARE( weird.queryItem("cmd"), QString("'echo $HOSTNAME'") );

  weird = ":pictures"; // for KFileDialog's startDir
  QVERIFY( weird.isValid() );
  QVERIFY( weird.protocol().isEmpty() );
  QVERIFY( weird.host().isEmpty() );
  QCOMPARE( weird.path(), QString( "pictures" ) );
  QCOMPARE( weird.url(), QString( "pictures" ) ); // # BUG: the : is missing

  weird = "::keyword"; // for KFileDialog's startDir
  QVERIFY( weird.isValid() );
  QVERIFY( weird.protocol().isEmpty() );
  QVERIFY( weird.host().isEmpty() );
  QCOMPARE( weird.path(), QString( ":keyword" ) );
  QCOMPARE( weird.url(), QString( ":keyword" ) ); // # BUG: the : is missing

  KUrl broken;
  broken = "ptal://mlc:usb:PC_970";
  QVERIFY( !broken.isValid() );
  QEXPECT_FAIL( "", "QUrl doesn't provide the initial string if it's an invalid url...", Continue );
  QCOMPARE( broken.url(), QString("ptal://mlc:usb:PC_970") ); // FAILS - but we need it...

  QUrl brokenUrl( "ptal://mlc:usb:PC_970" );
  QVERIFY( !brokenUrl.isValid() );
  QEXPECT_FAIL( "", "QUrl doesn't provide the initial string if it's an invalid url...", Continue );
  QCOMPARE( brokenUrl.toString(), QString("ptal://mlc:usb:PC_970") );
  QEXPECT_FAIL( "", "QUrl doesn't provide the initial string if it's an invalid url...", Continue );
  QCOMPARE( brokenUrl.toEncoded(), QByteArray("ptal://mlc:usb:PC_970") );

  QUrl dxOffEagle( "http://something/newpage.html?[{\"foo: bar\"}]", QUrl::TolerantMode);
  QVERIFY(dxOffEagle.isValid());
  QCOMPARE(QString(dxOffEagle.toEncoded()), QString("http://something/newpage.html?%5B%7B%22foo:%20bar%22%7D%5D"));
  QUrl dxOffEagle2;
  dxOffEagle2.setUrl( "http://something/newpage.html?[{\"foo: bar\"}]", QUrl::TolerantMode);
  QVERIFY(dxOffEagle2.isValid());
  QCOMPARE(dxOffEagle.toEncoded(), dxOffEagle2.toEncoded());

  QUrl dxOffEagle3;
  dxOffEagle3.setEncodedUrl( "http://something/newpage.html?[{\"foo: bar\"}]", QUrl::TolerantMode);
  QEXPECT_FAIL("","Issue N183630, task ID 183874; works with setUrl so we do that in _setEncodedUrl now", Continue);
  QVERIFY(dxOffEagle3.isValid());
  //QEXPECT_FAIL("","Issue N183630, task ID 183874", Continue);
  //QCOMPARE(dxOffEagle.toEncoded(), dxOffEagle3.toEncoded());

  QUrl javascript;
  javascript.setUrl("javascript:window.location+\"__flashplugin_unique__\"", QUrl::TolerantMode);
  QVERIFY(javascript.isValid());
  javascript.setEncodedUrl("javascript:window.location+\"__flashplugin_unique__\"", QUrl::TolerantMode);
  QEXPECT_FAIL("","Issue N183630, task ID 183874", Continue);
  QVERIFY(javascript.isValid());
}

void KUrlTest::testMailto()
{
  const QString faure = "faure@kde.org";
  const QString mailtoFaure = "mailto:" + faure;
  KUrl umail1 ( mailtoFaure );
  QCOMPARE( umail1.protocol(), QString("mailto") );
  QCOMPARE( umail1.path(), QString(faure) );
  QVERIFY( !KUrl::isRelativeUrl(mailtoFaure) );

  // Make sure populateMimeData() works correct:
  // 1. the text/plain part of the mimedata should not contain the mailto: part
  // 2. the uri-list part of the mimedata should contain the mailto: part
  QMimeData md;
  umail1.populateMimeData(&md);
  QCOMPARE(md.text(), faure);
  KUrl::List uriList = KUrl::List::fromMimeData(&md);
  QCOMPARE(uriList.size(), 1);
  KUrl first = uriList.first();
  QCOMPARE(first.protocol(), QString("mailto"));
  QCOMPARE(first.path(), faure);

  KUrl mailtoOnly( "mailto:" );
  QVERIFY( mailtoOnly.isValid() ); // KDE3 said invalid, QUrl is more tolerant

  KUrl url1( "mailto:user@host.com" );
  QCOMPARE( url1.url(), QString("mailto:user@host.com") );
  QCOMPARE( url1.url(KUrl::LeaveTrailingSlash), QString("mailto:user@host.com") );

#if 0
  // I wrote this test in the very first kurltest, but there's no proof that it's actually valid.
  // Andreas says this is broken, i.e. against rfc2368.
  // Let's see if the need ever comes up.
  KUrl umail2 ( "mailto:Faure David <faure@kde.org>" );
  QCOMPARE( umail2.protocol(), QString("mailto") );
  QCOMPARE( umail2.path(), QString("Faure David <faure@kde.org>") );
  QVERIFY( !KUrl::isRelativeUrl("mailto:faure@kde.org") );
#endif
}

void KUrlTest::testSmb()
{
  KUrl smb("smb://domain;username:password@server/share");
  QVERIFY( smb.isValid() );
  QCOMPARE( smb.user(), QString("domain;username") );
  smb = "smb:/";
  QVERIFY( smb.isValid() );
  smb = "smb://"; // KDE3: kurl.cpp rev 1.106 made it invalid. Valid again with QUrl.
  QVERIFY( smb.isValid() );
  smb = "smb://host";
  QVERIFY( smb.isValid() );
  smb = "smb:///";
  QVERIFY( smb.isValid() );

}

void KUrlTest::testOtherProtocols()
{
  KUrl about("about:");
  QCOMPARE(about.path(), QString());
  QCOMPARE(about.protocol(), QString("about"));

  KUrl aboutKonqueror("about:konqueror");
  QCOMPARE(aboutKonqueror.path(), QString("konqueror"));


  KUrl leo( "data:text/html,http://www.invalid/" );
  QVERIFY( leo.isValid() );
  QCOMPARE( leo.protocol(), QString("data" ) );
  QCOMPARE( leo.url(), QString("data:text/html,http://www.invalid/" ) );
  QCOMPARE( leo.path(), QString("text/html,http://www.invalid/" ) );

  KUrl ptal( "ptal://mlc:usb@PC_970" ); // User=mlc, password=usb, host=PC_970
  QVERIFY( ptal.isValid() );
  QCOMPARE( ptal.host(), QString("pc_970") );
  QCOMPARE( ptal.user(), QString("mlc") );
  QCOMPARE( ptal.pass(), QString("usb") );
}

void KUrlTest::testUtf8()
{
  QTextCodec* codec = QTextCodec::codecForName( "ISO-8859-1" );
  QVERIFY( codec != 0 );
  QTextCodec::setCodecForLocale( codec );

#if 0
  {
  QUrl utest;
  utest.setScheme( "file" );
  utest.setPath( QString::fromUtf8( "/home/dfaure/Matériel" ) );
  printf( "utest.toString()=%s\n", utest.toString().toLatin1().constData() );
  printf( "utest.path()=%s\n", utest.path().toLatin1().constData() );
  printf( "utest.toEncoded()=%s\n", utest.toEncoded().data() );
  }
#endif

  // UTF8 tests
  KUrl uloc;
  uloc.setPath( QString::fromUtf8( "/home/dfaure/Matériel" ) );
  QCOMPARE( uloc.url(), QString( "file:///home/dfaure/Mat%C3%A9riel") ); // KDE3 would say %E9 here; but from now on URLs are always utf8 encoded.
  QCOMPARE( uloc.path(), QString::fromUtf8( "/home/dfaure/Matériel") );
  QCOMPARE( uloc.prettyUrl(), QString::fromUtf8( "file:///home/dfaure/Matériel") );
  QCOMPARE( uloc.pathOrUrl(), QString::fromUtf8( "/home/dfaure/Matériel") );             // ... but that's why pathOrUrl is nicer.
  QCOMPARE( uloc.url(), QString( "file:///home/dfaure/Mat%C3%A9riel") );
  uloc = KUrl("file:///home/dfaure/Mat%C3%A9riel");
  QCOMPARE( uloc.path(), QString::fromUtf8("/home/dfaure/Matériel") );
  QCOMPARE( uloc.url(), QString("file:///home/dfaure/Mat%C3%A9riel") );

  KUrl umlaut1("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel");
  QCOMPARE(umlaut1.url(), QString("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"));

  KUrl umlaut2("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"); // was ,106
  QCOMPARE(umlaut2.url(), QString("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"));

  KUrl urlWithUnicodeChar( QString::fromUtf8("file:///home/dfaure/Matériel") );
  QCOMPARE( uloc.url(), QString("file:///home/dfaure/Mat%C3%A9riel") );
}

void KUrlTest::testOtherEncodings()
{
  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "koi8-r" ) );
  KUrl baseURL( "file:/home/coolo" );
  KUrl russian = KUrl::fromPath( baseURL.directory(KUrl::AppendTrailingSlash) + QString::fromUtf8( "фгн7" ) );
  //QCOMPARE( russian.url(), QString("file:///home/%C6%C7%CE7" ) ); // KDE3: was not using utf8
  QCOMPARE( russian.url(), QString("file:///home/%D1%84%D0%B3%D0%BD7") ); // QUrl uses utf8

  KUrl utf8_1("audiocd:/By%20Name/15%20Geantra%C3%AE.wav");
  QCOMPARE( utf8_1.fileName(), QString::fromUtf8("15 Geantraî.wav") );

  // KDE3: url had %2F, and fileName had '/'. But this is wrong, %2F means '/',
  // and filenames have to use %2F, so the url needs to have %252F.
  // KIO::encodeFileName takes care of that.
  KUrl utf8_2("audiocd:/By%20Name/15%252FGeantra%C3%AE.wav");
  QCOMPARE( utf8_2.path(), QString::fromUtf8( "/By Name/15%2FGeantraî.wav" ) );
  QCOMPARE( utf8_2.fileName(), QString::fromUtf8("15%2FGeantraî.wav") );
}

void KUrlTest::testPathOrURL()
{
  // passing path or url to the constructor: both work
  KUrl uloc( LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel" );
  QCOMPARE( uloc.path(), QString(LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel") );
  uloc = KUrl( "http://www.kde.org" );
  QCOMPARE( uloc.pathOrUrl(), uloc.url() );
  QCOMPARE( uloc.pathOrUrl(KUrl::AddTrailingSlash), QString("http://www.kde.org/") );
  uloc = KUrl( QString("www.kde.org" ) );
  QVERIFY( uloc.isValid() ); // KDE3: was invalid. But it's now a url with path="www.kde.org", ok.
  uloc = KUrl( "index.html" );
  QVERIFY( uloc.isValid() ); // KDE3: was invalid; same as above
  uloc = KUrl( "" );
  QVERIFY( !uloc.isValid() );
#ifdef Q_WS_WIN
#ifdef Q_CC_MSVC
#pragma message ("port KUser")
#else
#warning port KUser
#endif
#else
  KUser currentUser;
  const QString userName = currentUser.loginName();
  QVERIFY( !userName.isEmpty() );
  uloc = KUrl(QString::fromUtf8("~%1/konqtests/Matériel").arg(userName));
  QCOMPARE( uloc.path(), QString::fromUtf8("%1/konqtests/Matériel").arg(currentUser.homeDir()) );
#endif

  // pathOrUrl tests
  uloc = KUrl( LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel" );
  QCOMPARE( uloc.pathOrUrl(), uloc.path() );
  uloc = "http://www.kde.org";
  QCOMPARE( uloc.url(), QString("http://www.kde.org") );
  uloc = "file:///home/dfaure/konq%20tests/Mat%C3%A9riel#ref";
  QCOMPARE( uloc.pathOrUrl(), QString::fromUtf8("file:///home/dfaure/konq tests/Matériel#ref" ) );
  uloc = "file:///home/dfaure/konq%20tests/Mat%C3%A9riel?query";
  QCOMPARE( uloc.pathOrUrl(), QString::fromUtf8("file:///home/dfaure/konq tests/Matériel?query" ) );
  uloc = KUrl( LOCAL_FILE_PREFIX"/home/dfaure/file#with#hash" );
  QCOMPARE( uloc.pathOrUrl(), QString(LOCAL_FILE_PREFIX"/home/dfaure/file#with#hash" ) );

  // test creation of url from pathOrUrl
  uloc = KUrl( QString::fromUtf8("http://www.kde.org/home/andreas/täst") );
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
  uloc = KUrl( "http://www.kde.org/home/andreas/t%C3%A4st");
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
  uloc = KUrl( QString::fromUtf8("file:///home/andreas/täst") );
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
  uloc = KUrl( "file:///home/andreas/t%C3%A4st");
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
  uloc = KUrl( "http://www.kde.org/home/kde?foobar#test" );
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
  uloc = KUrl( "http://www.kde.org/home/%andreas");
  QCOMPARE( KUrl( uloc.pathOrUrl() ), uloc );
}

void KUrlTest::testAssignment()
{
  // passing path or url to the constructor: both work
  KUrl uloc;
  uloc = LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel";
  QCOMPARE( uloc.path(), QString(LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel") );
  KUrl u2;
  u2 = uloc;
  QCOMPARE( u2.path(), QString(LOCAL_FILE_PREFIX"/home/dfaure/konqtests/Mat%C3%A9riel") );
  uloc = "http://www.kde.org";
  QCOMPARE( uloc.pathOrUrl(), uloc.url() );
  uloc = QString("www.kde.org" );
  QVERIFY( uloc.isValid() );
  uloc = KUrl( "index.html" );
  QVERIFY( uloc.isValid() );
  uloc = KUrl( "" );
  QVERIFY( !uloc.isValid() );
#ifdef Q_WS_WIN
#ifdef Q_CC_MSVC
#pragma message ("port KUser")
#else
#warning port KUser
#endif
#else
  KUser currentUser;
  const QString userName = currentUser.loginName();
  QVERIFY( !userName.isEmpty() );
  uloc = QString::fromUtf8("~%1/konqtests/Matériel").arg(userName);
  QCOMPARE( uloc.path(), QString::fromUtf8("%1/konqtests/Matériel").arg(currentUser.homeDir()) );
  uloc = QByteArray('~' + userName.toUtf8() + "/konqtests/Matériel");
  QCOMPARE( uloc.path(), QString::fromUtf8("%1/konqtests/Matériel").arg(currentUser.homeDir()) );

  // Assigning a KUrl to a QUrl and back
  QUrl qurl = uloc;
  QCOMPARE( qurl.toEncoded(), uloc.toEncoded() );
  uloc = KUrl(qurl);
  QCOMPARE( qurl.toEncoded(), uloc.toEncoded() );
  QCOMPARE( uloc.path(), QString::fromUtf8("%1/konqtests/Matériel").arg(currentUser.homeDir()) );
#endif
}

void KUrlTest::testQueryItem()
{
  KUrl theKow( "http://www.google.de/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25" );
  QCOMPARE( theKow.queryItem("q"), QString("frerich") );
  QCOMPARE( theKow.queryItem("hl"), QString("de") );
  QCOMPARE( theKow.queryItem("lr"), QString("lang de") ); // the '+' got decoded
  QCOMPARE( theKow.queryItem("InterstellarCounselor"), QString() );
  QCOMPARE( theKow.queryItem("empty"), QString("") );
  QCOMPARE( theKow.queryItem("test"), QString("+ :%") );
  theKow.addQueryItem("a", "b+c" );
  QCOMPARE( theKow.url(), QString("http://www.google.de/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25&a=b%2Bc") ); // KDE3 would use b%2Bc, but this is more correct
  QCOMPARE( theKow.queryItem("a"), QString("b+c") ); // note that the '+' remained

  // checks for queryItems(), which returns a QMap<QString,QString>:
  KUrl queryUrl( "mailto:Marc%20Mutz%20%3cmutz@kde.org%3E?"
		 "Subject=subscribe+me&"
		 "body=subscribe+mutz%40kde.org&"
		 "Cc=majordomo%40lists.kde.org" );
  QCOMPARE(QStringList(queryUrl.queryItems(0).keys()).join(", "),
	QString( "Cc, Subject, body" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(KUrl::CaseInsensitiveKeys).keys()).join(", "),
	QString( "body, cc, subject" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(0).values()).join(", "),
	QString( "majordomo@lists.kde.org, subscribe me, subscribe mutz@kde.org" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(KUrl::CaseInsensitiveKeys).values()).join(", "),
	QString( "subscribe mutz@kde.org, majordomo@lists.kde.org, subscribe me" ) );
  // TODO check for QUrl::queryItems

}

void KUrlTest::testEncodeString()
{
  // Needed for #49616
  QCOMPARE( QUrl::toPercentEncoding( "C++" ), QByteArray("C%2B%2B") );
  QCOMPARE( QUrl::fromPercentEncoding( "C%2B%2B" ), QString("C++") );
  QString output = QUrl::fromPercentEncoding( "C%00%0A" );
  QString expected = QString::fromLatin1("C\0\n", 3); // no reason to stop at %00, in fact
  QCOMPARE( output.size(), expected.size() );
  QCOMPARE( output, expected );
  QCOMPARE( QUrl::fromPercentEncoding( "C%A" ), QString("C%A") ); // % A is not percent-encoding  (pct-encoded = "%" HEXDIG HEXDIG)

  QCOMPARE( QUrl::toPercentEncoding( "%" ), QByteArray("%25") );
  QCOMPARE( QUrl::toPercentEncoding( ":" ), QByteArray("%3A") );
}

void KUrlTest::testIdn()
{
  //qDebug( "trying QUrl with fromPercentEncoding" );
  QUrl qurltest( QUrl::fromPercentEncoding( "http://\303\244.de" ) ); // a+trema in utf8
  QVERIFY( qurltest.isValid() );

  //qDebug( "trying QUrl with fromEncoded" );
  QUrl qurl = QUrl::fromEncoded( "http://\303\244.de" ); // a+trema in utf8
  QVERIFY( qurl.isValid() );
  QCOMPARE( qurl.toEncoded(), QByteArray( "http://xn--4ca.de" ) );

  //qDebug( "and now trying KUrl" );
  KUrl thiago( QString::fromUtf8( "http://\303\244.de" ) ); // a+trema in utf8
  QVERIFY( thiago.isValid() );
  QCOMPARE( thiago.url(), QString("http://xn--4ca.de") );   // Non-ascii is allowed in IDN domain names.

#if 0
  // A broken test - not using utf8. and amantia forgot the real-world testcase.
  KUrl amantia( "http://%E1.foo.de" );
  QVERIFY( amantia.isValid() );
  QCOMPARE( amantia.url(), QString("http://xn--80a.foo.de") );   // Non-ascii is allowed in IDN domain names.
#endif

  // A more valid test for % in hostnames:
  KUrl uwp( "http://%C3%A4.de" );
  QVERIFY( uwp.isValid() );
  QCOMPARE( thiago.url(), QString("http://xn--4ca.de") ); // as above
}

void KUrlTest::testUriMode()
{
  KUrl url1;
#if 0 // ###### TODO KUri
  url1 = "http://www.foobar.com/";
  QCOMPARE(url1.uriMode(), KUrl::URL);
  url1 = "mailto:user@host.com";
  QCOMPARE(url1.uriMode(), KUrl::Mailto);

  url1 = "data:text/plain,foobar?gazonk=flarp";
  QCOMPARE(url1.uriMode(), KUrl::RawURI);

  QCOMPARE( url1.path(), QString("text/plain,foobar?gazonk=flarp") );
#endif
  url1 = "mailto:User@Host.COM?subject=Hello";
  QCOMPARE( url1.path(), QString("User@Host.COM") ); // KDE3: "User@host.com". Does it matter?
}
