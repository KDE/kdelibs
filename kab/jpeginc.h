/*
	To fix the ridiculous INT32 redefinition problem.
	Ok, it's a hack. But it works.
	
	(c)Sirtaj Singh Kang, 1996.

	$Id$

	This now written by tegla@katalin.csoma.elte.hu
*/

#ifndef XMD_H
#define XMD_H
#define _SKIP_JPEG_INT32
#endif

extern "C" {
#include<jpeglib.h>
}

#ifdef _SKIP_JPEG_INT32
#undef _SKIP_JPEG_INT32
#undef XMD_H
#endif
