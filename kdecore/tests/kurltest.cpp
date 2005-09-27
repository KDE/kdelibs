#include <config.h>

#include <kurl.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qtextcodec.h>
#include <qdatastream.h>
#include <qmap.h>
#include <assert.h>
#include <kcmdlineargs.h>

static bool check(QString txt, QString a, QString b)
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

void testAdjustPath()
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

int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "kurltest", 0, 0, 0, 0 );
  KApplication app( false, false );

  KURL::List lst;

  KURL emptyURL;
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");
  check( "prettyURL()", emptyURL.prettyURL(), "");

  emptyURL = "";
  check( "KURL::isValid()", emptyURL.isValid() ? "TRUE":"FALSE", "FALSE");
  check( "KURL::isEmpty()", emptyURL.isEmpty() ? "TRUE":"FALSE", "TRUE");

  KURL fileURL = "file:/";
  check( "KURL::isEmpty()", fileURL.isEmpty() ? "TRUE":"FALSE", "FALSE");

  fileURL = "file:///";
  check( "KURL::isEmpty()", fileURL.isEmpty() ? "TRUE":"FALSE", "FALSE");

  KURL baseURL ("hTTp://www.foo.bar:80" );
  check( "KURL::isValid()", baseURL.isValid() ? "TRUE":"FALSE", "TRUE");
  check( "KURL::protocol()", baseURL.protocol(), "http"); // lowercase
  KURL url1 ( baseURL, "//www1.foo.bar" );
  check( "KURL::host()", url1.host(), "www1.foo.bar");
  check( "KURL::url()", url1.url(), "http://www1.foo.bar");

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

  QString u1 = "file:/home/dfaure/my#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

  u1 = "file:/home/dfaure/my#%2f";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#%2f");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::encodedHtmlRef()", url1.ref(), "%2f");
  check("KURL::htmlRef()", url1.htmlRef(), "/");

  url1 = KURL(url1, "#%6a");
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#%6a");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::encodedHtmlRef()", url1.ref(), "%6a");
  check("KURL::htmlRef()", url1.htmlRef(), "j");

  u1 = "file:///home/dfaure/my#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///home/dfaure/my#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::upURL()", url1.upURL().url(), "file:///home/dfaure/");

  url1 = "gg:www.kde.org";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "TRUE" );

  url1= "KDE";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "FALSE" );

  url1= "$HOME/.kde/share/config";
  check("KURL::isValid()", url1.isValid()?"TRUE":"FALSE", "FALSE" );

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:///opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject::connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:///opt/kde2/qt2/doc/html/");

  url1 = KURL( QByteArray( "http://www.kde.org/foo.cgi?foo=bar" ) );
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

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  url1 = u1;
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

  KURL notPretty("http://ferret.lmh.ox.ac.uk/%7Ekdecvs/");
  check("KURL::prettyURL()", notPretty.prettyURL(), "http://ferret.lmh.ox.ac.uk/~kdecvs/");
  KURL notPretty2("file:/home/test/directory%20with%20spaces");
  check("KURL::prettyURL()", notPretty2.prettyURL(), "file:///home/test/directory with spaces");
  KURL notPretty3("fish://foo/%23README%23");
  check("KURL::prettyURL()", notPretty3.prettyURL(), "fish://foo/%23README%23");
  KURL url15581("http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  check("KURL::prettyURL()", url15581.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces in url.html");
  check("KURL::url()", url15581.url(), "http://alain.knaff.linux.lu/bug-reports/kde/spaces%20in%20url.html");
  KURL url15582("http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::prettyURL()", url15582.prettyURL(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%in%url.html");
  check("KURL::url()", url15582.url(), "http://alain.knaff.linux.lu/bug-reports/kde/percentage%25in%25url.html");

  KURL carsten;
  carsten.setPath("/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");
  check("KURL::path()", carsten.path(), "/home/gis/src/kde/kdelibs/kfile/.#kfiledetailview.cpp.1.18");

  KURL charles;
  charles.setPath( "/home/charles/foo%20moo" );
  check("KURL::path()", charles.path(), "/home/charles/foo%20moo");
  KURL charles2("file:/home/charles/foo%20moo");
  check("KURL::path()", charles2.path(), "/home/charles/foo moo");

  KURL udir;
  printf("\n* Empty URL\n");
  check("KURL::url()", udir.url(), QString::null);
  check("KURL::isEmpty()", udir.isEmpty() ? "ok" : "ko", "ok");
  check("KURL::isValid()", udir.isValid() ? "ok" : "ko", "ko");
  udir = udir.upURL();
  check("KURL::upURL()", udir.upURL().isEmpty() ? "ok" : "ko", "ok");

  udir.setPath("/home/dfaure/file.txt");
  printf("\n* URL is %s\n",udir.url().ascii());
  check("KURL::path()", udir.path(), "/home/dfaure/file.txt");
  check("KURL::url()", udir.url(), "file:///home/dfaure/file.txt");
  check("KURL::directory(false,false)", udir.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", udir.directory(true,false), "/home/dfaure");

  KURL u2( QByteArray("/home/dfaure/") );
  printf("\n* URL is %s\n",u2.url().ascii());
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

  u2.cd("..");
  check("KURL::cd(\"..\")", u2.url(), "file:///home");
  u2.cd("thomas");
  check("KURL::cd(\"thomas\")", u2.url(), "file:///home/thomas");
  u2.cd("../");
  check("KURL::cd(\"../\")", u2.url(), "file:///home/");
  u2.cd("/opt/kde/bin/");
  check("KURL::cd(\"/opt/kde/bin/\")", u2.url(), "file:///opt/kde/bin/");
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
  check("KURL::setFileName()", u2.url(), "file:///home/dfaure/myfile.txt");
  u2.setFileName( "myotherfile.txt" );
  check("KURL::setFileName()", u2.url(), "file:///home/dfaure/myotherfile.txt");
  // more tricky, renaming a directory (kpropsdlg.cc, line ~ 238)
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
  check("KURL::setFileName()", u2.url(), "file:///specials/");

  const char * u6 = "ftp://host/dir1/dir2/myfile.txt";
  printf("\n* URL is %s\n",u6);
  check("KURL::hasSubURL()", KURL(u6).hasSubURL() ? "yes" : "no", "no");
  lst.clear();
  lst = KURL::split( KURL(u6) );
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
  check("urlcmp(empty, empty)", urlcmp("","",false,true)?"ok":"ko","ok");
  check("urlcmp(empty, empty)", urlcmp("","")?"ok":"ko","ok");
  check("urlcmp(empty, not empty)", urlcmp("",ucmp1)?"ok":"ko","ko");
  check("urlcmp(empty, not empty)", urlcmp("",ucmp1,false,true)?"ok":"ko","ko");
  check("urlcmp(malformed, not empty)", urlcmp("file",ucmp1)?"ok":"ko","ko");
  check("urlcmp(malformed, not empty)", urlcmp("file",ucmp1,false,true)?"ok":"ko","ko");

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
  check("KURL::isParentOf()", ftpUrl.isParentOf( "file:////ftp/host/subdir/") ? "yes" : "no", "no");
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
  check("upURL() removes query", waba1.upURL().url(),
        "http://www.kde.org/cgi/test.cgi");

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
  check("http: URL with empty path string query", waba1.query(),
        "?link=home_c_login_login");

  waba1 = "http://a:389?b=c";
  check( "http: URL with port, query, and empty path; url", waba1.url(), "http://a:389?b=c" );
  check( "http: URL with port, query, and empty path; host", waba1.host(), "a" );
  check( "http: URL with port, query, and empty path; port", QString::number( waba1.port() ), "389" );
  check( "http: URL with port, query, and empty path; path", waba1.path(), "" );
  check( "http: URL with port, query, and empty path; query", waba1.query(), "?b=c" );

  // Urls without path (BR21387)
  waba1 = "http://meine.db24.de#link=home_c_login_login";
  check("http: URL with empty path string", waba1.url(),
        "http://meine.db24.de#link=home_c_login_login");
  check("http: URL with empty path string path", waba1.path(),
        "");

  waba1 = "http://a:389#b=c";
  check( "http: URL with port, ref, and empty path; url", waba1.url(), "http://a:389#b=c" );
  check( "http: URL with port, ref, and empty path; host", waba1.host(), "a" );
  check( "http: URL with port, ref, and empty path; port", QString::number( waba1.port() ), "389" );
  check( "http: URL with port, ref, and empty path; path", waba1.path(), "" );
  check( "http: URL with port, ref, and empty path; ref", waba1.ref(), "b=c" );
  check( "http: URL with port, ref, and empty path; query", waba1.query(), "" );

  // IPV6
  waba1 = "http://[::FFFF:129.144.52.38]:81/index.html";
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

  // Streaming operators
  KURL origURL( "http://www.website.com/directory/?#ref" );
  waba1 = "http://[::ffff:129.144.52.38]:81?query";
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
      check( "Streaming valid URL", origURL.url(), restoredURL.url() );
      stream >> restoredURL;
      check( "Streaming invalid URL", restoredURL.isValid()?"valid":"malformed", "malformed" );
      check( "Streaming invalid URL", restoredURL.url(), "file:" );
      stream >> restoredURL;
      check( "Streaming ipv6 URL with query", restoredURL.url(), waba1.url() );
  }

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

  check("man: URL, is relative", KURL::isRelativeURL("man:mmap") ? "true" : "false", "false");
  check("javascript: URL, is relative", KURL::isRelativeURL("javascript:doSomething()") ? "true" : "false", "false");
  // more isRelative
  check("file: URL, is relative", KURL::isRelativeURL("file:///blah") ? "true" : "false", "false");
  check("/path, is relative", KURL::isRelativeURL("/path") ? "true" : "false", "true"); // arguable, but necessary for KURL( baseURL, "//www1.foo.bar" );
  check("something, is relative", KURL::isRelativeURL("something") ? "true" : "false", "true");
  KURL about("about:konqueror");
  check("about:",about.path(),"konqueror");

  KURL ulong("https://swww.gad.de:443/servlet/CookieAccepted?MAIL=s@gad.de&VER=25901");
  check("host",ulong.host(),"swww.gad.de");
  check("path",ulong.path(),"/servlet/CookieAccepted");

  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "iso-8859-1" ) );
  // UTF8 tests
  KURL uloc("/home/dfaure/konqtests/Matériel");
  check("url",uloc.url().latin1(),"file:///home/dfaure/konqtests/Mat%E9riel");
  check("pretty",uloc.prettyURL(),"file:///home/dfaure/konqtests/Matériel"); // escaping the letter would be correct too
  check("pretty + strip",uloc.prettyURL(0, KURL::StripFileProtocol),"/home/dfaure/konqtests/Matériel"); // escaping the letter would be correct too
  // 106 is MIB for UTF-8
  check("UTF8",uloc.url(0, 106),"file:///home/dfaure/konqtests/Mat%C3%A9riel");
  uloc = KURL("file:///home/dfaure/konqtests/Mat%C3%A9riel", 106);
  check("UTF8 path", uloc.path(), "/home/dfaure/konqtests/Matériel");

  // fromPathOrURL tests
  uloc = KURL::fromPathOrURL( "/home/dfaure/konqtests/Mat%E9riel" );
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

  testAdjustPath();

