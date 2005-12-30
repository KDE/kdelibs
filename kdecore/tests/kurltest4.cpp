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
#include <stdio.h>
#include <stdlib.h>

#define KURL KURL4

void KURLTest::testEmptyURL()
{
  qDebug("* Empty URL\n");
  KURL emptyURL;
  QVERIFY( !emptyURL.isValid() );
  QVERIFY( emptyURL.isEmpty() );
  QVERIFY( emptyURL.prettyURL().isEmpty() );

  emptyURL = "";
  QVERIFY( !emptyURL.isValid() );
  QVERIFY( emptyURL.isEmpty() );

  KURL fileURL = "file:/";
  QVERIFY( !fileURL.isEmpty() );

  fileURL = "file:///";
  QVERIFY( !fileURL.isEmpty() );

  KURL udir;
  QCOMPARE( udir.url(), QString() );
  QVERIFY( udir.isEmpty() );
  QVERIFY( !udir.isValid() );
  udir = udir.upURL();
  QVERIFY( udir.upURL().isEmpty() );
}

void KURLTest::testIsValid()
{
  qDebug("* isValid tests\n");
  KURL url1 = "gg:www.kde.org";
  QVERIFY( url1.isValid() );

  url1 = "KDE";
  QVERIFY( url1.isValid() ); // KDE3 difference: was FALSE

  url1 = "$HOME/.kde/share/config";
  QVERIFY( url1.isValid() ); // KDE3 difference: was FALSE
}

void KURLTest::testSetQuery()
{
  qDebug("* setQuery tests\n");
  KURL url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi?foo=bar" ) );
  QCOMPARE( url1.query(), QString("?foo=bar" ) );
  url1.setQuery( "toto=titi&kde=rocks" );
  QCOMPARE( url1.query(), QString("?toto=titi&kde=rocks" ) );
  url1.setQuery( "?kde=rocks&a=b" );
  QCOMPARE( url1.query(), QString("?kde=rocks&a=b" ) );
  url1.setQuery( "?" );
  QCOMPARE( url1.query(), QString("?" ) );
  url1.setQuery( "" );
  QCOMPARE( url1.query(), QString("?" ) );
  url1.setQuery( QString::null );
  QCOMPARE( url1.query(), QString() );
}

void KURLTest::testSetRef()
{
  qDebug( "* setRef tests\n" );
  KURL url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
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
  url1.setRef( QString::null );
  QCOMPARE( url1.ref(), QString() );

  qDebug("* setHTMLRef tests\n");
  url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi#foo=bar" ) );
  QCOMPARE( url1.htmlRef(), QString("foo=bar") );
  url1.setHTMLRef( "toto=titi&kde=rocks" );
  QCOMPARE( url1.htmlRef(), QString("toto=titi&kde=rocks") );
  url1.setHTMLRef( "kde=rocks&a=b" );
  QCOMPARE( url1.htmlRef(), QString("kde=rocks&a=b") );
  url1.setHTMLRef( "#" );
  QCOMPARE( url1.htmlRef(), QString("#") );
  url1.setHTMLRef( "" );
  QCOMPARE( url1.htmlRef(), QString("") );
  url1.setHTMLRef( QString::null );
  QCOMPARE( url1.htmlRef(), QString() );
}

void KURLTest::testQUrl()
{
  QUrl url1( "file:///home/dfaure/my#%2f" );
  QCOMPARE( url1.toString(), QString( "file:///home/dfaure/my#%2f" ) );
}

