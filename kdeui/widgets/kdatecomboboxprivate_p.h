/*
    Copyright 2011 John Layt <john@layt.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDATECOMBOBOXPRIVATE_P_H
#define KDATECOMBOBOXPRIVATE_P_H

#include "kcombobox.h"

class QMenu;

class KDateComboBoxPrivate : public KComboBox
{
public:
    KDateComboBoxPrivate(QWidget *parent);
    virtual ~KDateComboBoxPrivate();
    virtual void showPopup();
    //virtual void hidePopup();
    QMenu *m_dateMenu;
};

#endif
