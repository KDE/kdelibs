/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCHAR_H
#define QCHAR_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QString;

struct QLatin1Char
{
public:
    inline explicit QLatin1Char(char c) : ch(c) {}
    inline const char toLatin1() const { return ch; }
    inline const ushort unicode() const { return ushort(uchar(ch)); }

private:
    char ch;
};


class Q_CORE_EXPORT QChar {
public:
    QChar();
#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN_CONSTRUCTOR QChar(char c);
    QT_ASCII_CAST_WARN_CONSTRUCTOR QChar(uchar c);
#endif
    QChar(QLatin1Char ch);
    QChar(uchar c, uchar r);
    inline QChar(ushort rc) : ucs(rc){}
    QChar(short rc);
    QChar(uint rc);
    QChar(int rc);
    enum SpecialCharacter {
        Null = 0x0000,
        Nbsp = 0x00a0,
        ReplacementCharacter = 0xfffd,
        ObjectReplacementCharacter = 0xfffc,
        ByteOrderMark = 0xfeff,
        ByteOrderSwapped = 0xfffe,
#ifdef QT3_SUPPORT
        null = Null,
        replacement = ReplacementCharacter,
        byteOrderMark = ByteOrderMark,
        byteOrderSwapped = ByteOrderSwapped,
        nbsp = Nbsp,
#endif
        ParagraphSeparator = 0x2029,
        LineSeparator = 0x2028
    };
    QChar(SpecialCharacter sc);

    // Unicode information

    enum Category
    {
        NoCategory,

        Mark_NonSpacing,          //   Mn
        Mark_SpacingCombining,    //   Mc
        Mark_Enclosing,           //   Me

        Number_DecimalDigit,      //   Nd
        Number_Letter,            //   Nl
        Number_Other,             //   No

        Separator_Space,          //   Zs
        Separator_Line,           //   Zl
        Separator_Paragraph,      //   Zp

        Other_Control,            //   Cc
        Other_Format,             //   Cf
        Other_Surrogate,          //   Cs
        Other_PrivateUse,         //   Co
        Other_NotAssigned,        //   Cn

        Letter_Uppercase,         //   Lu
        Letter_Lowercase,         //   Ll
        Letter_Titlecase,         //   Lt
        Letter_Modifier,          //   Lm
        Letter_Other,             //   Lo

        Punctuation_Connector,    //   Pc
        Punctuation_Dash,         //   Pd
        Punctuation_Open,         //   Ps
        Punctuation_Close,        //   Pe
        Punctuation_InitialQuote, //   Pi
        Punctuation_FinalQuote,   //   Pf
        Punctuation_Other,        //   Po

        Symbol_Math,              //   Sm
        Symbol_Currency,          //   Sc
        Symbol_Modifier,          //   Sk
        Symbol_Other,             //   So

        Punctuation_Dask = Punctuation_Dash // oops
    };

    enum Direction
    {
        DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS, DirON,
        DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN
    };

    enum Decomposition
    {
        NoDecomposition,
        Canonical,
        Font,
        NoBreak,
        Initial,
        Medial,
        Final,
        Isolated,
        Circle,
        Super,
        Sub,
        Vertical,
        Wide,
        Narrow,
        Small,
        Square,
        Compat,
        Fraction

#ifdef QT3_SUPPORT
        , Single = NoDecomposition
#endif
    };

    enum Joining
    {
        OtherJoining, Dual, Right, Center
    };

    enum CombiningClass
    {
        Combining_BelowLeftAttached       = 200,
        Combining_BelowAttached           = 202,
        Combining_BelowRightAttached      = 204,
        Combining_LeftAttached            = 208,
        Combining_RightAttached           = 210,
        Combining_AboveLeftAttached       = 212,
        Combining_AboveAttached           = 214,
        Combining_AboveRightAttached      = 216,

        Combining_BelowLeft               = 218,
        Combining_Below                   = 220,
        Combining_BelowRight              = 222,
        Combining_Left                    = 224,
        Combining_Right                   = 226,
        Combining_AboveLeft               = 228,
        Combining_Above                   = 230,
        Combining_AboveRight              = 232,

        Combining_DoubleBelow             = 233,
        Combining_DoubleAbove             = 234,
        Combining_IotaSubscript           = 240
    };

