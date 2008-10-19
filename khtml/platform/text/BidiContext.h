/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007 Apple Inc.  All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef BidiContext_h
#define BidiContext_h

#include <wtf/Assertions.h>
#include <wtf/RefPtr.h>
#include <wtf/unicode/Unicode.h>
#include <QChar>

namespace WTF {
    namespace Unicode {
        enum Direction {
            LeftToRight = QChar::DirL,
            RightToLeft = QChar::DirR,
            EuropeanNumber = QChar::DirEN,
            EuropeanNumberSeparator = QChar::DirES,
            EuropeanNumberTerminator = QChar::DirET,
            ArabicNumber = QChar::DirAN,
            CommonNumberSeparator = QChar::DirCS,
            BlockSeparator = QChar::DirB,
            SegmentSeparator = QChar::DirS,
            WhiteSpaceNeutral = QChar::DirWS,
            OtherNeutral = QChar::DirON,
            LeftToRightEmbedding = QChar::DirLRE,
            LeftToRightOverride = QChar::DirLRO,
            RightToLeftArabic = QChar::DirAL,
            RightToLeftEmbedding = QChar::DirRLE,
            RightToLeftOverride = QChar::DirRLO,
            PopDirectionalFormat = QChar::DirPDF,
            NonSpacingMark = QChar::DirNSM,
            BoundaryNeutral = QChar::DirBN
        };
        inline Direction direction(unsigned c) { return (Direction)QChar::direction(c); }
    }
}

namespace WebCore {

// Used to keep track of explicit embeddings.
class BidiContext {
public:
    BidiContext(unsigned char level, WTF::Unicode::Direction direction, bool override = false, BidiContext* parent = 0)
        : m_level(level)
        , m_direction(direction)
        , m_override(override)
        , m_parent(parent)
        , m_refCount(0)
    {
        ASSERT(direction == WTF::Unicode::LeftToRight || direction == WTF::Unicode::RightToLeft);
    }

    void ref() const { m_refCount++; }
    void deref() const
    {
        m_refCount--;
        if (m_refCount <= 0)
            delete this;
    }

    BidiContext* parent() const { return m_parent.get(); }
    unsigned char level() const { return m_level; }
    WTF::Unicode::Direction dir() const { return static_cast<WTF::Unicode::Direction>(m_direction); }
    bool override() const { return m_override; }

private:
    unsigned char m_level;
    unsigned m_direction : 5; // Direction
    bool m_override : 1;
    RefPtr<BidiContext> m_parent;
    mutable int m_refCount;
};

bool operator==(const BidiContext&, const BidiContext&);

} // namespace WebCore

#endif // BidiContext_h