#if QT_VERSION < 300
  qt_set_locale_codec( KGlobal::charsets()->codecForName( "koi8-r" ) );
#else
  QTextCodec::setCodecForLocale( KGlobal::charsets()->codecForName( "koi8-r" ) );
#endif
  baseURL = "file:/home/coolo";
  KURL russian = baseURL.directory(false, true) + QString::fromLocal8Bit( "ÆÇÎ7" );
  check( "russian", russian.url(), "file:///home/%C6%C7%CE7" );

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
  check( "decode_string('C%00A')", KURL::decode_string( "C%00%A" ), "C" ); // we stop at %00

  check( "encode_string('%')", KURL::encode_string( "%" ), "%25" );
  check( "encode_string(':')", KURL::encode_string( ":" ), "%3A" );

  KURL amantia( "http://%E1.foo.de" );
  check("amantia.isValid()", amantia.isValid() ? "true" : "false", "true");
#ifdef HAVE_IDNA_H
  check("amantia.url()", amantia.url(), "http://xn--80a.foo.de");   // Non-ascii is allowed in IDN domain names.
#else
  check("amantia.url()", amantia.url(), "http://?.foo.de"); // why not
#endif

  KURL thiago( QString::fromUtf8( "http://\303\244.de" ) ); // ä in utf8
  check("thiago.isValid()", thiago.isValid() ? "true" : "false", "true");
