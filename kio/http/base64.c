/*
 * $Id$
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "base64.h"

/* Encode a zero-terminated string in base64.  Returns the malloc-ed
   encoded line.
   Note that the string may not contain NUL characters.  */
char *base64_encode_line(const char *s)
{
  return base64_encode_string(s, strlen(s));
}

/*
 * Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)
 *
 * Permission to use, copy, modify, and distribute this material
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies, and that the name of Bellcore not be
 * used in advertising or publicity pertaining to this
 * material without the specific, prior written permission
 * of an authorized representative of Bellcore.  BELLCORE
 * MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
 * OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
 */
char* base64_encode_string( const char *buf, unsigned int len )
{
  char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char * out;
  int inPos  = 0;
  int outPos = 0;
  int c1, c2, c3;
  unsigned int i;

  out=malloc( (len*4/3)+8 );

  /* Get three characters at a time and encode them. */
  for (i=0; i < len/3; ++i) {
      c1 = buf[inPos++] & 0xFF;
      c2 = buf[inPos++] & 0xFF;
      c3 = buf[inPos++] & 0xFF;
      out[outPos++] = basis_64[(c1 & 0xFC) >> 2];
      out[outPos++] = basis_64[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
      out[outPos++] = basis_64[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
      out[outPos++] = basis_64[c3 & 0x3F];
  }

  /* Encode the remaining one or two characters. */

  switch (len % 3) {
      case 0:
          break;
      case 1:
          c1 = buf[inPos] & 0xFF;
          out[outPos++] = basis_64[(c1 & 0xFC) >> 2];
          out[outPos++] = basis_64[((c1 & 0x03) << 4)];
          out[outPos++] = '=';
          out[outPos++] = '=';
          break;
      case 2:
          c1 = buf[inPos++] & 0xFF;
          c2 = buf[inPos] & 0xFF;
          out[outPos++] = basis_64[(c1 & 0xFC) >> 2];
          out[outPos++] = basis_64[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
          out[outPos++] = basis_64[((c2 & 0x0F) << 2)];
          out[outPos++] = '=';
          break;
  }
  out[outPos] = 0;
  return out;
}
