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
  char * u1 = "file:/home/dfaure/my tar file.tgz#gzip:/decompress#tar:/";
  KURL url1(u1);
  printf("\n* URL is %s\n",u1);
  check("KURL::url()", url1.url(), "file:/home/dfaure/my tar file.tgz#gzip:/decompress#tar:/");  // K2URL returns encoded
  check("KURL::protocol()", url1.protocol(), "file");
  check("KURL::path()", url1.path(), "/home/dfaure/my tar file.tgz#gzip:/decompress#tar:/"); // K2URL doesn't return reference
  check("KURL::host()", url1.host(), "");
  check("KURL::reference()", url1.reference(), "");  // K2URL returns gzip:/ ...
  check("KURL::hasSubProtocol()", url1.hasSubProtocol() ? "yes" : "no", "yes");

  KURL url2("file://atlas/dfaure");
  check("KURL::host()", url2.host(), "atlas"); // says Coolo

  KURL u2("/home/dfaure/");
  u2.cd("..");
  printf("\n* URL is %s",u2.url().data());
  check("KURL::cd(\"..\")", u2.url(), "file:/home");
  u2.cd("thomas");
  check("KURL::cd(\"thomas\")", u2.url(), "file:/home/thomas");
  u2.cd("/opt/kde/bin/");
  check("KURL::cd(\"/opt/kde/bin/\")", u2.url(), "file:/opt/kde/bin/"); // no trailing slash for K2URL (minor)

  char * u3 = "ftp://host/dir1/dir2/myfile.txt";
  printf("\n* URL is %s\n",u3);
  check("KURL::hasSubProtocol()", KURL(u3).hasSubProtocol() ? "yes" : "no", "no");

  KURL umail1 ( "mailto:faure@kde.org" );
  check("mailto: URL, small form", umail1.protocol(), "mailto");
  check("mailto: URL, small form", umail1.path(), "faure@kde.org");
  KURL umail2 ( "mailto:Faure David <faure@kde.org>" );
  check("mailto: URL, general form", umail2.protocol(), "mailto");
  check("mailto: URL, general form", umail2.path(), "Faure David <faure@kde.org>");

  printf("\nTest OK !\n");
}
