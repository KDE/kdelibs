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

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

Q_SIGNALS:
    /**
     * Is emitted if the toggle button is hovered and indicates
     * whether a edit hint should be shown by the parent widget.
     */
    void showEditHint(bool show);

protected:
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private slots:
    void updateToolTip();

private:
    QPixmap m_pixmap;
};

#endif