void KURLTest::testSimpleMethods() // to test parsing, mostly
{
  KURL mlc = "http://mlc:80/";
  QVERIFY( mlc.isValid() );
  QCOMPARE( mlc.port(), 80 );
  QCOMPARE( mlc.path(), QString("/") );

  KURL ulong("https://swww.gad.de:443/servlet/CookieAccepted?MAIL=s@gad.de&VER=25901");
  QCOMPARE(ulong.host(),QString("swww.gad.de") );
  QCOMPARE(ulong.path(),QString("/servlet/CookieAccepted") );

  KURL fileURL( "file:///home/dfaure/myfile" );
  QCOMPARE( fileURL.url(), QString("file:///home/dfaure/myfile") );
  QCOMPARE( fileURL.path(), QString("/home/dfaure/myfile") );
  QVERIFY( fileURL.hasRef() );

  QString u1 = "file:/home/dfaure/my#myref";
  KURL url1 = u1;
  // KDE3 difference: QUrl doesn't resolve file:/ into file:///
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#myref") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/") );

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
  // KDE3: was %2f, but this is OK too
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#/") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.ref().toLower(), QString("%2f") );
  QCOMPARE( url1.encodedHtmlRef().toLower(), QString("%2f") );
  QCOMPARE( url1.htmlRef(), QString("/") );

  u1 = "file:///home/dfaure/my#%23";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#%23") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.ref(), QString("%23") );
  QCOMPARE( url1.encodedHtmlRef(), QString("%23") );
  QCOMPARE( url1.htmlRef(), QString("#") );

#if 0 // TODO
  url1 = KURL(url1, "#%6a");
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#%6a") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.ref(), QString("j") );
  QCOMPARE( url1.encodedHtmlRef().toLower(), QString("%6a") );
  QCOMPARE( url1.htmlRef(), QString("j") );
#endif

  u1 = "file:///home/dfaure/my#myref";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my#myref") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/") );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("QObject::connect") );
  QCOMPARE( url1.upURL().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  u1 = "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("QObject::connect") );
  QCOMPARE( url1.upURL().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect") );
  QVERIFY( url1.hasRef() );
  QVERIFY( url1.hasHTMLRef() );
  QVERIFY( !url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("QObject:connect") );
  QCOMPARE( url1.upURL().url(), QString("file:///opt/kde2/qt2/doc/html/") );

  KURL carsten;
  carsten.setPath("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");
  QCOMPARE( carsten.path(), QString("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18") );

  KURL charles;
  charles.setPath( "/home/charles/foo%20moo" );
  QCOMPARE( charles.path(), QString("/home/charles/foo%20moo") );
  KURL charles2("file:/home/charles/foo%20moo");
  QCOMPARE( charles2.path(), QString("/home/charles/foo moo") );
}

void KURLTest::testEmptyQueryOrRef()
{

  QUrl url = QUrl::fromEncoded( "http://www.kde.org" );
  QCOMPARE( url.toEncoded(), QByteArray( "http://www.kde.org" ) );
  QCOMPARE( url.encodedQuery(), QByteArray() );
  url = QUrl::fromEncoded( "http://www.kde.org?" );
  QCOMPARE( url.toEncoded(), QByteArray( "http://www.kde.org?" ) );
  QCOMPARE( url.encodedQuery(), QByteArray() ); // note that QByteArray() == QByteArray("")

  url = QUrl::fromEncoded( "http://www.kde.org" );
  QVERIFY( url.encodedQuery().isNull() ); // FAILS
  url = QUrl::fromEncoded( "http://www.kde.org?" );
  QVERIFY( !url.encodedQuery().isNull() );
  QVERIFY( !url.encodedQuery().isEmpty() );

  KURL noQuery( "http://www.kde.org");
  QCOMPARE( noQuery.query(), QString() ); // query at all

  // Empty queries should be preserved!
  //QUrl qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi?", QUrl::TolerantMode);
  //QCOMPARE( qurl.toEncoded().constData(), "http://www.kde.org/cgi/test.cgi?");
  KURL waba1 = "http://www.kde.org/cgi/test.cgi?";
  QCOMPARE( waba1.url(), QString( "http://www.kde.org/cgi/test.cgi?" ) );
  QCOMPARE( waba1.query(), QString( "?" ) ); // empty query

  // Empty references should be preserved
  waba1 = "http://www.kde.org/cgi/test.cgi#";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi#") );
  QVERIFY( waba1.hasRef() );
  QVERIFY( waba1.hasHTMLRef() );
  QCOMPARE( waba1.encodedHtmlRef(), QString() );
  //qurl = QUrl::fromEncoded("http://www.kde.org/cgi/test.cgi#", QUrl::TolerantMode);
  //QCOMPARE( qurl.toEncoded().constData(), "http://www.kde.org/cgi/test.cgi#" );

  KURL tobi1 = "http://host.net/path/?#http://brokenäadsfküpoij31ü029muß2890zupycÜ*!*'O´+ß0i";
  QCOMPARE(tobi1.query(), QString("?")); // query is empty

  tobi1 = "http://host.net/path/#no-query";
  QCOMPARE(tobi1.query(), QString("")); // no query

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
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello=My%20Value") );

  // URL with ':' in query (':' should NOT be encoded!)
  waba1 = "http://www.kde.org/cgi/test.cgi?hello:My Value";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello:My%20Value") );
  QCOMPARE( waba1.upURL().url(), QString("http://www.kde.org/cgi/test.cgi") );

  // URLs who forgot to encode spaces in the query.
  waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value+20";
  QCOMPARE( waba1.url(), QString("http://www.kde.org/cgi/test.cgi?hello=My%20Value+20") );
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
#if 0 // TODO re-enable once testEmptyQueryOrRef works
  QCOMPARE( waba1.query(), QString() );
#endif
}

void KURLTest::testPathAndQuery()
{
#if 0                                                                   \
  // this KDE3 test fails, but Tobias Anton didn't say where it came from, and Andreas Hanssen (TT) says:
  // "I can't see any reason to support this; it looks like a junk artifact from older days.
  // Everything after # is the fragment. Parsing what comes after # is broken; tolerant or not."
  KURL tobi0("http://some.host.net/path/to/file#fragmentPrecedes?theQuery");
  QCOMPARE( tobi0.ref(), QString("fragmentPrecedes") );
  QCOMPARE( tobi0.query(), QString("?theQuery") );
#endif

  KURL tobi1 = "http://host.net/path?myfirstquery#andsomeReference";
  tobi1.setEncodedPathAndQuery("another/path/?another&query");
  QCOMPARE( tobi1.query(), QString("?another&query") );
  QCOMPARE( tobi1.path(), QString("another/path/") );
  tobi1.setEncodedPathAndQuery("another/path?another&query");
  QCOMPARE( tobi1.query(), QString("?another&query") );
  QCOMPARE( tobi1.path(), QString("another/path") );

  KURL url1 = "ftp://user%40host.com@ftp.host.com/var/www/";
  QCOMPARE( url1.user(), QString("user@host.com" ) );
  QCOMPARE( url1.host(), QString("ftp.host.com" ) );
  KURL up = url1.upURL();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/var/") );
  up = up.upURL();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/") );
  up = up.upURL();
  QCOMPARE( up.url(), QString("ftp://user%40host.com@ftp.host.com/") ); // unchanged
}

void KURLTest::testSetFileName() // and addPath
{
  kdDebug() << k_funcinfo << endl;
  KURL u2 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
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
  KURL newUrl = tmpurl;
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
  QCOMPARE( qurl2.fileName(), QString::fromLatin1( "Print To File (PDF%2FAcrobat)" ) );
  QCOMPARE( qurl2.toEncoded().constData(), "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)" );

  // even more tricky
  u2 = "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)";
  qDebug("* URL is %s",u2.url().ascii());
  QCOMPARE( u2.path(), QString("/specials/Print To File (PDF%2FAcrobat)") );
  QCOMPARE( u2.fileName(), QString("Print To File (PDF%2FAcrobat)") );
  u2.setFileName( "" );
  QCOMPARE( u2.url(), QString("print:/specials/") );

  u2 = "file:/specials/Print";
  qDebug("* URL is %s",u2.url().ascii());
  QCOMPARE( u2.path(), QString("/specials/Print") );
  QCOMPARE( u2.fileName(), QString("Print") );
  u2.setFileName( "" );
  QCOMPARE( u2.url(), QString("file:///specials/") );

  const char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  qDebug("* URL is %s",u3);
  QVERIFY( !KURL(u3).hasSubURL() );

  KURL::List lst = KURL::split( KURL(u3) );
  QCOMPARE( lst.count(), 1 );
  QCOMPARE( lst.first().url(), QString("ftp://host/dir1/dir2/myfile.txt") );

  // cdUp code
  KURL lastUrl = lst.last();
  QString dir = lastUrl.directory( true, true );
  QCOMPARE(  dir, QString("/dir1/dir2") );
}

void KURLTest::testDirectory()
{
  KURL udir;
  udir.setPath("/home/dfaure/file.txt");
  qDebug( "URL is %s", qPrintable( udir.url() ) );
  QCOMPARE( udir.path(), QString("/home/dfaure/file.txt") );
  QCOMPARE( udir.url(), QString("file:///home/dfaure/file.txt") );
  QCOMPARE( udir.directory(false,false), QString("/home/dfaure/") );
  QCOMPARE( udir.directory(true,false), QString("/home/dfaure") );

  KURL u2( QByteArray("file:///home/dfaure/") );
  qDebug("* URL is %s",u2.url().ascii());
  // not ignoring trailing slash
  QCOMPARE( u2.directory(false,false), QString("/home/dfaure/") );
  QCOMPARE( u2.directory(true,false), QString("/home/dfaure") );
  // ignoring trailing slash
  QCOMPARE( u2.directory(false,true), QString("/home/") );
  QCOMPARE( u2.directory(true,true), QString("/home") );

  // cleanPath() tests (before cd() since cd uses that)
  u2.cleanPath();
  QCOMPARE( u2.url(), QString("file:///home/dfaure/") );
  u2.addPath( "/..//foo" );
  QCOMPARE( u2.url(), QString("file:///home/dfaure/..//foo") );
  u2.cleanPath(false);
  QCOMPARE( u2.url(), QString("file:///home//foo") );
  u2.cleanPath(true);
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
  qDebug("* URL is %s",u2.url().ascii());
  u2.cd("pub");
  QCOMPARE( u2.url(), QString("ftp://ftp.kde.org/pub") );
  u2 = u2.upURL();
  QCOMPARE( u2.url(), QString("ftp://ftp.kde.org/") );
}

// In KDE4, prettyURL does pretty much the same as url(), except for removing passwords from URLs.
// We don't have a way to decode %20 into ' ' and still avoid decoding everything (e.g. %23 must remain %23)
// but we don't really need it anymore; it was primarily for paths in konq's locationbar, but it uses pathOrURL() now.
void KURLTest::testPrettyURL()
{
  kdDebug() << k_funcinfo << endl;
  KURL notPretty("http://ferret.lmh.ox.ac.uk/%7Ekdecvs/");
  QCOMPARE( notPretty.prettyURL(), QString("http://ferret.lmh.ox.ac.uk/~kdecvs/") );
  KURL notPretty2("file:/home/test/directory%20with%20spaces");
  // KDE3: QCOMPARE( notPretty2.prettyURL(), QString("file:///home/test/directory with spaces") );
  QCOMPARE( notPretty2.prettyURL(), QString("file:///home/test/directory%20with%20spaces") );

  KURL notPretty3("fish://foo/%23README%23");
  QCOMPARE( notPretty3.prettyURL(), QString("fish://foo/%23README%23") );
  KURL url15581("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  // KDE3: QCOMPARE( url15581.prettyURL(), QString("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html") );
  QCOMPARE( url15581.prettyURL(), QString("http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html") );
  QCOMPARE( url15581.url(), QString("http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html") );

  // KDE3 test was for parsing "percentage%in%url.html", but this is not supported; too broken.
  KURL url15581bis("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html");
  QCOMPARE( url15581bis.prettyURL(), QString("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html") );
  QCOMPARE( url15581bis.url(), QString("http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html") );

  KURL urlWithPass("ftp://user:password@ftp.kde.org/path");
  QCOMPARE( urlWithPass.pass(), QString::fromLatin1( "password" ) );
  QCOMPARE( urlWithPass.prettyURL(), QString::fromLatin1( "ftp://user@ftp.kde.org/path" ) );
}

void KURLTest::testIsRelative()
{
  kdDebug() << k_funcinfo << endl;
  QVERIFY( !KURL::isRelativeURL("man:mmap") );
  QVERIFY( !KURL::isRelativeURL("javascript:doSomething()") );
  QVERIFY( !KURL::isRelativeURL("file:///blah") );
  // arguable, but necessary for KURL( baseURL, "//www1.foo.bar" );
  QVERIFY( KURL::isRelativeURL("/path") );
  QVERIFY( KURL::isRelativeURL("something") );
}

void KURLTest::testAdjustPath()
{
    KURL url1("file:///home/kde/");
    url1.adjustPath(0);
    QCOMPARE(  url1.path(), QString("/home/kde/" ) );
    url1.adjustPath(-1);
    QCOMPARE(  url1.path(), QString("/home/kde" ) );
    url1.adjustPath(-1);
    QCOMPARE(  url1.path(), QString("/home/kde" ) );
    url1.adjustPath(1);
    QCOMPARE(  url1.path(), QString("/home/kde/" ) );

    KURL url2("file:///home/kde//");
    url2.adjustPath(0);
    QCOMPARE(  url2.path(), QString("/home/kde//" ) );
    url2.adjustPath(-1);
    QCOMPARE(  url2.path(), QString("/home/kde" ) );
    url2.adjustPath(1);
    QCOMPARE(  url2.path(), QString("/home/kde/" ) );

    KURL ftpurl1("ftp://ftp.kde.org/");
    ftpurl1.adjustPath(0);
    QCOMPARE(  ftpurl1.path(), QString("/" ) );
    ftpurl1.adjustPath(-1);
    QCOMPARE(  ftpurl1.path(), QString("/" ) );

    KURL ftpurl2("ftp://ftp.kde.org///");
    ftpurl2.adjustPath(0);
    QCOMPARE(  ftpurl2.path(), QString("///" ) );
    ftpurl2.adjustPath(-1); // should remove all but trailing slash
    QCOMPARE(  ftpurl2.path(), QString("/" ) );
    ftpurl2.adjustPath(1);
    QCOMPARE(  ftpurl2.path(), QString("/" ) );

    // Equivalent tests written by the KDirLister maintainer :)

    KURL u3( QByteArray("ftp://brade@ftp.kde.org///") );
    u3.adjustPath(-1);
    QCOMPARE( u3.url(), QString("ftp://brade@ftp.kde.org/") );

    KURL u4( QByteArray("ftp://brade@ftp.kde.org/kde///") );
    u4.adjustPath(-1);
    QCOMPARE( u4.url(), QString("ftp://brade@ftp.kde.org/kde") );

    // applying adjustPath(-1) twice should not yield two different urls
    // (follows from the above test)
    KURL u5 = u4;
    u5.adjustPath(-1);
    QCOMPARE( u5.url(), u4.url() );
}

void KURLTest::testIPV6()
{
  kdDebug() << k_funcinfo << endl;
  // IPV6
  KURL waba1 = "http://[::FFFF:129.144.52.38]:81/index.html";
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
}

void KURLTest::testBaseURL() // those are tests for the KURL(base,relative) constructor
{
  kdDebug() << k_funcinfo << endl;
  KURL baseURL ("hTTp://www.foo.bar:80" );
  QVERIFY( baseURL.isValid() );
  QCOMPARE( baseURL.protocol(), QString( "http" ) ); // lowercase
  QCOMPARE( baseURL.port(), 80 );

  QString relativeURL = "//www1.foo.bar";
  QVERIFY( KURL::isRelativeURL( relativeURL ) );

  // Mimick what KURL(2 urls) does:
  QUrl qurl;
  qurl = "hTTp://www.foo.bar:80";
  QCOMPARE( qurl.toEncoded().constData(), "hTTp://www.foo.bar:80" );
  qurl.setHost( QString() );
  qurl.setPath( QString() );
  QCOMPARE( qurl.toEncoded().constData(), "hTTp://:80" );

  KURL url1 ( baseURL, relativeURL );
  QCOMPARE( url1.url(), QString("http://www1.foo.bar"));
  QCOMPARE( url1.host(), QString("www1.foo.bar"));

  baseURL = "http://www.foo.bar";
  KURL rel_url( baseURL, "/top//test/../test1/file.html" );
  QCOMPARE( rel_url.url(), QString("http://www.foo.bar/top//test1/file.html" ));


  baseURL = "http://www.foo.bar/top//test2/file2.html";
  QCOMPARE( baseURL.url(), QString("http://www.foo.bar/top//test2/file2.html" ));

  baseURL = "file:/usr/local/src/kde2/////kdelibs/kio";
  QCOMPARE( baseURL.url(), QString("file:///usr/local/src/kde2/////kdelibs/kio" ));

  baseURL = "http://www.foo.bar";
  KURL rel_url2( baseURL, "mailto:bastian@kde.org" );
  QCOMPARE( rel_url2.url(), QString("mailto:bastian@kde.org" ));

  baseURL = "mailto:bastian@kde.org?subject=hello";
  QCOMPARE( baseURL.url(), QString("mailto:bastian@kde.org?subject=hello" ));

  baseURL = "file:/usr/local/src/kde2/kdelibs/kio/";
  KURL url2( baseURL, "../../////kdebase/konqueror" );
  QCOMPARE( url2.url(), QString("file:///usr/local/src/kde2/////kdebase/konqueror" ));


  // WABA: The following tests are to test the handling of relative URLs as
  //       found on web-pages.

  KURL waba1( "http://www.website.com/directory/?hello#ref" );
  {
     KURL waba2( waba1, "relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html") );
  }
  {
     KURL waba2( waba1, "../relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  {
     KURL waba2( waba1, "down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/down/relative.html") );
  }
  {
     KURL waba2( waba1, "/down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/down/relative.html") );
  }
  {
     KURL waba2( waba1, "//www.kde.org/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.kde.org/relative.html") );
  }
  {
     KURL waba2( waba1, "relative.html?query=test&name=harry");
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
     KURL waba2( waba1, "?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?query=test&name=harry") );
  }
  {
     KURL waba2( waba1, "relative.html#with_reference");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html#with_reference") );
  }
  {
     KURL waba2( waba1, "#");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?hello#") );
  }
  {
     KURL waba2( waba1, "");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/?hello#ref") );
  }
  {
     KURL base( "http://faure@www.kde.org" ); // no path
     KURL waba2( base, "filename.html");
     QCOMPARE( waba2.url(), QString("http://faure@www.kde.org/filename.html") );
  }
  {
     KURL base( "http://faure:pass@www.kde.org:81?query" );
     KURL rel1( base, "http://www.kde.org/bleh/"); // same host
     QCOMPARE( rel1.url(), QString("http://faure:pass@www.kde.org/bleh/") );
     KURL rel2( base, "http://www.yahoo.org"); // different host
     QCOMPARE( rel2.url(), QString("http://www.yahoo.org") );
  }

  waba1 = "http://www.website.com/directory/filename?bla#blub";
  {
     KURL waba2( waba1, "relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html") );
  }
  {
     KURL waba2( waba1, "../relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  {
     KURL waba2( waba1, "down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/down/relative.html") );
  }
  {
     KURL waba2( waba1, "/down/relative.html");
     QCOMPARE( waba2.url(), QString("http://www.website.com/down/relative.html") );
  }
  {
     KURL waba2( waba1, "relative.html?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html?query=test&name=harry") );
  }
  {
     KURL waba2( waba1, "?query=test&name=harry");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/filename?query=test&name=harry") );
  }
  {
     KURL waba2( waba1, "relative.html#with_reference");
     QCOMPARE( waba2.url(), QString("http://www.website.com/directory/relative.html#with_reference") );
  }
  {
      KURL waba2( waba1, "http:/relative.html"); // "rfc 1606 loophole"
      QCOMPARE( waba2.url(), QString("http://www.website.com/relative.html") );
  }
  waba1.setUser("waldo");
  QCOMPARE( waba1.url(), QString("http://waldo@www.website.com/directory/filename?bla#blub") );
  waba1.setUser("waldo/bastian");
  QCOMPARE( waba1.url(), QString("http://waldo%2Fbastian@www.website.com/directory/filename?bla#blub") );
  waba1.setRef( QString::null );
  waba1.setPass( "pass" );
  waba1.setDirectory( "/foo" );
  waba1.setProtocol( "https" );
  waba1.setHost( "web.com" );
  waba1.setPort( 881 );
  QCOMPARE( waba1.url(), QString("https://waldo%2Fbastian:pass@web.com:881/foo/?bla") );
  waba1.setDirectory( "/foo/" );
  QCOMPARE( waba1.url(), QString("https://waldo%2Fbastian:pass@web.com:881/foo/?bla") );
}

