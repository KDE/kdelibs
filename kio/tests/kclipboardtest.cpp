#include <kio/paste.h>
#include <kdebug.h>
#include <qapplication.h>

int main(int argc, char **argv)
{
  QApplication app(argc,argv);
  kdDebug() << "isClipboardEmpty=" << KIO::isClipboardEmpty() << endl;
  return 0;
}
