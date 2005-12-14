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

#include "qttest_kde.h"
#include "kurltest4.h"
#include "kurltest4.moc"

QTTEST_KDEMAIN( KURLTest, NoGUI )

#include <config.h>
#include <kcmdlineargs.h> // before the #define

#include "kurl4.h"
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qtextcodec.h>
#include <qdatastream.h>
#include <qmap.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define KURL KURL4

#if 0
// #define instead of static function to preserve line numbers
#define check( str, val1, val2 ) \
    qDebug( "%s", str );         \
    QCOMPARE( val1, val2 );
#endif

// I need a const char* overload...
static void check( const char* str, const QString& val1, const QString& val2 )
{
    qDebug( "%s", str );
    QCOMPARE( val1, val2 );
}

void KURLTest::testEmptyURL()
{
  qDebug("* Empty URL\n");
  KURL emptyURL;
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");
  QVERIFY( emptyURL.prettyURL().isEmpty() );

  emptyURL = "";
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");

  KURL fileURL = "file:/";
  check( "KURL::isEmpty()", fileURL.isEmpty() ? "TRUE":"FALSE", "FALSE");

  fileURL = "file:///";
  check( "KURL::isEmpty()", fileURL.isEmpty() ? "TRUE":"FALSE", "FALSE");

  KURL udir;
  check("KURL::url()", udir.url(), QString::null);
  check("KURL::isEmpty()", udir.isEmpty() ? "ok" : "ko", "ok");
  check("KURL::isValid()", udir.isValid() ? "ok" : "ko", "ko");
  udir = udir.upURL();
  check("KURL::upURL()", udir.upURL().isEmpty() ? "ok" : "ko", "ok");
}

void KURLTest::testIsValid()
{
  qDebug("* isValid tests\n");
  KURL url1 = "gg:www.kde.org";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "TRUE" );

  url1 = "KDE";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "TRUE" ); // KDE3 difference: was FALSE.

  url1 = "$HOME/.kde/share/config";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "TRUE" ); // KDE3 difference: was FALSE.
}

void KURLTest::testSetQuery()
{
  qDebug("* setQuery tests\n");
  KURL url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi?foo=bar" ) );
  check("query", url1.query(), "?foo=bar" );
  url1.setQuery( "toto=titi&kde=rocks" );
  check("query", url1.query(), "?toto=titi&kde=rocks" );
  url1.setQuery( "?kde=rocks&a=b" );
  check("query", url1.query(), "?kde=rocks&a=b" );
  url1.setQuery( "?" );
  check("setQuery(\"?\") -> query", url1.query(), "?" );
  url1.setQuery( "" );
  check("setQuery(\"\") -> query", url1.query(), "?" );
  url1.setQuery( QString::null );
  check("setQuery(QString::null) -> query", url1.query(), QString::null );
}

void KURLTest::testSetRef()
{
  qDebug("* setRef tests\n");
  KURL url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
  check("ref", url1.ref(), "foo%3Dbar" ); // KDE3 difference: was foo=bar
#if 0// ditto (TODO)
  url1.setRef( "toto=titi&kde=rocks" );
  check("ref", url1.ref(), "toto=titi&kde=rocks" );
  url1.setRef( "kde=rocks&a=b" );
  check("ref", url1.ref(), "kde=rocks&a=b" );
  url1.setRef( "#" );
  check("setRef(\"#\") -> ref", url1.ref(), "#" );
#endif
  url1.setRef( "" );
  check("setRef(\"\") -> ref", url1.ref(), "" );
  url1.setRef( QString::null );
  check("setRef(QString::null) -> ref", url1.ref(), QString::null );

  qDebug("* setHTMLRef tests\n");
  url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
  check("htmlRef", url1.htmlRef(), "foo=bar" );
  url1.setHTMLRef( "toto=titi&kde=rocks" );
  check("htmlRef", url1.htmlRef(), "toto=titi&kde=rocks" );
  url1.setHTMLRef( "kde=rocks&a=b" );
  check("htmlRef", url1.htmlRef(), "kde=rocks&a=b" );
  url1.setHTMLRef( "#" );
  check("setHTMLRef(\"#\") -> ref", url1.htmlRef(), "#" );
  url1.setHTMLRef( "" );
  check("setHTMLRef(\"\") -> ref", url1.htmlRef(), "" );
  url1.setHTMLRef( QString::null );
  check("setHTMLRef(QString::null) -> ref", url1.htmlRef(), QString::null );
}

void KURLTest::testQUrl()
{
  QUrl url1( "file:///home/dfaure/my#%2f" );
  QCOMPARE( url1.toString(), QString( "file:///home/dfaure/my#%2f" ) );
}

void KURLTest::testSimpleMethods() // to test parsing, mostly
{
  KURL mlc = "http://mlc:80/";
  check("isValid()?", mlc.isValid() ? "true" : "false", "true");
  check("port()?", QString::number(mlc.port()), "80");
  check("path()?", mlc.path(), "/");

  KURL ulong("https://swww.gad.de:443/servlet/CookieAccepted?MAIL=s@gad.de&VER=25901");
  check("host",ulong.host(),"swww.gad.de");
  check("path",ulong.path(),"/servlet/CookieAccepted");

  KURL fileURL( "file:///home/dfaure/myfile" );
  check("fileURL url()", fileURL.url(), "file:///home/dfaure/myfile");
  check("fileURL path()", fileURL.path(), "/home/dfaure/myfile");
  check("fileURL hasRef()", fileURL.hasRef() ? "yes" : "no", "yes");

  QString u1 = "file:/home/dfaure/my#myref";
  KURL url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#myref"); // KDE3 difference: QUrl doesn't resolve file:/ into file:///
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

#if 0
  QUrl qurl = QUrl::fromEncoded( "file:///home/dfaure/my#%23" );
  printf( "toString = %s\n", qurl.toString().latin1() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
  qurl = QUrl::fromEncoded( "file:///home/dfaure/my#%2f" );
  printf( "toString = %s\n", qurl.toString().latin1() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
  qurl = QUrl::fromEncoded( "file:///home/dfaure/my#/" );
  printf( "toString = %s\n", qurl.toString().latin1() );
  printf( "toEncoded = %s\n", qurl.toEncoded().data() );
#endif

  u1 = "file:///home/dfaure/my#%2f";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#/"); // KDE3: was %2f, but this is OK too
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::ref()", url1.ref().toLower(), "%2f");
  check("KURL::encodedHtmlRef()", url1.encodedHtmlRef().toLower(), "%2f");
  check("KURL::htmlRef()", url1.htmlRef(), "/");

  u1 = "file:///home/dfaure/my#%23";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#%23");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::ref()", url1.ref(), "%23");
  check("KURL::encodedHtmlRef()", url1.encodedHtmlRef(), "%23");
  check("KURL::htmlRef()", url1.htmlRef(), "#");

#if 0 // TODO
  url1 = KURL(url1, "#%6a");
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#%6a");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::ref()", url1.ref(), "j");
  check("KURL::encodedHtmlRef()", url1.encodedHtmlRef().toLower(), "%6a");
  check("KURL::htmlRef()", url1.htmlRef(), "j");
#endif

  u1 = "file:///home/dfaure/my#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject::connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:///opt/kde2/qt2/doc/html/");

  u1 = "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject::connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:///opt/kde2/qt2/doc/html/");

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject:connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:///opt/kde2/qt2/doc/html/");

  KURL carsten;
  carsten.setPath("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");
  check("KURL::path()", carsten.path(), "/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");

  KURL charles;
  charles.setPath( "/home/charles/foo%20moo" );
  check("KURL::path()", charles.path(), "/home/charles/foo%20moo");
  KURL charles2("file:/home/charles/foo%20moo");
  check("KURL::path()", charles2.path(), "/home/charles/foo moo");
}

