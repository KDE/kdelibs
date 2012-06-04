/* This file is part of the KDE libraries
   Copyright (C) 2007-2008 Per Øyvind Karlsen <peroyvind@mandriva.org>

   Based on kbzip2filter:
   Copyright (C) 2000 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KXZFILTER_H
#define KXZFILTER_H

#include <config-compression.h>

#if HAVE_XZ_SUPPORT

#include "kfilterbase.h"

/**
 * Internal class used by KFilterDev
 * @internal
 */
class KXzFilter : public KFilterBase
{
public:
    KXzFilter();
    virtual ~KXzFilter();

    virtual bool init( int);

    enum Flag {
        AUTO = 0,
        LZMA = 1,
        LZMA2 = 2,
        BCJ = 3, //X86
        POWERPC = 4,
        IA64 = 5,
        ARM = 6,
        ARMTHUMB = 7,
        SPARC = 8
    };

    virtual bool init( int, Flag flag, const QVector<unsigned char>& props);
    virtual int mode() const;
    virtual bool terminate();
    virtual void reset();
    virtual bool readHeader() { return true; } // lzma handles it by itself ! Cool !
    virtual bool writeHeader( const QByteArray & ) { return true; }
    virtual void setOutBuffer( char * data, uint maxlen );
    virtual void setInBuffer( const char * data, uint size );
    virtual int  inBufferAvailable() const;
    virtual int  outBufferAvailable() const;
    virtual Result uncompress();
    virtual Result compress( bool finish );
private:
    class Private;
    Private* const d;
};

#endif

#endif // KXZFILTER_H
