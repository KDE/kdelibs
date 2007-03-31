/* This file is part of the KDE libraries
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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
#ifndef KSHORTCUTWIDGET_H
#define KSHORTCUTWIDGET_H

#include <QWidget>
#include "kshortcut.h"

class KShortcutWidgetPrivate;

class KDEUI_EXPORT KShortcutWidget : public QWidget
{
    Q_OBJECT
public:
    KShortcutWidget(QWidget *parent = 0);
    ~KShortcutWidget();

    void setModifierlessAllowed(bool allow);
    bool isModifierlessAllowed();

    void setHaveClearButtons(bool show);

    KShortcut shortcut() const;

Q_SIGNALS:
    void shortcutChanged(KShortcut cut);

public Q_SLOTS:
    void setShortcut(KShortcut cut);
    void clearShortcut();

private:
    Q_PRIVATE_SLOT(d, void priKeySequenceChanged(QKeySequence))
    Q_PRIVATE_SLOT(d, void altKeySequenceChanged(QKeySequence))

private:
    friend class KShortcutWidgetPrivate;
    KShortcutWidgetPrivate *const d;
};

#endif //KSHORTCUTWIDGET_H
