/*
* imagemagick.cpp -- QImageIO read/write handlers for
*       the ImageMagick graphic library
*
*       Copyright (c) May 1999, Alex Zepeda.  Distributed under
*       the LGPL.
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
  //#include <defines.h>
#include <magick.h>
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
  (void) strcpy(image_info.filename, io->fileName().ascii());
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
    x=0;
    y=0;
    p=im_image->pixels;
    q=pixels;
    for (i=0; i < (int) im_image->packets; i++) {
      for (j=0; j <= ((int) p->length); j++) {
	// Alex: I think this is a Qt bug that's swapping the blue and green
	WriteQuantum(im_image, p->blue,q);
	WriteQuantum(im_image, p->green,q);
	WriteQuantum(im_image, p->red,q);
	if (Latin1Compare(image_info.magick,"RGBA") == 0) {
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
  
  io->setImage( qt_image );
  io->setStatus( 0 );
  return;
}

void kimgio_imagemagick_write( QImageIO * )
{
  //QIODevice *f = ( iio->ioDevice() );
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
			       kimgio_imagemagick_read, kimgio_imagemagick_write );
    // ImageMagick Image File Formt
    QImageIO::defineIOHandler( "MIF", "^id\=ImageMagick", 0,
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
