 
/**
* kimgio.h -- Implementation of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

static int registered = 0;

#include"jpeg.h"
#include"xview.h"
#include"eps.h"
#include"tiffr.h"

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#include<qimage.h>

void kimgioRegister(void)
{
	if( registered ) {
		return;
	}

	registered = 1;

#ifdef HAVE_LIBJPEG
	// JPEG
	QImageIO::defineIOHandler("JFIF","^\377\330\377\340..JFIF", 0,
			kimgio_jpeg_read, kimgio_jpeg_write );
#endif

	// XV thumbnails
	QImageIO::defineIOHandler( "XV", "^P7 332", 0, 
		kimgio_xv_read, kimgio_xv_write );

	QImageIO::defineIOHandler("PS","^%!PS-Adobe-[1-2]", 0,
                kimgio_epsf_read, kimgio_epsf_write );

#ifdef HAVE_LIBTIFF
	QImageIO::defineIOHandler("TIFF","[MI][MI]", 0,
                kimgio_tiff_read, kimgio_tiff_write );
#endif
}
