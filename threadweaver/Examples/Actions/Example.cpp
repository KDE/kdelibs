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
#include "Example.h"

#include <Action.h>
#include <ThreadWeaver.h>
#include <ExecutePolicy.h>

#include <QDebug>

using namespace ThreadWeaver;

Example::Example() {
    widget = new Ui_Example();
    widget->setupUi(this);

    m_action = new Action();
    Weaver *weaver = new Weaver(this);
    m_action->setWeaver(weaver);

    refresh();
    switchPolicy("Direct");
    widget->actionCount->setText("0");

    connect(m_action, SIGNAL(triggered(const QVariant&)),
            this, SLOT(waiter()), Qt::DirectConnection);
    connect(m_action, SIGNAL(updateUi(const QVariant&)),
            this, SLOT(alterRow(const QVariant&)), Qt::DirectConnection);
    connect(widget->refresh, SIGNAL(pressed()), this, SLOT(refresh()));
    connect(widget->policies, SIGNAL(activated(const QString &)),
            this, SLOT(switchPolicy(const QString &)));
    connect(widget->items, SIGNAL(itemPressed(QListWidgetItem *)),
            this, SLOT(itemClicked(QListWidgetItem *)));
}

void Example::refresh() {
    widget->items->clear();
    QStringList list;
    for(int i=1; i < 12; i++)
        list << QString("Row %1").arg(i);
    widget->items->addItems(list);
}

void Example::itemClicked(QListWidgetItem *item) {
    QVariant *params = new QVariant(widget->items->row(item));
    m_action->execute(params);
    updateCount(false);
}

void Example::waiter() {
    m_running++;
    sleep(3);
    m_running--;
}

void Example::updateCount(bool fromAction) {
    QString string = QString::number(m_action->jobCount() + (fromAction?-1:0));
    widget->actionCount->setText(string);
}

void Example::alterRow(const QVariant &variant) {
    widget->items->item(variant.toInt())->setText("clicked");
    updateCount(true);
}

void Example::switchPolicy(const QString &newPolicy) {
    if(newPolicy == "Direct")
        m_action->setExecutePolicy(ExecutePolicy::directPolicy);
    else if(newPolicy == "Queued")
        m_action->setExecutePolicy(ExecutePolicy::queuedPolicy);
    else if(newPolicy == "SimpleQueued")
        m_action->setExecutePolicy(ExecutePolicy::simpleQueuedPolicy);
    else if(newPolicy == "OnlyLast")
        m_action->setExecutePolicy(ExecutePolicy::onlyLastPolicy);
    else
        qWarning() << "SwitchPolicy value not understood:" << newPolicy;
}

#include "Example.moc"
