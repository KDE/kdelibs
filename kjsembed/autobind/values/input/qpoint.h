/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOINT_H
#define QPOINT_H

#include <QtCore/qnamespace.h>

QT_MODULE(Core)

class Q_CORE_EXPORT QPoint
{
public:
    QPoint();
    QPoint(int xpos, int ypos);

    bool isNull() const;

    int x() const;
    int y() const;
    void setX(int x);
    void setY(int y);

    int manhattanLength() const;

    int &rx();
    int &ry();

    QPoint &operator+=(const QPoint &p);
    QPoint &operator-=(const QPoint &p);
    QPoint &operator*=(qreal c);
    QPoint &operator/=(qreal c);

    friend inline bool operator==(const QPoint &, const QPoint &);
    friend inline bool operator!=(const QPoint &, const QPoint &);
    friend inline const QPoint operator+(const QPoint &, const QPoint &);
    friend inline const QPoint operator-(const QPoint &, const QPoint &);
    friend inline const QPoint operator*(const QPoint &, qreal);
    friend inline const QPoint operator*(qreal, const QPoint &);
    friend inline const QPoint operator-(const QPoint &);
    friend inline const QPoint operator/(const QPoint &, qreal);

private:

#if defined(Q_OS_MAC)
    int yp;
    int xp;
#else
    int xp;
    int yp;
#endif
};

Q_DECLARE_TYPEINFO(QPoint, Q_MOVABLE_TYPE);

/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QPoint &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QPoint &);
#endif

/*****************************************************************************
  QPoint inline functions
 *****************************************************************************/

inline QPoint::QPoint()
{ xp=0; yp=0; }

inline QPoint::QPoint(int xpos, int ypos)
{ xp = xpos; yp = ypos; }

inline bool QPoint::isNull() const
{ return xp == 0 && yp == 0; }

inline int QPoint::x() const
{ return xp; }

inline int QPoint::y() const
{ return yp; }

inline void QPoint::setX(int xpos)
{ xp = xpos; }

inline void QPoint::setY(int ypos)
{ yp = ypos; }

inline int &QPoint::rx()
{ return xp; }

inline int &QPoint::ry()
{ return yp; }

inline QPoint &QPoint::operator+=(const QPoint &p)
{ xp+=p.xp; yp+=p.yp; return *this; }

inline QPoint &QPoint::operator-=(const QPoint &p)
{ xp-=p.xp; yp-=p.yp; return *this; }

inline QPoint &QPoint::operator*=(qreal c)
{ xp = qRound(xp*c); yp = qRound(yp*c); return *this; }

inline bool operator==(const QPoint &p1, const QPoint &p2)
{ return p1.xp == p2.xp && p1.yp == p2.yp; }

inline bool operator!=(const QPoint &p1, const QPoint &p2)
{ return p1.xp != p2.xp || p1.yp != p2.yp; }

inline const QPoint operator+(const QPoint &p1, const QPoint &p2)
{ return QPoint(p1.xp+p2.xp, p1.yp+p2.yp); }

inline const QPoint operator-(const QPoint &p1, const QPoint &p2)
{ return QPoint(p1.xp-p2.xp, p1.yp-p2.yp); }

inline const QPoint operator*(const QPoint &p, qreal c)
{ return QPoint(qRound(p.xp*c), qRound(p.yp*c)); }

inline const QPoint operator*(qreal c, const QPoint &p)
{ return QPoint(qRound(p.xp*c), qRound(p.yp*c)); }

inline const QPoint operator-(const QPoint &p)
{ return QPoint(-p.xp, -p.yp); }

inline QPoint &QPoint::operator/=(qreal c)
{
    Q_ASSERT(!qFuzzyCompare(c, 0));
    xp = qRound(xp/c);
    yp = qRound(yp/c);
    return *this;
}

inline const QPoint operator/(const QPoint &p, qreal c)
{
    Q_ASSERT(!qFuzzyCompare(c, 0));
    return QPoint(qRound(p.xp/c), qRound(p.yp/c));
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QPoint &);
#endif





class Q_CORE_EXPORT QPointF
{
public:
    QPointF();
    QPointF(const QPoint &p);
    QPointF(qreal xpos, qreal ypos);

