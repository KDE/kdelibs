// This library is distributed under the conditions of the GNU LGPL.
#ifndef XVIEW_H
#define XVIEW_H

class QImageIO;

extern "C" {   
void kimgio_xv_read( QImageIO * );
void kimgio_xv_write( QImageIO * );
}

#endif
