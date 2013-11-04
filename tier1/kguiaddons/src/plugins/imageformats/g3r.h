/**
* QImageIO Routines to read/write g3 (fax) images.
* copyright (c) 2000, Matthias Hölzer-Klüpfel <hoelzer@kde.org>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef KIMG_G3R_H
#define KIMG_G3R_H

class QImageIO;

extern "C" {
  void kimgio_g3_read( QImageIO *io );
  void kimgio_g3_write( QImageIO *io );
}

#endif
