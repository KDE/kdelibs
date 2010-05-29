/**
* QImageIO Routines to read/write g3 (fax) images.
* (c) 2000, Matthias Hölzer-Klüpfel
*
* This library is distributed under the conditions of the GNU LGPL.
*
* $Id: g3r.h 49984 2000-05-19 14:32:40Z hoelzer $
*/

#ifndef KIMG_G3R_H
#define KIMG_G3R_H

class QImageIO;

extern "C" {
  void kimgio_g3_read( QImageIO *io );
  void kimgio_g3_write( QImageIO *io );
}

#endif
