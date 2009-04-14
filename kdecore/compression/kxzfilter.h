/* This file is part of the KDE libraries
   Copyright (C) 2007-2008 Per Øyvind Karlsen <peroyvind@mandriva.org>

   Based on kbzip2filter:
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __klzmafilter__h
#define __klzmafilter__h

#include <config.h>

#if defined( HAVE_XZ_SUPPORT )

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

    virtual void init( int );
    virtual int mode() const;
    virtual void terminate();
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

#endif /* __klzmafilter__h */
