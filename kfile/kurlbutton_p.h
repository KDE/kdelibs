/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
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

#ifndef KURLBUTTON_P_H
#define KURLBUTTON_P_H

#include <QtGui/QColor>
#include <QtGui/QPushButton>

class KUrl;
class QEvent;
class KUrlNavigator;

/**
 * @brief Base class for buttons of the URL navigator.
 *
 * Each button of the URL navigator contains an URL, which
 * is set as soon as the button has been clicked.
 */
class KUrlButton : public QPushButton
{
    Q_OBJECT

public:
    explicit KUrlButton(KUrlNavigator* parent);
    virtual ~KUrlButton();
    KUrlNavigator* urlNavigator() const;

protected:
    enum DisplayHint {
        ActivatedHint = 1,
        EnteredHint = 2,
        DraggedHint = 4,
        PopupActiveHint = 8
    };

    enum { BorderWidth = 2 };

    void setDisplayHintEnabled(DisplayHint hint, bool enable);
    bool isDisplayHintEnabled(DisplayHint hint) const;

    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent* event);

    void drawHoverBackground(QPainter* painter);

    /** Returns the foreground color by respecting the current display hint. */
    QColor foregroundColor() const;

private:
    int m_displayHint;
    KUrlNavigator* m_urlNavigator;
};

inline KUrlNavigator* KUrlButton::urlNavigator() const
{
    return m_urlNavigator;
}

#endif
