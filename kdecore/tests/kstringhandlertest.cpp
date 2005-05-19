#include "kstringhandler.h"
#include <iostream>
using std::cout;
using std::endl;

bool check(const QString &txt, const QString &a, const QString &b)
{
    if ( a != b ) {
        cout << "ERROR: Tested " << txt.latin1() << ", expected" << endl;
        cout << "'" << b.latin1() << "' (" << b.length() << " chars)" << endl;
        cout << "but got" << endl;
        cout << "'" << a.latin1() << "' (" << a.length() << " chars)" << endl;
        exit( 1 );
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
       " bridge. lazy the over jumped fox brown quick The"); 
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

  test = "http://www.foo.org/story$806";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	 "<a href=\"http://www.foo.org/story$806\">http://www.foo.org/story$806</a>" );

#if 0
  // XFAIL - i.e. this needs to be fixed, but has never been
  test = "&lt;a href=www.foo.com&gt;";
  check( "tagURLs()", KStringHandler::tagURLs( test ),
	 "&lt;a href=<a href=\"www.foo.com\">www.foo.com</a>&gt;" );
#endif

  cout << "All OK!" << endl;
}
