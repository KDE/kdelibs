/*
* imagemagick.h -- Prototypes for QImageIO read/write handlers for
*	the ImageMagick graphics library.
*
*	Copyright (c) May 1999, Alex Zepeda.
*	Distributed under the LGPL.
*
*	$Id$
*/
#ifndef	_QIM_H
#define _QIM_H

class QImageIO;

void kimgio_imagemagick_read( QImageIO *io );
void kimgio_imagemagick_write(QImageIO *io );

#endif