void KURLTest::testEmptyQueryOrRef()
{
  KURL noQuery( "http://www.kde.org");
  QCOMPARE( noQuery.query(), QString( "" ) ); // no query at all

  // Empty queries should be preserved!
  //QUrl qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi?", QUrl::TolerantMode);
  //QCOMPARE( qurl.toEncoded().constData(), "http://www.kde.org/cgi/test.cgi?");
  KURL waba1 = "http://www.kde.org/cgi/test.cgi?";
  QCOMPARE( waba1.url(), QString( "http://www.kde.org/cgi/test.cgi?" ) );
  QCOMPARE( waba1.query(), QString( "?" ) ); // empty query

  // Empty references should be preserved
  waba1 = "http://www.kde.org/cgi/test.cgi#";
  check("http: URL with empty reference string", waba1.url(),
        "http://www.kde.org/cgi/test.cgi#");
  check("hasRef()", waba1.hasRef()?"true":"false","true");
  check("hasHTMLRef()", waba1.hasHTMLRef()?"true":"false","true");
  check("encodedHtmlRef()", waba1.encodedHtmlRef(),QString::null);
  //qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi#", QUrl::TolerantMode);
  //QCOMPARE( qurl.toEncoded().constData(), "http://www.kde.org/cgi/test.cgi#" );
}

void KURLTest::testParsingTolerance()
{
  // URLs who forgot to encode spaces in the query - the QUrl version first
  QUrl incorrectEncoded = QUrl::fromEncoded( "http://www.kde.org/cgi/qurl.cgi?hello=My Value" );
  QVERIFY( incorrectEncoded.isValid() );
  QVERIFY( !incorrectEncoded.toEncoded().isEmpty() );
  qDebug( "%s", incorrectEncoded.toEncoded().data() );
  QCOMPARE( incorrectEncoded.toEncoded().constData(),
           "http://www.kde.org/cgi/qurl.cgi?hello=My%20Value" );

  // URLs who forgot to encode spaces in the query.
  KURL waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value";
  //QVERIFY( waba1.isValid() );
  check("http: URL with incorrect encoded query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello=My%20Value");

  // URL with ':' in query (':' should NOT be encoded!)
  waba1 = "http://www.kde.org/cgi/test.cgi?hello:My Value";
  check("http: URL with ':' in query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello:My%20Value");
  check("upURL() removes query", waba1.upURL().url(),
        "http://www.kde.org/cgi/test.cgi");

  // URLs who forgot to encode spaces in the query.
  waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value+20";
  check("http: URL with incorrect encoded query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello=My%20Value+20");
}

void KURLTest::testURLsWithoutPath()
{
  // Urls without path (BR21387)
  KURL waba1 = "http://meine.db24.de?link=home_c_login_login"; // has query
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
  qDebug( "%s", qPrintable( waba1.url() ) );
  QCOMPARE( waba1.url(), QString( "http://a:389#b=c" ) );
  QCOMPARE( waba1.host(), QString( "a" ) );
  QCOMPARE( waba1.port(), 389 );
  QCOMPARE( waba1.path(), QString( "" ) );
  QCOMPARE( waba1.ref(), QString( "b%3Dc" ) ); // was b=c with KDE3, but the docu says encoded, so encoding the = is ok
  QCOMPARE( waba1.htmlRef(), QString( "b=c" ) );
  QCOMPARE( waba1.query(), QString( "" ) );
}

void KURLTest::testPathAndQuery()
{
  KURL tobi1("http://some.host.net/path/to/file#fragmentPrecedes?theQuery");
  check("wrong order of query and hypertext reference #1", tobi1.ref(), "fragmentPrecedes");
  check("wrong order of query and hypertext reference #2", tobi1.query(), "?theQuery");

  tobi1 = "http://host.net/path/?#http://brokenäadsfküpoij31ü029muß2890zupycÜ*!*'O´+ß0i";
  check("zero-length query",tobi1.query(),"?");

  tobi1 = "http://host.net/path/#no-query";
  check("no query", tobi1.query(),"");

  tobi1 = "http://host.net/path?myfirstquery#andsomeReference";
  tobi1.setEncodedPathAndQuery("another/path/?another&query");
  check("setEncodedPathAndQuery test#1", tobi1.query(), "?another&query");
  check("setEncodedPathAndQuery test#2", tobi1.path(), "another/path/");
  tobi1.setEncodedPathAndQuery("another/path?another&query");
  check("setEncodedPathAndQuery test#1", tobi1.query(), "?another&query");
  check("setEncodedPathAndQuery test#2", tobi1.path(), "another/path");

  KURL url1 = "ftp://user%40host.com@ftp.host.com/var/www/";
  check("user()?", url1.user(), "user@host.com" );
  check("host()?", url1.host(), "ftp.host.com" );
  KURL up = url1.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/var/");
  up = up.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/");
  up = up.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/"); // unchanged
}

void KURLTest::testSetFileName() // and addPath
{
  kdDebug() << k_funcinfo << endl;
  KURL u2 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
  qDebug( "* URL is %s", qPrintable( u2.url() ) );

  u2.setFileName( "myfile.txt" );
  check("KURL::setFileName()", u2.url(), "file:///home/dfaure/myfile.txt");
  u2.setFileName( "myotherfile.txt" );
  check("KURL::setFileName()", u2.url(), "file:///home/dfaure/myotherfile.txt");

  // more tricky, renaming a directory (kpropertiesdialog.cc)
  QString tmpurl = "file:/home/dfaure/myolddir/";
  if ( tmpurl.at(tmpurl.length() - 1) == '/')
      // It's a directory, so strip the trailing slash first
      tmpurl.truncate( tmpurl.length() - 1);
  KURL newUrl = tmpurl;
  newUrl.setFileName( "mynewdir" );
  check("KURL::setFileName() special", newUrl.url(), "file:///home/dfaure/mynewdir");

  // addPath tests
  newUrl.addPath( "subdir" );
  check("KURL::addPath(\"subdir\")", newUrl.url(), "file:///home/dfaure/mynewdir/subdir");
  newUrl.addPath( "/foo/" );
  check("KURL::addPath(\"/foo/\")", newUrl.url(), "file:///home/dfaure/mynewdir/subdir/foo/");
  u2 = "http://www.kde.org"; // no path
  u2.addPath( "subdir" );
  check("KURL::addPath(\"subdir\")", u2.url(), "http://www.kde.org/subdir");
  u2.addPath( "" );
  check("KURL::addPath(\"subdir\")", u2.url(), "http://www.kde.org/subdir"); // unchanged

  QUrl qurl2 = QUrl::fromEncoded( "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)", QUrl::TolerantMode );
  QCOMPARE( qurl2.path(), QString::fromLatin1("/specials/Print To File (PDF%2FAcrobat)") );
  QCOMPARE( qurl2.fileName(), QString::fromLatin1( "Print To File (PDF%2FAcrobat)" ) );
  QCOMPARE( qurl2.toEncoded().constData(), "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)" );

  // even more tricky
  u2 = "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)";
  qDebug("* URL is %s",u2.url().ascii());
  check("KURL::path()", u2.path(), "/specials/Print To File (PDF%2FAcrobat)");
  check("KURL::fileName()", u2.fileName(), "Print To File (PDF%2FAcrobat)");
  u2.setFileName( "" );
  check("KURL::setFileName()", u2.url(), "print:/specials/");

  u2 = "file:/specials/Print";
  qDebug("* URL is %s",u2.url().ascii());
  check("KURL::path()", u2.path(), "/specials/Print");
  check("KURL::fileName()", u2.fileName(), "Print");
  u2.setFileName( "" );
  check("KURL::setFileName()", u2.url(), "file:///specials/");

  const char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  qDebug("* URL is %s",u3);
  check("KURL::hasSubURL()", KURL(u3).hasSubURL() ? "yes" : "no", "no");

  KURL::List lst = KURL::split( KURL(u3) );
  check("KURL::split()", lst.count()==1 ? "1" : "error", "1");
  check("KURL::split()", lst.first().url(), "ftp://host/dir1/dir2/myfile.txt");

  // cdUp code
  KURL lastUrl = lst.last();
  QString dir = lastUrl.directory( true, true );
  check( "KURL::directory(true,true)", dir, "/dir1/dir2");
}

