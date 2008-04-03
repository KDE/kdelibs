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

#include "anglewidget.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

AngleWidget::AngleWidget(QWidget *parent)
    : QWidget(parent), m_iface(0)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    QLabel *l = new QLabel("current angle:", this);
    m_currentAngle = new QSpinBox(this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_currentAngle);
    l->setBuddy(m_currentAngle);
    m_currentAngle->setRange(1, 1);

    hlayout = new QHBoxLayout(this);
    topLayout->addLayout(hlayout);
    l = new QLabel("available angles:", this);
    m_availableAngles = new QLabel("0", this);
    hlayout->addWidget(l);
    hlayout->addWidget(m_availableAngles);
}

AngleWidget::~AngleWidget()
{
}

void AngleWidget::setInterface(MediaController *i)
{
    if (m_iface) {
        disconnect(m_iface, 0, m_currentAngle, 0);
        disconnect(m_currentAngle, 0, m_iface, 0);
        disconnect(m_iface, 0, m_availableAngles, 0);
    }
    m_iface = i;
    if (m_iface) {
        connect(m_iface, SIGNAL(availableAnglesChanged(int)), SLOT(availableAnglesChanged(int)));
        connect(m_iface, SIGNAL(availableAnglesChanged(int)), m_availableAngles, SLOT(setNum(int)));
        connect(m_iface, SIGNAL(angleChanged(int)), m_currentAngle, SLOT(setValue(int)));
        connect(m_currentAngle, SIGNAL(valueChanged(int)), m_iface, SLOT(setCurrentAngle(int)));
    }
}

void AngleWidget::availableAnglesChanged(int x) { m_currentAngle->setMaximum(x); }

