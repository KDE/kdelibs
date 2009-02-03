/***************************************************************************
 *   Copyright (C) 2006 by Aaron J. Seigo (<aseigo@kde.org>)               *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/
#ifndef KPROTOCOLCOMBO_P_H
#define KPROTOCOLCOMBO_P_H

#include "kurlbutton_p.h"

class KUrlNavigator;

class QMenu;

/**
 * @brief A combobox listing available protocols.
 *
 * The widget is used by the URL navigator for offering the available
 * protocols for non-local URLs.
 *
 * @see KUrlNavigator
 */
class KProtocolCombo : public KUrlButton
{
    Q_OBJECT

public:
    explicit KProtocolCombo(const QString& protocol, KUrlNavigator* parent = 0);

    QString currentProtocol() const;

    void setCustomProtocols(const QStringList &protocols);

    virtual QSize sizeHint() const;

public Q_SLOTS:
    void setProtocol(const QString& protocol);

Q_SIGNALS:
    void activated(const QString& protocol);

protected:
    virtual bool event(QEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private Q_SLOTS:
    void setProtocol(QAction* action);

private:
    void updateMenu();

    enum { ArrowSize = 10 };

    QStringList m_protocols;
    QMenu* m_menu;
};

#endif
