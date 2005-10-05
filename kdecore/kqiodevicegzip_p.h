/* This file is part of the KDE project
   Copyright (C) 2001,2005 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef KDELIBS_KQIODEVICEGZIP_H
#define KDELIBS_KQIODEVICEGZIP_H

#include <qiodevice.h>
#include <qstring.h>
#include <qfile.h>

#include <zlib.h>


/**
 * \brief QIODevice class for a gzipped file
 * \internal This class is internal to KDE. 
 * The class KFilterDev should be used instead.
 */
class KQIODeviceGZip : public QIODevice
{
public:
    KQIODeviceGZip(const QString& filename);
    ~KQIODeviceGZip(void);

    bool open(int mode);
    void close(void);
    void flush(void);

    Offset size(void) const;
    Offset  at(void) const;
    bool at(Offset pos);
    bool atEnd(void) const;
    bool reset (void);

    Q_LONG readBlock( char *data, Q_ULONG maxlen );
    Q_LONG writeBlock( const char *data, Q_ULONG len );

    int getch(void);
    int putch(int ch);
    int ungetch(int ch);
private:
    gzFile m_gzfile;
    int m_ungetchar;
    QString m_filename;
};


#endif // KDELIBS_KQIODEVICEGZIP_H
