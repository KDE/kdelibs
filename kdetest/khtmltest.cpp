// small test program for the khtml widget
//
// Author: Lars Knoll <knoll@kde.org>

#include <khtml.h>
#include <kapp.h>
#include <qfile.h>

int main( int argc, char **argv)
{
  KApplication a(argc, argv, "KHtmlTest");

  if( argc != 2 )
    printf("Usage: khtmltest filename\n");

  KHTMLWidget v;

  a.setMainWidget( &v );

  QFile f(argv[1]);
  f.open(IO_ReadOnly);

  char s[2048];

  v.begin();
  while( f.readBlock(s, 1023) > 0 )
    v.write(s);
  v.end();

  v.show();
  return a.exec();
}
