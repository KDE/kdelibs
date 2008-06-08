/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
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

#include "kurldropdownbutton_p.h"
#include "kurlnavigator.h"

#include <kglobalsettings.h>

#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>

KUrlDropDownButton::KUrlDropDownButton(KUrlNavigator* parent) :
    KUrlButton(parent)
{
    setText("...");
}

KUrlDropDownButton::~KUrlDropDownButton()
{
}

QSize KUrlDropDownButton::sizeHint() const
{
    int width = fontMetrics().width(text()) + 4 * BorderWidth;
    if (width < minimumWidth()) {
        width = minimumWidth();
    }
    return QSize(width, KUrlButton::sizeHint().height());
}

void KUrlDropDownButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    drawHoverBackground(&painter);

    // draw '...'
    painter.setPen(foregroundColor());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

#include "kurldropdownbutton_p.moc"
