#include <kstringhandler.h>
#include <stdio.h>
#include <stdlib.h>
#include <kdebug.h>

bool check(QString txt, QString a, QString b)
{
    kdDebug() << txt << ": checking '" << endl << a << endl << "' against expected value '"
              << endl << b << "'... " << endl;
  if (a == b) 
    printf("ok\n");
  else {
    printf("KO ! \n");
    exit(1);
  }
  return true; 
}

int main(int argc, char *argv[]) 
{
  QString test = "The quick brown fox jumped over the lazy bridge. ";

  check("word(test, 3)", 
	KStringHandler::word(test, 2), 
	"brown");
  check("word(test, \"3:5\")", 
	KStringHandler::word(test, "2:4"), 
	"brown fox jumped");
  check("insword(test, \"very\", 1)", 
	KStringHandler::insword(test, "very", 1),
       "The very quick brown fox jumped over the lazy bridge. ");
  check("setword(test, \"very\", 1)", 
	KStringHandler::setword(test, "very", 1),
       "The very brown fox jumped over the lazy bridge. ");
  check("remrange(test, \"4:6\")", 
	KStringHandler::remrange(test, "4:6"),
       "The quick brown fox lazy bridge. " );
  check("remrange(test, \"4:8\")", 
	KStringHandler::remrange(test, "4:8"),
       "The quick brown fox ");
  check("remword(test, 4)", 
	KStringHandler::remword(test, 4),
       "The quick brown fox over the lazy bridge. "); 
  check("remword(test, \"lazy\")", 
	KStringHandler::remword(test, "lazy"),
       "The quick brown fox jumped over the bridge. "); 
  check("capwords(test)", 
	KStringHandler::capwords(test),
       "The Quick Brown Fox Jumped Over The Lazy Bridge. "); 
  check("reverse(test)", 
	KStringHandler::reverse(test),
       "bridge. lazy the over jumped fox brown quick The"); 
  QString result;
  result = KStringHandler::ljust(test, 70);
  if (result.length() != 70)
  {
     printf("Length = %d, expected 70.\n", result.length());
     exit(1);
  }
  check("ljust(test, 70)", 
	result,
        "The quick brown fox jumped over the lazy bridge.                      ");
  result = KStringHandler::rjust(test, 70);
  if (result.length() != 70)
  {
     printf("Length = %d, expected 70.\n", result.length());
     exit(1);
  }
  check("rjust(test, 70)", 
	result,
        "                      The quick brown fox jumped over the lazy bridge.");
  result = KStringHandler::center(test, 70);
  if (result.length() != 70)
  {
     printf("Length = %d, expected 70.\n", result.length());
     exit(1);
  }
  check("center(test, 70)", 
	result,
        "           The quick brown fox jumped over the lazy bridge.           ");

  test = "Click on http://foo@bar:www.kde.org/yoyo/dyne.html#a1 for info.";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	"Click on <a href=\"http://foo@bar:www.kde.org/yoyo/dyne.html#a1\">http://foo@bar:www.kde.org/yoyo/dyne.html#a1</a> for info." );

  test = "&lt;a href=www.foo.com&gt;";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	 "&lt;a href=<a href=\"www.foo.com\">www.foo.com</a>&gt;" );

  printf("\nTest OK !\n");
}
