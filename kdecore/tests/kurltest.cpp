#include <kurl.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qtextcodec.h>

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

#if QT_VERSION < 300
extern void qt_set_locale_codec( QTextCodec *codec );
#endif

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kurltest",false,false);
  KURL::List lst;

  KURL emptyURL;
  check( "KURL::isMalformed()", emptyURL.isMalformed() ? "TRUE":"FALSE", "TRUE");
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");

  emptyURL = "";
  check( "KURL::isMalformed()", emptyURL.isMalformed() ? "TRUE":"FALSE", "TRUE");
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");

  KURL baseURL ("http://www.foo.bar:80" );
  check( "KURL::isMalformed()", baseURL.isMalformed() ? "TRUE":"FALSE", "FALSE");
  KURL url1 ( baseURL, "//www1.foo.bar" );
  check( "KURL::url()", url1.url(), "http://www1.foo.bar");

  baseURL = "http://www.foo.bar";
  KURL rel_url( baseURL, "/top//test/../test1/file.html" );
  check( "KURL::url()", rel_url.url(), "http://www.foo.bar/top//test1/file.html" );

  baseURL = "http://www.foo.bar/top//test2/file2.html";
  check( "KURL::url()", baseURL.url(), "http://www.foo.bar/top//test2/file2.html" );

  baseURL = "file:/usr/local/src/kde2/////kdelibs/kio";
  check( "KURL::url()", baseURL.url(), "file:/usr/local/src/kde2/////kdelibs/kio" );

  baseURL = "file:/usr/local/src/kde2/kdelibs/kio/";
  KURL url2( baseURL, "../../////kdebase/konqueror" );
  check( "KURL::url()", url2.url(), "file:/usr/local/src/kde2/////kdebase/konqueror" );

  QString u1 = "file:/home/dfaure/my#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/my#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure/");

  url1 = "gg:www.kde.org";
  check("KURL::isMalformed()", url1.isMalformed()?"TRUE":"FALSE", "FALSE" );

  url1= "KDE";
  check("KURL::isMalformed()", url1.isMalformed()?"TRUE":"FALSE", "TRUE" );

  url1= "$HOME/.kde/share/config";
  check("KURL::isMalformed()", url1.isMalformed()?"TRUE":"FALSE", "TRUE" );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject::connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:/opt/kde2/qt2/doc/html/");

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject:connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:/opt/kde2/qt2/doc/html/");

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::isLocalFile()", url1.isLocalFile() ? "yes" : "no", "no"); // Not strictly local!
  //check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  //check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure/");

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
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure/");

#if 0
// This URL is broken, '#' should be escaped.
  u1 = "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "no");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::prettyURL()", url1.upURL().url(), "file:/home/dfaure/cdrdao-1.1.5/dao/#CdrDriver.cc#");
