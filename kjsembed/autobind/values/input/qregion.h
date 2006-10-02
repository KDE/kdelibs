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

#ifndef QREGION_H
#define QREGION_H

#include <QtCore/qatomic.h>
#include <QtCore/qrect.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

template <class T> class QVector;
class QVariant;

#if defined(Q_WS_QWS) || defined(Q_WS_X11) || defined(Q_WS_MAC)
struct QRegionPrivate;
#endif

class QBitmap;

class Q_GUI_EXPORT QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion(int x, int y, int w, int h, RegionType t = Rectangle);
    QRegion(const QRect &r, RegionType t = Rectangle);
    QRegion(const QPolygon &pa, Qt::FillRule fillRule = Qt::OddEvenFill);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QRegion(const QPolygon &pa, bool winding);
#endif
    QRegion(const QRegion &region);
    QRegion(const QBitmap &bitmap);
    ~QRegion();
    QRegion &operator=(const QRegion &);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool isNull() const { return isEmpty(); }
#endif
    bool isEmpty() const;

    bool contains(const QPoint &p) const;
    bool contains(const QRect &r) const;

    void translate(int dx, int dy);
    inline void translate(const QPoint &p) { translate(p.x(), p.y()); }
    QRegion translated(int dx, int dy) const;
    inline QRegion translated(const QPoint &p) const { return translated(p.x(), p.y()); }

    // ### Qt 5: make these four functions QT4_SUPPORT
    QRegion unite(const QRegion &r) const;
    QRegion intersect(const QRegion &r) const;
    QRegion subtract(const QRegion &r) const;
    QRegion eor(const QRegion &r) const;

    inline QRegion united(const QRegion &r) const { return unite(r); }
    inline QRegion intersected(const QRegion &r) const { return intersect(r); }
    inline QRegion subtracted(const QRegion &r) const { return subtract(r); }
    inline QRegion xored(const QRegion &r) const { return eor(r); }

    bool intersects(const QRegion &r) const;
    bool intersects(const QRect &r) const;

    QRect boundingRect() const;
    QVector<QRect> rects() const;
    void setRects(const QRect *rect, int num);

    const QRegion operator|(const QRegion &r) const;
    const QRegion operator+(const QRegion &r) const;
    const QRegion operator&(const QRegion &r) const;
    const QRegion operator-(const QRegion &r) const;
    const QRegion operator^(const QRegion &r) const;
    QRegion& operator|=(const QRegion &r);
    QRegion& operator+=(const QRegion &r);
    QRegion& operator&=(const QRegion &r);
    QRegion& operator-=(const QRegion &r);
    QRegion& operator^=(const QRegion &r);

    bool operator==(const QRegion &r) const;
    inline bool operator!=(const QRegion &r) const { return !(operator==(r)); }
    operator QVariant() const;

#ifdef qdoc
    Handle handle() const;
#endif
#ifndef qdoc
#if defined(Q_WS_WIN)
    inline HRGN    handle() const { return d->rgn; }
#elif defined(Q_WS_X11)
    inline Region handle() const { if(!d->rgn) updateX11Region(); return d->rgn; }
#elif defined(Q_WS_MAC)
    inline RgnHandle handle() const { return handle(false); }
    RgnHandle handle(bool require_rgn) const;
#elif defined(Q_WS_QWS)
    // QGfx_QWS needs this for region drawing
    inline void *handle() const { return d->qt_rgn; }
#endif
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRegion &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRegion &);
#endif
private:
    QRegion copy() const;   // helper of detach.
    void detach();
#if defined(Q_WS_WIN)
    QRegion winCombine(const QRegion &r, int num) const;
#elif defined(Q_WS_X11)
    void updateX11Region() const;
    void *clipRectangles(int &num) const;
    friend void *qt_getClipRects(const QRegion &r, int &num);
#elif defined(Q_WS_MAC)
    friend QRegion qt_mac_convert_mac_region(RgnHandle rgn);
#endif
    void exec(const QByteArray &ba, int ver = 0);
    struct QRegionData {
        QBasicAtomic ref;
#if defined(Q_WS_WIN)
        HRGN   rgn;
#elif defined(Q_WS_X11)
        Region rgn;
        void *xrectangles;
#elif defined(Q_WS_MAC)
        mutable RgnHandle rgn;
#endif
#if defined(Q_WS_QWS) || defined(Q_WS_X11) || defined(Q_WS_MAC)
        QRegionPrivate *qt_rgn;
#endif
    };
#if defined(Q_WS_WIN)
    friend class QETWidget;
#endif
    struct QRegionData *d;
    static struct QRegionData shared_empty;
    static void cleanUp(QRegionData *x);
};

/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRegion &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRegion &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QRegion &);
#endif

QT_END_HEADER

#endif // QREGION_H
