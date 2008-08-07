/*
    This file is part of the KDE libraries

    Copyright (C) 2008 Wang Hoi (zealot.hoi@gmail.com)

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

#include "kencodingprober.h"

#include "probers/nsCharSetProber.h"
#include "probers/nsUniversalDetector.h"
#include "probers/ChineseGroupProber.h"
#include "probers/JapaneseGroupProber.h"
#include "probers/nsSBCSGroupProber.h"
#include "probers/nsMBCSGroupProber.h"

#include <string.h>

#define MINIMUM_THRESHOLD (float)0.2

class KEncodingProberPrivate
{
public:
    KEncodingProberPrivate(): encoding(""), prober(NULL), mStart(true) {};
    ~KEncodingProberPrivate()
    {
        delete encoding;
        delete prober;
    }
    void unicodeTest(const char *aBuf, int aLen)
    {
        if (mStart)
        {
            mStart = false;
            if (aLen > 3)
            switch (aBuf[0])
            {
                case '\xEF':
                    if (('\xBB' == aBuf[1]) && ('\xBF' == aBuf[2]))
                    // EF BB BF  UTF-8 encoded BOM
                    encoding = "UTF-8";
                    break;
                case '\xFE':
                    if (('\xFF' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3]))
                        // FE FF 00 00  UCS-4, unusual octet order BOM (3412)
                        encoding = "ISO-10646-UCS-4";
                    else if ('\xFF' == aBuf[1])
                        // FE FF  UTF-16, big endian BOM
                        encoding = "UTF-16BE";
                        break;
                case '\x00':
                    if (('\x00' == aBuf[1]) && ('\xFE' == aBuf[2]) && ('\xFF' == aBuf[3]))
                        // 00 00 FE FF  UTF-32, big-endian BOM
                        encoding = "UTF-32BE";
                    else if (('\x00' == aBuf[1]) && ('\xFF' == aBuf[2]) && ('\xFE' == aBuf[3]))
                        // 00 00 FF FE  UCS-4, unusual octet order BOM (2143)
                        encoding = "ISO-10646-UCS-4";
                        break;
                case '\xFF':
                    if (('\xFE' == aBuf[1]) && ('\x00' == aBuf[2]) && ('\x00' == aBuf[3]))
                        // FF FE 00 00  UTF-32, little-endian BOM
                        encoding = "UTF-32LE";
                    else if ('\xFE' == aBuf[1])
                        // FF FE  UTF-16, little endian BOM
                        encoding = "UTF-16LE";
                        break;
            }  // switch

            if (encoding && strlen(encoding))
            {
                proberState = KEncodingProber::FoundIt;
                currentConfidence = 0.99;
            }
        }
    }
    KEncodingProber::ProberType proberType;
    KEncodingProber::ProberState proberState;
    float currentConfidence;
    const char *encoding;
    nsCharSetProber *prober;
    bool mStart;
};

KEncodingProber::KEncodingProber(KEncodingProber::ProberType proberType): d(new KEncodingProberPrivate())
{
    d->proberType = proberType;
    /* handle multi-byte encodings carefully , because they're hard to detect,
     *   and have to use some Stastics methods.
     * for single-byte encodings (most western encodings), nsUniversalDetector is ok,
     *   because encoding state machine can detect many such encodings.
     */ 
    switch (proberType) {
        case Arabic:
        case Baltic:
        case CentralEuropean:
        case Cyrillic:
        case Greek:
        case Hebrew:
        case NorthernSaami:
        case SouthEasternEurope:
        case Thai:
        case Turkish:
        case WesternEuropean:
            d->prober = new nsSBCSGroupProber();
            break;
        case Chinese:
            d->prober = new ChineseGroupProber();
            break;
        case Japanese:
            d->prober = new JapaneseGroupProber();
            break;
        case EasternAsia:
        case Korean:
            d->prober = new nsMBCSGroupProber();
            break;
        default:
            d->prober = new nsUniversalDetector();
            break;
    }
    reset();
}

KEncodingProber::~KEncodingProber()
{
    delete d;
}

void KEncodingProber::reset()
{
    d->proberState = KEncodingProber::Probing;
    d->currentConfidence = MINIMUM_THRESHOLD;
    d->encoding = "";
    d->mStart = true;
}

KEncodingProber::ProberState KEncodingProber::feed(const QByteArray &data)
{
    return feed(data.data(), data.size());
}

KEncodingProber::ProberState KEncodingProber::feed(const char* data, int len)
{
    if (d->proberState == Probing) {
        if (d->mStart) {
            d->unicodeTest(data, len);
            if (d->proberState == FoundIt)
                return d->proberState;
        }
        d->prober->HandleData(data, len);
        d->currentConfidence = d->prober->GetConfidence();
        d->encoding = strdup(d->prober->GetCharSetName());
        switch (d->prober->GetState())
        {
            case eNotMe:
                d->proberState = NotMe;
                break;
            case eFoundIt:
                d->proberState = FoundIt;
                break;
            default:
                d->proberState = Probing;
                break;
        }
    }
    return d->proberState;
}

KEncodingProber::ProberState KEncodingProber::getState() const
{
    return d->proberState;
}

const char* KEncodingProber::getEncodingName() const
{
    return strdup(d->encoding);
}

float KEncodingProber::getConfidence() const
{
    return d->currentConfidence;
}


