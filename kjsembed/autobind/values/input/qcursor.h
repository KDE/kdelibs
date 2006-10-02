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

#ifndef QCURSOR_H
#define QCURSOR_H

#include <QtCore/qpoint.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QVariant;

/*
  ### The fake cursor has to go first with old qdoc.
*/
#ifdef QT_NO_CURSOR

class Q_GUI_EXPORT QCursor
{
public:
    static QPoint pos();
    static void setPos(int x, int y);
    inline static void setPos(const QPoint &p) { setPos(p.x(), p.y()); }
private:
    QCursor();
};

#endif // QT_NO_CURSOR

#ifndef QT_NO_CURSOR

struct QCursorData;
class QBitmap;
class QPixmap;

#if defined(Q_WS_MAC)
void qt_mac_set_cursor(const QCursor *c, const QPoint &p);
#endif

class Q_GUI_EXPORT QCursor
{
public:
    QCursor();
    QCursor(Qt::CursorShape shape);
    QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX=-1, int hotY=-1);
    QCursor(const QPixmap &pixmap, int hotX=-1, int hotY=-1);
    QCursor(const QCursor &cursor);
    ~QCursor();
    QCursor &operator=(const QCursor &cursor);
    operator QVariant() const;

    Qt::CursorShape shape() const;
    void setShape(Qt::CursorShape newShape);

    const QBitmap *bitmap() const;
    const QBitmap *mask() const;
    QPixmap pixmap() const;
    QPoint hotSpot() const;

    static QPoint pos();
    static void setPos(int x, int y);
    inline static void setPos(const QPoint &p) { setPos(p.x(), p.y()); }

#ifdef qdoc
    HCURSOR_or_HANDLE handle() const;
    QCursor(HCURSOR cursor);
    QCursor(Qt::HANDLE cursor);
#endif

#ifndef qdoc
#if defined(Q_WS_WIN)
    HCURSOR handle() const;
    QCursor(HCURSOR cursor);
#elif defined(Q_WS_X11)
    Qt::HANDLE handle() const;
    QCursor(Qt::HANDLE cursor);
    static int x11Screen();
#elif defined(Q_WS_MAC)
    Qt::HANDLE handle() const;
#elif defined(Q_WS_QWS)
    int handle() const;
#endif
#endif

private:
    QCursorData *d;
#if defined(Q_WS_MAC)
    friend void qt_mac_set_cursor(const QCursor *c, const QPoint &p);
#endif
};

#ifdef QT3_SUPPORT
// CursorShape is defined in X11/X.h
#ifdef CursorShape
#define X_CursorShape CursorShape
#undef CursorShape
#endif
typedef Qt::CursorShape QCursorShape;
#ifdef X_CursorShape
#define CursorShape X_CursorShape
#endif
#endif

/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &outS, const QCursor &cursor);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &inS, QCursor &cursor);
#endif
#endif // QT_NO_CURSOR

QT_END_HEADER

#endif // QCURSOR_H
