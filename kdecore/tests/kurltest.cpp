#include <kurl.h>
#include <stdio.h>
#include <kapp.h>
#include <stdlib.h>
#include <kdebug.h>

////NOTE: for this test to run properly, you need first to
//// cd kdebase/kioslave/gzip ; cp *desktop $KDEDIR/share/config/protocols/
//// cd ../tar ; cp *desktop $KDEDIR/share/config/protocols/
////

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

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kurltest",false,false);
  KURL::List lst;

  QString u1 = "file:/home/dfaure/my#myref";
  KURL url1(u1);
  check("KURL::url()", url1.url(), "file:/home/dfaure/my#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure");

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject::connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject::connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:/opt/kde2/qt2/doc/html");

  u1 = "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/opt/kde2/qt2/doc/html/showimg-main-cpp.html#QObject:connect");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "QObject:connect");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "no");
  check("KURL::upURL()", url1.upURL().url(), "file:/opt/kde2/qt2/doc/html");

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/#myref");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "yes");
  check("KURL::htmlRef()", url1.htmlRef(), "myref");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure");

  u1 = "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/";
  url1 = u1;
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/#tar:/");
  check("KURL::hasRef()", url1.hasRef() ? "yes" : "no", "yes");
  check("KURL::hasHTMLRef()", url1.hasHTMLRef() ? "yes" : "no", "no");
  check("KURL::htmlRef()", url1.htmlRef(), "");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  check("KURL::upURL()", url1.upURL().url(), "file:/home/dfaure");

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

  KURL ftpUrl ( "ftp://ftp.de.kde.org" );
  printf("\n* URL is %s\n",ftpUrl.url().latin1());
  check("KURL::path()", ftpUrl.path(), QString::null);
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

  KURL waba1( "http://www.website.com/directory/" );
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
     KURL waba2( waba1, "relative.html#with_reference");
     check("http: Relative URL, with reference", waba2.url(), "http://www.website.com/directory/relative.html#with_reference");
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

  // Empty references should be left out.
  waba1 = "http://www.kde.org/cgi/test.cgi#";
  check("http: URL with empty reference string", waba1.url(),
        "http://www.kde.org/cgi/test.cgi");


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

  printf("\nTest OK !\n");
}