void KURLTest::testDirectory()
{
  KURL udir;
  udir.setPath("/home/dfaure/file.txt");
  qDebug( "URL is %s", qPrintable( udir.url() ) );
  check("KURL::path()", udir.path(), "/home/dfaure/file.txt");
  check("KURL::url()", udir.url(), "file:///home/dfaure/file.txt");
  check("KURL::directory(false,false)", udir.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", udir.directory(true,false), "/home/dfaure");

  KURL u2( QByteArray("file:///home/dfaure/") );
  qDebug("* URL is %s",u2.url().ascii());
  // not ignoring trailing slash
  check("KURL::directory(false,false)", u2.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", u2.directory(true,false), "/home/dfaure");
  // ignoring trailing slash
  check("KURL::directory(false,true)", u2.directory(false,true), "/home/");
  check("KURL::directory(true,true)", u2.directory(true,true), "/home");

  // cleanPath() tests (before cd() since cd uses that)
  u2.cleanPath();
  check("cleanPath(false)", u2.url(), "file:///home/dfaure/");
  u2.addPath( "/..//foo" );
  check("addPath", u2.url(), "file:///home/dfaure/..//foo");
  u2.cleanPath(false);
  check("cleanPath()", u2.url(), "file:///home//foo");
  u2.cleanPath(true);
  check("cleanPath()", u2.url(), "file:///home/foo");

  // cd() tests
  u2.cd("..");
  check("KURL::cd(\"..\")", u2.url(), "file:///home");
  u2.cd("thomas");
  check("KURL::cd(\"thomas\")", u2.url(), "file:///home/thomas");
  u2.cd("../");
  check("KURL::cd(\"../\")", u2.url(), "file:///home/");
  u2.cd("/opt/kde/bin/");
  check("KURL::cd(\"/opt/kde/bin/\")", u2.url(), "file:///opt/kde/bin/");
  u2 = "ftp://ftp.kde.org/";
  qDebug("* URL is %s",u2.url().ascii());
  u2.cd("pub");
  check("KURL::cd(\"pub\")", u2.url(), "ftp://ftp.kde.org/pub");
  u2 = u2.upURL();
  check("KURL::upURL()", u2.url(), "ftp://ftp.kde.org/");
}

// In KDE4, prettyURL does pretty much the same as url(), except for removing passwords from URLs.
// We don't have a way to decode %20 into ' ' and still avoid decoding everything (e.g. %23 must remain %23)
// but we don't really need it anymore; it was primarily for paths in konq's locationbar, but it uses pathOrURL() now.
void KURLTest::testPrettyURL()
{
  kdDebug() << k_funcinfo << endl;
  KURL notPretty("http://ferret.lmh.ox.ac.uk/%7Ekdecvs/");
  check("KURL::prettyURL()", notPretty.prettyURL(), "http://ferret.lmh.ox.ac.uk/~kdecvs/");
  KURL notPretty2("file:/home/test/directory%20with%20spaces");
  // KDE3: check("KURL::prettyURL()", notPretty2.prettyURL(), "file:///home/test/directory with spaces");
  check("KURL::prettyURL()", notPretty2.prettyURL(), "file:///home/test/directory%20with%20spaces");

  KURL notPretty3("fish://foo/%23README%23");
  check("KURL::prettyURL()", notPretty3.prettyURL(), "fish://foo/%23README%23");
  KURL url15581("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  // KDE3: check("KURL::prettyURL()", url15581.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  check("KURL::prettyURL()", url15581.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html");
  check("KURL::url()", url15581.url(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html");

  KURL url15581bis("http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::prettyURL()", url15581bis.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::url()", url15581bis.url(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html");

  KURL urlWithPass("ftp://user:password@ftp.kde.org/path");
  QCOMPARE( urlWithPass.pass(), QString::fromLatin1( "password" ) );
  QCOMPARE( urlWithPass.prettyURL(), QString::fromLatin1( "ftp://user@ftp.kde.org/path" ) );
}

void KURLTest::testIsRelative()
{
  kdDebug() << k_funcinfo << endl;
  check("man: URL, is relative", KURL::isRelativeURL("man:mmap") ? "true" : "false", "false");
  check("javascript: URL, is relative", KURL::isRelativeURL("javascript:doSomething()") ? "true" : "false", "false");
  // more isRelative
  check("file: URL, is relative", KURL::isRelativeURL("file:///blah") ? "true" : "false", "false");
  check("/path, is relative", KURL::isRelativeURL("/path") ? "true" : "false", "true"); // arguable, but necessary for KURL( baseURL, "//www1.foo.bar" );
  check("something, is relative", KURL::isRelativeURL("something") ? "true" : "false", "true");
}

void KURLTest::testAdjustPath()
{
    KURL url1("file:///home/kde/");
    url1.adjustPath(0);
    check( "adjustPath(0)", url1.path(), "/home/kde/" );
    url1.adjustPath(-1);
    check( "adjustPath(-1) removes last slash", url1.path(), "/home/kde" );
    url1.adjustPath(-1);
    check( "adjustPath(-1) again", url1.path(), "/home/kde" );
    url1.adjustPath(1);
    check( "adjustPath(1)", url1.path(), "/home/kde/" );

    KURL url2("file:///home/kde//");
    url2.adjustPath(0);
    check( "adjustPath(0)", url2.path(), "/home/kde//" );
    url2.adjustPath(-1);
    check( "adjustPath(-1) removes all trailing slashes", url2.path(), "/home/kde" );
    url2.adjustPath(1);
    check( "adjustPath(1)", url2.path(), "/home/kde/" );

    KURL ftpurl1("ftp://ftp.kde.org/");
    ftpurl1.adjustPath(0);
    check( "adjustPath(0)", ftpurl1.path(), "/" );
    ftpurl1.adjustPath(-1);
    check( "adjustPath(-1) preserves last slash", ftpurl1.path(), "/" );

    KURL ftpurl2("ftp://ftp.kde.org///");
    ftpurl2.adjustPath(0);
    check( "adjustPath(0)", ftpurl2.path(), "///" );
    ftpurl2.adjustPath(-1);
    check( "adjustPath(-1) removes all but last slash", ftpurl2.path(), "/" );
    ftpurl2.adjustPath(1);
    check( "adjustPath(1)", ftpurl2.path(), "/" );

    // Equivalent tests written by the KDirLister maintainer :)

    KURL u3( QByteArray("ftp://brade@ftp.kde.org///") );
    u3.adjustPath(-1);
    check("KURL::adjustPath()", u3.url(), "ftp://brade@ftp.kde.org/");

    KURL u4( QByteArray("ftp://brade@ftp.kde.org/kde///") );
    u4.adjustPath(-1);
    check("KURL::adjustPath()", u4.url(), "ftp://brade@ftp.kde.org/kde");

    // applying adjustPath(-1) twice should not yield two different urls
    // (follows from the above test)
    KURL u5 = u4;
    u5.adjustPath(-1);
    check("KURL::adjustPath()", u5.url(), u4.url());
}

void KURLTest::testIPV6()
{
  kdDebug() << k_funcinfo << endl;
  // IPV6
  KURL waba1 = "http://[::FFFF:129.144.52.38]:81/index.html";
  check("http: IPV6 host", waba1.host(),
        "::ffff:129.144.52.38");
  check("http: IPV6 port", QString("%1").arg(waba1.port()),
        "81");

  // IPV6
  waba1 = "http://waba:pass@[::FFFF:129.144.52.38]:81/index.html";
  check("http: IPV6 host", waba1.host(),
        "::ffff:129.144.52.38");
  check("http: IPV6 host", waba1.user(),
        "waba");
  check("http: IPV6 host", waba1.pass(),
        "pass");
  check("http: IPV6 port", QString("%1").arg(waba1.port()),
        "81");

  // IPV6
  waba1 = "http://www.kde.org/cgi/test.cgi";
  waba1.setHost("::ffff:129.144.52.38");
  check("http: IPV6 host", waba1.url(),
        "http://[::ffff:129.144.52.38]/cgi/test.cgi");
  waba1 = "http://[::ffff:129.144.52.38]/cgi/test.cgi";
  assert( waba1.isValid() );

  // IPV6 without path
  waba1 = "http://[::ffff:129.144.52.38]?query";
  assert( waba1.isValid() );
  check("http: IPV6 without path", waba1.url(),
        "http://[::ffff:129.144.52.38]?query");
  check("http: IPV6 without path; query", waba1.query(),
        "?query");
  waba1 = "http://[::ffff:129.144.52.38]#ref";
  assert( waba1.isValid() );
  check("http: IPV6 without path", waba1.url(),
        "http://[::ffff:129.144.52.38]#ref");
  check("http: IPV6 without path; ref", waba1.ref(),
        "ref");
  // IPV6 without path but with a port
  waba1 = "http://[::ffff:129.144.52.38]:81?query";
  assert( waba1.isValid() );
  check("http: IPV6 without path", waba1.url(),
        "http://[::ffff:129.144.52.38]:81?query");
  check("http: IPV6 without path; port", QString::number( waba1.port() ), "81" );
  check("http: IPV6 without path; query", waba1.query(), "?query");
  waba1 = "http://[::ffff:129.144.52.38]:81#ref";
  assert( waba1.isValid() );
  check("http: IPV6 without path", waba1.url(),
        "http://[::ffff:129.144.52.38]:81#ref");
  check("http: IPV6 without path; port", QString::number( waba1.port() ), "81" );
  check("http: IPV6 without path; ref", waba1.ref(), "ref");
}

void KURLTest::testBaseURL() // those are tests for the KURL(base,relative) constructor
{
  kdDebug() << k_funcinfo << endl;
  KURL baseURL ("hTTp://www.foo.bar:80" );
  check( "KURL::isValid()", baseURL.isValid() ? "TRUE":"FALSE", "TRUE");
  check( "KURL::protocol()", baseURL.protocol(), "http"); // lowercase
  KURL url1 ( baseURL, "//www1.foo.bar" );
  check( "KURL::url()", url1.url(), "http://www1.foo.bar");
  check( "KURL::host()", url1.host(), "www1.foo.bar");

  baseURL = "http://www.foo.bar";
  KURL rel_url( baseURL, "/top//test/../test1/file.html" );
  check( "KURL::url()", rel_url.url(), "http://www.foo.bar/top//test1/file.html" );


  baseURL = "http://www.foo.bar/top//test2/file2.html";
  check( "KURL::url()", baseURL.url(), "http://www.foo.bar/top//test2/file2.html" );

  baseURL = "file:/usr/local/src/kde2/////kdelibs/kio";
  check( "KURL::url()", baseURL.url(), "file:///usr/local/src/kde2/////kdelibs/kio" );

  baseURL = "http://www.foo.bar";
  KURL rel_url2( baseURL, "mailto:bastian@kde.org" );
  check( "KURL::url()", rel_url2.url(), "mailto:bastian@kde.org" );

  baseURL = "mailto:bastian@kde.org?subject=hello";
  check( "KURL::url()", baseURL.url(), "mailto:bastian@kde.org?subject=hello" );

  baseURL = "file:/usr/local/src/kde2/kdelibs/kio/";
  KURL url2( baseURL, "../../////kdebase/konqueror" );
  check( "KURL::url()", url2.url(), "file:///usr/local/src/kde2/////kdebase/konqueror" );


  // WABA: The following tests are to test the handling of relative URLs as
  //       found on web-pages.

  KURL waba1( "http://www.website.com/directory/?hello#ref" );
  {
     KURL waba2( waba1, "relative.html");
     check("http: Relative URL, single file", waba2.url(), "http://www.website.com/directory/relative.html");
  }
  {
     KURL waba2( waba1, "../relative.html");
     check("http: Relative URL, single file, directory up", waba2.url(), "http://www.website.com/relative.html");
  }
  {
     KURL waba2( waba1, "down/relative.html");
     check("http: Relative URL, single file, directory down", waba2.url(), "http://www.website.com/directory/down/relative.html");
  }
  {
     KURL waba2( waba1, "/down/relative.html");
     check("http: Relative URL, full path", waba2.url(), "http://www.website.com/down/relative.html");
  }
  {
     KURL waba2( waba1, "//www.kde.org/relative.html");
     check("http: Relative URL, with host", waba2.url(), "http://www.kde.org/relative.html");
  }
  {
     KURL waba2( waba1, "relative.html?query=test&name=harry");
     check("http: Relative URL, with query", waba2.url(), "http://www.website.com/directory/relative.html?query=test&name=harry");
     waba2.removeQueryItem("query");
     check("http: Removing query item", waba2.url(), "http://www.website.com/directory/relative.html?name=harry");
     waba2.addQueryItem("age", "18");
     check("http: Adding query item", waba2.url(), "http://www.website.com/directory/relative.html?name=harry&age=18");
     waba2.addQueryItem("age", "21");
     check("http: Adding query item", waba2.url(), "http://www.website.com/directory/relative.html?name=harry&age=18&age=21");
     waba2.addQueryItem("fullname", "Harry Potter");
     check("http: Adding query item", waba2.url(), "http://www.website.com/directory/relative.html?name=harry&age=18&age=21&fullname=Harry%20Potter");
  }
  {
     KURL waba2( waba1, "?query=test&name=harry");
     check("http: Relative URL, with query and no filename", waba2.url(), "http://www.website.com/directory/?query=test&name=harry");
  }
  {
     KURL waba2( waba1, "relative.html#with_reference");
     check("http: Relative URL, with reference", waba2.url(), "http://www.website.com/directory/relative.html#with_reference");
  }
  {
     KURL waba2( waba1, "#");
     check("http: Relative URL, with empty reference", waba2.url(), "http://www.website.com/directory/?hello#");
  }
  {
     KURL waba2( waba1, "");
     check("http: Empty relative URL", waba2.url(), "http://www.website.com/directory/?hello#ref");
  }
  {
     KURL base( "http://faure@www.kde.org" ); // no path
     KURL waba2( base, "filename.html");
     check("http: Relative URL, orig URL had no path", waba2.url(), "http://faure@www.kde.org/filename.html");
  }
  {
     KURL base( "http://faure:pass@www.kde.org:81?query" );
     KURL rel1( base, "http://www.kde.org/bleh/"); // same host
     check("http: Relative URL, orig URL had username", rel1.url(), "http://faure:pass@www.kde.org/bleh/");
     KURL rel2( base, "http://www.yahoo.org"); // different host
     check("http: Relative URL, orig URL had username", rel2.url(), "http://www.yahoo.org");
  }

  waba1 = "http://www.website.com/directory/filename?bla#blub";
  {
     KURL waba2( waba1, "relative.html");
     check("http: Relative URL, single file", waba2.url(), "http://www.website.com/directory/relative.html");
  }
  {
     KURL waba2( waba1, "../relative.html");
     check("http: Relative URL, single file, directory up", waba2.url(), "http://www.website.com/relative.html");
  }
  {
     KURL waba2( waba1, "down/relative.html");
     check("http: Relative URL, single file, directory down", waba2.url(), "http://www.website.com/directory/down/relative.html");
  }
  {
     KURL waba2( waba1, "/down/relative.html");
     check("http: Relative URL, full path", waba2.url(), "http://www.website.com/down/relative.html");
  }
  {
     KURL waba2( waba1, "relative.html?query=test&name=harry");
     check("http: Relative URL, with query", waba2.url(), "http://www.website.com/directory/relative.html?query=test&name=harry");
  }
  {
     KURL waba2( waba1, "?query=test&name=harry");
     check("http: Relative URL, with query and no filename", waba2.url(), "http://www.website.com/directory/filename?query=test&name=harry");
  }
  {
     KURL waba2( waba1, "relative.html#with_reference");
     check("http: Relative URL, with reference", waba2.url(), "http://www.website.com/directory/relative.html#with_reference");
  }
  {
      KURL waba2( waba1, "http:/relative.html"); // "rfc 1606 loophole"
      check("http: Strange relative URL", waba2.url(), "http://www.website.com/relative.html");
  }
  waba1.setUser("waldo");
  check("http: Set user", waba1.url(), "http://waldo@www.website.com/directory/filename?bla#blub");
  waba1.setUser("waldo/bastian");
  check("http: Set user with slash in it", waba1.url(), "http://waldo%2Fbastian@www.website.com/directory/filename?bla#blub");
  waba1.setRef( QString::null );
  waba1.setPass( "pass" );
  waba1.setDirectory( "/foo" );
  waba1.setProtocol( "https" );
  waba1.setHost( "web.com" );
  waba1.setPort( 881 );
  check("http: setRef/setPass/setDirectory/setHost/setPort", waba1.url(), "https://waldo%2Fbastian:pass@web.com:881/foo/?bla");
  waba1.setDirectory( "/foo/" );
  check("http: setDirectory #2", waba1.url(), "https://waldo%2Fbastian:pass@web.com:881/foo/?bla");
}

void KURLTest::testSubURL()
{
  kdDebug() << k_funcinfo << endl;
  QString u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  KURL url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::isLocalFile()", url1.isLocalFile() ? "yes" : "no", "no"); // Not strictly local!
  //check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  //check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

  u1 = "error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi";
  url1 = u1;
  check("KURL::url()", url1.url(), "error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::isLocalFile()", url1.isLocalFile() ? "yes" : "no", "no");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  //check("KURL::htmlRef()", url1.htmlRef(), "myref");

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

  u1 = "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

#if 0
// This URL is broken, '#' should be escaped.
  u1 = "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "no");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::prettyURL()", url1.upURL().url(), "file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#");
#endif

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");
}

void KURLTest::testSetUser()
{
  // The KURL equality test below works because in Qt4 null == empty.
  QString str1 = QString::null;
  QString str2 = "";
  assert( str1 == str2 );

  KURL emptyUserTest1( "http://www.foobar.com/");
  QVERIFY( emptyUserTest1.user().isEmpty() );
  QVERIFY( !emptyUserTest1.user().isNull() ); // small change compared to KURL-3.5
  KURL emptyUserTest2( "http://www.foobar.com/");
  emptyUserTest2.setUser( "" );
  //assert( emptyUserTest2.user().isNull() );
  QCOMPARE( emptyUserTest1==emptyUserTest2?"TRUE":"FALSE","TRUE" );
  emptyUserTest2.setPass( "" );
  QCOMPARE( emptyUserTest1==emptyUserTest2?"TRUE":"FALSE","TRUE" );
  emptyUserTest2.setUser( "foo" );
  QCOMPARE( emptyUserTest2.user(), QString::fromLatin1( "foo" ) );
  emptyUserTest2.setUser( QString::null );
  QCOMPARE( emptyUserTest1==emptyUserTest2, true );
}

void KURLTest::testComparisons()
{
  kdDebug() << k_funcinfo << endl;
  /// Comparisons
  QString ucmp1 = "ftp://ftp.de.kde.org/dir";
  QString ucmp2 = "ftp://ftp.de.kde.org/dir/";
  check("urlcmp(only slash difference)", urlcmp(ucmp1,ucmp2)?"ko":"ok","ok");
  
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

  check("urlcmp(only slash difference, ignore_trailing)", urlcmp(ucmp1,ucmp2,true,false)?"ok":"ko","ok");

  QString ucmp3 = "ftp://ftp.de.kde.org/dir/#";
  check("urlcmp(only hash difference)", urlcmp(ucmp2,ucmp3)?"ko":"ok","ok");
  check("urlcmp(only hash difference, ignore_ref)", urlcmp(ucmp2,ucmp3,false,true)?"ok":"ko","ok");
  check("urlcmp(slash and hash difference, ignore_trailing, ignore_ref)", urlcmp(ucmp2,ucmp3,true,true)?"ok":"ko","ok");
  check("urlcmp(empty, empty)", urlcmp("","",false,true)?"ok":"ko","ok");
  check("urlcmp(empty, empty)", urlcmp("","")?"ok":"ko","ok");
  check("urlcmp(empty, not empty)", urlcmp("",ucmp1)?"ok":"ko","ko");
  check("urlcmp(empty, not empty)", urlcmp("",ucmp1,false,true)?"ok":"ko","ko");
  check("urlcmp(malformed, not empty)", urlcmp("file",ucmp1)?"ok":"ko","ko");
  check("urlcmp(malformed, not empty)", urlcmp("file",ucmp1,false,true)?"ok":"ko","ko");

  KURL ftpUrl ( "ftp://ftp.de.kde.org" );
  qDebug("* URL is %s",ftpUrl.url().latin1());
  check("KURL::path()", ftpUrl.path(), QString::null);
  ftpUrl = "ftp://ftp.de.kde.org/";
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp.de.kde.org/host/subdir/") ? "yes" : "no", "yes");
  ftpUrl = "ftp://ftp/host/subdir/";
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdi") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/blah/") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/blah/subdir") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "file:////ftp/host/subdir/") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/subsub") ? "yes" : "no", "yes");
}

void KURLTest::testStreaming()
{
  kdDebug() << k_funcinfo << endl;
  // Streaming operators
  KURL origURL( "http://www.website.com/directory/?#ref" );
  KURL waba1 = "http://[::ffff:129.144.52.38]:81?query";
  QByteArray buffer;
  {
      QDataStream stream( &buffer, QIODevice::WriteOnly );
      stream << origURL
             << KURL( "file:" ) // an invalid one
             << waba1; // the IPv6 one
  }
  {
      QDataStream stream( buffer );
      KURL restoredURL;
      stream >> restoredURL;
      check( "Streaming valid URL", restoredURL.url(), origURL.url() );
      stream >> restoredURL;
      check( "Streaming invalid URL", restoredURL.isValid()?"valid":"malformed", "malformed" );
      check( "Streaming invalid URL", restoredURL.url(), "file:" );
      stream >> restoredURL;
      check( "Streaming ipv6 URL with query", restoredURL.url(), waba1.url() );
  }
}

void KURLTest::testBrokenStuff()
{
  kdDebug() << k_funcinfo << endl;
  // Broken stuff
  KURL waba1 = "file:a";
  check("Broken stuff #1 path", waba1.path(), "a");
  check("Broken stuff #1 fileName(false)", waba1.fileName(false), "a");
  check("Broken stuff #1 fileName(true)", waba1.fileName(true), "a");
  check("Broken stuff #1 directory(false, false)", waba1.directory(false, false), "");
  check("Broken stuff #1 directory(true, false)", waba1.directory(true, false), "");
  check("Broken stuff #1 directory(false, true)", waba1.directory(true, true), "");

  waba1 = "file:a/";
  check("Broken stuff #2 path", waba1.path(), "a/");
  check("Broken stuff #2 fileName(false)", waba1.fileName(false), "");
  check("Broken stuff #2 fileName(true)", waba1.fileName(true), "a");
  check("Broken stuff #2 directory(false, false)", waba1.directory(false, false), "a/");
  check("Broken stuff #2 directory(true, false)", waba1.directory(true, false), "a");
  check("Broken stuff #2 directory(false, true)", waba1.directory(true, true), "");

  waba1 = "file:";
  check("Broken stuff #3 empty", waba1.isEmpty()?"EMPTY":"NOT", "NOT");
  check("Broken stuff #3 valid", waba1.isValid()?"VALID":"MALFORMED", "MALFORMED");
  check("Broken stuff #3 path", waba1.path(), "");
  check("Broken stuff #3 fileName(false)", waba1.fileName(false), "");
  check("Broken stuff #3 fileName(true)", waba1.fileName(true), "");
  check("Broken stuff #3 directory(false, false)", waba1.directory(false, false), "");
  check("Broken stuff #3 directory(true, false)", waba1.directory(true, false), "");
  check("Broken stuff #3 directory(false, true)", waba1.directory(true, true), "");
  KURL broken;
  broken.setPath( QString::null );
  check("Broken stuff #4 empty", broken.isEmpty()?"EMPTY":"NOT", "NOT");
  // It's valid: because isValid refers to parsing, not to what happens afterwards.
  check("Broken stuff #4 valid", broken.isValid()?"VALID":"MALFORMED", "VALID");
  check("Broken stuff #4 path", broken.path(), "");
  broken = "file://"; // just because coolo wondered
  check("Broken stuff #5 empty", broken.isEmpty()?"EMPTY":"NOT", "NOT");
  check("Broken stuff #5 valid", broken.isValid()?"VALID":"MALFORMED", "MALFORMED");
  check("Broken stuff #5 path", broken.path(), "");
  broken = "file";
  check("Broken stuff #6 valid", broken.isValid()?"VALID":"MALFORMED", "MALFORMED");

  broken = "LABEL=USB_STICK"; // 71430, can we use KURL for this?
  check("Broken stuff #6 valid", broken.isValid()?"VALID":"MALFORMED", "MALFORMED");
  check("Broken stuff #6 empty", broken.isEmpty()?"EMPTY":"NOT", "NOT");
  check("Broken stuff #6 path", broken.path(), "");

#if 0 // BROKEN?
  // UNC like names
  KURL unc1("FILE://localhost/home/root");
  check("UNC, with localhost", unc1.path(), "/home/root");
  check("UNC, with localhost", unc1.url(), "file:///home/root");
#endif
  KURL unc2("file:///home/root");
  check("UNC, with empty host", unc2.path(), "/home/root");
  check("UNC, with empty host", unc2.url(), "file:///home/root");

  {
     KURL unc3("FILE://remotehost/home/root");
#if 0 // BROKEN?
     check("UNC, with remote host", unc3.path(), "//remotehost/home/root");
#endif
     check("UNC, with remote host", unc3.url(), "file://remotehost/home/root");
     KURL url2("file://atlas/dfaure");
     check("KURL::host()", url2.host(), "atlas");
     check("KURL::path()", url2.path(), "/dfaure");
     //check("KURL::path()", url3.path(), "//atlas/dfaure"); // says Waba
     //KURL url3("file:////atlas/dfaure");
     //check("KURL::path()", url3.path(), "//atlas/dfaure"); // says Waba

     KURL url4(url2, "//remotehost/home/root");
     check("KURL::host()", url4.host(), "remotehost");
     check("KURL::path()", url4.path(), "/home/root");
  }

  broken = "ptal://mlc:usb:PC_970";
  check("isValid()?", broken.isValid() ? "true" : "false", "false");
  check("url()", broken.url(), "ptal://mlc:usb:PC_970");

  KURL weird;
  weird = "http://strange<hostname>/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://strange<username>@strange<hostname>/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://strange<username>@ok_hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "true");
  check("weird.host()", weird.host(), "ok_hostname");

  weird = "http://strange;hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://strange;username@strange;hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://strange;username@ok_hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "true");
  check("weird.host()", weird.host(), "ok_hostname");

  weird = "http://strange;username:password@strange;hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://strange;username:password@ok_hostname/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "true");
  check("weird.host()", weird.host(), "ok_hostname");

  weird = "http://[strange;hostname]/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "false");

  weird = "http://[::fff:1:23]/";
  check("weird.isValid()", weird.isValid() ? "true" : "false", "true");
  check("weird.host()", weird.host(), "::fff:1:23");
}

void KURLTest::testMailto()
{
  kdDebug() << k_funcinfo << endl;
  KURL umail1 ( "mailto:faure@kde.org" );
  check("mailto: URL, general form", umail1.protocol(), "mailto");
  check("mailto: URL, general form", umail1.path(), "faure@kde.org");
  check("mailto: URL, is relative", KURL::isRelativeURL("mailto:faure@kde.org") ? "true" : "false", "false");
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  check("mailto: URL, general form", umail2.protocol(), "mailto");
  check("mailto: URL, general form", umail2.path(), "Faure David <faure@kde.org>");
  check("isRelativeURL(\"mailto:faure@kde.org\")", KURL::isRelativeURL("mailto:faure@kde.org") ? "yes" : "no", "no");
  KURL umail3 ( "mailto:" );
  check("mailto: invalid URL", umail3.isValid()?"valid":"malformed", "malformed");

  KURL url1 = "mailto:user@host.com";
  check("KURL(\"mailto:user@host.com\").url()", url1.url(), "mailto:user@host.com");
  check("KURL(\"mailto:user@host.com\").url(0)", url1.url(0), "mailto:user@host.com");
}

void KURLTest::testSmb()
{
  kdDebug() << k_funcinfo << endl;
  KURL smb("smb://domain;username:password@server/share");
  check("smb.isValid()", smb.isValid() ? "true" : "false", "true");
  check("smb.user()", smb.user(), "domain;username");
  smb = "smb:/";
  check("smb:/", smb.isValid()?"VALID":"MALFORMED", "VALID");
  smb = "smb://"; // kurl.cpp rev 1.106
  check("smb://", smb.isValid()?"VALID":"MALFORMED", "MALFORMED");
  smb = "smb://host";
  check("smb://host", smb.isValid()?"VALID":"MALFORMED", "VALID");
  smb = "smb:///";
  check("smb:///", smb.isValid()?"VALID":"MALFORMED", "VALID");

}

void KURLTest::testOtherProtocols()
{
  kdDebug() << k_funcinfo << endl;
  KURL about("about:konqueror");
  check("about:",about.path(),"konqueror");


  KURL leo = "data:text/html,http://www.invalid/";
  check("data URL: isValid", leo.isValid()?"valid":"malformed", "valid" );
  check("data URL: protocol", leo.protocol(), "data" );
  check("data URL: url", leo.url(), "data:text/html,http://www.invalid/" );
  check("data URL: path", leo.path(), "text/html,http://www.invalid/" );

  KURL ptal = "ptal://mlc:usb@PC_970"; // User=mlc, password=usb, host=PC_970
  check("isValid()?", ptal.isValid() ? "true" : "false", "true");
  check("host()?", ptal.host(), "pc_970");
  check("user()?", ptal.user(), "mlc");
  check("pass()?", ptal.pass(), "usb");
}

void KURLTest::testUtf8()
{
  kdDebug() << k_funcinfo << endl;
  QTextCodec* codec = QTextCodec::codecForName( "ISO-8859-1" );
  assert( codec );
  QTextCodec::setCodecForLocale( codec );

  {
  QUrl utest;
  utest.setScheme( "file" );
  utest.setPath( QLatin1String( "/home/dfaure/Matériel" ) );
  printf( "utest.toString()=%s\n", utest.toString().toLatin1().constData() );
  printf( "utest.path()=%s\n", utest.path().toLatin1().constData() );
  printf( "utest.toEncoded()=%s\n", utest.toEncoded().data() );
  }

  // UTF8 tests
  KURL uloc;
  uloc.setPath( QString::fromLatin1( "/home/dfaure/Matériel" ) ); // TODO convert this file to utf8 and use fromUtf8 here; but check below for russian
  QCOMPARE( uloc.url(), QString( "file:///home/dfaure/Mat%C3%A9riel") ); // KDE3 would say %E9 here; but from now on URLs are always utf8 encoded.
  QCOMPARE( uloc.path(), QString( "/home/dfaure/Matériel") );
  QCOMPARE( uloc.prettyURL(), QString( "file:///home/dfaure/Mat%C3%A9riel") ); // KDE3 wouldn't escape the letters here...
  QCOMPARE( uloc.pathOrURL(), QString( "/home/dfaure/Matériel") );             // ... but that's why pathOrURL is nicer.
  QCOMPARE( uloc.url(), QString( "file:///home/dfaure/Mat%C3%A9riel") );
  uloc = KURL("file:///home/dfaure/Mat%C3%A9riel");
  QCOMPARE( uloc.path(), QString("/home/dfaure/Matériel") );
  QCOMPARE( uloc.url(), QString("file:///home/dfaure/Mat%C3%A9riel") );

  KURL umlaut1("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel");
  QCOMPARE(umlaut1.url(), QString("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"));

  KURL umlaut2("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"); // was ,106
  QCOMPARE(umlaut2.url(), QString("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel"));
}

void KURLTest::testOtherEncodings()
{
  kdDebug() << k_funcinfo << endl;
  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "koi8-r" ) );
  KURL baseURL = "file:/home/coolo";
  KURL russian = baseURL.directory(false, true) + QString::fromLocal8Bit( "ÆÇÎ7" );
  check( "russian", russian.url(), "file:///home/%C6%C7%CE7" );
}

