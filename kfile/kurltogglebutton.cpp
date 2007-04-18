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

#include "kurltogglebutton_p.h"
#include "kurlnavigator.h"

#include <kglobalsettings.h>
#include <klocale.h>

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

KUrlToggleButton::KUrlToggleButton(KUrlNavigator* parent) :
    KUrlButton(parent)
{
    setToolTip(i18n("Click to Edit Location"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

KUrlToggleButton::~KUrlToggleButton()
{
}

void KUrlToggleButton::paintEvent(QPaintEvent* event)
{
    if (isDisplayHintEnabled(EnteredHint)) {
        QPainter painter(this);
        painter.setClipRect(event->rect());

        QColor foregroundColor = KGlobalSettings::buttonTextColor();
        if (!urlNavigator()->isActive()) {
            QColor dimmColor(palette().brush(QPalette::Background).color());
            foregroundColor = mixColors(foregroundColor, dimmColor);
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(foregroundColor);
        painter.drawRect(0, 2, 2, height() - 6);
    }
}

#include "kurltogglebutton_p.moc"
