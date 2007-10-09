/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "effectitem.h"
#include <QtCore/QModelIndex>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListView>

#include <Phonon/BackendCapabilities>
#include <Phonon/EffectWidget>

using Phonon::EffectWidget;

EffectItem::EffectItem(const EffectDescription &desc, const QPoint &pos, QGraphicsView *widget)
    : SinkItem(pos, widget),
    m_effect(desc)
{
    setBrush(QColor(255, 200, 0, 150));

    QHBoxLayout *hlayout = new QHBoxLayout(m_frame);
    hlayout->setMargin(0);

    EffectWidget *w = new EffectWidget(&m_effect, m_frame);
    hlayout->addWidget(w);
}
