#include <kurl.h>
#include <stdio.h>

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

int main() 
{
  KURLList lst;

  char * u1 = "file:/home/dfaure/my tar file.tgz#gzip:/decompress#tar:/";
  KURL url1(u1);
  printf("\n* URL to be split is %s\n",u1);
  check("KURL::url()", url1.url(), "file:/home/dfaure/my%20tar%20file.tgz#gzip:/decompress#tar:/");
  check("KURL::protocol()", url1.protocol(), "file");
  check("KURL::path()", url1.path(), "/home/dfaure/my tar file.tgz");
  check("KURL::host()", url1.host(), "");
  check("KURL::ref()", url1.ref(), "gzip:/decompress#tar:/");
  check("KURL::hasSubURL()", url1.hasSubURL() ? "yes" : "no", "yes");
  KURL::split( u1, lst );
  KURL * u = lst.first();
  for ( ; u ; u = lst.next())
    printf("---> %s\n",u->url().data());

  KURL url2("file://atlas/dfaure");
  check("KURL::host()", url2.host(), "atlas"); // says Coolo

  KURL u2("/home/dfaure/");
  u2.cd("..");
  printf("\n* URL is %s",u2.url().data());
  check("KURL::cd(\"..\")", u2.url(), "file:/home");
  u2.cd("thomas");
  check("KURL::cd(\"thomas\")", u2.url(), "file:/home/thomas");
  u2.cd("/opt/kde/bin/");
  check("KURL::cd(\"/opt/kde/bin/\")", u2.url(), "file:/opt/kde/bin");

  char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  printf("\n* URL is %s\n",u3);
  check("KURL::hasSubURL()", KURL(u3).hasSubURL() ? "yes" : "no", "no");
  lst.clear();
  KURL::split( u3, lst );
  check("KURL::split()", lst.count()==1 ? "1" : "error", "1");
  check("KURL::split()", lst.first()->url(), "ftp://host/dir1/dir2/myfile.txt");
  // cdUp code
  KURL * lastUrl = lst.getLast();
  QString dir = lastUrl->directory( true, true );
  check( "KURL::directory(true,true)", dir, "/dir1/dir2");
  lastUrl->setPath( dir );
  QString _url;
  KURL::join( lst, _url );
  check( "up one directory", _url, "ftp://host/dir1/dir2");

  KURL umail1 ( "mailto:faure@kde.org" );
  check("mailto: URL, general form", umail1.protocol(), "mailto");
  check("mailto: URL, general form", umail1.path(), "faure@kde.org");
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  check("mailto: URL, general form", umail2.protocol(), "mailto");
  check("mailto: URL, general form", umail2.path(), "Faure David <faure@kde.org>");
  printf("\nTest OK !\n");
}
