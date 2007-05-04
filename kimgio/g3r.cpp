/**
* QImageIO Routines to read/write g3 (fax) images.
* copyright (c) 2000, Matthias Hölzer-Klüpfel <hoelzer@kde.org>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#include "g3r.h"

#include <config.h>

#include <tiffio.h>

#include <qimage.h>
#include <qfile.h>

KDE_EXPORT void kimgio_g3_read( QImageIO *io )
{
    // This won't work if io is not a QFile !
  TIFF *tiff = TIFFOpen(QFile::encodeName(io->fileName()), "r");
  if (!tiff)
    return;

  uint32 width, height;
  tsize_t scanlength;

  if( TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width ) != 1
      || TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height ) != 1 )
      return;
  scanlength = TIFFScanlineSize(tiff);

  QImage image(width, height, 1, 0, QImage::BigEndian);

  if (image.isNull() || scanlength != image.bytesPerLine())
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


KDE_EXPORT void kimgio_g3_write(QImageIO *)
{
	// TODO: stub
}

