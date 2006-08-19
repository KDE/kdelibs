/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "ui_Example.h"

namespace ThreadWeaver {
    class Action;
}

class Example : public QWidget {
    Q_OBJECT
public:
    Example();

private slots:
    void refresh();
    void itemClicked(QListWidgetItem *item);
    void waiter();
    void alterRow(const QVariant &variant);
    void switchPolicy(const QString &newPolicy);

private:
    void updateCount(bool fromAction);

private:
    Ui_Example *widget;
    ThreadWeaver::Action *m_action;
    int m_running;
};
#endif