void KURLTest::testSubURL()
{
  kdDebug() << k_funcinfo << endl;
  QString u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  KURL url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref") );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.isLocalFile() );  // Not strictly local!
  QVERIFY( url1.hasSubURL() );
  //QCOMPARE( url1.htmlRef(), QString("myref") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/") );

  u1 = "error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi";
  url1 = u1;
  QCOMPARE( url1.url(), QString("error:/?error=14&errText=Unknown%20host%20asdfu.adgi.sdfgoi#http://asdfu.adgi.sdfgoi") );
  QVERIFY( url1.hasSubURL() );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.isLocalFile() );
  QVERIFY( !url1.hasHTMLRef() );

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/") );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/") );

  u1 = "file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/") );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/") );

#if 0
// This URL is broken, '#' should be escaped.
  u1 = "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#") );
  QVERIFY( !url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#") );
#endif

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
  url1 = u1;
  QCOMPARE( url1.url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README") );
  QVERIFY( url1.hasRef() );
  QVERIFY( !url1.hasHTMLRef() );
  QVERIFY( url1.hasSubURL() );
  QCOMPARE( url1.htmlRef(), QString("") );
  QCOMPARE( url1.upURL().url(), QString("file:///home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/") );
}

void KURLTest::testSetUser()
{
  // The KURL equality test below works because in Qt4 null == empty.
  QString str1 = QString::null;
  QString str2 = "";
  QVERIFY( str1 == str2 );

  KURL emptyUserTest1( "http://www.foobar.com/");
  QVERIFY( emptyUserTest1.user().isEmpty() );
  QVERIFY( !emptyUserTest1.user().isNull() ); // small change compared to KURL-3.5
  KURL emptyUserTest2( "http://www.foobar.com/");
  emptyUserTest2.setUser( "" );
  //QVERIFY( emptyUserTest2.user().isNull() );
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
  QVERIFY( urlcmp(ucmp1,ucmp2) );

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

  QVERIFY( urlcmp(ucmp1,ucmp2,true,false) ); //only slash difference, ignore_trailing

  QString ucmp3 = "ftp://ftp.de.kde.org/dir/#";
  QVERIFY( urlcmp(ucmp2,ucmp3) ); // (only hash difference)
  QVERIFY( urlcmp(ucmp2,ucmp3,false,true) ); // (only hash difference, ignore_ref)
  QVERIFY( urlcmp(ucmp2,ucmp3,true,true) ); // (slash and hash difference, ignore_trailing, ignore_ref)
  QVERIFY( urlcmp("","",false,true) ); // (empty, empty)
  QVERIFY( urlcmp("","") ); // (empty, empty)
  QVERIFY( urlcmp("",ucmp1) ); // (empty, not empty)
  QVERIFY( urlcmp("",ucmp1,false,true) ); // (empty, not empty)
  QVERIFY( !urlcmp("file",ucmp1) ); // (malformed, not empty)
  QVERIFY( !urlcmp("file",ucmp1,false,true) ); // (malformed, not empty)

  KURL ftpUrl ( "ftp://ftp.de.kde.org" );
  qDebug("* URL is %s",ftpUrl.url().latin1());
  QCOMPARE( ftpUrl.path(), QString());
  ftpUrl = "ftp://ftp.de.kde.org/";
  QVERIFY( ftpUrl.isParentOf( "ftp://ftp.de.kde.org/host/subdir/") );
  ftpUrl = "ftp://ftp/host/subdir/";
  QVERIFY( ftpUrl.isParentOf( "ftp://ftp/host/subdir/") );
  QVERIFY( ftpUrl.isParentOf( "ftp://ftp/host/subdir") );
  QVERIFY( !ftpUrl.isParentOf( "ftp://ftp/host/subdi") );
  QVERIFY( ftpUrl.isParentOf( "ftp://ftp/host/subdir/blah/") );
  QVERIFY( !ftpUrl.isParentOf( "ftp://ftp/blah/subdir") );
  QVERIFY( !ftpUrl.isParentOf( "file:////ftp/host/subdir/") );
  QVERIFY( ftpUrl.isParentOf( "ftp://ftp/host/subdir/subsub") );
}

void KURLTest::testStreaming()
{
  kdDebug() << k_funcinfo << endl;
  // Streaming operators
  KURL origURL( "http://www.website.com/directory/?#ref" );
  KURL invalid = ""; // which other invalid url could we try here?
  QVERIFY( !invalid.isValid() );
  KURL waba1 = "http://[::ffff:129.144.52.38]:81?query";
  QByteArray buffer;
  {
      QDataStream stream( &buffer, QIODevice::WriteOnly );
      stream << origURL
             << invalid
             << waba1; // the IPv6 one
  }
  {
      QDataStream stream( buffer );
      KURL restoredURL;
      stream >> restoredURL; // streaming valid url
      QCOMPARE( restoredURL.url(), origURL.url() );
      stream >> restoredURL; // streaming invalid url
      QVERIFY( !restoredURL.isValid() );
      QCOMPARE( restoredURL.url(), invalid.url() );
      stream >> restoredURL; // streaming ipv6 url with query
      QCOMPARE( restoredURL.url(), waba1.url() );
  }
}

void KURLTest::testBrokenStuff()
{
  kdDebug() << k_funcinfo << endl;
  // Broken stuff
  KURL waba1 = "file:a";
  QCOMPARE( waba1.path(), QString("a") );
  QCOMPARE( waba1.fileName(false), QString("a") );
  QCOMPARE( waba1.fileName(true), QString("a") );
  QCOMPARE( waba1.directory(false, false), QString("") );
  QCOMPARE( waba1.directory(true, false), QString("") );
  QCOMPARE( waba1.directory(true, true), QString("") );

  waba1 = "file:a/";
  QCOMPARE( waba1.path(), QString("a/") );
  QCOMPARE( waba1.fileName(false), QString("") );
  QCOMPARE( waba1.fileName(true), QString("a") );
  QCOMPARE( waba1.directory(false, false), QString("a/") );
  QCOMPARE( waba1.directory(true, false), QString("a") );
  QCOMPARE( waba1.directory(true, true), QString("") );

  waba1 = "file:";
  QVERIFY( !waba1.isEmpty() );
  QVERIFY( waba1.isValid() ); // KDE3: was invalid. Now it's qurl with scheme="file".
  QCOMPARE( waba1.path(), QString("") );
  QCOMPARE( waba1.fileName(false), QString("") );
  QCOMPARE( waba1.fileName(true), QString("") );
  QCOMPARE( waba1.directory(false, false), QString("") );
  QCOMPARE( waba1.directory(true, false), QString("") );
  QCOMPARE( waba1.directory(true, true), QString("") );
  KURL broken;
  broken.setPath( QString::null );
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

  broken = "LABEL=USB_STICK"; // 71430, can we use KURL for this?
  QVERIFY( broken.isValid() ); // KDE3 difference: QUrl likes this one too
  QVERIFY( !broken.isEmpty() );
  QCOMPARE( broken.path(), QString("LABEL=USB_STICK") ); // was "" in KDE3

#if 0 // BROKEN?
  // UNC like names
  KURL unc1("FILE://localhost/home/root");
  QCOMPARE( unc1.path(), QString("/home/root") );
  QCOMPARE( unc1.url(), QString("file:///home/root") );
#endif
  KURL unc2("file:///home/root"); // with empty host
  QCOMPARE( unc2.path(), QString("/home/root") );
  QCOMPARE( unc2.url(), QString("file:///home/root") );

  {
     KURL unc3("FILE://remotehost/home/root");
#if 0 // BROKEN?
     QCOMPARE( unc3.path(), QString("//remotehost/home/root") );
#endif
     QCOMPARE( unc3.url(), QString("file://remotehost/home/root") );
     KURL url2("file://atlas/dfaure");
     QCOMPARE( url2.host(), QString("atlas") );
     QCOMPARE( url2.path(), QString("/dfaure") );
     //QCOMPARE( url3.path(), QString("//atlas/dfaure"); // says Wab)a
     //KURL url3("file:////atlas/dfaure");
     //QCOMPARE( url3.path(), QString("//atlas/dfaure"); // says Wab)a

     KURL url4(url2, "//remotehost/home/root");
     QCOMPARE( url4.host(), QString("remotehost") );
     QCOMPARE( url4.path(), QString("/home/root") );
  }

  broken = "ptal://mlc:usb:PC_970";
  QVERIFY( !broken.isValid() );
  QCOMPARE( broken.url(), QString("ptal://mlc:usb:PC_970") );

  KURL weird;
  weird = "http://strange<hostname>/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange<username>@strange<hostname>/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange<username>@ok_hostname/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://strange;hostname/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange;username@strange;hostname/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange;username@ok_hostname/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://strange;username:password@strange;hostname/";
  QVERIFY( !weird.isValid() );

  weird = "http://strange;username:password@ok_hostname/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("ok_hostname") );

  weird = "http://[strange;hostname]/";
  QVERIFY( !weird.isValid() );

  weird = "http://[::fff:1:23]/";
  QVERIFY( weird.isValid() );
  QCOMPARE( weird.host(), QString("::fff:1:23") );
}

void KURLTest::testMailto()
{
  kdDebug() << k_funcinfo << endl;
  KURL umail1 ( "mailto:faure@kde.org" );
  QCOMPARE( umail1.protocol(), QString("mailto") );
  QCOMPARE( umail1.path(), QString("faure@kde.org") );
  QVERIFY( !KURL::isRelativeURL("mailto:faure@kde.org") );
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  QCOMPARE( umail2.protocol(), QString("mailto") );
  QCOMPARE( umail2.path(), QString("Faure David <faure@kde.org>") );
  QVERIFY( !KURL::isRelativeURL("mailto:faure@kde.org") );
  KURL umail3 ( "mailto:" );
  QVERIFY( !umail3.isValid() );

  KURL url1 = "mailto:user@host.com";
  QCOMPARE( url1.url(), QString("mailto:user@host.com") );
  QCOMPARE( url1.url(0), QString("mailto:user@host.com") );
}

void KURLTest::testSmb()
{
  kdDebug() << k_funcinfo << endl;
  KURL smb("smb://domain;username:password@server/share");
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

void KURLTest::testOtherProtocols()
{
  kdDebug() << k_funcinfo << endl;
  KURL about("about:konqueror");
  QCOMPARE(about.path(),QString("konqueror") );


  KURL leo = "data:text/html,http://www.invalid/";
  QVERIFY( leo.isValid() );
  QCOMPARE( leo.protocol(), QString("data" ) );
  QCOMPARE( leo.url(), QString("data:text/html,http://www.invalid/" ) );
  QCOMPARE( leo.path(), QString("text/html,http://www.invalid/" ) );

  KURL ptal = "ptal://mlc:usb@PC_970"; // User=mlc, password=usb, host=PC_970
  QVERIFY( ptal.isValid() );
  QCOMPARE( ptal.host(), QString("pc_970") );
  QCOMPARE( ptal.user(), QString("mlc") );
  QCOMPARE( ptal.pass(), QString("usb") );
}

void KURLTest::testUtf8()
{
  kdDebug() << k_funcinfo << endl;
  QTextCodec* codec = QTextCodec::codecForName( "ISO-8859-1" );
  QVERIFY( codec != 0 );
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
  QCOMPARE( russian.url(), QString("file:///home/%C6%C7%CE7" ) );
}

void KURLTest::testPathOrURL()
{
  kdDebug() << k_funcinfo << endl;
  // fromPathOrURL tests
  KURL uloc = KURL::fromPathOrURL( "/home/dfaure/konqtests/Mat%E9riel" );
  QCOMPARE( uloc.path(), QString("/home/dfaure/konqtests/Mat%E9riel") );
  uloc = KURL::fromPathOrURL( "http://www.kde.org" );
  QCOMPARE( uloc.pathOrURL(), uloc.url() );
  uloc = KURL::fromPathOrURL( QString("www.kde.org" ) );
  QVERIFY( uloc.isValid() ); // KDE3: was invalid. But it's now a url with path="www.kde.org", ok.
  uloc = KURL::fromPathOrURL( "index.html" );
  QVERIFY( uloc.isValid() ); // KDE3: was invalid; same as above
  uloc = KURL::fromPathOrURL( "" );
  QVERIFY( !uloc.isValid() );

  // pathOrURL tests
  uloc = KURL::fromPathOrURL( "/home/dfaure/konqtests/Mat%E9riel" );
  QCOMPARE( uloc.pathOrURL(), uloc.path() );
  uloc = "http://www.kde.org";
  QCOMPARE( uloc.url(), QString("http://www.kde.org") );
  uloc = "file:///home/dfaure/konq%20tests/Mat%E9riel#ref";
  QCOMPARE( uloc.pathOrURL(), QString("file:///home/dfaure/konq tests/Matériel#ref" ) );
  uloc = "file:///home/dfaure/konq%20tests/Mat%E9riel?query";
  QCOMPARE( uloc.pathOrURL(), QString("file:///home/dfaure/konq tests/Matériel?query" ) );
  uloc = KURL::fromPathOrURL( "/home/dfaure/file#with#hash" );
  QCOMPARE( uloc.pathOrURL(), QString("/home/dfaure/file#with#hash" ) );
}

void KURLTest::testQueryItem()
{
  kdDebug() << k_funcinfo << endl;
  KURL theKow = "http://www.google.de/search?q=frerich&hlx=xx&hl=de&empty=&lr=lang+de&test=%2B%20%3A%25";
  QCOMPARE( theKow.queryItem("q"), QString("frerich") );
  QCOMPARE( theKow.queryItem("hl"), QString("de") );
  QCOMPARE( theKow.queryItem("lr"), QString("lang de") );
  QCOMPARE( theKow.queryItem("InterstellarCounselor"), QString() );
  QCOMPARE( theKow.queryItem("empty"), QString("") );
  QCOMPARE( theKow.queryItem("test"), QString("+ :%") );

  // checks for queryItems(), which returns a QMap<QString,QString>:
  KURL queryUrl( "mailto:Marc%20Mutz%20%3cmutz@kde.org%3E?"
		 "Subject=subscribe+me&"
		 "body=subscribe+mutz%40kde.org&"
		 "Cc=majordomo%40lists.kde.org" );
  QCOMPARE(QStringList(queryUrl.queryItems(0).keys()).join(", "),
	QString( "Cc, Subject, body" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).keys()).join(", "),
	QString( "body, cc, subject" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(0).values()).join(", "),
	QString( "majordomo@lists.kde.org, subscribe me, subscribe mutz@kde.org" ) );
  QCOMPARE(QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).values()).join(", "),
	QString( "subscribe mutz@kde.org, majordomo@lists.kde.org, subscribe me" ) );
  // TODO check for QUrl::queryItems

}

