/**
* QImageIO Routines to read (and perhaps in the future, write) images
* in the high definition EXR format.
* (c) 2003, Brad Hards <bradh@frogmouth.net>
*
* This library is distributed under the conditions of the GNU GPL.
*
*/

#ifndef KIMG_EXR_H
#define KIMG_EXR_H

class QImageIO;

extern "C" {
  void kimgio_exr_read (QImageIO *io);
  void kimgio_exr_write (QImageIO *io);
}

#endif