#endif

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/README");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");

  KURL notPretty("http://ferret.lmh.ox.ac.uk/%7Ekdecvs/");
  check("KURL::prettyURL()", notPretty.prettyURL(), "http://ferret.lmh.ox.ac.uk/~kdecvs/");
  KURL notPretty2("file:/home/test/directory%20with%20spaces");
  check("KURL::prettyURL()", notPretty2.prettyURL(), "file:/home/test/directory with spaces");
  KURL url15581("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  check("KURL::prettyURL()", url15581.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  check("KURL::url()", url15581.url(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html");
  KURL url15582("http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::prettyURL()", url15582.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::url()", url15582.url(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html");

  KURL carsten;
  carsten.setPath("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");
  check("KURL::path()", carsten.path(), "/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");

  KURL udir;
  printf("\n* Empty URL\n");
  check("KURL::url()", udir.url(), QString::null);
  check("KURL::isEmpty()", udir.isEmpty() ? "ok" : "ko", "ok");
  check("KURL::isMalformed()", udir.isMalformed() ? "ok" : "ko", "ok");
  udir = udir.upURL();
  check("KURL::upURL()", udir.upURL().isEmpty() ? "ok" : "ko", "ok");

  udir.setPath("/home/dfaure/file.txt");
  printf("\n* URL is %s\n",udir.url().ascii());
  check("KURL::path()", udir.path(), "/home/dfaure/file.txt");
  check("KURL::url()", udir.url(), "file:/home/dfaure/file.txt");
  check("KURL::directory(false,false)", udir.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", udir.directory(true,false), "/home/dfaure");

  KURL u2("/home/dfaure/");
  printf("\n* URL is %s\n",u2.url().ascii());
  // not ignoring trailing slash
  check("KURL::directory(false,false)", u2.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", u2.directory(true,false), "/home/dfaure");
  // ignoring trailing slash
  check("KURL::directory(false,true)", u2.directory(false,true), "/home/");
  check("KURL::directory(true,true)", u2.directory(true,true), "/home");
  u2.cd("..");
  check("KURL::cd(\"..\")", u2.url(), "file:/home");
  u2.cd("thomas");
  check("KURL::cd(\"thomas\")", u2.url(), "file:/home/thomas");
  u2.cd("/opt/kde/bin/");
  check("KURL::cd(\"/opt/kde/bin/\")", u2.url(), "file:/opt/kde/bin/");
  u2 = "ftp://ftp.kde.org/";
  printf("\n* URL is %s\n",u2.url().ascii());
  u2.cd("pub");
  check("KURL::cd(\"pub\")", u2.url(), "ftp://ftp.kde.org/pub");
  u2 = u2.upURL();
  check("KURL::upURL()", u2.url(), "ftp://ftp.kde.org/");
  u2 = u1;
  printf("\n* URL is %s\n",u2.url().ascii());
  // setFileName
  u2.setFileName( "myfile.txt" );
  check("KURL::setFileName()", u2.url(), "file:/home/dfaure/myfile.txt");
  u2.setFileName( "myotherfile.txt" );
  check("KURL::setFileName()", u2.url(), "file:/home/dfaure/myotherfile.txt");
  // more tricky, renaming a directory (kpropsdlg.cc, line ~ 238)
      QString tmpurl = "file:/home/dfaure/myolddir/";
      if ( tmpurl.at(tmpurl.length() - 1) == '/')
          // It's a directory, so strip the trailing slash first
          tmpurl.truncate( tmpurl.length() - 1);
      KURL newUrl = tmpurl;
      newUrl.setFileName( "mynewdir" );
  check("KURL::setFileName() special", newUrl.url(), "file:/home/dfaure/mynewdir");
  // even more tricky
  u2 = "print:/specials/Print%20To%20File%20(PDF%2FAcrobat)";
  printf("\n* URL is %s\n",u2.url().ascii());
  check("KURL::path()", u2.path(), "/specials/Print To File (PDF/Acrobat)");
  check("KURL::fileName()", u2.fileName(), "Print To File (PDF/Acrobat)");
  u2.setFileName( "" );
  check("KURL::setFileName()", u2.url(), "print:/specials/");

  u2 = "file:/specials/Print";
  printf("\n* URL is %s\n",u2.url().ascii());
  check("KURL::path()", u2.path(), "/specials/Print");
  check("KURL::fileName()", u2.fileName(), "Print");
  u2.setFileName( "" );
  check("KURL::setFileName()", u2.url(), "file:/specials/");

  const char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  printf("\n* URL is %s\n",u3);
  check("KURL::hasSubURL()", KURL(u3).hasSubURL() ? "yes" : "no", "no");
  lst.clear();
  lst = KURL::split( KURL(u3) );
  check("KURL::split()", lst.count()==1 ? "1" : "error", "1");
  check("KURL::split()", lst.first().url(), "ftp://host/dir1/dir2/myfile.txt");
  // cdUp code
  KURL lastUrl = lst.last();
  QString dir = lastUrl.directory( true, true );
  check( "KURL::directory(true,true)", dir, "/dir1/dir2");

  /// Comparisons
  QString ucmp1 = "ftp://ftp.de.kde.org/dir";
  QString ucmp2 = "ftp://ftp.de.kde.org/dir/";
  check("urlcmp(only slash difference)", urlcmp(ucmp1,ucmp2)?"ko":"ok","ok");
  check("urlcmp(only slash difference, ignore_trailing)", urlcmp(ucmp1,ucmp2,true,false)?"ok":"ko","ok");
  QString ucmp3 = "ftp://ftp.de.kde.org/dir/#";
  check("urlcmp(only hash difference)", urlcmp(ucmp2,ucmp3)?"ko":"ok","ok");
  check("urlcmp(only hash difference, ignore_ref)", urlcmp(ucmp2,ucmp3,false,true)?"ok":"ko","ok");
  check("urlcmp(slash and hash difference, ignore_trailing, ignore_ref)", urlcmp(ucmp2,ucmp3,true,true)?"ok":"ko","ok");

  KURL ftpUrl ( "ftp://ftp.de.kde.org" );
  printf("\n* URL is %s\n",ftpUrl.url().latin1());
  check("KURL::path()", ftpUrl.path(), QString::null);
  ftpUrl = "ftp://ftp.de.kde.org/";
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp.de.kde.org/host/subdir/") ? "yes" : "no", "yes");
  ftpUrl = "ftp://ftp/host/subdir/";
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdi") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/blah/") ? "yes" : "no", "yes");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/blah/subdir") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "file://ftp/host/subdir/") ? "yes" : "no", "no");
  check("KURL::isParentOf()", ftpUrl.isParentOf( "ftp://ftp/host/subdir/subsub") ? "yes" : "no", "yes");

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
  waba1.setUser("waldo");
  check("http: Set user", waba1.url(), "http://waldo@www.website.com/directory/filename?bla#blub");
  waba1.setUser("waldo/bastian");
  check("http: Set user with slash in it", waba1.url(), "http://waldo%2Fbastian@www.website.com/directory/filename?bla#blub");

  // Empty queries should be preserved!
  waba1 = "http://www.kde.org/cgi/test.cgi?";
  check("http: URL with empty query string", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?");

  // Empty references should be preserved
  waba1 = "http://www.kde.org/cgi/test.cgi#";
  check("http: URL with empty reference string", waba1.url(),
        "http://www.kde.org/cgi/test.cgi#");
  check("hasRef()", waba1.hasRef()?"true":"false","true");
  check("hasHTMLRef()", waba1.hasHTMLRef()?"true":"false","true");
  check("encodedHtmlRef()", waba1.encodedHtmlRef(),QString::null);

  // URLs who forgot to encode spaces in the query.
  waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value";
  check("http: URL with incorrect encoded query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello=My%20Value");

  // URL with ':' in query (':' should NOT be encoded!)
  waba1.setQuery("hello:My Value");
  check("http: URL with ':' in query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello:My%20Value");

  // URLs who forgot to encode spaces in the query.
  waba1 = "http://www.kde.org/cgi/test.cgi?hello=My Value+20";
  check("http: URL with incorrect encoded query", waba1.url(),
        "http://www.kde.org/cgi/test.cgi?hello=My%20Value+20");

  // Urls without path (BR21387)
  waba1 = "http://meine.db24.de?link=home_c_login_login";
  check("http: URL with empty path string", waba1.url(),
        "http://meine.db24.de?link=home_c_login_login");
  check("http: URL with empty path string path", waba1.path(),
        "");
  check("http: URL with empty path string path", waba1.query(),
        "?link=home_c_login_login");

  // Urls without path (BR21387)
  waba1 = "http://meine.db24.de#link=home_c_login_login";
  check("http: URL with empty path string", waba1.url(),
        "http://meine.db24.de#link=home_c_login_login");
  check("http: URL with empty path string path", waba1.path(),
        "");

  // IPV6
  waba1 = "http://[::FFFF:129.144.52.38]:81/index.html";
  check("http: IPV6 host", waba1.host(),
        "::FFFF:129.144.52.38");
  check("http: IPV6 port", QString("%1").arg(waba1.port()),
        "81");

  // IPV6
  waba1 = "http://waba:pass@[::FFFF:129.144.52.38]:81/index.html";
  check("http: IPV6 host", waba1.host(),
        "::FFFF:129.144.52.38");
  check("http: IPV6 host", waba1.user(),
        "waba");
  check("http: IPV6 host", waba1.pass(),
        "pass");
  check("http: IPV6 port", QString("%1").arg(waba1.port()),
        "81");

  // IPV6
  waba1 = "http://www.kde.org/cgi/test.cgi";
  waba1.setHost("::FFFF:129.144.52.38");
  check("http: IPV6 host", waba1.url(),
        "http://[::FFFF:129.144.52.38]/cgi/test.cgi");

  // Broken stuff
  waba1 = "file:a";
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


  // UNC like names
  KURL unc1("FILE://localhost/home/root");
  check("UNC, with localhost", unc1.path(), "/home/root");
  check("UNC, with localhost", unc1.url(), "file:/home/root");
  KURL unc2("file:///home/root");
  check("UNC, with empty host", unc2.path(), "/home/root");
  check("UNC, with empty host", unc2.url(), "file:/home/root");

  {
     KURL unc3("FILE://remotehost/home/root");
     check("UNC, with remote host", unc3.path(), "//remotehost/home/root");
     check("UNC, with remote host", unc3.url(), "file://remotehost/home/root");
     KURL url2("file://atlas/dfaure");
     check("KURL::host()", url2.path(), "//atlas/dfaure"); // says Waba
     KURL url3("file:////atlas/dfaure");
     check("KURL::host()", url3.path(), "//atlas/dfaure"); // says Waba
  }

  KURL umail1 ( "mailto:faure@kde.org" );
  check("mailto: URL, general form", umail1.protocol(), "mailto");
  check("mailto: URL, general form", umail1.path(), "faure@kde.org");
  check("mailto: URL, is relative", KURL::isRelativeURL("mailto:faure@kde.org") ? "true" : "false", "false");
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  check("mailto: URL, general form", umail2.protocol(), "mailto");
  check("mailto: URL, general form", umail2.path(), "Faure David <faure@kde.org>");
  check("isRelativeURL(\"mailto:faure@kde.org\")", KURL::isRelativeURL("mailto:faure@kde.org") ? "yes" : "no", "no");

  check("man: URL, is relative", KURL::isRelativeURL("man:mmap") ? "true" : "false", "false");
  check("javascript: URL, is relative", KURL::isRelativeURL("javascript:doSomething()") ? "true" : "false", "false");
  // more isRelative
  check("file: URL, is relative", KURL::isRelativeURL("file:/blah") ? "true" : "false", "false");
  check("/path, is relative", KURL::isRelativeURL("/path") ? "true" : "false", "true"); // arguable
  check("something, is relative", KURL::isRelativeURL("something") ? "true" : "false", "true");
  KURL about("about:konqueror");
  check("about:",about.path(),"konqueror");

  KURL ulong("https://swww.gad.de:443/servlet/CookieAccepted?MAIL=s@gad.de&VER=25901");
  check("host",ulong.host(),"swww.gad.de");

#if QT_VERSION < 300
  qt_set_locale_codec( KGlobal::charsets()->codecForName( "iso-8859-1" ) );
#else
  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "iso-8859-1" ) );
