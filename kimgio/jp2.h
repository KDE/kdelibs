// This library is distributed under the conditions of the GNU LGPL.
#ifndef KIMG_JP2_H
#define KIMG_JP2_H

class QImageIO;

extern "C" {
	void kimgio_jp2_read( QImageIO* io );
	void kimgio_jp2_write( QImageIO* io );

	void kimgio_j2k_read( QImageIO* io );
	void kimgio_j2k_write( QImageIO* io );
} // extern "C"

#endif

