/* This file is part of the KDE project
   Copyright (C) 2003 Dominik Seichter <domseichter@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_TGA_H
#define KIMG_TGA_H

class QImageIO;

extern "C" {
void kimgio_tga_read( QImageIO * );
void kimgio_tga_write( QImageIO * );
}

#endif
 
