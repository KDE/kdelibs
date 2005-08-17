/* This file is part of the KDE project
   Copyright (C) 2003 Ignacio Castaño <castano@ludicon.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_PSD_H
#define KIMG_PSD_H

#include <kdemacros.h>

class QImageIO;

extern "C" {
KDE_EXPORT void kimgio_psd_read( QImageIO * );
KDE_EXPORT void kimgio_psd_write( QImageIO * );
}

#endif
 
