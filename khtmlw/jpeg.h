/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
			  (C) 1997 Sirtaj Singh Kang (taj@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
////////////////
//
// jpeg.h -- QImage IO handler declaration for JFIF JPEG graphic format,
//           using IJG JPEG library.
//

// $Id$

#ifndef _SSK_JPEG_QHANDLERS_H
#define _SSK_JPEG_QHANDLERS_H

#include "jpeginc.h"

//////
// JPEG IO handlers for QImage.
//

void read_jpeg_jfif(QImageIO *image);
void write_jpeg_jfif(QImageIO *image);

//////
// Plug-in source manager for IJG JPEG compression/decompression library
//


// 
// Source control structure.
// 

typedef struct {
  struct jpeg_source_mgr pub;   /* public fields */

  QDataStream *infile;                /* Pointer to QIODevice object */
  JOCTET * buffer;              /* start of buffer */
  boolean start_of_file;        /* have we gotten any data yet? */
} qimageio_jpeg_source_mgr;

void qimageio_jpeg_src(j_decompress_ptr cinfo, QDataStream *image);

//
// Source manager file request plug-in methods.
//

void qimageio_init_source(j_decompress_ptr cinfo);
boolean qimageio_fill_input_buffer(j_decompress_ptr cinfo);
void qimageio_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
void qimageio_term_source(j_decompress_ptr cinfo);

// Buffer for file reads (max bytes)
// ?? Is this good enough? Bigger, perhaps?

#define INPUT_BUFFER_SIZE 4096

#endif
