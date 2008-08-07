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
#ifndef KENCODINGPROBER_H
#define KENCODINGPROBER_H

#include <kdecore_export.h>
#include <QtCore/QString>

class KEncodingProberPrivate;

/**
 * @short Provides encoding detection(probe) capabilities.
 *
 * Probe the encoding of raw data only.
 * In the case it can't find it, return the most possible encoding it guessed.
 *
 * Always do Unicode probe regardless the ProberType
 *
 * Feed data to it serveral times until ProberState changed to FoundIt/NotMe,
 * or the Confidence reach a value you think acceptable.
 *
 * Intended lifetime of the object: one instance per ProberType.
 *
 * Typical use:
 * \code
 * QByteArray data, moredata;
 * ...
 * KEncodingProber prober(KEncodingProber::Chinese);
 * prober.feed(data);
 * prober.feed(moredata);
 * if (prober.getConfidence() > 0.6)
 *    QString out = QTextCodec::codeForName(prober.getEncodingName())->toUnicode(data);
 * \endcode
 *
 * at least 256 characters are needed to change the ProberState from Probing to other states.
 * if you don't have so many characters to probe, 
 * decide whether to accept the encoding it guessed so far according to the Confidence by yourself.
 *
 * @short Guess encoding of char array
 *
 */
class KDECORE_EXPORT KEncodingProber
{
public:

    /**
     * Sure find the encoding / not included in current ProberType / need more data to make final decision
     */
    enum ProberState {
        FoundIt,
        NotMe,
        Probing
    };

    enum ProberType {
        Universal,
        Arabic,
        Baltic,
        CentralEuropean,
        Chinese,
        Cyrillic,
        EasternAsia,
        Greek,
        Hebrew,
        Japanese,
        Korean,
        NorthernSaami,
        SouthEasternEurope,
        Thai,
        Turkish,
        Unicode,
        WesternEuropean
    };
    
    /**
     * Default ProberType is Universal(detect all possibe encodings)
     */
    KEncodingProber(ProberType proberType=Universal);

    ~KEncodingProber();

    /**
     * reset the prober's internal state and data.
     */
    void reset();

    /**
     * The main class method
     *
     * feed data to the prober
     *
     * @returns the ProberState after probe the feeded data
     */
    ProberState feed(const QByteArray &data);
    ProberState feed(const char* data, int len);

    /**
     * @returns the prober's current ProberState
     *
     */
    ProberState getState() const;

    /**
     * @returns the name of the best encoding it guessed so far
     */
    const char* getEncodingName() const;

    /**
     * @returns the confidence(sureness) of encoding it guessed so far (0.0 ~ 0.99)
     */
    float getConfidence() const;
    
private:
    KEncodingProberPrivate* const d;
};

#endif
