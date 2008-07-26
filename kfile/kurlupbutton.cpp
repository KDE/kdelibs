/*****************************************************************************
 * Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kurlupbutton_p.h"

#include <klocale.h>
#include <kicon.h>

#include <QtGui/QPainter>

KUrlUpButton::KUrlUpButton(KUrlNavigator* parent) :
    KUrlButton(parent)
{
    setIcon(KIcon("go-up"));
    setToolTip(i18nc("@info:tooltip", "Parent Folder"));
}

KUrlUpButton::~KUrlUpButton()
{
}

QSize KUrlUpButton::sizeHint() const
{
    const int height = KUrlButton::sizeHint().height();
    return QSize(height, height);
}

void KUrlUpButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    drawHoverBackground(&painter);

    // draw icon
    const QPixmap pixmap = icon().pixmap(QSize(22, 22), isEnabled() ? QIcon::Normal : QIcon::Disabled);
    const int x = (width() -  pixmap.width()) / 2;
    const int y = (height() - pixmap.height()) / 2;
    painter.drawPixmap(x, y, pixmap);
}

#include "kurlupbutton_p.moc"
