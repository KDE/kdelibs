#ifndef XVIEW_H
#define XVIEW_H

#include <qimage.h>
#include <qpixmap.h>

void read_xv_file( QImageIO* );
void write_xv_file( const char *_filename, QPixmap &_pixmap );

#endif
