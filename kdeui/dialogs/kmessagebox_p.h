/* This file is part of the KDE libraries
   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KMESSAGEBOX_P_H
#define KMESSAGEBOX_P_H

#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtGui/QWidget>

class BackgroundIconWidget : public QWidget
{
    Q_OBJECT
public:
    BackgroundIconWidget(const QIcon &_icon, bool _alignTop, QWidget *parent)
      : QWidget(parent),
        icon(_icon),
        alignTop(_alignTop),
        iconSize(64),
        paintIcon(false) {}

    void setIconSize(int size) {
        iconSize = size;
        paintIcon = true;
        setMinimumHeight(size);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        if (paintIcon) { // do not unnecessary paints of the pixmap before we need it...
            QPainter p(this);
            p.setOpacity(0.25);
            p.drawPixmap(QPoint(width() - iconSize, alignTop ? 0 : height() / 2 - (iconSize / 2)),
                         icon.pixmap(iconSize));
        }
        QWidget::paintEvent(event);
    }

private:
    QIcon icon;
    bool alignTop;
    int iconSize;
    bool paintIcon;
};

#endif /* KMESSAGEBOX_P_H */