    enum UnicodeVersion {
        Unicode_Unassigned,
        Unicode_1_1,
        Unicode_2_0,
        Unicode_2_1_2,
        Unicode_3_0,
        Unicode_3_1,
        Unicode_3_2,
        Unicode_4_0
    };
    // ****** WHEN ADDING FUNCTIONS, CONSIDER ADDING TO QCharRef TOO

    int digitValue() const;
    QChar toLower() const;
    QChar toUpper() const;

    Category category() const;
    Direction direction() const;
    Joining joining() const;
    bool hasMirrored() const;
    inline bool isLower() const { return category() == Letter_Lowercase; }
    inline bool isUpper() const { return category() == Letter_Uppercase; }

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool mirrored() const { return hasMirrored(); }
#endif
    QChar mirroredChar() const;
    QString decomposition() const;
    Decomposition decompositionTag() const;
    unsigned char combiningClass() const;

    UnicodeVersion unicodeVersion() const;

    const char toAscii() const;
    inline const char toLatin1() const;
    inline const ushort unicode() const { return ucs; }
#ifdef Q_NO_PACKED_REFERENCE
    inline ushort &unicode() { return const_cast<ushort&>(ucs); }
#else
    inline ushort &unicode() { return ucs; }
#endif

    static QChar fromAscii(char c);
    static QChar fromLatin1(char c);

    inline bool isNull() const { return ucs == 0; }
    bool isPrint() const;
    bool isPunct() const;
    bool isSpace() const;
    bool isMark() const;
    bool isLetter() const;
    bool isNumber() const;
    bool isLetterOrNumber() const;
    bool isDigit() const;
    bool isSymbol() const;

    inline uchar cell() const { return uchar(ucs & 0xff); }
    inline uchar row() const { return uchar((ucs>>8)&0xff); }
    inline void setCell(uchar cell);
    inline void setRow(uchar row);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QChar lower() const { return toLower(); }
    inline QT3_SUPPORT QChar upper() const { return toUpper(); }
    static inline QT3_SUPPORT bool networkOrdered() {
        return QSysInfo::ByteOrder == QSysInfo::BigEndian;
    }
    inline QT3_SUPPORT const char latin1() const { return toLatin1(); }
    inline QT3_SUPPORT const char ascii() const { return toAscii(); }
#endif

private:
#ifdef QT_NO_CAST_FROM_ASCII
    QChar(char c);
    QChar(uchar c);
#endif
    ushort ucs;
} Q_PACKED;

Q_DECLARE_TYPEINFO(QChar, Q_MOVABLE_TYPE);

inline QChar::QChar() : ucs(0) {}

inline const char QChar::toLatin1() const { return ucs > 0xff ? '\0' : char(ucs); }
inline QChar QChar::fromLatin1(char c) { return QChar(ushort(uchar(c))); }

inline QChar::QChar(uchar c, uchar r) : ucs((r << 8) | c){}
inline QChar::QChar(short rc) : ucs(ushort(rc)){}
inline QChar::QChar(uint rc) : ucs(ushort(rc & 0xffff)){}
inline QChar::QChar(int rc) : ucs(ushort(rc & 0xffff)){}
inline QChar::QChar(SpecialCharacter s) : ucs(ushort(s)) {}
inline QChar::QChar(QLatin1Char ch) : ucs(ch.unicode()) {}

inline void QChar::setCell(uchar acell)
{ ucs = (ucs & 0xff00) + acell; }
inline void QChar::setRow(uchar arow)
{ ucs = (ushort(arow)<<8) + (ucs&0xff); }

inline bool operator==(QChar c1, QChar c2) { return c1.unicode() == c2.unicode(); }
inline bool operator!=(QChar c1, QChar c2) { return c1.unicode() != c2.unicode(); }
inline bool operator<=(QChar c1, QChar c2) { return c1.unicode() <= c2.unicode(); }
inline bool operator>=(QChar c1, QChar c2) { return c1.unicode() >= c2.unicode(); }
inline bool operator<(QChar c1, QChar c2) { return c1.unicode() < c2.unicode(); }
inline bool operator>(QChar c1, QChar c2) { return c1.unicode() > c2.unicode(); }

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QChar &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QChar &);
#endif

QT_END_HEADER

#endif // QCHAR_H
