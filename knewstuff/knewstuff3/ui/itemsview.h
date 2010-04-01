/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_ITEMSVIEW_H
#define KNEWSTUFF3_UI_ITEMSVIEW_H

#include <QtGui/QListView>

namespace KNS3
{
class ItemsView: public QListView
{
public:
    ItemsView(QWidget* parent = 0);
    
protected:
    void wheelEvent(QWheelEvent* event);
};

} // end KNS namespace

#endif