#endif
  // UTF8 tests
  KURL uloc("/home/dfaure/konqtests/Matériel");
  check("locale8bit",uloc.url().latin1(),"file:/home/dfaure/konqtests/Mat%E9riel"); // escaping the letter would be correct too
  check("pretty",uloc.prettyURL(),"file:/home/dfaure/konqtests/Matériel"); // escaping the letter would be correct too
  // 106 is MIB for UTF-8
  check("UTF8",uloc.url(0, 106),"file:/home/dfaure/konqtests/Mat%C3%A9riel");
  uloc = KURL("file:/home/dfaure/konqtests/Mat%C3%A9riel", 106);
  check("UTF8 path", uloc.path(), "/home/dfaure/konqtests/Matériel");

#if QT_VERSION < 300
  qt_set_locale_codec( KGlobal::charsets()->codecForName( "koi8-r" ) );
#else
  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "koi8-r" ) );
#endif
  baseURL = "file:/home/coolo";
  KURL russian = baseURL.directory(false, true) + QString::fromLocal8Bit( "ÆÇÎ7" );
  check( "russian", russian.url(), "file:/home/%C6%C7%CE7" );

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
	QStringList(queryUrl.queryItems().keys()).join(", "),
	"Cc, Subject, body" );
  check("queryItems (c.i.s. keys)",
	QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).keys()).join(", "),
	"body, cc, subject" );
  check("queryItems (values; c.s. keys)",
	QStringList(queryUrl.queryItems().values()).join(", "),
	"majordomo@lists.kde.org, subscribe me, subscribe mutz@kde.org" );
  check("queryItems (values; c.i.s. keys)",
	QStringList(queryUrl.queryItems(KURL::CaseInsensitiveKeys).values()).join(", "),
	"subscribe mutz@kde.org, majordomo@lists.kde.org, subscribe me" );

  KURL umlaut1("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel");
  check("umlaut1.url()", umlaut1.url(), "http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel");

  KURL umlaut2("http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel", 106);
  check("umlaut2.url()", umlaut2.url(), "http://www.clever-tanken.de/liste.asp?ort=N%FCrnberg&typ=Diesel");

  // Needed for #49616
  check( "encode_string('C++')", KURL::encode_string( "C++" ), "C%2B%2B" );
  check( "decode_string('C%2B%2B')", KURL::decode_string( "C%2B%2B" ), "C++" );

  check( "encode_string('%')", KURL::encode_string( "%" ), "%25" );
  check( "encode_string(':')", KURL::encode_string( ":" ), "%3A" );

  KURL smb("smb://domain;username:password@server/share");
  check("smb.isValid()", smb.isValid() ? "true" : "false", "true");
  check("smb.user()", smb.user(), "domain;username");

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

  KURL com1("http://server.com/dir/", ".");
  check("com1.url()", com1.url(), "http://server.com/dir/");

  KURL com2("http://server.com/dir/blubb/", "blah/");
  check("com2.url()", com2.url(), "http://server.com/dir/blubb/blah/");

  printf("\nTest OK !\n");
}

