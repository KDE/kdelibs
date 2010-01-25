/*****************************************************************************
 * Copyright (C) 2006-2010 by Peter Penz <peter.penz@gmx.at>                 *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
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

#include "kurlbutton_p.h"

#include <kcolorscheme.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kurl.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStyle>
#include <QStyleOptionFocusRect>

KUrlButton::KUrlButton(QWidget* parent) :
    QPushButton(parent),
    m_displayHint(0)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMinimumHeight(parent->minimumHeight());
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(pressed()), parent, SLOT(requestActivation()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(activate()));
}

KUrlButton::~KUrlButton()
{
}


void KUrlButton::setActive(bool active)
{
    QFont adjustedFont(font());
    if (active) {
        setDisplayHintEnabled(ActivatedHint, true);
        adjustedFont.setBold(true);
    } else {
        setDisplayHintEnabled(ActivatedHint, false);
        adjustedFont.setBold(false);
    }

    setFont(adjustedFont);
    update();
}

bool KUrlButton::isActive() const
{
    return isDisplayHintEnabled(ActivatedHint);
}

void KUrlButton::setDisplayHintEnabled(DisplayHint hint,
                                       bool enable)
{
    if (enable) {
        m_displayHint = m_displayHint | hint;
    } else {
        m_displayHint = m_displayHint & ~hint;
    }
    update();
}

bool KUrlButton::isDisplayHintEnabled(DisplayHint hint) const
{
    return (m_displayHint & hint) > 0;
}

void KUrlButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);
    setDisplayHintEnabled(EnteredHint, true);
    update();
}

void KUrlButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    setDisplayHintEnabled(EnteredHint, false);
    update();
}

void KUrlButton::drawHoverBackground(QPainter* painter)
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor backgroundColor = isHighlighted ? palette().color(QPalette::Highlight) : Qt::transparent;
    if (!isActive() && isHighlighted) {
        backgroundColor.setAlpha(128);
    }

    if (backgroundColor != Qt::transparent) {
        // TODO: the backgroundColor should be applied to the style
        QStyleOptionViewItemV4 option;
        option.initFrom(this);
        option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        option.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, this);
    }
}

QColor KUrlButton::foregroundColor() const
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor foregroundColor = palette().color(foregroundRole());

    const bool active = isActive();
    int alpha = active ? 255 : 128;
    if ((!isDisplayHintEnabled(ActivatedHint) || !active) && !isHighlighted) {
        alpha -= alpha / 4;
    }
    foregroundColor.setAlpha(alpha);

    return foregroundColor;
}

void KUrlButton::activate()
{
    setActive(true);
}

#include "kurlbutton_p.moc"
