
/////////////
// formats.cpp -- define QImage IO handlers for new graphic
// file formats.

// Sirtaj Kang, Oct '96.

// formats.cpp,v 0.6 1996/11/04 07:41:21 ssk Exp

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<qimage.h>

#include "formats.h"
#include "html.h"

void KHTMLWidget::registerFormats()
{

#ifdef HAVE_LIBGIF
        QImageIO::defineIOHandler("GIF", "^GIF[0-9][0-9][a-z]", 0, 
                read_gif_file, NULL);
#endif

#ifdef HAVE_LIBJPEG
        QImageIO::defineIOHandler("JFIF","^\377\330\377\340..JFIF", 0,
                read_jpeg_jfif, NULL);
#endif

#ifdef HAVE_LIBPNG
	QImageIO::defineIOHandler("PNG","^\211PNG\015\012\32\012", 0,
		read_png_file, NULL);	
#endif

//
// Add new format handlers here
//

}
