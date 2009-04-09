/*
   This file is part of the KDE libraries
   Copyright (c) 2002 Waldo Bastian <bastian@kde.org>
   Copyright 2009 David Faure <faure@kde.org>

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

#include "httpfilter.h"
#include <kgzipfilter.h>
#include <kdebug.h>

#include <kio/global.h>
#include <klocale.h>

#include <stdio.h>

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
   connect(last, SIGNAL(output(QByteArray)),
           this, SLOT(slotInput(QByteArray)));
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
      disconnect(last, SIGNAL(output(QByteArray)), 0, 0);
      filter->chain(last);
   }
   last = filter;
   connect(filter, SIGNAL(output(QByteArray)),
           this, SIGNAL(output(QByteArray)));
   connect(filter, SIGNAL(error(int,QString)),
           this, SIGNAL(error(int,QString)));
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


HTTPFilterGZip::HTTPFilterGZip(bool deflate)
    : m_deflateMode(deflate),
      m_firstData(true),
      m_finished(false)
{
    m_needGzipHeader = !deflate;
    // We can't use KFilterDev because it assumes it can read as much data as necessary
    // from the underlying device. It's a pull strategy, while we have to do
    // a push strategy.
    m_gzipFilter = new KGzipFilter;
}

HTTPFilterGZip::~HTTPFilterGZip()
{
    m_gzipFilter->terminate();
    delete m_gzipFilter;

}

/*
  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).

  Use /usr/include/zlib.h as the primary source of documentation though.
*/

void
HTTPFilterGZip::slotInput(const QByteArray &d)
{
    //kDebug() << "Got" << d.size() << "bytes as input";
    if (!d.isEmpty()) {

        if (m_firstData) {
            bool zlibHeader = m_deflateMode;
            if (m_deflateMode) {
                // Autodetect broken webservers (thanks Microsoft) who send raw-deflate
                // instead of zlib-headers-deflate when saying Content-Encoding: deflate.
                const char firstChar = d[0];
                if ((firstChar & 0x0f) != 8) {
                    // In a zlib header, CM should be 8 (cf RFC 1950)
                    zlibHeader = false;
                } else if (d.size() > 1) {
                    const char flg = d[1];
                    if ((firstChar * 256 + flg) % 31 != 0) { // Not a multiple of 31? invalid zlib header then
                        zlibHeader = false;
                    }
                }
                //if (!zlibHeader)
                //    kDebug() << "Bad webserver, uses raw-deflate instead of zlib-deflate...";
            }
            m_gzipFilter->init(QIODevice::ReadOnly, zlibHeader ? KGzipFilter::ZlibHeader : KGzipFilter::RawDeflateOrGzip);
            m_firstData = false;
        }

        if (m_needGzipHeader && !m_unprocessedHeaderData.isEmpty()) {
            m_unprocessedHeaderData.append(d);
            m_gzipFilter->setInBuffer(m_unprocessedHeaderData.constData(), m_unprocessedHeaderData.size());
        } else {
            m_gzipFilter->setInBuffer(d.constData(), d.size());
        }

        if (m_needGzipHeader) {
            if (m_gzipFilter->readHeader()) {
                m_needGzipHeader = false;
                m_unprocessedHeaderData.clear();
            } else {
                // not enough data yet?
                // (testcase: http://www.zlib.net/zlib_faq.html, I get 5 or 6 bytes first)
                m_unprocessedHeaderData = d;
                return;
            }
        }
    }
    while (!m_gzipFilter->inBufferEmpty() && !m_finished) {
        char buf[8192];
        m_gzipFilter->setOutBuffer(buf, sizeof(buf));
        KFilterBase::Result result = m_gzipFilter->uncompress();
        //kDebug() << "uncompress returned" << result;
        switch (result) {
        case KFilterBase::Ok:
        case KFilterBase::End:
        {
            const int bytesOut = sizeof(buf) - m_gzipFilter->outBufferAvailable();
            if (bytesOut) {
                emit output(QByteArray(buf, bytesOut));
            }
            if (result == KFilterBase::End) {
                //kDebug() << "done, bHasFinished=true";
                emit output(QByteArray());
                m_finished = true;
            }
            break;
        }
        case KFilterBase::Error:
            kWarning() << "Error from KGZipFilter";
            emit error( KIO::ERR_SLAVE_DEFINED, i18n("Receiving corrupt data."));
            m_finished = true; // exit this while loop
            break;
        }
    }
#if 0
    // We can't assume that the caller has used Z_FINISH.
    // So we have to assume that "end of input data" means "end of decompressed data".
    // See bug 117683 and testcase in bug 188935.
    if (d.isEmpty() && !m_finished) {
        kDebug() << "ERROR: done but m_finished=false";
        emit error( KIO::ERR_SLAVE_DEFINED, i18n("Unexpected end of data, some information may be lost."));
    }
#endif
}

HTTPFilterDeflate::HTTPFilterDeflate()
    : HTTPFilterGZip(true)
{
}

#include "httpfilter.moc"
