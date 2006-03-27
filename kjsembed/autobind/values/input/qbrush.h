/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QBRUSH_H
#define QBRUSH_H

#include <QtCore/qpair.h>
#include <QtCore/qpoint.h>
#include <QtCore/qvector.h>
#include <QtGui/qcolor.h>

QT_MODULE(Gui)

struct QBrushData;
class QPixmap;
class QGradient;
class QVariant;

class Q_GUI_EXPORT QBrush
{
public:
    QBrush();
    QBrush(Qt::BrushStyle bs);
    QBrush(const QColor &color, Qt::BrushStyle bs=Qt::SolidPattern);
    QBrush(Qt::GlobalColor color, Qt::BrushStyle bs=Qt::SolidPattern);

    QBrush(const QColor &color, const QPixmap &pixmap);
    QBrush(Qt::GlobalColor color, const QPixmap &pixmap);
    QBrush(const QPixmap &pixmap);

    QBrush(const QBrush &brush);

    QBrush(const QGradient &gradient);

    ~QBrush();
    QBrush &operator=(const QBrush &brush);
    operator QVariant() const;

    inline Qt::BrushStyle style() const;
    void setStyle(Qt::BrushStyle);

    QPixmap texture() const;
    void setTexture(const QPixmap &pixmap);

    inline const QColor &color() const;
    void setColor(const QColor &color);
    inline void setColor(Qt::GlobalColor color);

    const QGradient *gradient() const;

    bool isOpaque() const;

    bool operator==(const QBrush &b) const;
    inline bool operator!=(const QBrush &b) const { return !(operator==(b)); }

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT operator const QColor&() const;
    QT3_SUPPORT QPixmap *pixmap() const;
    inline QT3_SUPPORT void setPixmap(const QPixmap &pixmap) { setTexture(pixmap); }
#endif

private:
#if defined(Q_WS_X11)
    friend class QX11PaintEngine;
#endif
    friend class QPainter;
    void detach(Qt::BrushStyle newStyle);
    void init(const QColor &color, Qt::BrushStyle bs);
    QBrushData *d;
    void cleanUp(QBrushData *x);
};

inline void QBrush::setColor(Qt::GlobalColor acolor)
{ setColor(QColor(acolor)); }

Q_DECLARE_TYPEINFO(QBrush, Q_MOVABLE_TYPE);

/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QBrush &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QBrush &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QBrush &);
#endif

struct QBrushData
{
    QAtomic ref;
    Qt::BrushStyle style;
    QColor color;
};

inline Qt::BrushStyle QBrush::style() const { return d->style; }
inline const QColor &QBrush::color() const { return d->color; }

#ifdef QT3_SUPPORT
inline QBrush::operator const QColor&() const { return d->color; }
#endif


/*******************************************************************************
 * QGradients
 */
class QGradientPrivate;

typedef QPair<qreal, QColor> QGradientStop;
typedef QVector<QGradientStop> QGradientStops;

class Q_GUI_EXPORT QGradient
{
public:
    enum Type {
        LinearGradient,
        RadialGradient,
        ConicalGradient,
        NoGradient
    };

    enum Spread {
        PadSpread,
        ReflectSpread,
        RepeatSpread
    };

    QGradient();

    Type type() const { return m_type; }

    inline void setSpread(Spread spread);
    Spread spread() const { return m_spread; }

    void setColorAt(qreal pos, const QColor &color);

    void setStops(const QGradientStops &stops);
    QGradientStops stops() const;

    bool operator==(const QGradient &gradient); // 5.0 - remove me
    bool operator==(const QGradient &gradient) const;


private:
    friend class QLinearGradient;
    friend class QRadialGradient;
    friend class QConicalGradient;

    Type m_type;
    Spread m_spread;
    QGradientStops m_stops;
    union {
        struct {
            qreal x1, y1, x2, y2;
        } linear;
        struct {
            qreal cx, cy, fx, fy, radius;
        } radial;
        struct {
            qreal cx, cy, angle;
        } conical;
    } m_data;
    void *dummy;
};

inline void QGradient::setSpread(Spread aspread)
{ m_spread = aspread; }

class Q_GUI_EXPORT QLinearGradient : public QGradient
{
public:
    QLinearGradient(const QPointF &start, const QPointF &finalStop);
    QLinearGradient(qreal xStart, qreal yStart, qreal xFinalStop, qreal yFinalStop);

    QPointF start() const;
    QPointF finalStop() const;
};


class Q_GUI_EXPORT QRadialGradient : public QGradient
{
public:
    QRadialGradient(const QPointF &center, qreal radius, const QPointF &focalPoint = QPointF());
    QRadialGradient(qreal cx, qreal cy, qreal radius, qreal fx=0, qreal fy=0);

    QPointF center() const;
    QPointF focalPoint() const;
    qreal radius() const;
};


class Q_GUI_EXPORT QConicalGradient : public QGradient
{
public:
    QConicalGradient(const QPointF &center, qreal startAngle);
    QConicalGradient(qreal cx, qreal cy, qreal startAngle);

    QPointF center() const;
    qreal angle() const;
};

#endif // QBRUSH_H
