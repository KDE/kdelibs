////////////////
//
// jpeg.h -- QImage IO handler declaration for JFIF JPEG graphic format,
//	SirtaJ Singh Kang <taj@kde.org>, December 1996.
//           using IJG JPEG library.
//

// $Id$

#ifndef _SSK_JPEG_QHANDLERS_H
#define _SSK_JPEG_QHANDLERS_H

class QImageIO;

//////
// JPEG IO handlers for QImage.
//

void kimgio_jpeg_read(QImageIO *image);
void kimgio_jpeg_write(QImageIO *image);

#endif
