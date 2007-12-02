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

#ifndef KURLDROPDOWNBUTTON_P_H
#define KURLDROPDOWNBUTTON_P_H

#include "kurlbutton_p.h"

class KUrlNavigator;

/**
 * @brief Button of the URL navigator which offers a drop down menu
 *        of hidden paths.
 *
 * The button will only be shown if the width of the URL navigator is
 * too small to show the whole path.
 */
class KUrlDropDownButton : public KUrlButton
{
    Q_OBJECT

public:
    explicit KUrlDropDownButton(KUrlNavigator* parent);
    virtual ~KUrlDropDownButton();

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent* event);
};

#endif
