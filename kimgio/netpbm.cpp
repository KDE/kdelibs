////////////////////////////////////////////////////
//
// Transparent support for several files formats in Qt Pixmaps,
// using the NetPBM tools.
//
// Dirk Schoenberger, Jul 1997.

// This library is distributed under the conditions of the GNU LGPL.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <qimage.h>
#include <qfile.h>
#include <kprocess.h>
#include <ktempfile.h>

//////
// the real filter.
//

void import_graphic (const char *filter, QImageIO *image)
{
  QImage myimage;

  KTempFile tmp;
  tmp.close();

  QString cmd = filter;
  cmd += " ";
  cmd += KShellProcess::quote(image->fileName());
  cmd += " > ";
  cmd += KShellProcess::quote(tmp.name());

  system (QFile::encodeName(cmd));
  myimage.load (tmp.name());

  tmp.unlink();

  image->setImage (myimage);
  image->setStatus (0);
}

//////
// PCX IO handlers for QImage.
//

void read_pcx (QImageIO *image)
{
  import_graphic ("pcxtoppm", image);
}

//////
// IFF IO handlers for QImage.
//

void read_ilbm (QImageIO *image)
{
  import_graphic ("ilbmtoppm", image);
}

//////
// TGA IO handlers for QImage.
//

void read_tga (QImageIO *image)
{
  import_graphic ("tgatoppm", image);
}

