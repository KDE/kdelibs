#include <kurl.h>
#include <stdio.h>
#include <kprotocolmanager.h>
#include <kapp.h>
bool check(QString txt, QString a, QString b)
{
  printf("%s : checking '%s' against expected value '%s'... ",
         txt.data(), a.data(), b.data());
  if (a == b)
    printf("ok\n");
  else {
    printf("KO ! \n");
    exit(1);
  }
}

int main(int argc, char *argv[]) 
{
  KApplication app(argc,argv,"kurltest");
  KURL::List lst;

//  char * u1 = "file:/home/dfaure/my tar file.tgz#gzip:/decompress#tar:/";
  const char * u1 = "tar:/#gzip:/decompress#file:/home/dfaure/my%20tar%20file.tgz";
  KURL url1(u1);
  printf("\n* URL to be split is %s\n",u1);
  //check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/decompress#tar:/");
  check("KURL::url()", url1.url(), "tar:/#gzip:/decompress#file:/home/dfaure/my%20tar%20file.tgz");
  check("KURL::protocol()", url1.protocol(), "tar");
  check("KURL::path()", url1.path(), "/");
  check("KURL::hasPath()", url1.hasPath()  ? "yes" : "no", "yes");
  check("KURL::host()", url1.host(), QString::null);
  check("KURL::ref()", url1.ref(), "gzip:/decompress#file:/home/dfaure/my%20tar%20file.tgz");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  lst = KURL::split( u1 );
/*
  KURL * u = lst.first();
  for ( ; u ; u = lst.next())
    printf("---> %s\n",u->url().data());
*/


  KURL udir("/home/dfaure/file.txt");
  printf("\n* URL is %s\n",udir.url().data());
  check("KURL::directory(false,false)", udir.directory(false,false), "/home/dfaure/");
  check("KURL::directory(true,false)", udir.directory(true,false), "/home/dfaure");
  check("KURL::path()", udir.path(), "/home/dfaure/file.txt");

  KURL u2("/home/dfaure/");
  printf("\n* URL is %s\n",u2.url().data());
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
  printf("\n* URL is %s\n",u2.url().data());
  u2.cd("pub");
  check("KURL::cd(\"pub\")", u2.url(), "ftp://ftp.kde.org/pub");
  u2 = u2.upURL();
  check("KURL::upURL()", u2.url(), "ftp://ftp.kde.org/");
  u2 = u1;
  printf("\n* URL is %s\n",u2.url().data());
  u2.cd("dir");
  check("KURL::cd(\"dir\")", u2.url(), "tar:/dir#gzip:/decompress#file:/home/dfaure/my%20tar%20file.tgz");
  u2 = u2.upURL();
  check("KURL::upURL()", u2.url(), "tar:/#gzip:/decompress#file:/home/dfaure/my%20tar%20file.tgz");
  u2 = u2.upURL();
  check("KURL::upURL()", u2.url(), "file:/home/dfaure/");
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
  lst = KURL::split( u3 );
  check("KURL::split()", lst.count()==1 ? "1" : "error", "1");
  check("KURL::split()", lst.first().url(), "ftp://host/dir1/dir2/myfile.txt");
  // cdUp code
  KURL lastUrl = lst.last();
  QString dir = lastUrl.directory( true, true );
  check( "KURL::directory(true,true)", dir, "/dir1/dir2");

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

  QString remoteProtocol( KProtocolManager::self().remoteFileProtocol() );
  if (remoteProtocol.isEmpty())
  {
     KURL unc3("FILE://remotehost/home/root");
     check("UNC, with remote host", unc3.path(), "//remotehost/home/root");
     check("UNC, with remote host", unc3.url(), "file://remotehost/home/root");
     KURL url2("file://atlas/dfaure");
     check("KURL::host()", url2.path(), "//atlas/dfaure"); // says Waba
     KURL url3("file:////atlas/dfaure");
     check("KURL::host()", url3.path(), "//atlas/dfaure"); // says Waba
  }
  else
  {
     KURL unc3("FILE://remotehost/home/root");
     check("UNC, with remote host", unc3.path(), "/home/root");
     check("UNC, with remote host", unc3.url(), remoteProtocol+"://remotehost/home/root");
     KURL url2("file://atlas/dfaure");
     check("KURL::host()", url2.path(), "/dfaure");
     check("KURL::url()", url2.path(), remoteProtocol+"://atlas/dfaure");
  }

  KURL umail1 ( "mailto:faure@kde.org" );
  check("mailto: URL, general form", umail1.protocol(), "mailto");
  check("mailto: URL, general form", umail1.path(), "faure@kde.org");
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  check("mailto: URL, general form", umail2.protocol(), "mailto");
  check("mailto: URL, general form", umail2.path(), "Faure David <faure@kde.org>");
  printf("\nTest OK !\n");
}
