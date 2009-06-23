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


#include "kurltogglebutton_p.h"
#include "kurlnavigator.h"

#include <kcolorscheme.h>
#include <kicon.h>
#include <klocale.h>

#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>

KUrlToggleButton::KUrlToggleButton(KUrlNavigator* parent) :
    KUrlButton(parent)
{
    setCheckable(true);
    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(updateToolTip()));
    connect(this, SIGNAL(clicked(bool)),
            this, SLOT(updateCursor()));
    m_pixmap = KIcon("dialog-ok").pixmap(16, 16);
    updateToolTip();
}

KUrlToggleButton::~KUrlToggleButton()
{
}

QSize KUrlToggleButton::sizeHint() const
{
    QSize size = KUrlButton::sizeHint();
    size.setWidth(m_pixmap.width() + 4);
    return size;
}

void KUrlToggleButton::enterEvent(QEvent* event)
{
    KUrlButton::enterEvent(event);
    updateCursor();
}

void KUrlToggleButton::leaveEvent(QEvent* event)
{
    KUrlButton::leaveEvent(event);
    setCursor(Qt::ArrowCursor);
}

void KUrlToggleButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());

    const int buttonWidth = width();
    const int buttonHeight = height();
    if (isChecked()) {
        const int x = (buttonWidth - m_pixmap.width()) / 2;
        const int y = (buttonHeight - m_pixmap.height()) / 2;
        painter.drawPixmap(QRect(x, y, m_pixmap.width(), m_pixmap.height()), m_pixmap);
    } else if (isDisplayHintEnabled(EnteredHint)) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().color(foregroundRole()));

        const int verticalGap = 4;
        const int caretWidth = 2;
        const int x = (layoutDirection() == Qt::LeftToRight) ? 0 : width() - caretWidth;
        painter.drawRect(x, verticalGap, caretWidth, buttonHeight - 2 * verticalGap);
    }
}

void KUrlToggleButton::updateToolTip()
{
    if (isChecked()) {
        setToolTip(i18n("Click for Location Navigation"));
    } else {
        setToolTip(i18n("Click to Edit Location"));
    }
}

void KUrlToggleButton::updateCursor()
{
    setCursor(isChecked() ? Qt::ArrowCursor : Qt::IBeamCursor);
}

#include "kurltogglebutton_p.moc"
