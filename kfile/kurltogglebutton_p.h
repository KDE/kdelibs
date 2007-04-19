/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (<peter.penz@gmx.at>)                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#ifndef KURLTOGGLEBUTTON_P_H
#define KURLTOGGLEBUTTON_P_H

#include "kurlbutton_p.h"
#include <QtGui/QPixmap>

class KUrlNavigator;

/**
 * @brief Represents the button of the URL navigator to switch to
 *        the editable mode.
 *
 * A cursor is shown when hovering the button.
 */
class KUrlToggleButton : public KUrlButton
{
    Q_OBJECT

public:
    explicit KUrlToggleButton(KUrlNavigator* parent);
    virtual ~KUrlToggleButton();

protected:
    virtual void paintEvent(QPaintEvent* event);

private slots:
    void updateToolTip();

private:
    QPixmap m_pixmap;
};

#endif
