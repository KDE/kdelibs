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

#include "imagepreviewwidget.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

#include <kstandarddirs.h>

#include <core/entryinternal.h>

using namespace KNS3;

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent) :
    QWidget(parent)
{
    //installEventFilter(this);
    
    QString framefile = KStandardDirs::locate("data", "knewstuff/pics/thumb_frame.png");
    m_frameImage = QPixmap(framefile);  
}

void ImagePreviewWidget::setImage(const QImage &preview)
{
    m_image = preview;
    m_scaledImage = QImage();
    updateGeometry();
    repaint();
}

void ImagePreviewWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    emit clicked();
}

void ImagePreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_scaledImage = QImage();
    repaint();
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    if (m_image.isNull()) {
        return;
    }
    
    QPainter painter(this);
    int margin = painter.fontMetrics().height() / 2;
    //painter.drawImage(contentsRect(), m_image);
    
    int width = contentsRect().width();
    int height = contentsRect().height();
    
    if (m_scaledImage.isNull()) {
        m_scaledImage = m_image.scaled(width - 2*margin, height - 2*margin, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    QPoint point;

    point.setX(contentsRect().left() + ((width - m_scaledImage.width()) / 2));
    point.setY(contentsRect().top() + ((height - m_scaledImage.height()) / 2));
    
    QPoint framePoint(point.x() - 5, point.y() - 5);
    painter.drawPixmap(framePoint, m_frameImage.scaled(m_scaledImage.width() + 10, m_scaledImage.height() + 10));
    painter.drawImage(point, m_scaledImage);
}

QSize ImagePreviewWidget::sizeHint() const
{
    if (m_image.isNull()) {
        return QSize();
    }
    QSize sh = m_image.size();
    sh.scale(maximumSize(), Qt::KeepAspectRatio);
    return sh;
}