void KURLTest::testPathOrURL()
{
  kdDebug() << k_funcinfo << endl;
  // fromPathOrURL tests
  KURL uloc = KURL::fromPathOrURL( "/home/dfaure/konqtests/Mat%E9riel" );
  check("fromPathOrURL path", uloc.path(), "/home/dfaure/konqtests/Mat%E9riel");
  uloc = KURL::fromPathOrURL( "http://www.kde.org" );
  check("pathOrURL url", uloc.pathOrURL(), uloc.url() );
  uloc = KURL::fromPathOrURL( "www.kde.org" );
  check("fromPathOrURL malformed", uloc.isValid()?"valid":"malformed", "malformed");
  uloc = KURL::fromPathOrURL( "index.html" );
  check("fromPathOrURL malformed", uloc.isValid()?"valid":"malformed", "malformed");
  uloc = KURL::fromPathOrURL( "" );
  check("fromPathOrURL malformed", uloc.isValid()?"valid":"malformed", "malformed");

  // pathOrURL tests
  uloc = KURL::fromPathOrURL( "/home/dfaure/konqtests/Mat%E9riel" );
  check("pathOrURL path", uloc.pathOrURL(), uloc.path() );
  uloc = "http://www.kde.org";
  check("pathOrURL url", uloc.url(), "http://www.kde.org");
  uloc = "file:///home/dfaure/konq%20tests/Mat%E9riel#ref";
  check("pathOrURL local file with ref", uloc.pathOrURL(), "file:///home/dfaure/konq tests/Matériel#ref" );
  uloc = "file:///home/dfaure/konq%20tests/Mat%E9riel?query";
  check("pathOrURL local file with query", uloc.pathOrURL(), "file:///home/dfaure/konq tests/Matériel?query" );
  uloc = KURL::fromPathOrURL( "/home/dfaure/file#with#hash" );
  check("pathOrURL local path with #", uloc.pathOrURL(), "/home/dfaure/file#with#hash" );
}

