////////////////////////////////////////////////////
//
// Transparent support for several files formats in Qt Pixmaps,
// using the NetPBM tools.
//
// Dirk Schoenberger, Jul 1997.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <qimage.h>

#define CMDBUFLEN     4096

//////
// the real filter.
//

void import_graphic (char *filter, QImageIO *image)
{
  char * tmpFileName;
  QImage myimage;

  char cmdBuf [CMDBUFLEN];

  tmpFileName = tmpnam(NULL);

  sprintf (cmdBuf, "%s %s > %s", filter, image->fileName(), tmpFileName);
//  printf (cmdBuf);
//  fflush (stdout);

  system (cmdBuf);
  myimage.load (tmpFileName);

  unlink (tmpFileName);

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

