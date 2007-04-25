/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (<peter.penz@gmx.at>)                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

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
