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
#include <QtGui/QPaintEvent>

KUrlDropDownButton::KUrlDropDownButton(KUrlNavigator* parent) :
    KUrlButton(parent)
{
}

KUrlDropDownButton::~KUrlDropDownButton()
{
}

void KUrlDropDownButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());

    const int buttonWidth = width();
    const int buttonHeight = height();

    // draw button background
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawRect(0, 0, buttonWidth, buttonHeight);

    // draw '...'
    painter.setPen(foregroundColor());
    painter.drawText(QRect(0, 0, buttonWidth, buttonHeight), Qt::AlignCenter, "...");
}

#include "kurldropdownbutton_p.moc"
