/////////////////////////////////////////////////
//
// formats.h -- include declarations for various file format
//              handlers.
//
// formats.h,v 0.4 1996/11/04 07:42:05 ssk Exp

#ifndef _SSK_GFORMATS_H
#define _SSK_GFORMATS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBJPEG 
#include "jpeg.h"
#endif

#ifdef HAVE_LIBPNG
#include "kpng.h"
#endif

#ifdef HAVE_LIBGIF
#include "gif.h"
#endif


// Include declaration headers for format handlers here (should 
// only need 2 per format; I've got only one, since no writers yet).

#endif