void KURLTest::testEncodeString()
{
  kdDebug() << k_funcinfo << endl;
  // Needed for #49616
  QCOMPARE( KURL::encode_string( "C++" ), QString("C%2B%2B") );
  QCOMPARE( KURL::decode_string( "C%2B%2B" ), QString("C++") );
  QCOMPARE( KURL::decode_string( "C%00%A" ), QString("C") ); // we stop at %00

  QCOMPARE( KURL::encode_string( "%" ), QString("%25") );
  QCOMPARE( KURL::encode_string( ":" ), QString("%3A") );
}

void KURLTest::testIdn()
{
  kdDebug() << k_funcinfo << endl;

  //qDebug( "trying QUrl with fromPercentEncoding" );
  QUrl qurltest( QUrl::fromPercentEncoding( "http://\303\244.de" ) ); // ä in utf8
  QVERIFY( qurltest.isValid() );

  //qDebug( "trying QUrl with fromEncoded" );
  QUrl qurl = QUrl::fromEncoded( "http://\303\244.de" ); // ä in utf8
  QVERIFY( qurl.isValid() );
  QCOMPARE( qurl.toEncoded().constData(), "http://xn--4ca.de" );

  //qDebug( "and now trying KURL" );
  KURL thiago( QString::fromUtf8( "http://\303\244.de" ) ); // ä in utf8
  QVERIFY( thiago.isValid() );
  QCOMPARE( thiago.url(), QString("http://xn--4ca.de") );   // Non-ascii is allowed in IDN domain names.

#if 0
  // A broken test - not using utf8. and amantia forgot the real-world testcase.
  KURL amantia( "http://%E1.foo.de" );
  QVERIFY( amantia.isValid() );
  QCOMPARE( amantia.url(), QString("http://xn--80a.foo.de") );   // Non-ascii is allowed in IDN domain names.
#endif

  // A more valid test for % in hostnames:
  KURL uwp( "http://%C3%A4.de" );
  QVERIFY( uwp.isValid() );
  QCOMPARE( thiago.url(), QString("http://xn--4ca.de") ); // as above
}

