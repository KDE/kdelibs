/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_IMAGEPREVIEWWIDGET_H
#define KNEWSTUFF3_UI_IMAGEPREVIEWWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QImage>

namespace KNS3 {

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget *parent = 0);

    void setImage(const QImage& preview);

    virtual QSize sizeHint() const;
    
signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    
private:
    QImage m_image;
    QImage m_scaledImage;
    QPixmap m_frameImage;
};

}

#endif // IMAGEPREVIEWWIDGET_H