#ifdef HAVE_IDNA_H
  check("thiago.url()", thiago.url(), "http://xn--4ca.de");   // Non-ascii is allowed in IDN domain names.
#else
  check("thiago.url()", thiago.url(), QString::fromUtf8( "http://\303\244.de" ) );
#endif


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

  KURL utf8_1("audiocd:/By%20Name/15%20Geantra%C3%AE.wav", 106);
  check("utf8_1.fileName()", utf8_1.fileName(), QLatin1String("15 Geantraî.wav"));

  KURL utf8_2("audiocd:/By%20Name/15%2fGeantra%C3%AE.wav", 106);
  check("utf8_2.fileName()", utf8_2.fileName(), QLatin1String("15/Geantraî.wav"));

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
  printf("\nURL=%s\n", local_file_3.url().latin1());
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

  baseURL = "http://www.kde.org/index.html";
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html#help\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html#help"), "#help");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html?help=true\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html?help=true"), "index.html?help=true");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/contact.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/contact.html"), "contact.html");
  check("relativeURL(\"http://www.kde.org/index.html\", \"ftp://ftp.kde.org/pub/kde\")", KURL::relativeURL(baseURL, "ftp://ftp.kde.org/pub/kde"), "ftp://ftp.kde.org/pub/kde");
  check("relativeURL(\"http://www.kde.org/index.html\", \"http://www.kde.org/index.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/index.html"), "./");

  baseURL = "http://www.kde.org/info/index.html";
  check("relativeURL(\"http://www.kde.org/info/index.html\", \"http://www.kde.org/bugs/contact.html\")", KURL::relativeURL(baseURL, "http://www.kde.org/bugs/contact.html"), "../bugs/contact.html");

  baseURL = "ptal://mlc:usb:PC_970";
  check("isValid()?", baseURL.isValid() ? "true" : "false", "false");
  check("url()", baseURL.url(), "ptal://mlc:usb:PC_970");

  baseURL = "http://mlc:80/";
  check("isValid()?", baseURL.isValid() ? "true" : "false", "true");
  check("port()?", QString::number(baseURL.port()), "80");
  check("path()?", baseURL.path(), "/");

  baseURL = "ptal://mlc:usb@PC_970"; // User=mlc, password=usb, host=PC_970
  check("isValid()?", baseURL.isValid() ? "true" : "false", "true");
  check("host()?", baseURL.host(), "pc_970");
  check("user()?", baseURL.user(), "mlc");
  check("pass()?", baseURL.pass(), "usb");

  weird = "ftp://user%40host.com@ftp.host.com/var/www/";
  check("user()?", weird.user(), "user@host.com" );
  check("host()?", weird.host(), "ftp.host.com" );
  KURL up = weird.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/var/");
  up = up.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/");
  up = up.upURL();
  check("KURL::upURL()", up.url(), "ftp://user%40host.com@ftp.host.com/"); // unchanged

  KURL ldap = "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)";
  check("host()?", ldap.host(), "host.com");
  check("port()?", QString("%1").arg(ldap.port()), "6666");
  check("path()?", ldap.path(), "/o=University of Michigan,c=US");
  check("query()?", ldap.query(), "??sub?(cn=Babs%20Jensen)");
  check("url()?", ldap.url(), "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)");
  ldap.setQuery("??sub?(cn=Karl%20Marx)");
  check("query()?", ldap.query(), "??sub?(cn=Karl%20Marx)");
  check("url()?", ldap.url(), "ldap://host.com:6666/o=University%20of%20Michigan,c=US??sub?(cn=Karl%20Marx)");

  KURL leo = "data:text/html,http://www.invalid/";
  check("data URL: isValid", leo.isValid()?"valid":"malformed", "valid" );
  check("data URL: protocol", leo.protocol(), "data" );
  check("data URL: url", leo.url(), "data:text/html,http://www.invalid/" );
  check("data URL: path", leo.path(), "text/html,http://www.invalid/" );

  // URI Mode tests
  url1 = "http://www.foobar.com/";
  check("KURL(\"http://www.foobar.com/\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::URL));
  url1 = "mailto:user@host.com";
  check("KURL(\"mailto:user@host.com\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::Mailto));
  check("KURL(\"mailto:user@host.com\").url()", url1.url(), "mailto:user@host.com");
  check("KURL(\"mailto:user@host.com\").url(0, 106)", url1.url(0, 106), "mailto:user@host.com");
  url1 = "data:text/plain,foobar?gazonk=flarp";
  check("KURL(\"data:text/plain,foobar?gazonk=flarp\").uriMode()", QString::number(url1.uriMode()), QString::number(KURL::RawURI));
  check("KURL(\"data:text/plain,foobar?gazonk=flarp\").path()", url1.path(), "text/plain,foobar?gazonk=flarp");
  url1 = "mailto:User@Host.COM?subject=Hello";
  check("KURL(\"mailto:User@Host.COM?subject=Hello\").path()", url1.path(), "User@host.com");

  printf("\nTest OK !\n");
}