void KURLTest::testUriMode()
{
  kdDebug() << k_funcinfo << endl;
  KURL url1;
#if 0 // ###### TODO KUri
  url1 = "http://www.foobar.com/";
  QCOMPARE(url1.uriMode(), KURL::URL);
  url1 = "mailto:user@host.com";
  QCOMPARE(url1.uriMode(), KURL::Mailto);

  url1 = "data:text/plain,foobar?gazonk=flarp";
  QCOMPARE(url1.uriMode(), KURL::RawURI);

  QCOMPARE( url1.path(), QString("text/plain,foobar?gazonk=flarp") );
#endif
  url1 = "mailto:User@Host.COM?subject=Hello";
  QCOMPARE( url1.path(), QString("User@Host.COM") ); // KDE3: "User@host.com". Does it matter?
}

void KURLTest::testOther()
{
  // TODO categorize the rest of the tests

  KURL com1("http://server.com/dir/", ".");
  QCOMPARE( com1.url(), QString("http://server.com/dir/") );

  KURL com2("http://server.com/dir/blubb/", "blah/");
  QCOMPARE( com2.url(), QString("http://server.com/dir/blubb/blah/") );

  KURL utf8_1("audiocd:/By%20Name/15%20Geantra%C3%AE.wav"/*, 106*/);
  QCOMPARE( utf8_1.fileName(), QLatin1String("15 Geantraî.wav") );

  KURL utf8_2("audiocd:/By%20Name/15%2fGeantra%C3%AE.wav"/*, 106*/);
  QCOMPARE( utf8_2.fileName(), QLatin1String("15/Geantraî.wav") );

  QUrl qurl_newline_1 = QUrl::fromEncoded( "http://www.foo.bar/foo/bar\ngnork", QUrl::TolerantMode );
  QVERIFY( qurl_newline_1.isValid() );
  QCOMPARE( qurl_newline_1.toEncoded().constData(), "http://www.foo.bar/foo/bar%0Agnork" );

  KURL url_newline_1("http://www.foo.bar/foo/bar\ngnork");
  QCOMPARE( url_newline_1.url(), QLatin1String("http://www.foo.bar/foo/bar%0Agnork") );

  KURL url_newline_2("http://www.foo.bar/foo?bar\ngnork");
  QCOMPARE( url_newline_2.url(), QLatin1String("http://www.foo.bar/foo?bar%0Agnork") );

  KURL local_file_1("file://localhost/my/file");
  QVERIFY( local_file_1.isLocalFile() );

  KURL local_file_2("file://www.kde.org/my/file");
  QVERIFY( !local_file_2.isLocalFile() );

  KURL local_file_3;
  local_file_3.setHost(getenv("HOSTNAME"));
  local_file_3.setPath("/my/file");
  qDebug("URL=%s\n", local_file_3.url().latin1());
  QVERIFY( local_file_3.isLocalFile() );

  KURL local_file_4("file:///my/file");
  QVERIFY( local_file_4.isLocalFile() );

  KURL local_file_5;
  local_file_5.setPath("/foo?bar");
  QCOMPARE( local_file_5.url(), QString("file:///foo%3Fbar") );

  QString basePath = "/home/bastian";

  QCOMPARE( KURL::relativePath(basePath, "/home/bastian"), QString("./") );
  bool b;
  QCOMPARE( KURL::relativePath(basePath, "/home/bastian/src/plugins", &b), QString("./src/plugins"));
  QVERIFY( b );
  QCOMPARE( KURL::relativePath(basePath, "./src/plugins"), QString("./src/plugins") );
  QCOMPARE( KURL::relativePath(basePath, "/home/waba/src/plugins", &b), QString("../waba/src/plugins") );
  QVERIFY( b );
  QCOMPARE( KURL::relativePath(basePath, "/"), QString("../../"));

  QCOMPARE( KURL::relativePath("/", "/"), QString("./") );
  QCOMPARE( KURL::relativePath("/", "/home/bastian"), QString("./home/bastian") );
  QCOMPARE( KURL::relativePath("", "/home/bastian"), QString("/home/bastian") );

  KURL baseURL = "http://www.kde.org/index.html";
  QCOMPARE( KURL::relativeURL(baseURL, "http://www.kde.org/index.html#help"), QString("#help") );
  QCOMPARE( KURL::relativeURL(baseURL, "http://www.kde.org/index.html?help=true"), QString("index.html?help=true") );
  QCOMPARE( KURL::relativeURL(baseURL, "http://www.kde.org/contact.html"), QString("contact.html") );
  QCOMPARE( KURL::relativeURL(baseURL, "ftp://ftp.kde.org/pub/kde"), QString("ftp://ftp.kde.org/pub/kde") );
  QCOMPARE( KURL::relativeURL(baseURL, "http://www.kde.org/index.html"), QString("./") );

  baseURL = "http://www.kde.org/info/index.html";
  QCOMPARE( KURL::relativeURL(baseURL, "http://www.kde.org/bugs/contact.html"), QString( "../bugs/contact.html") );

  KURL ldap = "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)";
  QCOMPARE( ldap.host(), QString("host.com") );
  QCOMPARE( ldap.port(), 6666 );
  QCOMPARE( ldap.path(), QString("/o=University of Michigan,c=US") );
  QCOMPARE( ldap.query(), QString("??sub?(cn=Babs%20Jensen)") );
  QCOMPARE( ldap.url(), QString("ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)") );
  ldap.setQuery("??sub?(cn=Karl%20Marx)");
  QCOMPARE( ldap.query(), QString("??sub?(cn=Karl%20Marx)") );
  QCOMPARE( ldap.url(), QString("ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Karl%20Marx)") );
}
