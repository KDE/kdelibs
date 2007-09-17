/*  This file is part of the KDE libraries
 *  Copyright 2007 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kmimemagicrule_p.h"
#include <QIODevice>
#include <kdebug.h>
#include <QByteArrayMatcher>

/*
 * Historical note:
 * The notion of indents is used differently from the old file(1) magic file.
 * It is not enough that a top-level rule matches for the search to be over;
 * in file(1) subrules were used as refinement (and in KMimeMagic they were
 * mandatory if the toplevel rule didn't have a mimetype associated with it).
 * Here they are mandatory.
 * We need at least one continuation at every level to match, and then the match is valid:
[50:application/x-kformula]
>0=^B^_<8B>
1>10=^GKOffice
2>18=^Xapplication/x-kformula^D^F
>0=^DPK^C^D
1>30=^Hmimetype
2>38=^Vapplication/x-kformula
 * Either it's an old (tar) file and the first hierarchy (0,1,2 levels) matches,
 * or it's a newer file (zip) file and the second hierarchy (0,1,2 levels) has to match.
 *
 */

static bool testMatches(QIODevice* device, QByteArray& availableData, const QList<KMimeMagicMatch>& matches, const QString& mimeType)
{
    for ( QList<KMimeMagicMatch>::const_iterator it = matches.begin(), end = matches.end() ;
          it != end ; ++it ) {
        const KMimeMagicMatch& match = *it;
        if (match.match(device, availableData, mimeType)) {
            // One of the hierarchies matched -> mimetype recognized.
            return true;
        }
    }
    return false;
}

// Taken from QByteArray::indexOf, but that one uses strncmp so it stops on '\0',
// replaced with memcmp here...
static int indexOf(const QByteArray& that, const QByteArray &ba)
{
    const int l = that.size();
    const int ol = ba.size();
    if (ol > l)
        return -1;
    if (ol == 0)
        return 0;
    if (ol == 1)
        return that.indexOf(*ba.constData());

    if (l > 500 && ol > 5)
        return QByteArrayMatcher(ba).indexIn(that);

    const char *needle = ba.data();
    const char *haystack = that.data();
    const char *end = that.data() + (l - ol);
    const uint ol_minus_1 = ol - 1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + needle[idx]);
        hashHaystack = ((hashHaystack<<1) + haystack[idx]);
    }
    hashHaystack -= *(haystack + ol_minus_1);

    while (haystack <= end) {
        hashHaystack += *(haystack + ol_minus_1);
        if (hashHaystack == hashNeedle  && *needle == *haystack
             && memcmp(needle, haystack, ol) == 0)
            return haystack - that.data();

        if (ol_minus_1 < sizeof(uint) * 8 /*CHAR_BIT*/)
            hashHaystack -= (*haystack) << ol_minus_1;
        hashHaystack <<= 1;

        ++haystack;
    }
    return -1;
}


bool KMimeMagicRule::match(QIODevice* device, QByteArray& availableData) const
{
    return testMatches(device, availableData, m_matches, m_mimetype);
}

bool KMimeMagicMatch::match(QIODevice* device, QByteArray& availableData, const QString& mimeType) const
{
    // First, check that "this" matches, then we'll dive into subMatches if any.

    const qint64 deviceSize = device->size();
    const qint64 mDataSize = m_data.size();
    if (m_rangeStart + mDataSize > deviceSize)
        return false; // file is too small

    // Read in one block all the data we'll need
    // Example: m_data="ABC", m_rangeLength=3 -> we need 3+3-1=5 bytes (ABCxx,xABCx,xxABC would match)
    const int dataNeeded = qMin(mDataSize + m_rangeLength - 1, deviceSize - m_rangeStart);
    QByteArray readData;

    /*kDebug() << "need " << dataNeeded << " bytes of data starting at " << m_rangeStart
             << "  - availableData has " << availableData.size() << " bytes,"
             << " device has " << deviceSize << " bytes." << endl;*/

    if (m_rangeStart + dataNeeded > availableData.size() && availableData.size() < deviceSize) {
        // Need to read from device
        if (!device->seek(m_rangeStart))
            return false;
        readData.resize(dataNeeded);
        const int nread = device->read(readData.data(), dataNeeded);
        //kDebug() << "readData (from device): reading" << dataNeeded << "bytes.";
        if (nread < mDataSize)
            return false; // error (or not enough data but we checked for that already)
        if (m_rangeStart == 0 && readData.size() > availableData.size()) {
            availableData = readData; // update cache
        }
        if (nread < readData.size()) {
            // File big enough to contain m_data, but not big enough for the full rangeLength.
            // Pad with zeros.
            memset(readData.data() + nread, 0, dataNeeded - nread);
        }
        //kDebug() << "readData (from device) at pos " << m_rangeStart << ":" << readData;
    } else {
        readData = QByteArray::fromRawData(availableData.constData() + m_rangeStart,
                                           dataNeeded);
        // Warning, readData isn't null-terminated so this kDebug
        // gives valgrind warnings (when printing as char* data).
        //kDebug() << "readData (from availableData) at pos " << m_rangeStart << ":" << readData;
    }

    // All we need to do now, is to look for m_data in readData (whose size is dataNeeded).
    // Either as a simple indexOf search, or applying the mask.

    bool found = false;
    if (m_mask.isEmpty()) {
        //kDebug() << "m_data=" << m_data;
        found = ::indexOf(readData, m_data) != -1;
        //if (found)
        //    kDebug() << "Matched readData=" << readData << "with m_data=" << m_data << "so this is" << mimeType;
    } else {
        const char* mask = m_mask.constData();
        const char* refData = m_data.constData();
        const char* readDataBase = readData.constData();
        // Example (continued from above):
        // deviceSize is 4, so dataNeeded was max'ed to 4.
        // maxStartPos = 4 - 3 + 1 = 2, and indeed
        // we need to check for a match a positions 0 and 1 (ABCx and xABC).
        const qint64 maxStartPos = dataNeeded - mDataSize + 1;
        for (int i = 0; i < maxStartPos; ++i) {
            const char* d = readDataBase + i;
            bool valid = true;
            for (int off = 0; off < mDataSize; ++off ) {
                if ( ((*d++) & mask[off]) != ((refData[off] & mask[off])) ) {
                    valid = false;
                    break;
                }
            }
            if (valid)
                found = true;
        }
    }
    if (!found)
        return false;

    // No submatch? Then we are done.
    if (m_subMatches.isEmpty())
        return true;

    // Check that one of the submatches matches too
    return testMatches(device, availableData, m_subMatches, mimeType);
}
