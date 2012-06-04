/* This file is part of the KDE libraries
   Copyright (C) 2007-2008 Per Øyvind Karlsen <peroyvind@mandriva.org>

   Based on kbzip2filter:
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

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

#include "kxzfilter.h"

#include <config-compression.h>

#if HAVE_XZ_SUPPORT
extern "C" {
	#include <lzma.h>
}

#include <QDebug>

#include <qiodevice.h>


class KXzFilter::Private
{
public:
    Private()
    : isInitialized(false)
    {
        memset(&zStream, 0, sizeof(zStream));
        mode = 0;
    }

    lzma_stream zStream;
    lzma_filter filters[5];
    unsigned char props[5];
    int mode;
    bool isInitialized;
    KXzFilter::Flag flag;
};

KXzFilter::KXzFilter()
    :d(new Private)
{
}


KXzFilter::~KXzFilter()
{
    delete d;
}

bool KXzFilter::init( int mode)
{
    QVector<unsigned char> props;
    return init(mode, AUTO, props);
}

bool KXzFilter::init( int mode, Flag flag, const QVector<unsigned char>& properties )
{
    if (d->isInitialized) {
        terminate();
    }

    d->flag = flag;
    lzma_ret result;
    d->zStream.next_in = 0;
    d->zStream.avail_in = 0;
    if ( mode == QIODevice::ReadOnly ) {
        switch (flag) {
        case AUTO:
            /* We set the memlimit for decompression to 100MiB which should be
            * more than enough to be sufficient for level 9 which requires 65 MiB.
            */
            result = lzma_auto_decoder(&d->zStream, 100<<20, 0);
            if (result != LZMA_OK) {
                qWarning() << "lzma_auto_decoder returned" << result;
                return false;
            }
            break;
        case LZMA:
        {
            d->filters[0].id = LZMA_FILTER_LZMA1;
            d->filters[0].options = NULL;
            d->filters[1].id = LZMA_VLI_UNKNOWN;
            d->filters[1].options = NULL;

            Q_ASSERT(properties.size() == 5);
            unsigned char props[5];
            for (int i = 0; i < properties.size(); ++i) {
                props[i] = properties[i];
            }

            result = lzma_properties_decode(&d->filters[0], NULL, props, sizeof(props));
            if (result != LZMA_OK) {
                qWarning() << "lzma_properties_decode returned" << result;
                return false;
            }
            break;
        }
        case LZMA2:
        {
            d->filters[0].id = LZMA_FILTER_LZMA2;
            d->filters[0].options = NULL;
            d->filters[1].id = LZMA_VLI_UNKNOWN;
            d->filters[1].options = NULL;

            Q_ASSERT(properties.size() == 1);
            unsigned char props[1];
            props[0] = properties[0];

            result = lzma_properties_decode(&d->filters[0], NULL, props, sizeof(props));
            if (result != LZMA_OK) {
                qWarning() << "lzma_properties_decode returned" << result;
                return false;
            }
            break;
        }
        case BCJ:
        {
            d->filters[0].id = LZMA_FILTER_X86;
            d->filters[0].options = NULL;

            unsigned char props[5] = {0x5d, 0x00, 0x00, 0x08, 0x00};
            d->filters[1].id = LZMA_FILTER_LZMA1;
            d->filters[1].options = NULL;
            result = lzma_properties_decode(&d->filters[1], NULL, props, sizeof(props));
            if (result != LZMA_OK) {
                qWarning() << "lzma_properties_decode1 returned" << result;
                return false;
            }

            d->filters[2].id = LZMA_VLI_UNKNOWN;
            d->filters[2].options = NULL;

            break;
        }
        case POWERPC:
        case IA64:
        case ARM:
        case ARMTHUMB:
        case SPARC:
            //qDebug() << "flag" << flag << "props size" << properties.size();
            break;
        }

        if (flag != AUTO) {
            result = lzma_raw_decoder(&d->zStream, d->filters);
            if (result != LZMA_OK) {
                qWarning() << "lzma_raw_decoder returned" << result;
                return false;
            }
        }

    } else if ( mode == QIODevice::WriteOnly ) {
        if (flag == AUTO) {
            result = lzma_easy_encoder(&d->zStream, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC32);
        } else {
            if (LZMA2) {
                lzma_options_lzma lzma_opt;
                lzma_lzma_preset(&lzma_opt, LZMA_PRESET_DEFAULT);

                d->filters[0].id = LZMA_FILTER_LZMA2;
                d->filters[0].options = &lzma_opt;
                d->filters[1].id = LZMA_VLI_UNKNOWN;
                d->filters[1].options = NULL;
            }
            result = lzma_raw_encoder(&d->zStream, d->filters);
        }
        if (result != LZMA_OK) {
            qWarning() << "lzma_easy_encoder returned" << result;
            return false;
        }
    } else {
        //qWarning() << "Unsupported mode " << mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->mode = mode;
    d->isInitialized = true;
    return true;
}

int KXzFilter::mode() const
{
    return d->mode;
}

bool KXzFilter::terminate()
{
    if (d->mode == QIODevice::ReadOnly || d->mode == QIODevice::WriteOnly) {
        lzma_end(&d->zStream);
    } else {
        //qWarning() << "Unsupported mode " << d->mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->isInitialized = false;
    return true;
}

void KXzFilter::reset()
{
    //qDebug() << "KXzFilter::reset";
    // liblzma doesn't have a reset call...
    terminate();
    init( d->mode );
}

void KXzFilter::setOutBuffer( char * data, uint maxlen )
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = (uint8_t *)data;
}

void KXzFilter::setInBuffer( const char *data, unsigned int size )
{
    d->zStream.avail_in = size;
    d->zStream.next_in = (uint8_t *)const_cast<char *>(data);
}

int KXzFilter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}

int KXzFilter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

KXzFilter::Result KXzFilter::uncompress()
{
    //qDebug() << "Calling lzma_code with avail_in=" << inBufferAvailable() << " avail_out =" << outBufferAvailable();
    lzma_ret result;
    result = lzma_code(&d->zStream, LZMA_RUN);

    /*if (result != LZMA_OK) {
        qDebug() << "lzma_code returned " << result;
        //qDebug() << "KXzFilter::uncompress " << ( result == LZMA_STREAM_END ? KFilterBase::End : KFilterBase::Error );
    }*/

    switch (result) {
        case LZMA_OK:
                return KFilterBase::Ok;
        case LZMA_STREAM_END:
                return KFilterBase::End;
        default:
                return KFilterBase::Error;
    }
}

KXzFilter::Result KXzFilter::compress( bool finish )
{
    //qDebug() << "Calling lzma_code with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    lzma_ret result = lzma_code(&d->zStream, finish ? LZMA_FINISH : LZMA_RUN );
    switch (result) {
        case LZMA_OK:
                return KFilterBase::Ok;
                break;
        case LZMA_STREAM_END:
                //qDebug() << "  lzma_code returned " << result;
                return KFilterBase::End;
		break;
        default:
                //qDebug() << "  lzma_code returned " << result;
                return KFilterBase::Error;
                break;
    }
}

#endif  /* HAVE_XZ_SUPPORT */
