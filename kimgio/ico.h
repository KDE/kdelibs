
/*
 * $Id$
 * ico.h - kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 * 
 */

// You can use QImageIO::setParameters() to request a specific
// Icon out of an .ico file:
// "32:0" will return a hicolor 32x32 icon if available
// "48:256" will return a 256-color 48x48 icon if available
// "16:16" will return a 16-color 16x16 icon if available
// etc.
// Size takes precendence over depth when a match is searched.
// If no parameters are given, the default is 32x32 and hicolor
// if the display supports it or 16-color otherwise.
// The depth parameter can be omitted to use the default depth but
// a specific size

#ifndef	_ICO_H_
#define _ICO_H_

class QImageIO;

extern "C"
{
    void kimgio_ico_read(QImageIO *);
    void kimgio_ico_write(QImageIO *);
}

#endif
