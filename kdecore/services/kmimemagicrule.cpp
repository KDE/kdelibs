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

static bool testMatches(QIODevice* device, const QByteArray& beginning, const QList<KMimeMagicMatch>& matches)
{
    for ( QList<KMimeMagicMatch>::const_iterator it = matches.begin(), end = matches.end() ;
          it != end ; ++it ) {
        const KMimeMagicMatch& match = *it;
        if (match.match(device, beginning)) {
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


bool KMimeMagicRule::match(QIODevice* device, const QByteArray& beginning) const
{
    return testMatches(device, beginning, m_matches);
}

bool KMimeMagicMatch::match(QIODevice* device, const QByteArray& beginning) const
{
    // First, check that I match, then we'll dive into subMatches if any.
    const qint64 deviceSize = device->size();
    const qint64 rangeLength = qMin(m_rangeLength, deviceSize);

    // Read in one block all the data we'll need
    QByteArray readData;
    const qint64 dataNeeded = m_data.size() + rangeLength - 1;

    /*kDebug() << "need data from " << m_rangeStart
             << " to " << m_rangeStart + dataNeeded
             << "  - beginning has " << beginning.size() << " bytes,"
             << " device has " << deviceSize << " bytes." << endl;*/

    if (m_rangeStart + dataNeeded > beginning.size() && beginning.size() < deviceSize) {
        // Need to read from device
        if (m_rangeStart > deviceSize)
            return false;
        if (!device->seek(m_rangeStart))
            return false;
        readData.resize(dataNeeded);
        const int nread = device->read(readData.data(), dataNeeded);
        if (nread < m_data.size())
            return false;
        if (nread < readData.size()) {
            memset(readData.data() + nread, 0, dataNeeded - nread);
        }
        //kDebug() << "readData (from device) at pos " << m_rangeStart << ":" << readData << endl;
    } else {
        readData = QByteArray::fromRawData(beginning.constData() + m_rangeStart,
                                           dataNeeded);
        //kDebug() << "readData (from beginning) at pos " << m_rangeStart << ":" << readData << endl;
    }

    bool found = false;

    if (m_mask.isEmpty()) {
        //kDebug() << "m_data=" << m_data << endl;
        found = ::indexOf(readData, m_data) != -1;
    } else {
        const char* mask = m_mask.data();
        const char* refData = m_data.data();
        const char* readDataBase = readData.data();
        for (int i = 0; i < rangeLength; ++i) {
            const char* d = readDataBase + i;
            bool valid = true;
            for (int off = 0; off < m_data.size(); ++off ) {
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
    return testMatches(device, beginning, m_subMatches);
}
