
/*
 * $Id$
 * ico.h - kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte.starostik@t-online.de>
 * 
 */

#ifndef	_ICO_H_
#define _ICO_H_

class QImageIO;

extern "C"
{
    void kimgio_ico_read(QImageIO *);
}

#endif
