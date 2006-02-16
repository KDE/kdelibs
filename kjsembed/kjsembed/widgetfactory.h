/*
*  Copyright (C) 2005, Ian Reinhart Geiser <geiseri@kde.org>
*  Copyright (C) 2006, Matt Broadstone <mbroadst@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/

#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#include <QtDesigner/QFormBuilder>

class QWidget;
class QLayout;
class QAction;

/**
* Because the trolls cant seem to grasp the fact that people wish to use their toolkit we need this class
*/
class WidgetFactory : public QFormBuilder
{
    public:
        WidgetFactory();
        virtual ~WidgetFactory();
        /**
        * Create a new QWidget by name
        */
        QWidget *widget(const QString &widgetName, QWidget *parentWidget, const QString &name);
        /**
        * Create a new QLayout by name
        */
        QLayout *layout(const QString &layoutName, QObject *parent, const QString &name);
        /**
        * Create a new QAction by name
        */
        QAction *action(QObject *parent, const QString &name);
};
#endif
