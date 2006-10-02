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

#ifndef QBITMAP_H
#define QBITMAP_H

#include <QtGui/qpixmap.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QVariant;

class Q_GUI_EXPORT QBitmap : public QPixmap
{
public:
    QBitmap();
    QBitmap(const QPixmap &);
    QBitmap(int w, int h);
    explicit QBitmap(const QSize &);
    explicit QBitmap(const QString &fileName, const char *format=0);
    ~QBitmap();

    QBitmap &operator=(const QPixmap &);
    operator QVariant() const;

    inline void clear() { fill(Qt::color0); }

    static QBitmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);
    static QBitmap fromData(const QSize &size, const uchar *bits,
                            QImage::Format monoFormat = QImage::Format_MonoLSB);

    QBitmap transformed(const QMatrix &) const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT_CONSTRUCTOR QBitmap(int w, int h, bool clear);
    inline QT3_SUPPORT_CONSTRUCTOR QBitmap(const QSize &, bool clear);
    QT3_SUPPORT_CONSTRUCTOR QBitmap(int w, int h, const uchar *bits, bool isXbitmap=false);
    QT3_SUPPORT_CONSTRUCTOR QBitmap(const QSize &, const uchar *bits, bool isXbitmap=false);
    inline QT3_SUPPORT QBitmap xForm(const QMatrix &matrix) const { return transformed(matrix); }
    QT3_SUPPORT_CONSTRUCTOR QBitmap(const QImage &image) { *this = fromImage(image); }
    QT3_SUPPORT QBitmap &operator=(const QImage &image) { *this = fromImage(image); return *this; }
#endif
};
Q_DECLARE_SHARED(QBitmap)

#ifdef QT3_SUPPORT
inline QBitmap::QBitmap(int w, int h, bool clear)
    : QPixmap(QSize(w, h), BitmapType)
{
    if (clear) this->clear();
}

inline QBitmap::QBitmap(const QSize &size, bool clear)
    : QPixmap(size, BitmapType)
{
    if (clear) this->clear();
}
#endif

QT_END_HEADER

#endif // QBITMAP_H
