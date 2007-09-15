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

#ifndef ANGLEWIDGET_H
#define ANGLEWIDGET_H

#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QSpinBox>
#include <Phonon/MediaController>

using Phonon::MediaController;

class AngleWidget : public QWidget
{
    Q_OBJECT
    public:
        AngleWidget(QWidget *parent = 0);
        ~AngleWidget();
        void setInterface(MediaController *i);

    private slots:
        void availableAnglesChanged(int);

    private:
        MediaController *m_iface;
        QSpinBox *m_currentAngle;
        QLabel *m_availableAngles;
};

#endif // ANGLEWIDGET_H
