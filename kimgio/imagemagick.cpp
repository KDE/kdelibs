/*
* imagemagick.cpp -- QImageIO read/write handlers for
*       the ImageMagick graphic library
*
*       Copyright (c) May 1999, Alex Zepeda.  Distributed under
*       the LGPL.
*
* Portions Copyright 1998 E. I. du Pont de Nemours and Company
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files
* ("ImageMagick"), to deal in ImageMagick without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of ImageMagick, and to
* permit persons to whom the ImageMagick is furnished to do so, subject
* to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of ImageMagick.
*
* The software is provided "as is", without warranty of any kind, express
* or implied, including but not limited to the warranties of
* merchantability, fitness for a particular purpose and noninfringement.
* In no event shall E. I. du Pont de Nemours and Company be liable for
* any claim, damages or other liability, whether in an action of
* contract, tort or otherwise, arising from, out of or in connection with
* ImageMagick or the use or other dealings in ImageMagick.
*
* Except as contained in this notice, the name of the E. I. du Pont de
* Nemours and Company shall not be used in advertising or otherwise to
* promote the sale, use or other dealings in ImageMagick without prior
* written authorization from the E. I. du Pont de Nemours and Company.
*
* The official ImageMagick WWW page is
*
*  http://www.wizards.dupont.com/magick/ImageMagick.html
*
*
*       $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#define QT_CLEAN_NAMESPACE
#include <qimage.h>
#include <qfile.h>

extern "C" {
#include <magick/magick.h>
};
#if defined(__cplusplus) || defined(c_plusplus)
#undef class
#endif

#define WriteQuantum(image, quantum,q)  \
{  \
  if (image->depth <= 8) \
    *q++=DownScale(quantum); \
  else \
    { \
      value=(quantum); \
      if ((QuantumDepth-image->depth) > 0) \
        value*=257; \
      *q++=value >> 8; \
      *q++=value; \
    } \
}

void kimgio_imagemagick_read( QImageIO *io )
{
  QImage qt_image;
  Image *im_image;
  ImageInfo image_info;

  // Initialize the image info structure and read an image.
  GetImageInfo(&image_info);
  (void) strcpy(image_info.filename, QFile::encodeName(io->fileName()));
  im_image=ReadImage(&image_info);
  if (im_image == (Image *) NULL)
    return;
  if (!qt_image.create(im_image->columns, im_image->rows, 32))
    return;

  int x, y, i, j;
  unsigned char *pixels, *q;
  unsigned short value;
  RunlengthPacket *p;

  TransformRGBImage(im_image,RGBColorspace);
  switch (image_info.interlace) {
  default:
  case NoInterlace: {
    pixels=(unsigned char *)AllocateMemory(im_image->columns*sizeof(RunlengthPacket));
    x=y=0;
    p=im_image->pixels;
    q=pixels;
    for (i=0; i < (int) im_image->packets; i++) {
      for (j=0; j <= ((int) p->length); j++) {
	// Alex: I think this is a Qt bug that's swapping the blue and green
	WriteQuantum(im_image, p->blue,q);
	WriteQuantum(im_image, p->green,q);
	WriteQuantum(im_image, p->red,q);
	if (Latin1Compare(image_info.magick, "RGBA") == 0) {
	  WriteQuantum(im_image, im_image->matte ? p->index : MaxRGB, q);
	} else {
	  // If we don't have an alpha channel, pretend it's not there
	  // by making it completely transparent.
	  WriteQuantum(im_image, 0, q);
	}
	x++;
	if (x == (int) im_image->columns) {
	  memcpy(qt_image.scanLine(y), (char *)pixels, q-pixels);
	  q=pixels;
	  x=0;
	  y++;
	}
      }
      p++;
    }
    FreeMemory((char *) pixels);
    break;
  }
  }
  DestroyImage(im_image);
  io->setImage( qt_image );
  io->setStatus( 0 );
  return;
}

void kimgio_imagemagick_write( QImageIO *io )
{
  //QIODevice *f = ( iio->ioDevice() );
  io->setStatus(-1); // We're really a stub function.
  return;
}

void kimgio_imagemagick_writePCX( QImageIO *io )
{
  QImage qt_image = io->image();
  int x, y, i, count, packet_size;
  unsigned char *p, *scanline;
  Image *im_image;
  ImageInfo image_info;
  RunlengthPacket *q;

  packet_size=4*(qt_image.depth() >> 3);
  im_image->columns=qt_image.width();
  im_image->rows=qt_image.height();

  GetImageInfo(&image_info);
  im_image = AllocateImage(&image_info);

  scanline=(unsigned char *) AllocateMemory(packet_size*qt_image.width()*sizeof(unsigned char));
  im_image->packets=im_image->columns*im_image->rows;
  im_image->pixels=(RunlengthPacket *)
      AllocateMemory(im_image->packets*sizeof(RunlengthPacket));

  for (y=0; y < (int) im_image->rows; y++) {
    p=scanline+packet_size*im_image->tile_info.x;
    for (x=0; x < (int) im_image->columns; x++) {
      ReadQuantum(q->red,p);
      ReadQuantum(q->green,p);
      ReadQuantum(q->blue,p);
      q->index=0;
      //if (im_image->matte)
      ReadQuantum(q->index,p);
      q->length=0;
      q++;
   }
  }
  (void) strcpy(image_info.filename, QFile::encodeName(io->fileName()));
  WritePCXImage(&image_info, im_image);
  io->setStatus(0); // We're really a stub function.
  return;
}



extern "C" void kimgio_init_imagemagick() {

    QImageIO::defineIOHandler( "PNG", "^.PNG", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    // I think only XV puts this kinda header on it
    QImageIO::defineIOHandler( "PICT", "^PICSoftware", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    QImageIO::defineIOHandler( "DCX", "\261h\336", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    QImageIO::defineIOHandler( "PCX", "\xa\x5\x1\x8", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_writePCX );
    // ImageMagick Image File Formt
    QImageIO::defineIOHandler( "MIF", "^id\\=ImageMagick", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    //Windows BMP/DIB
    QImageIO::defineIOHandler( "BMP", "^BM", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    // This should work, but some stupid fucking TIFF handler seems to be eating this.. damnit
    QImageIO::defineIOHandler( "FITS", "^SIMPLE  =", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    // Khoros Visualization image file
    QImageIO::defineIOHandler( "VIFF", "^\xab", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    // And pgb
    QImageIO::defineIOHandler( "PNM", "^P[4-6]", 0,
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
}