void KURLTest::testQueryItem()
{
  kdDebug() << k_funcinfo << endl;
  KURL theKow = "http://www.google.de/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25";
  check("queryItem (first item)", theKow.queryItem("q"), "frerich");
  check("queryItem (middle item)", theKow.queryItem("hl"), "de");
  check("queryItem (last item)", theKow.queryItem("lr"), "lang de");
  check("queryItem (invalid item)", theKow.queryItem("InterstellarCounselor"), QString::null);
  check("queryItem (empty item)", theKow.queryItem("empty"), "");
  check("queryItem (item with encoded chars)", theKow.queryItem("test"), "+ :%");

  // checks for queryItems(), which returns a QMap<QString,QString>:
  KURL queryUrl( "mailto:Marc%20Mutz%20%3cmutz@kde.org%3E?"
		 "Subject=subscribe+me&"
		 "body=subscribe+mutz%40kde.org&"
		 "Cc=majordomo%40lists.kde.org" );
  check("queryItems (c.s. keys)",
	QStringList(queryUrl.queryItems(0).keys()).join(", "),
	"Cc, Subject, body" );
  check("queryItems (c.i.s. keys)",
	QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).keys()).join(", "),
	"body, cc, subject" );
  check("queryItems (values; c.s. keys)",
	QStringList(queryUrl.queryItems(0).values()).join(", "),
	"majordomo@lists.kde.org, subscribe me, subscribe mutz@kde.org" );
  check("queryItems (values; c.i.s. keys)",
	QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).values()).join(", "),
	"subscribe mutz@kde.org, majordomo@lists.kde.org, subscribe me" );
  // TODO check for QUrl::queryItems

}

