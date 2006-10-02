/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QVariant;

class Q_GUI_EXPORT QSizePolicy
{
    Q_GADGET
    Q_ENUMS(Policy)
private:
    enum SizePolicyMasks {
        HSize = 4,
        HMask = 0x0f,
        VMask = HMask << HSize
    };
public:
    enum PolicyFlag {
        GrowFlag = 1,
        ExpandFlag = 2,
        ShrinkFlag = 4,
        IgnoreFlag = 8
    };

    enum Policy {
        Fixed = 0,
        Minimum = GrowFlag,
        Maximum = ShrinkFlag,
        Preferred = GrowFlag | ShrinkFlag,
        MinimumExpanding = GrowFlag | ExpandFlag,
        Expanding = GrowFlag | ShrinkFlag | ExpandFlag,
        Ignored = ShrinkFlag|GrowFlag|IgnoreFlag
    };

    QSizePolicy() : data(0) { }

    QSizePolicy(Policy horizontal, Policy vertical)
        : data(horizontal | (vertical<<HSize)) { }

    Policy horizontalPolicy() const { return static_cast<Policy>(data & HMask); }
    Policy verticalPolicy() const { return static_cast<Policy>((data & VMask) >> HSize); }

    void setHorizontalPolicy(Policy d) { data = (data & ~HMask) | d; }
    void setVerticalPolicy(Policy d) { data = (data & ~(HMask << HSize)) | (d << HSize); }

    Qt::Orientations expandingDirections() const {
        Qt::Orientations result;
        if (verticalPolicy() & ExpandFlag)
            result |= Qt::Vertical;
        if (horizontalPolicy() & ExpandFlag)
            result |= Qt::Horizontal;
        return result;
    }

    void setHeightForWidth(bool b) { data = b ? (data | (1 << 2*HSize)) : (data & ~(1 << 2*HSize));  }
    bool hasHeightForWidth() const { return data & (1 << 2*HSize); }

    bool operator==(const QSizePolicy& s) const { return data == s.data; }
    bool operator!=(const QSizePolicy& s) const { return data != s.data; }
    operator QVariant() const; // implemented in qabstractlayout.cpp

    int horizontalStretch() const { return data >> 24; }
    int verticalStretch() const { return (data >> 16) & 0xff; }
    void setHorizontalStretch(uchar stretchFactor) { data = (data&0x00ffffff) | (uint(stretchFactor)<<24); }
    void setVerticalStretch(uchar stretchFactor) { data = (data&0xff00ffff) | (uint(stretchFactor)<<16); }

    void transpose();

#ifdef QT3_SUPPORT
    typedef Policy SizeType;
#ifndef qdoc
    typedef Qt::Orientations ExpandData;
    enum {
        NoDirection = 0,
        Horizontally = 1,
        Vertically = 2,
        BothDirections = Horizontally | Vertically
    };
#else
    enum ExpandData {
        NoDirection = 0x0,
        Horizontally = 0x1,
        Vertically = 0x2,
        BothDirections = 0x3
    };
#endif // qdoc

    inline QT3_SUPPORT bool mayShrinkHorizontally() const
        { return horizontalPolicy() & ShrinkFlag; }
    inline QT3_SUPPORT bool mayShrinkVertically() const { return verticalPolicy() & ShrinkFlag; }
    inline QT3_SUPPORT bool mayGrowHorizontally() const { return horizontalPolicy() & GrowFlag; }
    inline QT3_SUPPORT bool mayGrowVertically() const { return verticalPolicy() & GrowFlag; }
    inline QT3_SUPPORT Qt::Orientations expanding() const { return expandingDirections(); }

    QT3_SUPPORT_CONSTRUCTOR QSizePolicy(Policy hor, Policy ver, bool hfw)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) { }

    QT3_SUPPORT_CONSTRUCTOR QSizePolicy(Policy hor, Policy ver, uchar hors, uchar vers, bool hfw = false)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) {
        setHorizontalStretch(hors);
        setVerticalStretch(vers);
    }

    inline QT3_SUPPORT Policy horData() const { return static_cast<Policy>(data & HMask); }
    inline QT3_SUPPORT Policy verData() const { return static_cast<Policy>((data & VMask) >> HSize); }
    inline QT3_SUPPORT void setHorData(Policy d) { setHorizontalPolicy(d); }
    inline QT3_SUPPORT void setVerData(Policy d) { setVerticalPolicy(d); }

    inline QT3_SUPPORT uint horStretch() const { return horizontalStretch(); }
    inline QT3_SUPPORT uint verStretch() const { return verticalStretch(); }
    inline QT3_SUPPORT void setHorStretch(uchar sf) { setHorizontalStretch(sf); }
    inline QT3_SUPPORT void setVerStretch(uchar sf) { setVerticalStretch(sf); }
#endif

private:
#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);
#endif
    QSizePolicy(int i) : data(i) { }

    quint32 data;
};

// implemented in qlayout.cpp
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);

inline void QSizePolicy::transpose() {
    Policy hData = horizontalPolicy();
    Policy vData = verticalPolicy();
    uchar hStretch = horizontalStretch();
    uchar vStretch = verticalStretch();
    setHorizontalPolicy(vData);
    setVerticalPolicy(hData);
    setHorizontalStretch(vStretch);
    setVerticalStretch(hStretch);
}

QT_END_HEADER

#endif // QSIZEPOLICY_H