    bool isNull() const;

    qreal x() const;
    qreal y() const;
    void setX(qreal x);
    void setY(qreal y);

    qreal &rx();
    qreal &ry();

    QPointF &operator+=(const QPointF &p);
    QPointF &operator-=(const QPointF &p);
    QPointF &operator*=(qreal c);
    QPointF &operator/=(qreal c);

    friend inline bool operator==(const QPointF &, const QPointF &);
    friend inline bool operator!=(const QPointF &, const QPointF &);
    friend inline const QPointF operator+(const QPointF &, const QPointF &);
    friend inline const QPointF operator-(const QPointF &, const QPointF &);
    friend inline const QPointF operator*(qreal, const QPointF &);
    friend inline const QPointF operator*(const QPointF &, qreal);
    friend inline const QPointF operator-(const QPointF &);
    friend inline const QPointF operator/(const QPointF &, qreal);

    QPoint toPoint() const;

private:
    friend class QMatrix;

    qreal xp;
    qreal yp;
};

Q_DECLARE_TYPEINFO(QPointF, Q_MOVABLE_TYPE);

/*****************************************************************************
  QPointF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QPointF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QPointF &);
#endif

/*****************************************************************************
  QPointF inline functions
 *****************************************************************************/

inline QPointF::QPointF() : xp(0), yp(0) { }

inline QPointF::QPointF(qreal xpos, qreal ypos) : xp(xpos), yp(ypos) { }

inline QPointF::QPointF(const QPoint &p) : xp(p.x()), yp(p.y()) { }

inline bool QPointF::isNull() const
{
    return qIsNull(xp) && qIsNull(yp);
}

inline qreal QPointF::x() const
{
    return xp;
}

inline qreal QPointF::y() const
{
    return yp;
}

inline void QPointF::setX(qreal xpos)
{
    xp = xpos;
}

inline void QPointF::setY(qreal ypos)
{
    yp = ypos;
}

inline qreal &QPointF::rx()
{
    return xp;
}

inline qreal &QPointF::ry()
{
    return yp;
}

inline QPointF &QPointF::operator+=(const QPointF &p)
{
    xp+=p.xp;
    yp+=p.yp;
    return *this;
}

inline QPointF &QPointF::operator-=(const QPointF &p)
{
    xp-=p.xp; yp-=p.yp; return *this;
}

inline QPointF &QPointF::operator*=(qreal c)
{
    xp*=c; yp*=c; return *this;
}

inline bool operator==(const QPointF &p1, const QPointF &p2)
{
    return qFuzzyCompare(p1.xp, p2.xp) && qFuzzyCompare(p1.yp, p2.yp);
}

inline bool operator!=(const QPointF &p1, const QPointF &p2)
{
    return !qFuzzyCompare(p1.xp, p2.xp) || !qFuzzyCompare(p1.yp, p2.yp);
}

inline const QPointF operator+(const QPointF &p1, const QPointF &p2)
{
    return QPointF(p1.xp+p2.xp, p1.yp+p2.yp);
}

inline const QPointF operator-(const QPointF &p1, const QPointF &p2)
{
    return QPointF(p1.xp-p2.xp, p1.yp-p2.yp);
}

inline const QPointF operator*(const QPointF &p, qreal c)
{
    return QPointF(p.xp*c, p.yp*c);
}

inline const QPointF operator*(qreal c, const QPointF &p)
{
    return QPointF(p.xp*c, p.yp*c);
}

inline const QPointF operator-(const QPointF &p)
{
    return QPointF(-p.xp, -p.yp);
}

inline QPointF &QPointF::operator/=(qreal c)
{
    Q_ASSERT(!qFuzzyCompare(c, 0));
    xp/=c;
    yp/=c;
    return *this;
}

inline const QPointF operator/(const QPointF &p, qreal c)
{
    Q_ASSERT(!qFuzzyCompare(c, 0));
    return QPointF(p.xp/c, p.yp/c);
}

inline QPoint QPointF::toPoint() const
{
    return QPoint(qRound(xp), qRound(yp));
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug d, const QPointF &p);
#endif

#endif // QPOINT_H