void KURLTest::testEncodeString()
{
  kdDebug() << k_funcinfo << endl;
  // Needed for #49616
  check( "encode_string('C++')", KURL::encode_string( "C++" ), "C%2B%2B" );
  check( "decode_string('C%2B%2B')", KURL::decode_string( "C%2B%2B" ), "C++" );
  check( "decode_string('C%00A')", KURL::decode_string( "C%00%A" ), "C" ); // we stop at %00

  check( "encode_string('%')", KURL::encode_string( "%" ), "%25" );
  check( "encode_string(':')", KURL::encode_string( ":" ), "%3A" );
}

void KURLTest::testIdn()
{
  kdDebug() << k_funcinfo << endl;

  QUrl qurltest( QUrl::fromPercentEncoding( "http://\303\244.de" ) ); // ä in utf8
  QVERIFY( qurltest.isValid() );

  QUrl qurl = QUrl::fromEncoded( "http://\303\244.de" ); // ä in utf8
  QVERIFY( qurl.isValid() );
  QCOMPARE( qurl.toEncoded().constData(), "http://xn--4ca.de" );

  KURL thiago( QString::fromUtf8( "http://\303\244.de" ) ); // ä in utf8
  check("thiago.isValid()", thiago.isValid() ? "true" : "false", "true");
  check("thiago.url()", thiago.url(), "http://xn--4ca.de");   // Non-ascii is allowed in IDN domain names.

  // A more broken test
  KURL amantia( "http://%E1.foo.de" );
  check("amantia.isValid()", amantia.isValid() ? "true" : "false", "true");
  check("amantia.url()", amantia.url(), "http://xn--80a.foo.de");   // Non-ascii is allowed in IDN domain names.
}

