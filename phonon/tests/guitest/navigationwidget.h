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

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QtGui/QWidget>
#include <phonon/mediacontroller.h>

using Phonon::MediaController;

class NavigationWidget : public QWidget
{
    Q_OBJECT
    public:
        NavigationWidget(QWidget *parent = 0);
        ~NavigationWidget();
        void setInterface(MediaController *i);

    private:
        MediaController *m_iface;
};

#endif // NAVIGATIONWIDGET_H
