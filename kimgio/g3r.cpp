// This library is distributed under the conditions of the GNU LGPL.

#include "config.h"

#ifdef HAVE_LIBTIFF

#include <tiffio.h>

#include <qimage.h>
#include <qfile.h>

#include "g3r.h"

void kimgio_g3_read( QImageIO *io )
{
  TIFF *tiff = TIFFOpen(QFile::encodeName(io->fileName()), "r");  
  if (!tiff)
    return;
 
  uint32 width, height;
  tsize_t scanlength;

  TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
  TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
  scanlength = TIFFScanlineSize(tiff);

  QImage image(width, height, 1, 0, QImage::BigEndian);
  
  if (scanlength != image.bytesPerLine())
    {
      TIFFClose(tiff);
      return;
    }

  for (uint32 y=0; y < height; y++)
    TIFFReadScanline(tiff, image.scanLine(y), y);

  TIFFClose(tiff);
  
  io->setImage(image);
  io->setStatus(0);
}


void kimgio_g3_write(QImageIO *)
{
	// TODO: stub
}


#endif