void KURLTest::testUriMode()
{
  kdDebug() << k_funcinfo << endl;
  KURL url1;
#if 0 // ###### TODO KUri
  url1 = "http://www.foobar.com/";
  check("KURL(\"http://www.foobar.com/\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::URL));
  url1 = "mailto:user@host.com";
  check("KURL(\"mailto:user@host.com\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::Mailto));

  url1 = "data:text/plain,foobar?gazonk=flarp";
  check("KURL(\"data:text/plain,foobar?gazonk=flarp\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::RawURI));

  check("KURL(\"data:text/plain,foobar?gazonk=flarp\").path()", url1.path(), "text/plain,foobar?gazonk=flarp");
#endif
  url1 = "mailto:User@Host.COM?subject=Hello";
  check("KURL(\"mailto:User@Host.COM?subject=Hello\").path()", url1.path(), "User@host.com");
}

void KURLTest::testOther()
{
  // TODO categorize the rest of the tests

  KURL com1("http://server.com/dir/", ".");
  check("com1.url()", com1.url(), "http://server.com/dir/");

  KURL com2("http://server.com/dir/blubb/", "blah/");
  check("com2.url()", com2.url(), "http://server.com/dir/blubb/blah/");

  KURL utf8_1("audiocd:/By%20Name/15%20Geantra%C3%AE.wav"/*, 106*/);
  check("utf8_1.fileName()", utf8_1.fileName(), QLatin1String("15 Geantraî.wav"));

  KURL utf8_2("audiocd:/By%20Name/15%2fGeantra%C3%AE.wav"/*, 106*/);
  check("utf8_2.fileName()", utf8_2.fileName(), QLatin1String("15/Geantraî.wav"));

  QUrl qurl_newline_1 = QUrl::fromEncoded( "http://www.foo.bar/foo/bar\ngnork", QUrl::TolerantMode );
  QVERIFY( qurl_newline_1.isValid() );
  QCOMPARE( qurl_newline_1.toEncoded().constData(), "http://www.foo.bar/foo/bar%0Agnork" );

  KURL url_newline_1("http://www.foo.bar/foo/bar\ngnork");
  check("url_newline_1.url()", url_newline_1.url(), QLatin1String("http://www.foo.bar/foo/bar%0Agnork"));

  KURL url_newline_2("http://www.foo.bar/foo?bar\ngnork");
  check("url_newline_2.url()", url_newline_2.url(), QLatin1String("http://www.foo.bar/foo?bar%0Agnork"));

  KURL local_file_1("file://localhost/my/file");
  check("local_file_1.isLocalFile()", local_file_1.isLocalFile() ? "true" : "false", "true");

  KURL local_file_2("file://www.kde.org/my/file");
  check("local_file_2.isLocalFile()", local_file_2.isLocalFile() ? "true" : "false", "false");

  KURL local_file_3;
  local_file_3.setHost(getenv("HOSTNAME"));
  local_file_3.setPath("/my/file");
  qDebug("URL=%s\n", local_file_3.url().latin1());
  check("local_file_3.isLocalFile()", local_file_3.isLocalFile() ? "true" : "false", "true");

  KURL local_file_4("file:///my/file");
  check("local_file_4.isLocalFile()", local_file_4.isLocalFile() ? "true" : "false", "true");

  KURL local_file_5;
  local_file_5.setPath("/foo?bar");
  check("local_file_5.url()", local_file_5.url(), "file:///foo%3Fbar");

  QString basePath = "/home/bastian";

  check("relativePath(\"/home/bastian\", \"/home/bastian\")", KURL::relativePath(basePath, "/home/bastian"), "./");
  bool b;
  check("relativePath(\"/home/bastian\", \"/home/bastian/src/plugins\")", KURL::relativePath(basePath, "/home/bastian/src/plugins", &b), "./src/plugins");
  check("Is a subdirectory?", b ? "true" : "false", "true");
  check("relativePath(\"/home/bastian\", \"./src/plugins\")", KURL::relativePath(basePath, "./src/plugins"), "./src/plugins");
  check("relativePath(\"/home/bastian\", \"/home/waba/src/plugins\")", KURL::relativePath(basePath, "/home/waba/src/plugins", &b), "../waba/src/plugins");
  check("Is a subdirectory?", b ? "true" : "false", "false");
  check("relativePath(\"/home/bastian\", \"/\")", KURL::relativePath(basePath, "/"), "../../");

  check("relativePath(\"/\", \"/\")", KURL::relativePath("/", "/"), "./");
  check("relativePath(\"/\", \"/home/bastian\")", KURL::relativePath("/", "/home/bastian"), "./home/bastian");
  check("relativePath(\"\", \"/home/bastian\")", KURL::relativePath("", "/home/bastian"), "/home/bastian");

  KURL baseURL = "http://www.kde.org/index.html";
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html#help\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html#help"), "#help");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html?help=true\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html?help=true"), "index.html?help=true");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/contact.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/contact.html"), "contact.html");
  check("relativeURL(\"http://www.kde.org/index.html\", \"ftp://ftp.kde.org/pub/kde\")", KURL::relativeURL(baseURL, "ftp://ftp.kde.org/pub/kde"), "ftp://ftp.kde.org/pub/kde");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html"), "./");

  baseURL = "http://www.kde.org/info/index.html";
  check("relativeURL(\"http://www.kde.org/info/index.html\", \"http://www.kde.org/bugs/contact.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/bugs/contact.html"), "../bugs/contact.html");

  KURL ldap = "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)";
  check("host()?", ldap.host(), "host.com");
  check("port()?", QString("%1").arg(ldap.port()), "6666");
  check("path()?", ldap.path(), "/o=University of Michigan,c=US");
  check("query()?", ldap.query(), "??sub?(cn=Babs%20Jensen)");
  check("url()?", ldap.url(), "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)");
  ldap.setQuery("??sub?(cn=Karl%20Marx)");
  check("query()?", ldap.query(), "??sub?(cn=Karl%20Marx)");
  check("url()?", ldap.url(), "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Karl%20Marx)");
}

