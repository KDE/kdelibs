/*
   This file is part of the KDE libraries
   Copyright (c) 2002 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kio/global.h>

#include <klocale.h>

#include "httpfilter.h"

HTTPFilterBase::HTTPFilterBase()
 : last(0)
{
}

HTTPFilterBase::~HTTPFilterBase()
{
   delete last;
}

void
HTTPFilterBase::chain(HTTPFilterBase *previous)
{
   last = previous;
   connect(last, SIGNAL(output(const QByteArray &)),
           this, SLOT(slotInput(const QByteArray &)));
}

HTTPFilterChain::HTTPFilterChain()
 : first(0)
{
}

void
HTTPFilterChain::addFilter(HTTPFilterBase *filter)
{
   if (!last)
   {
      first = filter;
   }
   else
   {
      disconnect(last, SIGNAL(output(const QByteArray &)), 0, 0);
      filter->chain(last);
   }
   last = filter;
   connect(filter, SIGNAL(output(const QByteArray &)),
           this, SIGNAL(output(const QByteArray &)));
   connect(filter, SIGNAL(error(int, const QString &)),
           this, SIGNAL(error(int, const QString &)));
}

void
HTTPFilterChain::slotInput(const QByteArray &d)
{
   if (first)
      first->slotInput(d);
   else
      emit output(d);      
}

HTTPFilterMD5::HTTPFilterMD5()
{
}

QString 
HTTPFilterMD5::md5()
{
   return QString::fromLatin1(context.base64Digest());
}

void 
HTTPFilterMD5::slotInput(const QByteArray &d)
{
   context.update(d);
   emit output(d);
}


HTTPFilterGZip::HTTPFilterGZip()
{
#ifdef DO_GZIP
  bHasHeader = false;
  bHasFinished = false;
  bPlainText = false;
  bEof = false;
  zstr.next_in = (Bytef *) Z_NULL;
  zstr.avail_in = 0;
  zstr.zalloc = Z_NULL;
  zstr.zfree = Z_NULL;
  zstr.opaque = Z_NULL;
  inflateInit2(&zstr, -MAX_WBITS);
#endif
}

HTTPFilterGZip::~HTTPFilterGZip()
{
#ifdef DO_GZIP
  inflateEnd(&zstr);
#endif
  
}

/* The get_byte() and checkHeader() functions are modified version from */
/* the correpsonding functions that can be found in zlib, the following */
/* copyright notice applies to these functions:                         */

/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.1.3, July 9th, 1998

  Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/

int
HTTPFilterGZip::get_byte()
{
#ifdef DO_GZIP
    if (bEof) return EOF;
    if (zstr.avail_in == 0)
    {
        bEof = true;
        return EOF;
    }
    zstr.avail_in--;
    zstr.total_in++;
    return *(zstr.next_in)++;
#else 
    return 0;
#endif
}

#ifdef DO_GZIP

static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */
#endif

// 0 : ok
// 1 : not gzip
// 2 : no header
int
HTTPFilterGZip::checkHeader()
{
#ifdef DO_GZIP
    uInt len;
    int c;

    /* Check the gzip magic header */
    for (len = 0; len < 2; len++) {
	c = get_byte();
	if (c != gz_magic[len]) {
	    if (len != 0) 
	    {
	       zstr.avail_in++;
	       zstr.next_in--;
	    }
	    if (c != EOF) {
		zstr.avail_in++;
		zstr.next_in--;
		return 1;
	    }
	    return 2;
	}
    }
    int method = get_byte(); /* method byte */
    int flags = get_byte(); /* flags byte */

    if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
	return bEof ? 2 : 1;
    }

    /* Discard time, xflags and OS code: */
    for (len = 0; len < 6; len++) (void)get_byte();

    if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
	len  =  (uInt)get_byte();
	len += ((uInt)get_byte())<<8;
	/* len is garbage if EOF but the loop below will quit anyway */
	while (len-- != 0 && get_byte() != EOF) ;
    }
    if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
	while ((c = get_byte()) != 0 && c != EOF) ;
    }
    if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
	while ((c = get_byte()) != 0 && c != EOF) ;
    }
    if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
	for (len = 0; len < 2; len++) (void)get_byte();
    }
    
    return bEof ? 2 : 0;
#else
    return 0;
#endif
} 

void 
HTTPFilterGZip::slotInput(const QByteArray &d)
{
#ifdef DO_GZIP
  if (bPlainText)
  {
     emit output(d);
     return;
  }
  if (d.size() == 0)
  {
     if (!bHasFinished)
     {
        // Make sure we get the last bytes still in the pipe.
        // Needed with "deflate".
        QByteArray flush(4);
        flush.fill(0);
        slotInput(flush);
        if (!bHasFinished && !bHasHeader)
        {
           // Send as-is
           emit output(headerData);
           bHasFinished = true;
        }
     }
     QByteArray
     // End of data
     emit output(QByteArray());
     if (!bHasFinished)
        emit error( KIO::ERR_SLAVE_DEFINED, i18n("Unexpected end of data, some information may be lost."));
     return;
  }
  if (bHasFinished)
     return;

  if (!bHasHeader)
  {
     bEof = false;
     if (headerData.isEmpty())
     {
        headerData = d;
     }
     else
     {
        // Add data to header.
        int orig_size = headerData.size();
        headerData.resize(orig_size+d.size());
        memcpy(headerData.data()+orig_size, d.data(), d.size());
     }

     zstr.avail_in = headerData.size();
     zstr.next_in = (Bytef *) headerData.data();     

     int result = checkHeader();
     if (result == 1)
     {
        bPlainText = true;
        output(headerData);
        return;
     }

     if (result != 0)
        return; // next time better

     bHasHeader = true;
  }
  else
  {
     zstr.avail_in = d.size();
     zstr.next_in = (Bytef *) d.data();
  }

  while( zstr.avail_in )
  {
     char buf[8192];
     zstr.next_out = (Bytef *) buf;
     zstr.avail_out = 8192;
     int result = inflate( &zstr, Z_NO_FLUSH );
     if ((result != Z_OK) && (result != Z_STREAM_END))
     {
        emit error( KIO::ERR_SLAVE_DEFINED, i18n("Receiving corrupt data."));
        break;
     }
     int bytesOut = 8192 - zstr.avail_out;
     if (bytesOut)
     {
        QByteArray d;
        d.setRawData( buf, bytesOut );
        emit output(d);
        d.resetRawData( buf, bytesOut );
     }
     if (result == Z_STREAM_END)
     {
        bHasFinished = true;
        return;
     }
  }  
#endif
}

HTTPFilterDeflate::HTTPFilterDeflate()
{
#ifdef DO_GZIP
  bHasHeader = true;
#endif
}

#include "httpfilter.moc"
