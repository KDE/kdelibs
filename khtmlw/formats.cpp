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

/////////////
// formats.cpp -- define QImage IO handlers for new graphic
// file formats.
// formats.cpp,v 0.6 1996/11/04 07:41:21 ssk Exp

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<qimage.h>

#include "formats.h"
#include "html.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

void KHTMLWidget::registerFormats()
{
#ifdef HAVE_LIBJPEG
        QImageIO::defineIOHandler("JFIF","^\377\330\377\340..JFIF", 0,
                read_jpeg_jfif, 0L);
#endif

#ifdef HAVE_LIBPNG
	QImageIO::defineIOHandler("PNG","^\211PNG\015\012\32\012", 0,
		read_png_file, 0L);	
#endif

//
// Add new format handlers here
//

}
